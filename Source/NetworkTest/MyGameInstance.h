// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"

#include "Containers/Ticker.h"
#include "Kismet/GameplayStatics.h"

#include <string>

#include "SocketRep.h"

#include "MyGameInstance.generated.h"

using namespace std;

/**
 * 
 */
UCLASS()
class NETWORKTEST_API UMyGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:

	void pstr(std::string str);

	void Init() override;

	void Shutdown() override;

	bool Tick(float DeltaSeconds);

	FDelegateHandle TickDelegateHandle;

	int tick = 0;

	SocketRep* socketRep;

	UFUNCTION(BlueprintCallable)
		void AddReplicationSocket(FString serverIp, int serverPort);
	
};
