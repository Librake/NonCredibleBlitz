// Copyright 2022 Vster

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "MultiplayerProjectileMasterClass.generated.h"

class UStaticMeshComponent;
class UProjectileMovementComponent;

UCLASS()
class MULTIPLAYERSHOOTERPLUGIN_API AMultiplayerProjectileMasterClass : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AMultiplayerProjectileMasterClass();

	// Use this in blueprints to avoid overriding C++ begin play
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Functions")
		void BeginPlay_BP();

	// Use this in blueprints to avoid overriding C++ Tick
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Functions")
		void Tick_BP(float DeltaTime);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
		UStaticMeshComponent* ProjectileMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
		UProjectileMovementComponent* ProjectileMovement;

	UFUNCTION(BlueprintCallable, Category = "Functions")
		virtual void OnHit();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Functions")
		void ShowHitParticle_BP(UParticleSystem* SelectedEffect, FVector Location);

	UPROPERTY(BlueprintReadWrite, Category = "Variables")
		AActor* OwningPlayer;

	UPROPERTY(BlueprintReadWrite, Category = "Variables")
		AActor* OwningGun;

	UPROPERTY(BlueprintReadWrite, Category = "Variables")
		AActor* HitActor;

	UPROPERTY(BlueprintReadWrite, Category = "Variables")
		FVector ShotDirection;

	UPROPERTY(BlueprintReadWrite, Category = "Variables")
		FHitResult ProjectileHitResult;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Variables")
		TSubclassOf<UDamageType> DamageType;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Variables", meta = (Tooltip = "If the projectile goes under this velocity it gets destroyed"))
		float MinimumVelocity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
		bool bPrintStringOfHitPhysicalMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
		float StringDuration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
		FColor StringColor;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};