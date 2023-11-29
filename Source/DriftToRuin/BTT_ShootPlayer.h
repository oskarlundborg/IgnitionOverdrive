// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTT_ShootPlayer.generated.h"

class AHomingMissileLauncher;
class AMinigun;
class APlayerTurret;
/**
 * 
 */
UCLASS()
class DRIFTTORUIN_API UBTT_ShootPlayer : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTT_ShootPlayer();
	bool InitializeEverything(UBehaviorTreeComponent& OwnerComp, EBTNodeResult::Type& Value1);
	EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory);
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

	bool HasKilled = false;

private:
	bool InitializeAIComponents(UBehaviorTreeComponent& OwnerComp);
	APawn* AIPawn;
	AAIController* AIController;
	UBlackboardComponent* BlackboardComp;
	AMinigun* Minigun = nullptr;
	APlayerTurret* PlayerTurret = nullptr;
	AHomingMissileLauncher* HomingMissileLauncher = nullptr;

	//enemy
	FVector EnemyLocation;

	//rotations
	FRotator NewRotation;
	FRotator TargetRotation;
	float InterpSpeed = 1;

	bool Overheating = false;
	bool MinigunPulledTrigger = false;
	//bool MissilePulledTrigger = false;
	bool MissileIsAvailable = false;

	bool InitializedComponents = false;

	int32 MissileChargeAmount = FMath::RandRange(1, 3);

	//result type
	EBTNodeResult::Type TaskResult;
};
