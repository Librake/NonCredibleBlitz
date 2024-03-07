// Copyright 2022 Vster

#include "MultiplayerProjectileMasterClass.h"
#include "Kismet/GamePlayStatics.h"
#include "MultiplayerShooterPlugin.h"
#include "MultiplayerGunMasterClass.h"

// Sets default values
AMultiplayerProjectileMasterClass::AMultiplayerProjectileMasterClass()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Projectile Mesh"));
	RootComponent = ProjectileMesh;
	ProjectileMesh->bReturnMaterialOnMove = true;
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Projectile Movement"));
	ProjectileMovement->InitialSpeed = 250000.0f;

	MinimumVelocity = 1000.0f;

	// Debug

	bPrintStringOfHitPhysicalMaterial = false;
	StringDuration = 5.0f;
	StringColor = FColor::Green;

	// End Debug

	bReplicates = true;

	NetUpdateFrequency = 66.0f;
	MinNetUpdateFrequency = 33.0f;
}

void AMultiplayerProjectileMasterClass::OnHit()
{
	if (HitActor && OwningGun && OwningPlayer)
	{
		if (HitActor != OwningPlayer)
		{
			AMultiplayerGunMasterClass* GunCast = Cast<AMultiplayerGunMasterClass>(OwningGun);

			if (GunCast)
			{
				EPhysicalSurface SurfaceType = UGameplayStatics::GetSurfaceType(ProjectileHitResult);

				UParticleSystem* SelectedEffect = nullptr;

				switch (SurfaceType)
				{
				case SURFACE_Head:
					SelectedEffect = GunCast->HeadHitParticle;
					break;
				case SURFACE_Chest:
					SelectedEffect = GunCast->ChestHitParticle;
					break;
				case SURFACE_Torso:
					SelectedEffect = GunCast->TorsoHitParticle;
					break;
				case SURFACE_Leg:
					SelectedEffect = GunCast->LegHitParticle;
					break;
				default:
					SelectedEffect = GunCast->DefaultHitParticle;
					break;
				}

				if (SelectedEffect)
				{
					if (GunCast->FireMode != 2)
					{
						UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), SelectedEffect, ProjectileHitResult.ImpactPoint, FRotator::ZeroRotator, GunCast->HitParticleScale);
					}
					else if (GunCast->FireMode == 2)
					{
						ShowHitParticle_BP(SelectedEffect, ProjectileHitResult.ImpactPoint);
					}
				}

				if (SurfaceType == SURFACE_Head)
				{
					UGameplayStatics::ApplyPointDamage(HitActor, GunCast->HeadDamage, ShotDirection, ProjectileHitResult, OwningPlayer->GetInstigatorController(), OwningPlayer, DamageType);

					if (bPrintStringOfHitPhysicalMaterial == true)
					{
						GEngine->AddOnScreenDebugMessage(-1, StringDuration, StringColor, "Head");
					}
				}
				else if (SurfaceType == SURFACE_Torso)
				{
					UGameplayStatics::ApplyPointDamage(HitActor, GunCast->TorsoDamage, ShotDirection, ProjectileHitResult, OwningPlayer->GetInstigatorController(), OwningPlayer, DamageType);

					if (bPrintStringOfHitPhysicalMaterial == true)
					{
						GEngine->AddOnScreenDebugMessage(-1, StringDuration, StringColor, "Torso");
					}
				}
				else if (SurfaceType == SURFACE_Leg)
				{
					UGameplayStatics::ApplyPointDamage(HitActor, GunCast->LegDamage, ShotDirection, ProjectileHitResult, OwningPlayer->GetInstigatorController(), OwningPlayer, DamageType);

					if (bPrintStringOfHitPhysicalMaterial == true)
					{
						GEngine->AddOnScreenDebugMessage(-1, StringDuration, StringColor, "Leg");
					}
				}
				else
				{
					UGameplayStatics::ApplyPointDamage(HitActor, GunCast->DefaultDamage, ShotDirection, ProjectileHitResult, OwningPlayer->GetInstigatorController(), OwningPlayer, DamageType);

					if (bPrintStringOfHitPhysicalMaterial == true)
					{
						if (SurfaceType == SURFACE_Chest)
						{
							GEngine->AddOnScreenDebugMessage(-1, StringDuration, StringColor, "Chest");
						}
						else
						{
							GEngine->AddOnScreenDebugMessage(-1, StringDuration, StringColor, "None");
						}
					}
				}
			}
		}
	}
}

// Called when the game starts or when spawned
void AMultiplayerProjectileMasterClass::BeginPlay()
{
	Super::BeginPlay();

	BeginPlay_BP();
}

// Called every frame
void AMultiplayerProjectileMasterClass::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	Tick_BP(DeltaTime);

	if (ProjectileMovement->Velocity.Size() < MinimumVelocity && ProjectileMovement->Velocity.Size() > (MinimumVelocity * -1.0f))
	{
		Destroy();
	}
}

void AMultiplayerProjectileMasterClass::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMultiplayerProjectileMasterClass, MinimumVelocity);
}