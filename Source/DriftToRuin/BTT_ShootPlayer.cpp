// Fill out your copyright notice in the Description page of Project Settings.


#include "BTT_ShootPlayer.h"

#include "AIController.h"
#include "EnemyVehiclePawn.h"
#include "Minigun.h"
#include "PlayerTurret.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/KismetMathLibrary.h"

UBTT_ShootPlayer::UBTT_ShootPlayer()
{
	bNotifyTick = true;
}

EBTNodeResult::Type UBTT_ShootPlayer::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::ExecuteTask(OwnerComp, NodeMemory);

	if (!InitializeAIComponents(OwnerComp))
	{
		UE_LOG(LogTemp, Warning, TEXT("failed to initalize components"));
		return EBTNodeResult::Failed;
	}
	
	UE_LOG(LogTemp, Warning, TEXT("Trying to shooting AI player"));
	TArray<AActor*> CarActors;
	//AEnemyVehiclePawn* EnemyPawn = Cast<AEnemyVehiclePawn>(AIPawn);
	AIPawn->GetAttachedActors(CarActors);
	///EnemyPawn->GetAllChildActors(CarActors);
	UE_LOG(LogTemp, Warning, TEXT("AI pawn name:  %s"), *AIPawn->GetName());
	UE_LOG(LogTemp, Warning, TEXT("current children: %d"), CarActors.Num());
	for (AActor* ChildActor : CarActors)
	{
		UE_LOG(LogTemp, Warning, TEXT("child actor: %s"), *ChildActor->GetName());
		if (PlayerTurret == nullptr)
		{
			PlayerTurret = Cast<APlayerTurret>(ChildActor);
		}

		if (PlayerTurret != nullptr)
		{
			UE_LOG(LogTemp, Warning, TEXT("child actor was playerturret: %s"), *ChildActor->GetName());
			break; // Exit the loop since we found what we were looking for
		}
	}

	PlayerTurret->GetAttachedActors(CarActors);
	
	for (AActor* ChildActor : CarActors)
	{
		UE_LOG(LogTemp, Warning, TEXT("child actor: %s"), *ChildActor->GetName());
		// Check if the child actor is of type AMinigun
		if (Minigun == nullptr)
		{
			Minigun = Cast<AMinigun>(ChildActor);
		}

		if (Minigun != nullptr)
		{
			UE_LOG(LogTemp, Warning, TEXT("child actor was playerturret: %s"), *ChildActor->GetName());
			break; // Exit the loop since we found what we were looking for
		}
	}

	if (Minigun == nullptr || PlayerTurret == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("minigun or player turret was null"));
		return EBTNodeResult::Failed;
	}

	EnemyLocation = BlackboardComp->GetValueAsVector("EnemyLocation");
	
	Minigun->PullTrigger();
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
	UE_LOG(LogTemp, Warning, TEXT("AI player shooting and rotating to turret, bullet now."));
	TargetRotation = UKismetMathLibrary::FindLookAtRotation(AIPawn->GetActorLocation(), EnemyLocation);

	NewRotation = FMath::RInterpTo(PlayerTurret->GetActorRotation(), TargetRotation,
								   GetWorld()->GetDeltaSeconds(), InterpSpeed);

	//Minigun->PullTrigger();
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
