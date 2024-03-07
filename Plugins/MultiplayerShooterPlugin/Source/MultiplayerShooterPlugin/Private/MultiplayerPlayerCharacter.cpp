// Copyright 2022 Vster

#include "MultiplayerPlayerCharacter.h"
#include "TimerManager.h"

// Sets default values
AMultiplayerPlayerCharacter::AMultiplayerPlayerCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	bUseDefaultPluginHUD = true;
	bCanShoot = true;
	Health = 100;
	MaxHealth = 100;
	RespawnDelay = 2.0f;
	HealthRegenAmount = 2;
	HealthRegenTickRate = 0.15f;
	HealthRegenInitialDelay = 2.5f;

	// 0 = No Weapons, 1 = Weapon1, 2 = Weapon2
	WeaponBeingUsed = 0;
	bCanSwitchWeapons = true;
	AmountOfTimeToSwitchWeapons = 0.75f;
	bPlayAnimationOnEveryWeaponSwitch = false;
	bIsSwitchingWeapons = false;
	bIsReloading = false;

	bReplicates = true;

	NetUpdateFrequency = 66.0f;
	MinNetUpdateFrequency = 33.0f;
}

void AMultiplayerPlayerCharacter::MoveForward(float Scale)
{
	AddMovementInput(GetActorForwardVector(), Scale);
}

void AMultiplayerPlayerCharacter::MoveRight(float Scale)
{
	AddMovementInput(GetActorRightVector(), Scale);
}

void AMultiplayerPlayerCharacter::Turn(float Scale)
{
	AddControllerYawInput(Scale);
}

void AMultiplayerPlayerCharacter::LookUp(float Scale)
{
	AddControllerPitchInput(Scale);
}

float AMultiplayerPlayerCharacter::TakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	GetWorldTimerManager().ClearTimer(HealthRegenTimerHandle);

	if (Health >= 1)
	{
		Health = Health - Damage;

		if (Health <= 0)
		{
			Health = 0;

			Die();
		}
		else if (Health >= 1)
		{
			GetWorldTimerManager().SetTimer(HealthRegenTimerHandle, this, &AMultiplayerPlayerCharacter::AutoHealthRegen, HealthRegenInitialDelay, false, HealthRegenInitialDelay);
		}
	}

	return Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);
}

void AMultiplayerPlayerCharacter::Die()
{
	if (GetLocalRole() < ROLE_Authority)
	{
		ServerDie();

		Die_BP();

		bCanShoot = false;

		StopFiring_BP();

		GetWorldTimerManager().SetTimer(RespawnTimerHandle, this, &AMultiplayerPlayerCharacter::Respawn, RespawnDelay, false, RespawnDelay);

		return;
	}

	if (GetLocalRole() == ROLE_Authority)
	{
		MulticastDie();
	}
}

void AMultiplayerPlayerCharacter::ServerDie_Implementation()
{
	Die();
}

bool AMultiplayerPlayerCharacter::ServerDie_Validate()
{
	return true;
}

void AMultiplayerPlayerCharacter::MulticastDie()
{
	Die_BP();

	bCanShoot = false;

	StopFiring_BP();

	GetWorldTimerManager().SetTimer(RespawnTimerHandle, this, &AMultiplayerPlayerCharacter::Respawn, RespawnDelay, false, RespawnDelay);
}

void AMultiplayerPlayerCharacter::OnRep_DieReplication()
{
	MulticastDie();
}

void AMultiplayerPlayerCharacter::AutoHealthRegen()
{
	if (Health >= 1 && Health < MaxHealth)
	{
		Health = Health + HealthRegenAmount;

		if (Health >= MaxHealth)
		{
			Health = MaxHealth;
		}
		else
		{
			GetWorldTimerManager().SetTimer(HealthRegenTimerHandle, this, &AMultiplayerPlayerCharacter::AutoHealthRegen, HealthRegenTickRate, false, HealthRegenTickRate);
		}
	}
}

