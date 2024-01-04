// Daniel Olsson

#include "BTT_SetBehaviorString.h"
#include "AIController.h"
#include "EnemyVehiclePawn.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTT_ShootPlayer::UBTT_ShootPlayer()
{
}

EBTNodeResult::Type UBTT_ShootPlayer::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::ExecuteTask(OwnerComp, NodeMemory);
	// Access the Blackboard
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	AEnemyVehiclePawn* AIPawn = Cast<AEnemyVehiclePawn>(OwnerComp.GetAIOwner()->GetPawn());
	if (AIPawn != nullptr && BlackboardComp != nullptr)
	{
		//set new behavior
		AIPawn->SetSwitchString(BlackboardComp->GetValueAsString("StringBehavior"));
		return EBTNodeResult::Succeeded;
	}
	UE_LOG(LogTemp, Warning, TEXT("failed to set drive and shoot"));
	return EBTNodeResult::Failed;
}
