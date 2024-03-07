// Copyright 2022 Vster

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "MultiplayerGunMasterClass.h"
#include "Camera/CameraComponent.h"
#include "Net/UnrealNetwork.h"
#include "MultiplayerPlayerCharacter.generated.h"

class UAnimationAsset;

USTRUCT()
struct FMultiplayerGiveWeapon1Replication
{
	GENERATED_BODY()

public:

};

USTRUCT()
struct FMultiplayerGiveWeapon2Replication
{
	GENERATED_BODY()

public:

};

USTRUCT()
struct FMultiplayerSwitchWeaponsReplication
{
	GENERATED_BODY()

public:

};

USTRUCT()
struct FMultiplayerSwitchWeapons1Replication
{
	GENERATED_BODY()

public:

};

USTRUCT()
struct FMultiplayerSwitchWeapons2Replication
{
	GENERATED_BODY()

public:

};

USTRUCT()
struct FMultiplayerRespawnReplication
{
	GENERATED_BODY()

public:

};

USTRUCT()
struct FMultiplayerGetRidOfWeaponsReplication
{
	GENERATED_BODY()

public:

};

USTRUCT()
struct FMultiplayerDieReplication
{
	GENERATED_BODY()

public:

};

UCLASS()
class MULTIPLAYERSHOOTERPLUGIN_API AMultiplayerPlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AMultiplayerPlayerCharacter();

	// Use this in blueprints to avoid overriding C++ begin play
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Functions")
		void BeginPlay_BP();

	// Use this in blueprints to avoid overriding C++ Tick
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Functions")
		void Tick_BP(float DeltaTime);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Functions")
		void PlayArmsAnimation_BP(UAnimationAsset* Animation, bool bLooping);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Functions")
		void SetArmsAnimationMode_BP(EAnimationMode::Type AnimationMode);

	UFUNCTION()
		virtual void MoveForward(float Scale);

	UFUNCTION()
		virtual void MoveRight(float Scale);

	UFUNCTION()
		virtual void Turn(float Scale);

	UFUNCTION()
		virtual void LookUp(float Scale);

	UFUNCTION()
		virtual float TakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	UFUNCTION(BlueprintCallable, Category = "Functions")
		virtual void Die();

	UFUNCTION(Server, Reliable, WithValidation, Category = "Functions")
		void ServerDie();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Functions")
		void Die_BP();

	UFUNCTION(BlueprintCallable, Category = "Functions")
		virtual void AutoHealthRegen();

	UFUNCTION(BlueprintCallable, Category = "Functions")
		virtual void Respawn();

	UFUNCTION(Server, Reliable, WithValidation, Category = "Functions")
		void ServerRespawn();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Functions")
		void Respawn_BP();

	UFUNCTION(BlueprintCallable, Category = "Functions")
		virtual void Fire();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Functions")
		void Fire_BP();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Functions")
		void StopFiring_BP();

	UFUNCTION(BlueprintCallable, Category = "Functions")
		virtual void GiveWeapon1();

	UFUNCTION(Server, Reliable, WithValidation, Category = "Functions")
		void ServerGiveWeapon1();

	UFUNCTION(BlueprintCallable, Category = "Functions")
		virtual void GiveWeapon2();

	UFUNCTION(Server, Reliable, WithValidation, Category = "Functions")
		void ServerGiveWeapon2();

	UFUNCTION(BlueprintCallable, Category = "Functions")
		void GetRidOfWeapons();

	UFUNCTION(Server, Reliable, WithValidation, Category = "Functions")
		void ServerGetRidOfWeapons();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Functions")
		void SetPlayerModelForWeapons_BP();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Functions")
		void ShowDefaultPluginHUD_BP();

	UFUNCTION(BlueprintCallable, Category = "Functions")
		virtual void SwitchWeapons();

	UFUNCTION(Server, Reliable, WithValidation, Category = "Functions")
		void ServerSwitchWeapons();

	UFUNCTION(BlueprintCallable, Category = "Functions")
		virtual void SwitchWeapons1();

	UFUNCTION(Server, Reliable, WithValidation, Category = "Functions")
		void ServerSwitchWeapons1();

	UFUNCTION(BlueprintCallable, Category = "Functions")
		virtual void SwitchWeapons2();

	UFUNCTION(Server, Reliable, WithValidation, Category = "Functions")
		void ServerSwitchWeapons2();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Functions")
		void SetWeapon1Hidden_BP(bool Hidden);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Functions")
		void SetWeapon2Hidden_BP(bool Hidden);

	UFUNCTION(BlueprintCallable, Category = "Functions")
		virtual void Reload();

	UFUNCTION(BlueprintCallable, Category = "Functions")
		virtual void Reload1();

	UFUNCTION(BlueprintCallable, Category = "Functions")
		virtual void Reload2();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables")
		bool bUseDefaultPluginHUD;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = 0), Replicated, Category = "Health")
		int Health;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Health")
		int MaxHealth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
		float RespawnDelay;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health", meta = (Tooltip = "The amount of health to regenerate on each tick"))
		int HealthRegenAmount;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health", meta = (Tooltip = "The amount of time between each tick"))
		float HealthRegenTickRate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
		float HealthRegenInitialDelay;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Guns")
		bool bCanShoot;

	UPROPERTY(BlueprintReadWrite, Replicated, Category = "Guns")
		AActor* Weapon1;

	UPROPERTY(BlueprintReadWrite, Replicated, Category = "Guns")
		AActor* Weapon2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shooting")
		FVector CameraLocation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shooting")
		FRotator CameraRotation;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Guns", meta = (Tooltip = "This will be weapon 1"))
		TSubclassOf<AMultiplayerGunMasterClass> DefaultWeapon1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Guns", meta = (Tooltip = "This will be weapon 2"))
		TSubclassOf<AMultiplayerGunMasterClass> DefaultWeapon2;

	// There were some issues having the camera be a C++ component so I made it a blueprint component
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables")
		UCameraComponent* PlayerBPCamera;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION()
		void OnRep_GiveWeapon1Replication();

	void MulticastGiveWeapon1();

	UFUNCTION()
		void OnRep_GiveWeapon2Replication();

	void MulticastGiveWeapon2();

	UFUNCTION()
		void OnRep_SwitchWeaponsReplication();

	void MulticastSwitchWeapons();

	UFUNCTION()
		void OnRep_SwitchWeapons1Replication();

	void MulticastSwitchWeapons1();

	UFUNCTION()
		void OnRep_SwitchWeapons2Replication();

	void MulticastSwitchWeapons2();

	UFUNCTION()
		void OnRep_RespawnReplication();

	void MulticastRespawn();

	UFUNCTION()
		void OnRep_GetRidOfWeaponsReplication();

	void MulticastGetRidOfWeapons();

	UFUNCTION()
		void OnRep_DieReplication();

	void MulticastDie();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Guns")
		int WeaponBeingUsed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Guns")
		bool bCanSwitchWeapons;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Guns", meta = (Tooltip = "How long it takes to switch weapons"))
		float AmountOfTimeToSwitchWeapons;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Guns", meta = (Tooltip = "If true weapon switch animation will restart if you switch weapons rapidly"))
		bool bPlayAnimationOnEveryWeaponSwitch;

	UPROPERTY(BlueprintReadWrite, Replicated, Category = "Variables")
		bool bIsSwitchingWeapons;

	UPROPERTY(BlueprintReadWrite, Replicated, Category = "Variables")
		bool bIsReloading;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations")
		UAnimationAsset* SwitchWeaponsAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations")
		UAnimationAsset* SwitchWeaponsAnimation1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations")
		UAnimationAsset* ThirdPersonSwitchWeaponsAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations")
		UAnimationAsset* ThirdPersonSwitchWeaponsAnimation1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations")
		UAnimationAsset* ReloadAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animations")
		UAnimationAsset* ReloadAnimation1;

	UPROPERTY(ReplicatedUsing = OnRep_GiveWeapon1Replication)
		FMultiplayerGiveWeapon1Replication GiveWeapon1Replication;

	UPROPERTY(ReplicatedUsing = OnRep_GiveWeapon2Replication)
		FMultiplayerGiveWeapon2Replication GiveWeapon2Replication;

	UPROPERTY(ReplicatedUsing = OnRep_SwitchWeaponsReplication)
		FMultiplayerSwitchWeaponsReplication SwitchWeaponsReplication;

	UPROPERTY(ReplicatedUsing = OnRep_SwitchWeapons1Replication)
		FMultiplayerSwitchWeapons1Replication SwitchWeapons1Replication;

	UPROPERTY(ReplicatedUsing = OnRep_SwitchWeapons2Replication)
		FMultiplayerSwitchWeapons2Replication SwitchWeapons2Replication;

	UPROPERTY(ReplicatedUsing = OnRep_RespawnReplication)
		FMultiplayerRespawnReplication RespawnReplication;

	UPROPERTY(ReplicatedUsing = OnRep_GetRidOfWeaponsReplication)
		FMultiplayerGetRidOfWeaponsReplication GetRidOfWeaponsReplication;

	UPROPERTY(ReplicatedUsing = OnRep_DieReplication)
		FMultiplayerDieReplication DieReplication;

	FTimerHandle RespawnTimerHandle;

	FTimerHandle HealthRegenTimerHandle;

	FTimerHandle SwitchWeaponsTimerHandle;

	FTimerHandle ReloadTimerHandle;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
};