void AMultiplayerPlayerCharacter::Respawn()
{
	if (GetLocalRole() < ROLE_Authority)
	{
		ServerRespawn();

		Respawn_BP();

		bCanShoot = true;
		Health = MaxHealth;

		GetRidOfWeapons();

		GetWorldTimerManager().SetTimerForNextTick(this, &AMultiplayerPlayerCharacter::GiveWeapon1);
		GetWorldTimerManager().SetTimerForNextTick(this, &AMultiplayerPlayerCharacter::GiveWeapon2);

		return;
	}

	if (GetLocalRole() == ROLE_Authority)
	{
		MulticastRespawn();
	}
}

void AMultiplayerPlayerCharacter::ServerRespawn_Implementation()
{
	Respawn();
}

bool AMultiplayerPlayerCharacter::ServerRespawn_Validate()
{
	return true;
}

void AMultiplayerPlayerCharacter::MulticastRespawn()
{
	Respawn_BP();

	bCanShoot = true;
	Health = MaxHealth;

	GetRidOfWeapons();

	GetWorldTimerManager().SetTimerForNextTick(this, &AMultiplayerPlayerCharacter::GiveWeapon1);
	GetWorldTimerManager().SetTimerForNextTick(this, &AMultiplayerPlayerCharacter::GiveWeapon2);
}

void AMultiplayerPlayerCharacter::OnRep_RespawnReplication()
{
	MulticastRespawn();
}

void AMultiplayerPlayerCharacter::Fire()
{
	if (bCanShoot == true)
	{
		if (WeaponBeingUsed == 1 && Weapon1)
		{
			AMultiplayerGunMasterClass* WeaponCast = Cast<AMultiplayerGunMasterClass>(Weapon1);

			if (WeaponCast)
			{
				if (WeaponCast->FireMode != 2 || WeaponCast->FireMode == 2 && WeaponCast->bIsBurstFiring == false)
				{
					WeaponCast->Fire();
				}
			}
		}
		else if (WeaponBeingUsed == 2 && Weapon2)
		{
			AMultiplayerGunMasterClass* WeaponCast = Cast<AMultiplayerGunMasterClass>(Weapon2);

			if (WeaponCast)
			{
				if (WeaponCast->FireMode != 2 || WeaponCast->FireMode == 2 && WeaponCast->bIsBurstFiring == false)
				{
					WeaponCast->Fire();
				}
			}
		}
	}
}

void AMultiplayerPlayerCharacter::GiveWeapon1()
{
	if (GetLocalRole() < ROLE_Authority)
	{
		ServerGiveWeapon1();

		if (DefaultWeapon1)
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.bNoFail = true;
			SpawnParams.Owner = this;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			Weapon1 = GetWorld()->SpawnActor<AMultiplayerGunMasterClass>(DefaultWeapon1, GetActorLocation(), GetActorRotation(), SpawnParams);

			if (Weapon1)
			{
				AMultiplayerGunMasterClass* WeaponCast = Cast<AMultiplayerGunMasterClass>(Weapon1);

				if (WeaponCast)
				{
					SetPlayerModelForWeapons_BP();

					WeaponCast->OwningPlayer = this;
					WeaponCast->SetOwner(this);
					WeaponCast->GetPickedUp();
					WeaponCast->SetActorHiddenInGame(true);

					WeaponBeingUsed = 1;
				}
			}
		}

		return;
	}

	if (GetLocalRole() == ROLE_Authority)
	{
		MulticastGiveWeapon1();
	}
}

void AMultiplayerPlayerCharacter::ServerGiveWeapon1_Implementation()
{
	GiveWeapon1();
}

bool AMultiplayerPlayerCharacter::ServerGiveWeapon1_Validate()
{
	return true;
}

void AMultiplayerPlayerCharacter::MulticastGiveWeapon1()
{
	if (DefaultWeapon1)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.bNoFail = true;
		SpawnParams.Owner = this;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		Weapon1 = GetWorld()->SpawnActor<AMultiplayerGunMasterClass>(DefaultWeapon1, GetActorLocation(), GetActorRotation(), SpawnParams);

		if (Weapon1)
		{
			AMultiplayerGunMasterClass* WeaponCast = Cast<AMultiplayerGunMasterClass>(Weapon1);

			if (WeaponCast)
			{
				SetPlayerModelForWeapons_BP();

				WeaponCast->OwningPlayer = this;
				WeaponCast->SetOwner(this);
				WeaponCast->GetPickedUp();
				WeaponCast->SetActorHiddenInGame(true);

				WeaponBeingUsed = 1;
			}
		}
	}
}

