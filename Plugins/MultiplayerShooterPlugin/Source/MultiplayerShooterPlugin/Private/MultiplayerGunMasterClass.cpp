// Copyright 2022 Vster

#include "MultiplayerGunMasterClass.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "MultiplayerShooterPlugin.h"
#include "MultiplayerPlayerCharacter.h"
#include "DrawDebugHelpers.h"
#include "MultiplayerProjectileMasterClass.h"

// Sets default values
AMultiplayerGunMasterClass::AMultiplayerGunMasterClass()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// If you want to use a static mesh instead replace this with "UStaticMeshComponent"
	GunMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Gun Mesh"));
	RootComponent = GunMesh;
	GunMesh->SetCollisionProfileName("NoCollision");
	GunMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GunMesh->bOnlyOwnerSee = true;
	BarrelEndScene = CreateDefaultSubobject<USceneComponent>(TEXT("Barrel End Location"));
	BarrelEndScene->SetupAttachment(GunMesh, NAME_None);

	// If you want to use a static mesh instead replace this with "UStaticMeshComponent"
	ThirdPersonGunMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Third Person Gun Mesh"));
	ThirdPersonGunMesh->SetCollisionProfileName("NoCollision");
	ThirdPersonGunMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ThirdPersonGunMesh->bOwnerNoSee = true;
	ThirdPersonGunMesh->SetIsReplicated(true);
	BarrelEndSceneThirdPerson = CreateDefaultSubobject<USceneComponent>(TEXT("Barrel End Location Third Person"));
	BarrelEndSceneThirdPerson->SetupAttachment(ThirdPersonGunMesh, NAME_None);

	WeaponName = "Rifle";
	bCanShoot = true;

	// 0 = Full Auto, 1 = Semi-Auto, 2 = Burst
	FireMode = 0;
	FireRate = 0.05f;
	BurstRate = 0.115f;
	AmountOfShotsInBurst = 3;
	AmountOfBurstShotsFired = 0;
	bIsBurstFiring = false;
	bIsShotgun = false;
	ShotgunAmountOfPellets = 8;
	ShotgunAmountOfPelletsShot = 0;
	ShotgunMinHorizontalSpread = -2.0f;
	ShotgunMaxHorizontalSpread = 2.0f;
	ShotgunMinVerticalSpread = -2.0f;
	ShotgunMaxVerticalSpread = 2.0f;
	bIsShotgunFiring = false;

	// To make the camera go up this needs to be negative
	VerticleRecoil = -0.15f;
	HorizontalRecoil = 0.075f;
	FireRange = 1000000.0f;
	bLineTraceStartsFromGunBarrel = false;
	AmmoInMagazine = 30;
	MaxAmmoInMagazine = 30;
	ReserveAmmo = 120;
	MaxReserveAmmo = 120;
	AmountOfTimeToReload = 0.5f;
	DefaultDamage = 35.0f;
	HeadDamage = 50.0f;
	TorsoDamage = 30.0f;
	LegDamage = 20.0f;
	Socket = "GripPoint";
	CameraShakeScale = 1.0f;
	bPlayUnrealDefaultArmsAnimation = true;
	bUsesProjectile = false;
	bProjectileSpawnsFromGunBarrel = false;
	DistanceInFrontOfCameraToSpawn = 50.0f;

	// Debug

	bShowDebugLine = false;
	DebugLineColor = FColor::Red;
	bDebugLinePersistent = false;
	DebugLineDuration = 10.0f;
	bPrintStringOfHitPhysicalMaterial = false;
	StringDuration = 5.0f;
	StringColor = FColor::Green;

	// End Debug

	bReplicates = true;

	NetUpdateFrequency = 66.0f;
	MinNetUpdateFrequency = 33.0f;
}

