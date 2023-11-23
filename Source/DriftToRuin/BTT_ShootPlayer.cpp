// Fill out your copyright notice in the Description page of Project Settings.


#include "BTT_ShootPlayer.h"

#include "AIController.h"
#include "Minigun.h"

UBTT_ShootPlayer::UBTT_ShootPlayer()
{
	bNotifyTick = true;
}

EBTNodeResult::Type UBTT_ShootPlayer::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::ExecuteTask(OwnerComp, NodeMemory);


	UE_LOG(LogTemp, Warning, TEXT("Trying to shooting AI player"));
	Minigun = AIPawn->GetComponentByClass<AMinigun>();
	if(Minigun == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("minigun was null"));
		return  EBTNodeResult::Failed;
	}
	
	TickTask(OwnerComp, NodeMemory, GetWorld()->DeltaTimeSeconds);
	
	if (HasKilled)
	{
		return EBTNodeResult::Succeeded;
	}
	return EBTNodeResult::InProgress;
}

void UBTT_ShootPlayer::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);
	UE_LOG(LogTemp, Warning, TEXT("Trying to shooting AI player"));
	
	Minigun->PullTrigger();
}

bool UBTT_ShootPlayer::InitializeAIComponents(UBehaviorTreeComponent& OwnerComp)
{
	// Get references to necessary components
	AIController = OwnerComp.GetAIOwner();
	ensureMsgf(AIController != nullptr, TEXT("AI controller was nullptr"));
	if (AIController != nullptr)
	{
		AIPawn = AIController->GetPawn();
	}

	// Access the Blackboard
	BlackboardComp = OwnerComp.GetBlackboardComponent();
	ensureMsgf(BlackboardComp != nullptr, TEXT("BlackboardComp was nullptr"));
	if (BlackboardComp == nullptr)
	{
		return false;
	}

	ensureMsgf(AIPawn != nullptr, TEXT("AI PAWN was nullptr"));
	if (AIPawn == nullptr)
	{
		return false;
	}
	return true;
}