void AMultiplayerPlayerCharacter::OnRep_GiveWeapon1Replication()
{
	MulticastGiveWeapon1();
}

void AMultiplayerPlayerCharacter::GiveWeapon2()
{
	if (GetLocalRole() < ROLE_Authority)
	{
		ServerGiveWeapon2();

		if (DefaultWeapon1 && DefaultWeapon2)
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.bNoFail = true;
			SpawnParams.Owner = this;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			Weapon2 = GetWorld()->SpawnActor<AMultiplayerGunMasterClass>(DefaultWeapon2, GetActorLocation(), GetActorRotation(), SpawnParams);

			if (Weapon2)
			{
				AMultiplayerGunMasterClass* WeaponCast = Cast<AMultiplayerGunMasterClass>(Weapon2);

				if (WeaponCast)
				{
					SetPlayerModelForWeapons_BP();

					WeaponCast->OwningPlayer = this;
					WeaponCast->SetOwner(this);
					WeaponCast->GetPickedUp();
					WeaponCast->SetActorHiddenInGame(true);
				}
			}
		}
		else if (!DefaultWeapon1 && DefaultWeapon2)
		{
			DefaultWeapon1 = DefaultWeapon2;
			DefaultWeapon2 = nullptr;

			GiveWeapon1();
		}

		return;
	}

	if (GetLocalRole() == ROLE_Authority)
	{
		MulticastGiveWeapon2();
	}
}

void AMultiplayerPlayerCharacter::ServerGiveWeapon2_Implementation()
{
	GiveWeapon2();
}

bool AMultiplayerPlayerCharacter::ServerGiveWeapon2_Validate()
{
	return true;
}

void AMultiplayerPlayerCharacter::MulticastGiveWeapon2()
{
	if (DefaultWeapon1 && DefaultWeapon2)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.bNoFail = true;
		SpawnParams.Owner = this;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		Weapon2 = GetWorld()->SpawnActor<AMultiplayerGunMasterClass>(DefaultWeapon2, GetActorLocation(), GetActorRotation(), SpawnParams);

		if (Weapon2)
		{
			AMultiplayerGunMasterClass* WeaponCast = Cast<AMultiplayerGunMasterClass>(Weapon2);

			if (WeaponCast)
			{
				SetPlayerModelForWeapons_BP();

				WeaponCast->OwningPlayer = this;
				WeaponCast->SetOwner(this);
				WeaponCast->GetPickedUp();
				WeaponCast->SetActorHiddenInGame(true);
			}
		}
	}
	else if (!DefaultWeapon1 && DefaultWeapon2)
	{
		DefaultWeapon1 = DefaultWeapon2;
		DefaultWeapon2 = nullptr;

		GiveWeapon1();
	}
}

void AMultiplayerPlayerCharacter::OnRep_GiveWeapon2Replication()
{
	MulticastGiveWeapon2();
}

void AMultiplayerPlayerCharacter::GetRidOfWeapons()
{
	if (GetLocalRole() < ROLE_Authority)
	{
		ServerGetRidOfWeapons();

		if (Weapon1)
		{
			Weapon1->Destroy();

			Weapon1 = nullptr;
		}

		if (Weapon2)
		{
			Weapon2->Destroy();

			Weapon2 = nullptr;
		}

		WeaponBeingUsed = 0;

		return;
	}

	if (GetLocalRole() == ROLE_Authority)
	{
		MulticastGetRidOfWeapons();
	}
}

void AMultiplayerPlayerCharacter::ServerGetRidOfWeapons_Implementation()
{
	GetRidOfWeapons();
}

bool AMultiplayerPlayerCharacter::ServerGetRidOfWeapons_Validate()
{
	return true;
}

void AMultiplayerPlayerCharacter::MulticastGetRidOfWeapons()
{
	if (Weapon1)
	{
		Weapon1->Destroy();

		Weapon1 = nullptr;
	}

	if (Weapon2)
	{
		Weapon2->Destroy();

		Weapon2 = nullptr;
	}

	WeaponBeingUsed = 0;
}