void AMultiplayerGunMasterClass::Fire()
{
	if (GetLocalRole() < ROLE_Authority)
	{
		ServerFire();

		if (bCanShoot == true && AmmoInMagazine >= 1)
		{
			OwningPlayer = GetOwner();

			AMultiplayerPlayerCharacter* PlayerCast = Cast<AMultiplayerPlayerCharacter>(OwningPlayer);

			if (PlayerCast)
			{
				if (FireMode != 2 || bIsBurstFiring == true)
				{
					if (bIsShotgun == false || bIsShotgun == true && bIsShotgunFiring == true)
					{
						if (bUsesProjectile == false || bIsShotgun == true)
						{
							if (bIsShotgun == false)
							{
								Fire_BP();
							}

							FVector TraceStartLocation;
							FRotator TraceStartRotation;

							if (bLineTraceStartsFromGunBarrel == true)
							{
								TraceStartLocation = BarrelEndScene->GetComponentLocation();
								TraceStartRotation = BarrelEndScene->GetComponentRotation();
							}
							else if (bLineTraceStartsFromGunBarrel == false)
							{
								TraceStartLocation = PlayerCast->CameraLocation;
								TraceStartRotation = PlayerCast->CameraRotation;
							}

							if (bIsShotgun == true)
							{
								ShotgunFireRotation = TraceStartRotation;

								SetShotgunFireRotation_BP();

								TraceStartRotation = ShotgunFireRotation;
							}

							FVector TraceDirection = TraceStartRotation.Vector();

							FVector TraceEnd = TraceStartLocation + (TraceDirection * FireRange);

							FCollisionQueryParams QueryParams;
							QueryParams.AddIgnoredActor(PlayerCast);
							QueryParams.AddIgnoredActor(this);
							QueryParams.bTraceComplex = true;
							QueryParams.bReturnPhysicalMaterial = true;

							FHitResult Hit;
							if (GetWorld()->LineTraceSingleByChannel(Hit, TraceStartLocation, TraceEnd, ECC_Visibility, QueryParams))
							{
								TraceEnd = Hit.ImpactPoint;

								AActor* HitActor = Hit.GetActor();

								EPhysicalSurface SurfaceType = UGameplayStatics::GetSurfaceType(Hit);

								UParticleSystem* SelectedEffect = nullptr;

								switch (SurfaceType)
								{
								case SURFACE_Head:
									SelectedEffect = HeadHitParticle;
									break;
								case SURFACE_Chest:
									SelectedEffect = ChestHitParticle;
									break;
								case SURFACE_Torso:
									SelectedEffect = TorsoHitParticle;
									break;
								case SURFACE_Leg:
									SelectedEffect = LegHitParticle;
									break;
								default:
									SelectedEffect = DefaultHitParticle;
									break;
								}

								if (SelectedEffect)
								{
									if (FireMode != 2)
									{
										UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), SelectedEffect, Hit.ImpactPoint, FRotator::ZeroRotator, HitParticleScale);
									}
									else if (FireMode == 2)
									{
										ShowHitParticle_BP(SelectedEffect, Hit.ImpactPoint);
									}
								}

								if (HitActor)
								{
									if (SurfaceType == SURFACE_Head)
									{
										UGameplayStatics::ApplyPointDamage(HitActor, HeadDamage, TraceDirection, Hit, PlayerCast->GetInstigatorController(), PlayerCast, DamageType);
									}
									else if (SurfaceType == SURFACE_Torso)
									{
										UGameplayStatics::ApplyPointDamage(HitActor, TorsoDamage, TraceDirection, Hit, PlayerCast->GetInstigatorController(), PlayerCast, DamageType);
									}
									else if (SurfaceType == SURFACE_Leg)
									{
										UGameplayStatics::ApplyPointDamage(HitActor, LegDamage, TraceDirection, Hit, PlayerCast->GetInstigatorController(), PlayerCast, DamageType);
									}
									else
									{
										UGameplayStatics::ApplyPointDamage(HitActor, DefaultDamage, TraceDirection, Hit, PlayerCast->GetInstigatorController(), PlayerCast, DamageType);
									}
								}

								// Debug

								if (bPrintStringOfHitPhysicalMaterial == true)
								{
									if (SurfaceType == SURFACE_Head)
									{
										GEngine->AddOnScreenDebugMessage(-1, StringDuration, StringColor, "Head");
									}
									else if (SurfaceType == SURFACE_Chest)
									{
										GEngine->AddOnScreenDebugMessage(-1, StringDuration, StringColor, "Chest");
									}
									else if (SurfaceType == SURFACE_Torso)
									{
										GEngine->AddOnScreenDebugMessage(-1, StringDuration, StringColor, "Torso");
									}
									else if (SurfaceType == SURFACE_Leg)
									{
										GEngine->AddOnScreenDebugMessage(-1, StringDuration, StringColor, "Leg");
									}
									else
									{
										GEngine->AddOnScreenDebugMessage(-1, StringDuration, StringColor, "None");
									}
								}

								// End Debug
							}

							// Debug

							if (bShowDebugLine == true)
							{
								DrawDebugLine(GetWorld(), TraceStartLocation, TraceEnd, DebugLineColor, bDebugLinePersistent, DebugLineDuration);
							}

							// End Debug

							if (bIsShotgun == false)
							{
								AmmoInMagazine = AmmoInMagazine - 1;
							}

							if (FireMode != 2 && bIsShotgun == false || FireMode == 2 && AmountOfBurstShotsFired >= AmountOfShotsInBurst && bIsShotgun == false)
							{
								bCanShoot = false;

								GetWorldTimerManager().SetTimer(FireRateTimerHandle, this, &AMultiplayerGunMasterClass::SetCanShootTrue, FireRate, false, FireRate);
							}
						}
						else if (bUsesProjectile == true && ProjectileToUse && bIsShotgun == false)
						{
							Fire_BP();

							FVector ProjectileSpawnLocation;
							FRotator ProjectileSpawnRotation;

							if (bProjectileSpawnsFromGunBarrel == true)
							{
								ProjectileSpawnLocation = BarrelEndScene->GetComponentLocation();
								ProjectileSpawnRotation = BarrelEndScene->GetComponentRotation();
							}
							else if (bProjectileSpawnsFromGunBarrel == false)
							{
								ProjectileSpawnLocation = PlayerCast->PlayerBPCamera->GetComponentLocation() + (PlayerCast->PlayerBPCamera->GetForwardVector() * DistanceInFrontOfCameraToSpawn);
								ProjectileSpawnRotation = PlayerCast->PlayerBPCamera->GetComponentRotation();
							}

							FActorSpawnParameters SpawnParams;
							SpawnParams.bNoFail = true;
							SpawnParams.Owner = PlayerCast;
							SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

							SpawnedProjectile = GetWorld()->SpawnActor<AActor>(ProjectileToUse, ProjectileSpawnLocation, ProjectileSpawnRotation, SpawnParams);

							if (SpawnedProjectile)
							{
								AMultiplayerProjectileMasterClass* ProjectileCast = Cast<AMultiplayerProjectileMasterClass>(SpawnedProjectile);

								if (ProjectileCast)
								{
									ProjectileCast->OwningPlayer = PlayerCast;
									ProjectileCast->OwningGun = this;
									ProjectileCast->ShotDirection = ProjectileSpawnRotation.Vector();
									ProjectileCast->ProjectileMesh->IgnoreActorWhenMoving(OwningPlayer, true);
								}
							}

							AmmoInMagazine = AmmoInMagazine - 1;

							if (FireMode != 2 || FireMode == 2 && AmountOfBurstShotsFired >= AmountOfShotsInBurst)
							{
								bCanShoot = false;

								GetWorldTimerManager().SetTimer(FireRateTimerHandle, this, &AMultiplayerGunMasterClass::SetCanShootTrue, FireRate, false, FireRate);
							}
						}
					}
					else if (bIsShotgun == true && bIsShotgunFiring == false)
					{
						if (ShotgunAmountOfPelletsShot < ShotgunAmountOfPellets)
						{
							ShotgunFire();
						}
					}
				}
			}
		}

		return;
	}

	if (GetLocalRole() == ROLE_Authority)
	{
		MulticastFire();
	}
}

