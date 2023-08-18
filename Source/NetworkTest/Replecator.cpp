// Fill out your copyright notice in the Description page of Project Settings.


#include "Replecator.h"

// Sets default values
AReplecator::AReplecator()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

AReplecator::~AReplecator()
{
	GLog->Log("SocketRepDeleting");
	delete socket;
	GLog->Log("SocketRepDeleted");
}

// Called when the game starts or when spawned
void AReplecator::BeginPlay()
{
	Super::BeginPlay();
	AddReplicationSocket();
}

// Called every frame
void AReplecator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AReplecator::AddReplicationSocket()
{
	socket = new SocketRep(1688, 7777, "192.168.0.138");
}



