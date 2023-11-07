// Fill out your copyright notice in the Description page of Project Settings.


#include "Minigun.h"

AMinigun::AMinigun()
{
	FireRate = 6.f;
	TraceDistance = 10000.f;
	bIsFiring = false;
	bIsOverheated = false;
	OverheatValue = 0.f;
	OverheatMax = 100.f;
	OverheatBuildUpRate = 0.5f;
	OverheatCoolDownRate = 3.f;
	OverheatCooldownDuration = 2.5f;
}

void AMinigun::BeginPlay()
{
	Super::BeginPlay();
}


void AMinigun::PullTrigger()
{
	Super::PullTrigger();
	
}

void AMinigun::ReleaseTrigger()
{
	Super::ReleaseTrigger();
	
}

void AMinigun::BuildUpOverheat()
{
	
}

void AMinigun::CoolDownWeapon()
{
	
}

void AMinigun::OverheatCooldown()
{
	
}

void AMinigun::UpdateOverheat()
{
	
}

void AMinigun::AdjustProjectileAimToCrosshair(FVector SpawnLocation, FRotator& ProjectileRotation)
{
	
}

void AMinigun::Fire()
{
	
}

void AMinigun::OnPullTrigger()
{
	
}

void AMinigun::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}