void AMultiplayerGunMasterClass::ServerFire_Implementation()
{
	Fire();
}

bool AMultiplayerGunMasterClass::ServerFire_Validate()
{
	return true;
}

void AMultiplayerGunMasterClass::MulticastFire()
{
	if (bCanShoot == true && AmmoInMagazine >= 1)
	{
		OwningPlayer = GetOwner();

		AMultiplayerPlayerCharacter* PlayerCast = Cast<AMultiplayerPlayerCharacter>(OwningPlayer);

		if (PlayerCast)
		{
			if (FireMode != 2 || bIsBurstFiring == true)
			{
				if (bIsShotgun == false || bIsShotgun == true && bIsShotgunFiring == true)
				{
					if (bUsesProjectile == false || bIsShotgun == true)
					{
						if (bIsShotgun == false)
						{
							Fire_BP();
						}

						FVector TraceStartLocation;
						FRotator TraceStartRotation;

						if (bLineTraceStartsFromGunBarrel == true)
						{
							TraceStartLocation = BarrelEndScene->GetComponentLocation();
							TraceStartRotation = BarrelEndScene->GetComponentRotation();
						}
						else if (bLineTraceStartsFromGunBarrel == false)
						{
							TraceStartLocation = PlayerCast->CameraLocation;
							TraceStartRotation = PlayerCast->CameraRotation;
						}

						if (bIsShotgun == true)
						{
							ShotgunFireRotation = TraceStartRotation;

							SetShotgunFireRotation_BP();

							TraceStartRotation = ShotgunFireRotation;
						}

						FVector TraceDirection = TraceStartRotation.Vector();

						FVector TraceEnd = TraceStartLocation + (TraceDirection * FireRange);

						FCollisionQueryParams QueryParams;
						QueryParams.AddIgnoredActor(PlayerCast);
						QueryParams.AddIgnoredActor(this);
						QueryParams.bTraceComplex = true;
						QueryParams.bReturnPhysicalMaterial = true;

						FHitResult Hit;
						if (GetWorld()->LineTraceSingleByChannel(Hit, TraceStartLocation, TraceEnd, ECC_Visibility, QueryParams))
						{
							TraceEnd = Hit.ImpactPoint;

							AActor* HitActor = Hit.GetActor();

							EPhysicalSurface SurfaceType = UGameplayStatics::GetSurfaceType(Hit);

							UParticleSystem* SelectedEffect = nullptr;

							switch (SurfaceType)
							{
							case SURFACE_Head:
								SelectedEffect = HeadHitParticle;
								break;
							case SURFACE_Chest:
								SelectedEffect = ChestHitParticle;
								break;
							case SURFACE_Torso:
								SelectedEffect = TorsoHitParticle;
								break;
							case SURFACE_Leg:
								SelectedEffect = LegHitParticle;
								break;
							default:
								SelectedEffect = DefaultHitParticle;
								break;
							}

							if (SelectedEffect)
							{
								if (FireMode != 2)
								{
									UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), SelectedEffect, Hit.ImpactPoint, FRotator::ZeroRotator, HitParticleScale);
								}
								else if (FireMode == 2)
								{
									ShowHitParticle_BP(SelectedEffect, Hit.ImpactPoint);
								}
							}

							if (HitActor)
							{
								if (SurfaceType == SURFACE_Head)
								{
									UGameplayStatics::ApplyPointDamage(HitActor, HeadDamage, TraceDirection, Hit, PlayerCast->GetInstigatorController(), PlayerCast, DamageType);
								}
								else if (SurfaceType == SURFACE_Torso)
								{
									UGameplayStatics::ApplyPointDamage(HitActor, TorsoDamage, TraceDirection, Hit, PlayerCast->GetInstigatorController(), PlayerCast, DamageType);
								}
								else if (SurfaceType == SURFACE_Leg)
								{
									UGameplayStatics::ApplyPointDamage(HitActor, LegDamage, TraceDirection, Hit, PlayerCast->GetInstigatorController(), PlayerCast, DamageType);
								}
								else
								{
									UGameplayStatics::ApplyPointDamage(HitActor, DefaultDamage, TraceDirection, Hit, PlayerCast->GetInstigatorController(), PlayerCast, DamageType);
								}
							}

							// Debug

							if (bPrintStringOfHitPhysicalMaterial == true)
							{
								if (SurfaceType == SURFACE_Head)
								{
									GEngine->AddOnScreenDebugMessage(-1, StringDuration, StringColor, "Head");
								}
								else if (SurfaceType == SURFACE_Chest)
								{
									GEngine->AddOnScreenDebugMessage(-1, StringDuration, StringColor, "Chest");
								}
								else if (SurfaceType == SURFACE_Torso)
								{
									GEngine->AddOnScreenDebugMessage(-1, StringDuration, StringColor, "Torso");
								}
								else if (SurfaceType == SURFACE_Leg)
								{
									GEngine->AddOnScreenDebugMessage(-1, StringDuration, StringColor, "Leg");
								}
								else
								{
									GEngine->AddOnScreenDebugMessage(-1, StringDuration, StringColor, "None");
								}
							}

							// End Debug
						}

						// Debug

						if (bShowDebugLine == true)
						{
							DrawDebugLine(GetWorld(), TraceStartLocation, TraceEnd, DebugLineColor, bDebugLinePersistent, DebugLineDuration);
						}

						// End Debug

						if (bIsShotgun == false)
						{
							AmmoInMagazine = AmmoInMagazine - 1;
						}

						if (FireMode != 2 && bIsShotgun == false || FireMode == 2 && AmountOfBurstShotsFired >= AmountOfShotsInBurst && bIsShotgun == false)
						{
							bCanShoot = false;

							GetWorldTimerManager().SetTimer(FireRateTimerHandle, this, &AMultiplayerGunMasterClass::SetCanShootTrue, FireRate, false, FireRate);
						}
					}
					else if (bUsesProjectile == true && ProjectileToUse && bIsShotgun == false)
					{
						Fire_BP();

						FVector ProjectileSpawnLocation;
						FRotator ProjectileSpawnRotation;

						if (bProjectileSpawnsFromGunBarrel == true)
						{
							ProjectileSpawnLocation = BarrelEndScene->GetComponentLocation();
							ProjectileSpawnRotation = BarrelEndScene->GetComponentRotation();
						}
						else if (bProjectileSpawnsFromGunBarrel == false)
						{
							ProjectileSpawnLocation = PlayerCast->PlayerBPCamera->GetComponentLocation() + (PlayerCast->PlayerBPCamera->GetForwardVector() * DistanceInFrontOfCameraToSpawn);
							ProjectileSpawnRotation = PlayerCast->PlayerBPCamera->GetComponentRotation();
						}

						FActorSpawnParameters SpawnParams;
						SpawnParams.bNoFail = true;
						SpawnParams.Owner = PlayerCast;
						SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

						SpawnedProjectile = GetWorld()->SpawnActor<AActor>(ProjectileToUse, ProjectileSpawnLocation, ProjectileSpawnRotation, SpawnParams);

						if (SpawnedProjectile)
						{
							AMultiplayerProjectileMasterClass* ProjectileCast = Cast<AMultiplayerProjectileMasterClass>(SpawnedProjectile);

							if (ProjectileCast)
							{
								ProjectileCast->OwningPlayer = PlayerCast;
								ProjectileCast->OwningGun = this;
								ProjectileCast->ShotDirection = ProjectileSpawnRotation.Vector();
								ProjectileCast->ProjectileMesh->IgnoreActorWhenMoving(OwningPlayer, true);
							}
						}

						AmmoInMagazine = AmmoInMagazine - 1;

						if (FireMode != 2 || FireMode == 2 && AmountOfBurstShotsFired >= AmountOfShotsInBurst)
						{
							bCanShoot = false;

							GetWorldTimerManager().SetTimer(FireRateTimerHandle, this, &AMultiplayerGunMasterClass::SetCanShootTrue, FireRate, false, FireRate);
						}
					}
				}
				else if (bIsShotgun == true && bIsShotgunFiring == false)
				{
					if (ShotgunAmountOfPelletsShot < ShotgunAmountOfPellets)
					{
						ShotgunFire();
					}
				}
			}
			else if (FireMode == 2 && bIsBurstFiring == false)
			{
				bIsBurstFiring = true;
				AmountOfBurstShotsFired = 0;

				BurstFire();
			}
		}
	}
}

