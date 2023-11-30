// Fill out your copyright notice in the Description page of Project Settings.


#include "BTT_ShootPlayer.h"

#include "AIController.h"
#include "EnemyVehiclePawn.h"
#include "BehaviorTree/BlackboardComponent.h"


UBTT_ShootPlayer::UBTT_ShootPlayer()
{
}


EBTNodeResult::Type UBTT_ShootPlayer::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::ExecuteTask(OwnerComp, NodeMemory);

	
	AEnemyVehiclePawn* Enemy = Cast<AEnemyVehiclePawn>(OwnerComp.GetAIOwner()->GetPawn());
	if (Enemy)
	{
		UE_LOG(LogTemp, Warning, TEXT("setting drive and shoot"));
		Enemy->SetSwitchString("DriveAndShoot");
		return EBTNodeResult::Succeeded;
	}
	UE_LOG(LogTemp, Warning, TEXT("failed to set drive and shoot"));
	return EBTNodeResult::Failed;
}
