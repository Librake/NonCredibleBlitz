// Copyright 2022 Vster

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "MultiplayerGunMasterClass.generated.h"

// If you want to use a static mesh instead replace this with "UStaticMeshComponent"
class USkeletalMeshComponent;

class USceneComponent;
class USoundBase;
class UDamageType;
class UParticleSystem;
class UAnimMontage;

USTRUCT()
struct FMultiplayerGunFireReplication
{
	GENERATED_BODY()

public:

};

USTRUCT()
struct FMultiplayerGunReloadReplication
{
	GENERATED_BODY()

public:

};

USTRUCT()
struct FMultiplayerGunBurstFireReplication
{
	GENERATED_BODY()

public:

};

UCLASS()
class MULTIPLAYERSHOOTERPLUGIN_API AMultiplayerGunMasterClass : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AMultiplayerGunMasterClass();

	// Use this in blueprints to avoid overriding C++ begin play
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Functions")
		void BeginPlay_BP();

	// Use this in blueprints to avoid overriding C++ Tick
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Functions")
		void Tick_BP(float DeltaTime);

	// If you want to use a static mesh instead replace this with "UStaticMeshComponent"
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
		USkeletalMeshComponent* GunMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
		USceneComponent* BarrelEndScene;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
		USceneComponent* BarrelEndSceneThirdPerson;

	// If you want to use a static mesh instead replace this with "UStaticMeshComponent"
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
		USkeletalMeshComponent* ThirdPersonGunMesh;

	UPROPERTY(BlueprintReadWrite, Category = "Components")
		USkeletalMeshComponent* ThirdPersonCharacterMesh;

	UFUNCTION(BlueprintCallable, Category = "Functions")
		virtual void Fire();

	UFUNCTION(Server, Reliable, WithValidation, Category = "Functions")
		void ServerFire();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Functions")
		void Fire_BP();

	UFUNCTION(BlueprintCallable, Category = "Functions")
		virtual void BurstFire();

	UFUNCTION(Server, Reliable, WithValidation, Category = "Functions")
		void ServerBurstFire();

	UFUNCTION(BlueprintCallable, Category = "Functions")
		void ShotgunFire();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Functions")
		void ShotgunFire_BP();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Functions")
		void SetShotgunFireRotation_BP();

	UFUNCTION(BlueprintCallable, Category = "Functions")
		virtual void SetCanShootTrue();

	UFUNCTION(BlueprintCallable, Category = "Functions")
		virtual void GetPickedUp();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Functions")
		void GetPickedUp_BP();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Functions")
		void SetThirdPerson_BP(bool ThirdPerson);

	UFUNCTION(BlueprintCallable, Category = "Functions")
		virtual void Reload();

	UFUNCTION(Server, Reliable, WithValidation, Category = "Functions")
		void ServerReload();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Functions")
		void ShowMuzzleFlash_BP();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Functions")
		void ShowHitParticle_BP(UParticleSystem* SelectedEffect, FVector Location);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables")
		FName WeaponName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shooting")
		bool bCanShoot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Shooting", meta = (Tooltip = "0 = Full Auto, 1 = Semi Auto, 2 = Burst (All fire modes work with shotgun)"))
		int FireMode;

	// Fire Rate is the amount of time in between shots, for example to make it fire 500 RPM, 500 / 60 = 8.33, 1 / 8.33 = 0.12
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Shooting", meta = (Tooltip = "The amount of time between shots"))
		float FireRate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shooting", meta = (Tooltip = "The time between each shot in the burst"))
		float BurstRate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shooting", meta = (ClampMin = 0))
		int AmountOfShotsInBurst;

	UPROPERTY()
		int AmountOfBurstShotsFired;

	UPROPERTY(Replicated)
		bool bIsBurstFiring;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shooting", meta = (Tooltip = "Does not work with projectile, if this is true it will use line trace"))
		bool bIsShotgun;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shooting")
		int ShotgunAmountOfPellets;

	UPROPERTY(BlueprintReadWrite, Category = "Variables")
		int ShotgunAmountOfPelletsShot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shooting")
		float ShotgunMinHorizontalSpread;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shooting")
		float ShotgunMaxHorizontalSpread;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shooting")
		float ShotgunMinVerticalSpread;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shooting")
		float ShotgunMaxVerticalSpread;

	UPROPERTY(BlueprintReadWrite, Category = "Variables")
		bool bIsShotgunFiring;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Shooting")
		float VerticleRecoil;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Shooting")
		float HorizontalRecoil;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Shooting", meta = (Tooltip = "How far the line trace will go (does not apply to projectiles)"))
		float FireRange;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shooting", meta = (Tooltip = "Weather the Line Trace starts from the player camera or the gun barrel (you might have to rotate the BarrelEndScene component)"))
		bool bLineTraceStartsFromGunBarrel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Ammo")
		int AmmoInMagazine;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Ammo")
		int MaxAmmoInMagazine;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ammo")
		int ReserveAmmo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ammo")
		int MaxReserveAmmo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ammo")
		float AmountOfTimeToReload;

	// Default Damage is also used for the chest
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Damage")
		float DefaultDamage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Damage")
		float HeadDamage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Damage")
		float TorsoDamage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Damage")
		float LegDamage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile", meta = (Tooltip = "Weather to use projectile or line trace (hitscan)"))
		bool bUsesProjectile;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile")
		TSubclassOf<AActor> ProjectileToUse;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile", meta = (Tooltip = "Weather the projectile spawns from the player camera or the gun barrel (you might have to rotate the BarrelEndScene component)"))
		bool bProjectileSpawnsFromGunBarrel;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = 1.0f), Category = "Projectile")
		float DistanceInFrontOfCameraToSpawn;

	UPROPERTY(BlueprintReadWrite, Category = "Variables")
		AActor* SpawnedProjectile;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound", meta = (Tooltip = "The default sound cuts out when played rapidly but changing the sound will fix that"))
		USoundBase* FireSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Camera Shake")
		float CameraShakeScale;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations")
		bool bPlayUnrealDefaultArmsAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations")
		UAnimMontage* FireAnimationMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup", meta = (Tooltip = "The location the gun will snap to when picked up, relative to the socket or parent actor"))
		FVector PickupLocation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup", meta = (Tooltip = "The rotation the gun will snap to when picked up, relative to the socket or parent actor"))
		FRotator PickupRotation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup", meta = (Tooltip = "The location the third person gun will snap to when picked up, relative to the socket or parent actor"))
		FVector PickupLocationThirdPerson;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup", meta = (Tooltip = "The rotation the third person gun will snap to when picked up, relative to the socket or parent actor"))
		FRotator PickupRotationThirdPerson;

	UPROPERTY(BlueprintReadWrite, Category = "Variables")
		FRotator ShotgunFireRotation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup", meta = (Tooltip = "The socket the gun will snap to when picked up"))
		FName Socket;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup", meta = (Tooltip = "The socket the third person gun will snap to when picked up"))
		FName SocketThirdPerson;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Replicated, Category = "Particle Effects")
		UParticleSystem* DefaultHitParticle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Replicated, Category = "Particle Effects")
		UParticleSystem* HeadHitParticle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Replicated, Category = "Particle Effects")
		UParticleSystem* ChestHitParticle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Replicated, Category = "Particle Effects")
		UParticleSystem* TorsoHitParticle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Replicated, Category = "Particle Effects")
		UParticleSystem* LegHitParticle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Replicated, Category = "Particle Effects")
		FVector HitParticleScale;

	UPROPERTY(BlueprintReadWrite, Category = "Variables")
		AActor* OwningPlayer;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Variables")
		TSubclassOf<UDamageType> DamageType;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION()
		void OnRep_FireReplication();

	void MulticastFire();

	UFUNCTION()
		void OnRep_ReloadReplication();

	void MulticastReload();

	UFUNCTION()
		void OnRep_BurstFireReplication();

	void MulticastBurstFire();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Particle Effects", meta = (Tooltip = "This will attach to the BarrelEndScene component, to change the size of the muzzle flash rescale the BarrelEndScene component"))
		UParticleSystem* MuzzleFlashParticle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
		bool bShowDebugLine;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
		FColor DebugLineColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
		bool bDebugLinePersistent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
		float DebugLineDuration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
		bool bPrintStringOfHitPhysicalMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
		float StringDuration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
		FColor StringColor;

	UPROPERTY(ReplicatedUsing = OnRep_FireReplication)
		FMultiplayerGunFireReplication FireReplication;

	UPROPERTY(ReplicatedUsing = OnRep_ReloadReplication)
		FMultiplayerGunReloadReplication ReloadReplication;

	UPROPERTY(ReplicatedUsing = OnRep_BurstFireReplication)
		FMultiplayerGunBurstFireReplication BurstFireReplication;

	FTimerHandle FireRateTimerHandle;

	FTimerHandle BurstTimerHandle;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};