void AMultiplayerPlayerCharacter::OnRep_GetRidOfWeaponsReplication()
{
	MulticastGetRidOfWeapons();
}

void AMultiplayerPlayerCharacter::SwitchWeapons()
{
	if (GetLocalRole() < ROLE_Authority)
	{
		ServerSwitchWeapons();

		if (bCanSwitchWeapons == true && Weapon1 && Weapon2 && WeaponBeingUsed != 0)
		{
			AMultiplayerGunMasterClass* Weapon1Cast = Cast<AMultiplayerGunMasterClass>(Weapon1);
			AMultiplayerGunMasterClass* Weapon2Cast = Cast<AMultiplayerGunMasterClass>(Weapon2);

			if (Weapon1Cast->bIsBurstFiring == false && Weapon2Cast->bIsBurstFiring == false)
			{
				if (bIsSwitchingWeapons == false && bIsReloading == false || bPlayAnimationOnEveryWeaponSwitch == true)
				{
					GetWorldTimerManager().ClearTimer(SwitchWeaponsTimerHandle);
					GetWorldTimerManager().ClearTimer(ReloadTimerHandle);

					StopFiring_BP();

					if (bIsSwitchingWeapons == true && bPlayAnimationOnEveryWeaponSwitch == true)
					{
						if (WeaponBeingUsed == 1)
						{
							Weapon1->SetActorHiddenInGame(false);
							Weapon2->SetActorHiddenInGame(true);
						}
						else if (WeaponBeingUsed == 2)
						{
							Weapon1->SetActorHiddenInGame(true);
							Weapon2->SetActorHiddenInGame(false);
						}
					}

					bCanShoot = false;
					bIsSwitchingWeapons = true;
					bIsReloading = false;

					if (WeaponBeingUsed == 1)
					{
						WeaponBeingUsed = 2;
					}
					else if (WeaponBeingUsed == 2)
					{
						WeaponBeingUsed = 1;
					}

					if (SwitchWeaponsAnimation)
					{
						PlayArmsAnimation_BP(SwitchWeaponsAnimation, false);
					}

					if (ThirdPersonSwitchWeaponsAnimation)
					{
						GetMesh()->PlayAnimation(ThirdPersonSwitchWeaponsAnimation, false);
					}

					GetWorldTimerManager().SetTimer(SwitchWeaponsTimerHandle, this, &AMultiplayerPlayerCharacter::SwitchWeapons1, AmountOfTimeToSwitchWeapons / 2, false, AmountOfTimeToSwitchWeapons / 2);
				}
				else if (bIsSwitchingWeapons == true && bPlayAnimationOnEveryWeaponSwitch == false || bIsReloading == true && bPlayAnimationOnEveryWeaponSwitch == false)
				{
					GetWorldTimerManager().ClearTimer(ReloadTimerHandle);

					bIsReloading = false;
					bIsSwitchingWeapons = true;

					if (WeaponBeingUsed == 1)
					{
						WeaponBeingUsed = 2;
					}
					else if (WeaponBeingUsed == 2)
					{
						WeaponBeingUsed = 1;
					}

					SwitchWeapons1();
				}
			}
		}

		return;
	}

	if (GetLocalRole() == ROLE_Authority)
	{
		MulticastSwitchWeapons();
	}
}

void AMultiplayerPlayerCharacter::ServerSwitchWeapons_Implementation()
{
	SwitchWeapons();
}

bool AMultiplayerPlayerCharacter::ServerSwitchWeapons_Validate()
{
	return true;
}

