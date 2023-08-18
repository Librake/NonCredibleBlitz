// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "GameFramework/Actor.h"
#include "SocketRep.h"

#include "Replecator.generated.h"

UCLASS()
class NETWORKTEST_API AReplecator : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AReplecator();
	~AReplecator();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	SocketRep* socket;

	UFUNCTION(BlueprintCallable)
		void AddReplicationSocket();

};