void AMultiplayerGunMasterClass::OnRep_FireReplication()
{
	MulticastFire();
}

void AMultiplayerGunMasterClass::BurstFire()
{
	if (GetLocalRole() < ROLE_Authority)
	{
		ServerBurstFire();

		if (OwningPlayer)
		{
			AMultiplayerPlayerCharacter* PlayerCast = Cast<AMultiplayerPlayerCharacter>(OwningPlayer);

			if (PlayerCast)
			{
				if (AmountOfBurstShotsFired < AmountOfShotsInBurst && PlayerCast->Health >= 1)
				{
					Fire();
					ShowMuzzleFlash_BP();

					AmountOfBurstShotsFired = AmountOfBurstShotsFired + 1;

					if (AmmoInMagazine >= 1)
					{
						GetWorldTimerManager().SetTimer(BurstTimerHandle, this, &AMultiplayerGunMasterClass::BurstFire, BurstRate, false, BurstRate);
					}
					else if (AmmoInMagazine <= 0)
					{
						bIsBurstFiring = false;
					}
				}
				else if (AmountOfBurstShotsFired >= AmountOfShotsInBurst || AmmoInMagazine <= 0 || PlayerCast->Health <= 0)
				{
					bIsBurstFiring = false;
				}
			}
		}

		return;
	}

	if (GetLocalRole() == ROLE_Authority)
	{
		MulticastBurstFire();
	}
}