void AMultiplayerPlayerCharacter::MulticastSwitchWeapons()
{
	if (bCanSwitchWeapons == true && Weapon1 && Weapon2 && WeaponBeingUsed != 0)
	{
		AMultiplayerGunMasterClass* Weapon1Cast = Cast<AMultiplayerGunMasterClass>(Weapon1);
		AMultiplayerGunMasterClass* Weapon2Cast = Cast<AMultiplayerGunMasterClass>(Weapon2);

		if (Weapon1Cast->bIsBurstFiring == false && Weapon2Cast->bIsBurstFiring == false)
		{
			if (bIsSwitchingWeapons == false && bIsReloading == false || bPlayAnimationOnEveryWeaponSwitch == true)
			{
				GetWorldTimerManager().ClearTimer(SwitchWeaponsTimerHandle);
				GetWorldTimerManager().ClearTimer(ReloadTimerHandle);

				StopFiring_BP();

				if (bIsSwitchingWeapons == true && bPlayAnimationOnEveryWeaponSwitch == true)
				{
					if (WeaponBeingUsed == 1)
					{
						Weapon1->SetActorHiddenInGame(false);
						Weapon2->SetActorHiddenInGame(true);
					}
					else if (WeaponBeingUsed == 2)
					{
						Weapon1->SetActorHiddenInGame(true);
						Weapon2->SetActorHiddenInGame(false);
					}
				}

				bCanShoot = false;
				bIsSwitchingWeapons = true;
				bIsReloading = false;

				if (WeaponBeingUsed == 1)
				{
					WeaponBeingUsed = 2;
				}
				else if (WeaponBeingUsed == 2)
				{
					WeaponBeingUsed = 1;
				}

				if (SwitchWeaponsAnimation)
				{
					PlayArmsAnimation_BP(SwitchWeaponsAnimation, false);
				}

				if (ThirdPersonSwitchWeaponsAnimation)
				{
					GetMesh()->PlayAnimation(ThirdPersonSwitchWeaponsAnimation, false);
				}

				GetWorldTimerManager().SetTimer(SwitchWeaponsTimerHandle, this, &AMultiplayerPlayerCharacter::SwitchWeapons1, AmountOfTimeToSwitchWeapons / 2, false, AmountOfTimeToSwitchWeapons / 2);
			}
			else if (bIsSwitchingWeapons == true && bPlayAnimationOnEveryWeaponSwitch == false || bIsReloading == true && bPlayAnimationOnEveryWeaponSwitch == false)
			{
				GetWorldTimerManager().ClearTimer(ReloadTimerHandle);

				bIsReloading = false;
				bIsSwitchingWeapons = true;

				if (WeaponBeingUsed == 1)
				{
					WeaponBeingUsed = 2;
				}
				else if (WeaponBeingUsed == 2)
				{
					WeaponBeingUsed = 1;
				}

				SwitchWeapons1();
			}
		}
	}
}

void AMultiplayerPlayerCharacter::OnRep_SwitchWeaponsReplication()
{
	MulticastSwitchWeapons();
}

void AMultiplayerPlayerCharacter::SwitchWeapons1()
{
	if (GetLocalRole() < ROLE_Authority)
	{
		ServerSwitchWeapons1();

		if (WeaponBeingUsed == 1)
		{
			Weapon1->SetActorHiddenInGame(false);
			Weapon2->SetActorHiddenInGame(true);

			SetWeapon1Hidden_BP(false);
			SetWeapon2Hidden_BP(true);
		}
		else if (WeaponBeingUsed == 2)
		{
			Weapon1->SetActorHiddenInGame(true);
			Weapon2->SetActorHiddenInGame(false);

			SetWeapon1Hidden_BP(true);
			SetWeapon2Hidden_BP(false);
		}

		if (SwitchWeaponsAnimation1)
		{
			PlayArmsAnimation_BP(SwitchWeaponsAnimation1, false);
		}

		if (ThirdPersonSwitchWeaponsAnimation1)
		{
			GetMesh()->PlayAnimation(ThirdPersonSwitchWeaponsAnimation1, false);
		}

		GetWorldTimerManager().SetTimer(SwitchWeaponsTimerHandle, this, &AMultiplayerPlayerCharacter::SwitchWeapons2, AmountOfTimeToSwitchWeapons / 2, false, AmountOfTimeToSwitchWeapons / 2);

		return;
	}

	if (GetLocalRole() == ROLE_Authority)
	{
		MulticastSwitchWeapons1();
	}
}

void AMultiplayerPlayerCharacter::ServerSwitchWeapons1_Implementation()
{
	SwitchWeapons1();
}

bool AMultiplayerPlayerCharacter::ServerSwitchWeapons1_Validate()
{
	return true;
}

