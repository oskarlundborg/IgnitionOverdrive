// Fill out your copyright notice in the Description page of Project Settings.


#include "BTT_ShootPlayer.h"

#include "AIController.h"
#include "EnemyVehiclePawn.h"
#include "HomingMissileLauncher.h"
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

	if (!InitializeEverything(OwnerComp, TaskResult))
	{
		UE_LOG(LogTemp, Warning, TEXT("initalizer faild"));	
		return TaskResult;
	}


	UE_LOG(LogTemp, Warning, TEXT("task is being run x time"));
	if (HasKilled)
	{
		HasKilled = false;
		return EBTNodeResult::Succeeded;
	}
	//will this value1 work, the first iterations
	return EBTNodeResult::InProgress;
}

void UBTT_ShootPlayer::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);


	/*if (InitializeEverything(OwnerComp, TaskResult) && !InitializedComponents)
	{
		InitializedComponents = true;
		TaskResult = EBTNodeResult::InProgress;
	}
	else
	{
		TaskResult = EBTNodeResult::Failed;
		return;
	}*/

	//UE_LOG(LogTemp, Warning, TEXT("AI player shooting and rotating to turret, bullet now."));
	EnemyLocation = BlackboardComp->GetValueAsVector("EnemyLocation");
	TargetRotation = UKismetMathLibrary::FindLookAtRotation(AIPawn->GetActorLocation(), EnemyLocation);

	NewRotation = FMath::RInterpTo(PlayerTurret->GetActorRotation(), TargetRotation,
	                               GetWorld()->GetDeltaSeconds(), InterpSpeed);
	if (PlayerTurret != nullptr)
	{
		PlayerTurret->SetActorRotation(NewRotation);
	}
	ABaseVehiclePawn* Enemy = Cast<ABaseVehiclePawn>(BlackboardComp->GetValueAsObject("Enemy"));

	if (Enemy && Enemy->GetIsDead())
	{
		UE_LOG(LogTemp, Warning, TEXT("enemy is dead"));
		HasKilled = true;
		return;
	}

	if (Minigun && Minigun->GetIsOverheated())
	{
		Overheating = true;
	}
	else if(Minigun->GetOverheatValue() < 0.2)
	{
		Overheating = false;
	}

	if (Overheating)
	{
		Minigun->ReleaseTrigger();
		MinigunPulledTrigger = false;
	}
	else if (!MinigunPulledTrigger)
	{
		MinigunPulledTrigger = true;
		Minigun->PullTrigger();
	}

	/*if(HomingMissileLauncher && !HomingMissileLauncher->GetIsOnCooldown() && HomingMissileLauncher->CheckTargetInRange(Enemy))
	{
		HomingMissileLauncher->PullTrigger();
		MissilePulledTrigger = true;
	}
	else if(HomingMissileLauncher->GetChargeAmount() == MissileChargeAmount)
	{
		HomingMissileLauncher->ReleaseTrigger();
	}*/
	

	
	//Minigun->PullTrigger();
}


bool UBTT_ShootPlayer::InitializeEverything(UBehaviorTreeComponent& OwnerComp, EBTNodeResult::Type& Value1)
{
	if (!InitializeAIComponents(OwnerComp))
	{
		UE_LOG(LogTemp, Warning, TEXT("failed to initalize components"));
		Value1 = EBTNodeResult::Failed;
		return false;
	}

	UE_LOG(LogTemp, Warning, TEXT("Trying to shooting AI player"));
	TArray<AActor*> CarActors;
	//AEnemyVehiclePawn* EnemyPawn = Cast<AEnemyVehiclePawn>(AIPawn);
	AIPawn->GetAttachedActors(CarActors);

	AEnemyVehiclePawn* VehiclePawn = Cast<AEnemyVehiclePawn>(AIPawn);
	if (VehiclePawn != nullptr)
	{
		VehiclePawn->SetSwitchString("DriveAndShoot");
	}

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
		if(HomingMissileLauncher == nullptr)
		{
			HomingMissileLauncher = Cast<AHomingMissileLauncher>(ChildActor);
		}

		if (Minigun != nullptr && HomingMissileLauncher != nullptr)
		{
			UE_LOG(LogTemp, Warning, TEXT("child actor was playerturret: %s"), *ChildActor->GetName());
			break; // Exit the loop since we found what we were looking for
		}
	}

	if (Minigun == nullptr || PlayerTurret == nullptr || HomingMissileLauncher == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("minigun or player turret or homing missile launcher was null"));
		Value1 = EBTNodeResult::Failed;
		return false;
	}

	EnemyLocation = BlackboardComp->GetValueAsVector("EnemyLocation");
	if (EnemyLocation.IsZero())
	{
		return false;
	}
	return true;
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
