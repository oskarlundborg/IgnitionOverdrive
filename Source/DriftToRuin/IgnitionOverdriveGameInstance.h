// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "IgnitionOverdriveGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class DRIFTTORUIN_API UIgnitionOverdriveGameInstance : public UGameInstance
{
	GENERATED_BODY()
	

public:

	UPROPERTY(BlueprintReadWrite)
	TArray<AActor*> PlayerStarts;

	TArray<AActor*> GetPlayerStarts();

};