void AMultiplayerGunMasterClass::ServerBurstFire_Implementation()
{
	BurstFire();
}

bool AMultiplayerGunMasterClass::ServerBurstFire_Validate()
{
	return true;
}

void AMultiplayerGunMasterClass::MulticastBurstFire()
{
	if (OwningPlayer)
	{
		AMultiplayerPlayerCharacter* PlayerCast = Cast<AMultiplayerPlayerCharacter>(OwningPlayer);

		if (PlayerCast)
		{
			if (AmountOfBurstShotsFired < AmountOfShotsInBurst && PlayerCast->Health >= 1)
			{
				Fire();
				ShowMuzzleFlash_BP();

				AmountOfBurstShotsFired = AmountOfBurstShotsFired + 1;

				if (AmmoInMagazine >= 1)
				{
					GetWorldTimerManager().SetTimer(BurstTimerHandle, this, &AMultiplayerGunMasterClass::BurstFire, BurstRate, false, BurstRate);
				}
				else if (AmmoInMagazine <= 0)
				{
					bIsBurstFiring = false;
				}
			}
			else if (AmountOfBurstShotsFired >= AmountOfShotsInBurst || AmmoInMagazine <= 0 || PlayerCast->Health <= 0)
			{
				bIsBurstFiring = false;
			}
		}
	}
}

