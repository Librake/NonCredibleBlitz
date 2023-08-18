// Fill out your copyright notice in the Description page of Project Settings.


#include "MyGameInstance.h"

void UMyGameInstance::pstr(std::string str)
{
    FString fstring(str.c_str());
    UE_LOG(LogTemp, Log, TEXT("%s"), *fstring);
}

bool UMyGameInstance::Tick(float DeltaSeconds)
{
    // Do your logic


   // pstr(to_string(tick));
   // tick++;
    return true;
}


void UMyGameInstance::Init()
{
    // Register delegate for ticker callback
    TickDelegateHandle = FTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateUObject(this, &UMyGameInstance::Tick));
    
    GLog->Log("Init");

    UWorld* TheWorld = GetWorld();

    //UGameplayStatics::OpenLevel(GetWorld(),)


    Super::Init();
}

void UMyGameInstance::Shutdown()
{
    // Unregister ticker delegate
    FTicker::GetCoreTicker().RemoveTicker(TickDelegateHandle);

    delete socketRep;

    Super::Shutdown();
}

void UMyGameInstance::AddReplicationSocket(FString serverIp, int serverPort)
{
    socketRep = new SocketRep(1688, serverPort, TCHAR_TO_UTF8(*serverIp));
}