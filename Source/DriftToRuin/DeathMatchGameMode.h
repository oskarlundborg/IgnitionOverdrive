// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "DeathMatchGameMode.generated.h"

/**
 * 
 */
UCLASS()
class DRIFTTORUIN_API ADeathMatchGameMode : public AGameModeBase
{
	GENERATED_BODY()


public:

	AActor* FindPlayerStart_Implementation(AController* Player, const FString& IncomingName) override;
	

};
