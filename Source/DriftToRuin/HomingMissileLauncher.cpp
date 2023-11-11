// Fill out your copyright notice in the Description page of Project Settings.


#include "HomingMissileLauncher.h"

AHomingMissileLauncher::AHomingMissileLauncher()
{
	
}

void AHomingMissileLauncher::PullTrigger()
{
	Super::PullTrigger();
	
}

void AHomingMissileLauncher::ReleaseTrigger()
{
	Super::ReleaseTrigger();
	
}

void AHomingMissileLauncher::BeginPlay()
{
	Super::BeginPlay();
	//SetActorScale3D(FVector(0.08f, 0.08f, 0.08f));
}

void AHomingMissileLauncher::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}