void AMultiplayerPlayerCharacter::MulticastSwitchWeapons1()
{
	if (WeaponBeingUsed == 1)
	{
		Weapon1->SetActorHiddenInGame(false);
		Weapon2->SetActorHiddenInGame(true);

		SetWeapon1Hidden_BP(false);
		SetWeapon2Hidden_BP(true);
	}
	else if (WeaponBeingUsed == 2)
	{
		Weapon1->SetActorHiddenInGame(true);
		Weapon2->SetActorHiddenInGame(false);

		SetWeapon1Hidden_BP(true);
		SetWeapon2Hidden_BP(false);
	}

	if (SwitchWeaponsAnimation1)
	{
		PlayArmsAnimation_BP(SwitchWeaponsAnimation1, false);
	}

	if (ThirdPersonSwitchWeaponsAnimation1)
	{
		GetMesh()->PlayAnimation(ThirdPersonSwitchWeaponsAnimation1, false);
	}

	GetWorldTimerManager().SetTimer(SwitchWeaponsTimerHandle, this, &AMultiplayerPlayerCharacter::SwitchWeapons2, AmountOfTimeToSwitchWeapons / 2, false, AmountOfTimeToSwitchWeapons / 2);
}

void AMultiplayerPlayerCharacter::OnRep_SwitchWeapons1Replication()
{
	MulticastSwitchWeapons1();
}

void AMultiplayerPlayerCharacter::SwitchWeapons2()
{
	if (GetLocalRole() < ROLE_Authority)
	{
		ServerSwitchWeapons2();

		SetArmsAnimationMode_BP(EAnimationMode::AnimationBlueprint);
		GetMesh()->SetAnimationMode(EAnimationMode::AnimationBlueprint);

		bCanShoot = true;
		bIsSwitchingWeapons = false;

		return;
	}

	if (GetLocalRole() == ROLE_Authority)
	{
		MulticastSwitchWeapons2();
	}
}

void AMultiplayerPlayerCharacter::ServerSwitchWeapons2_Implementation()
{
	SwitchWeapons2();
}

bool AMultiplayerPlayerCharacter::ServerSwitchWeapons2_Validate()
{
	return true;
}

void AMultiplayerPlayerCharacter::MulticastSwitchWeapons2()
{
	SetArmsAnimationMode_BP(EAnimationMode::AnimationBlueprint);
	GetMesh()->SetAnimationMode(EAnimationMode::AnimationBlueprint);

	bCanShoot = true;
	bIsSwitchingWeapons = false;
}

void AMultiplayerPlayerCharacter::OnRep_SwitchWeapons2Replication()
{
	MulticastSwitchWeapons2();
}

void AMultiplayerPlayerCharacter::Reload()
{
	if (bIsReloading == false && bIsSwitchingWeapons == false)
	{
		if (WeaponBeingUsed == 1 && Weapon1)
		{
			AMultiplayerGunMasterClass* WeaponCast = Cast<AMultiplayerGunMasterClass>(Weapon1);

			if (WeaponCast)
			{
				if (WeaponCast->AmmoInMagazine < WeaponCast->MaxAmmoInMagazine && WeaponCast->bIsBurstFiring == false && WeaponCast->ReserveAmmo >= 1)
				{
					bIsReloading = true;
					bCanShoot = false;

					if (ReloadAnimation)
					{
						PlayArmsAnimation_BP(ReloadAnimation, false);
					}

					GetWorldTimerManager().SetTimer(ReloadTimerHandle, this, &AMultiplayerPlayerCharacter::Reload1, WeaponCast->AmountOfTimeToReload, false, WeaponCast->AmountOfTimeToReload);
				}
			}
		}
		else if (WeaponBeingUsed == 2 && Weapon2)
		{
			AMultiplayerGunMasterClass* WeaponCast = Cast<AMultiplayerGunMasterClass>(Weapon2);

			if (WeaponCast)
			{
				if (WeaponCast->AmmoInMagazine < WeaponCast->MaxAmmoInMagazine && WeaponCast->bIsBurstFiring == false && WeaponCast->ReserveAmmo >= 1)
				{
					bIsReloading = true;
					bCanShoot = false;

					if (ReloadAnimation)
					{
						PlayArmsAnimation_BP(ReloadAnimation, false);
					}

					GetWorldTimerManager().SetTimer(ReloadTimerHandle, this, &AMultiplayerPlayerCharacter::Reload1, WeaponCast->AmountOfTimeToReload, false, WeaponCast->AmountOfTimeToReload);
				}
			}
		}
	}
}

