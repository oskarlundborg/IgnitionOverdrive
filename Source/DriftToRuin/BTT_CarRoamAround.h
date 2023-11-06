// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTT_CarRoamAround.generated.h"

/**
 * 
 */
UCLASS()
class DRIFTTORUIN_API UBTT_CarRoamAround : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Car Parameters")
	TArray<int32> MeterRange;

	UPROPERTY(EditAnywhere, Category = "Car Parameters")
	int32 LowestMeterDistance;

	UPROPERTY(EditAnywhere, Category = "Car Parameters")
	int32 HighestMeterDistance;

	UPROPERTY(EditAnywhere, Category = "Car Parameters")
	int32 InitialDegreeViewAtStartGame; //maybe this can be the camera view ?  

	EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory);

private:
	FVector PointLocation;

	bool MoveToPoint(FVector& Point, class APawn* Pawn);
};