void AMultiplayerGunMasterClass::OnRep_BurstFireReplication()
{
	MulticastBurstFire();
}

void AMultiplayerGunMasterClass::ShotgunFire()
{
	if (bIsShotgun == true)
	{
		if (ShotgunAmountOfPelletsShot < ShotgunAmountOfPellets)
		{
			bIsShotgunFiring = true;

			Fire();

			ShotgunAmountOfPelletsShot = ShotgunAmountOfPelletsShot + 1;

			if (ShotgunAmountOfPelletsShot >= ShotgunAmountOfPellets)
			{
				GetWorldTimerManager().SetTimer(FireRateTimerHandle, this, &AMultiplayerGunMasterClass::SetCanShootTrue, FireRate, false, FireRate);

				Fire_BP();
				ShotgunFire_BP();

				bIsShotgunFiring = false;

				AmmoInMagazine = AmmoInMagazine - 1;
			}
			else
			{
				ShotgunFire();
			}
		}
		else
		{
			GetWorldTimerManager().SetTimer(FireRateTimerHandle, this, &AMultiplayerGunMasterClass::SetCanShootTrue, FireRate, false, FireRate);

			bIsShotgunFiring = false;
		}
	}
}

void AMultiplayerGunMasterClass::SetCanShootTrue()
{
	bCanShoot = true;

	ShotgunAmountOfPelletsShot = 0;
}

void AMultiplayerGunMasterClass::GetPickedUp()
{
	GetPickedUp_BP();

	SetActorRelativeLocation(PickupLocation);
	SetActorRelativeRotation(PickupRotation);

	if (OwningPlayer && ThirdPersonCharacterMesh)
	{
		AMultiplayerPlayerCharacter* PlayerCast = Cast<AMultiplayerPlayerCharacter>(OwningPlayer);

		if (PlayerCast)
		{
			ThirdPersonGunMesh->AttachToComponent(ThirdPersonCharacterMesh, FAttachmentTransformRules::KeepWorldTransform, SocketThirdPerson);

			ThirdPersonGunMesh->SetRelativeLocation(PickupLocationThirdPerson);
			ThirdPersonGunMesh->SetRelativeRotation(PickupRotationThirdPerson);
		}
	}
}

