// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SplineActor.generated.h"

UCLASS()
class DRIFTTORUIN_API ASplineActor : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ASplineActor();
	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	class USplineComponent* SplineComponent;
};