void AMultiplayerPlayerCharacter::Reload1()
{
	if (WeaponBeingUsed == 1 && Weapon1)
	{
		AMultiplayerGunMasterClass* WeaponCast = Cast<AMultiplayerGunMasterClass>(Weapon1);

		if (WeaponCast)
		{
			WeaponCast->Reload();
		}
	}
	else if (WeaponBeingUsed == 2 && Weapon2)
	{
		AMultiplayerGunMasterClass* WeaponCast = Cast<AMultiplayerGunMasterClass>(Weapon2);

		if (WeaponCast)
		{
			WeaponCast->Reload();
		}
	}

	if (ReloadAnimation1)
	{
		PlayArmsAnimation_BP(ReloadAnimation1, false);
	}

	GetWorldTimerManager().SetTimer(ReloadTimerHandle, this, &AMultiplayerPlayerCharacter::Reload2, 0.4f, false, 0.4f);
}

void AMultiplayerPlayerCharacter::Reload2()
{
	SetArmsAnimationMode_BP(EAnimationMode::AnimationBlueprint);

	bCanShoot = true;
	bIsReloading = false;
}

// Called when the game starts or when spawned
void AMultiplayerPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	BeginPlay_BP();

	if (bUseDefaultPluginHUD == true)
	{
		ShowDefaultPluginHUD_BP();
	}

	GiveWeapon1();
	GiveWeapon2();
}

// Called every frame
void AMultiplayerPlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	Tick_BP(DeltaTime);
}

void AMultiplayerPlayerCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMultiplayerPlayerCharacter, bCanShoot);
	DOREPLIFETIME(AMultiplayerPlayerCharacter, Health);
	DOREPLIFETIME(AMultiplayerPlayerCharacter, MaxHealth);
	DOREPLIFETIME(AMultiplayerPlayerCharacter, Weapon1);
	DOREPLIFETIME(AMultiplayerPlayerCharacter, Weapon2);
	DOREPLIFETIME(AMultiplayerPlayerCharacter, WeaponBeingUsed);
	DOREPLIFETIME(AMultiplayerPlayerCharacter, bCanSwitchWeapons);
	DOREPLIFETIME(AMultiplayerPlayerCharacter, bIsSwitchingWeapons);
	DOREPLIFETIME(AMultiplayerPlayerCharacter, bIsReloading);
	DOREPLIFETIME_CONDITION(AMultiplayerPlayerCharacter, GiveWeapon1Replication, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(AMultiplayerPlayerCharacter, GiveWeapon2Replication, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(AMultiplayerPlayerCharacter, SwitchWeaponsReplication, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(AMultiplayerPlayerCharacter, SwitchWeapons1Replication, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(AMultiplayerPlayerCharacter, SwitchWeapons2Replication, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(AMultiplayerPlayerCharacter, RespawnReplication, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(AMultiplayerPlayerCharacter, GetRidOfWeaponsReplication, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(AMultiplayerPlayerCharacter, DieReplication, COND_SkipOwner);
}

// Called to bind functionality to input
void AMultiplayerPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("Move Forward / Backward", this, &AMultiplayerPlayerCharacter::MoveForward);
	PlayerInputComponent->BindAxis("Move Right / Left", this, &AMultiplayerPlayerCharacter::MoveRight);
	PlayerInputComponent->BindAxis("Turn", this, &AMultiplayerPlayerCharacter::Turn);
	PlayerInputComponent->BindAxis("Look Up", this, &AMultiplayerPlayerCharacter::LookUp);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AMultiplayerPlayerCharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &AMultiplayerPlayerCharacter::StopJumping);
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AMultiplayerPlayerCharacter::Fire_BP);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &AMultiplayerPlayerCharacter::StopFiring_BP);
	PlayerInputComponent->BindAction("Switch Weapons", IE_Pressed, this, &AMultiplayerPlayerCharacter::SwitchWeapons);
	PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &AMultiplayerPlayerCharacter::Reload);
}