void AMultiplayerGunMasterClass::Reload()
{
	if (GetLocalRole() < ROLE_Authority)
	{
		ServerReload();

		if (AmmoInMagazine < MaxAmmoInMagazine && ReserveAmmo >= 1)
		{
			int ShotsFired = MaxAmmoInMagazine - AmmoInMagazine;

			if (ShotsFired >= ReserveAmmo)
			{
				AmmoInMagazine = AmmoInMagazine + ReserveAmmo;
				ReserveAmmo = 0;
			}
			else if (ShotsFired < ReserveAmmo)
			{
				AmmoInMagazine = MaxAmmoInMagazine;
				ReserveAmmo = ReserveAmmo - ShotsFired;
			}
		}

		return;
	}

	if (GetLocalRole() == ROLE_Authority)
	{
		MulticastReload();
	}
}

void AMultiplayerGunMasterClass::ServerReload_Implementation()
{
	Reload();
}

bool AMultiplayerGunMasterClass::ServerReload_Validate()
{
	return true;
}

void AMultiplayerGunMasterClass::MulticastReload()
{
	if (AmmoInMagazine < MaxAmmoInMagazine && ReserveAmmo >= 1)
	{
		int ShotsFired = MaxAmmoInMagazine - AmmoInMagazine;

		if (ShotsFired >= ReserveAmmo)
		{
			AmmoInMagazine = AmmoInMagazine + ReserveAmmo;
			ReserveAmmo = 0;
		}
		else if (ShotsFired < ReserveAmmo)
		{
			AmmoInMagazine = MaxAmmoInMagazine;
			ReserveAmmo = ReserveAmmo - ShotsFired;
		}
	}
}

void AMultiplayerGunMasterClass::OnRep_ReloadReplication()
{
	MulticastReload();
}

// Called when the game starts or when spawned
void AMultiplayerGunMasterClass::BeginPlay()
{
	Super::BeginPlay();

	BeginPlay_BP();
}

// Called every frame
void AMultiplayerGunMasterClass::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	Tick_BP(DeltaTime);
}

void AMultiplayerGunMasterClass::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMultiplayerGunMasterClass, FireMode);
	DOREPLIFETIME(AMultiplayerGunMasterClass, FireRate);
	DOREPLIFETIME(AMultiplayerGunMasterClass, VerticleRecoil);
	DOREPLIFETIME(AMultiplayerGunMasterClass, HorizontalRecoil);
	DOREPLIFETIME(AMultiplayerGunMasterClass, FireRange);
	DOREPLIFETIME(AMultiplayerGunMasterClass, AmmoInMagazine);
	DOREPLIFETIME(AMultiplayerGunMasterClass, MaxAmmoInMagazine);
	DOREPLIFETIME(AMultiplayerGunMasterClass, DefaultDamage);
	DOREPLIFETIME(AMultiplayerGunMasterClass, HeadDamage);
	DOREPLIFETIME(AMultiplayerGunMasterClass, TorsoDamage);
	DOREPLIFETIME(AMultiplayerGunMasterClass, LegDamage);
	DOREPLIFETIME(AMultiplayerGunMasterClass, CameraShakeScale);
	DOREPLIFETIME(AMultiplayerGunMasterClass, DefaultHitParticle);
	DOREPLIFETIME(AMultiplayerGunMasterClass, HeadHitParticle);
	DOREPLIFETIME(AMultiplayerGunMasterClass, ChestHitParticle);
	DOREPLIFETIME(AMultiplayerGunMasterClass, TorsoHitParticle);
	DOREPLIFETIME(AMultiplayerGunMasterClass, LegHitParticle);
	DOREPLIFETIME(AMultiplayerGunMasterClass, HitParticleScale);
	DOREPLIFETIME(AMultiplayerGunMasterClass, bIsBurstFiring);
	DOREPLIFETIME_CONDITION(AMultiplayerGunMasterClass, FireReplication, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(AMultiplayerGunMasterClass, ReloadReplication, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(AMultiplayerGunMasterClass, BurstFireReplication, COND_SkipOwner);
}