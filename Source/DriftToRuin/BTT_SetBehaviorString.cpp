// Daniel Olsson

#include "BTT_SetBehaviorString.h"
#include "AIController.h"
#include "EnemyVehiclePawn.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTT_SetBehaviorString::UBTT_SetBehaviorString()
{
}

// Executes the behavior tree task to set the behavior string for the AI pawn.
// Returns the result of the task execution, indicating success or failure.

EBTNodeResult::Type UBTT_SetBehaviorString::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// Call the parent class's ExecuteTask method to ensure any necessary setup is performed.
	Super::ExecuteTask(OwnerComp, NodeMemory);

	// Access the blackboard component associated with the behavior tree.
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	
    
	// Get a reference to the AI pawn controlled by the AI controller, casting it to AEnemyVehiclePawn.
	AEnemyVehiclePawn* AIPawn = Cast<AEnemyVehiclePawn>(OwnerComp.GetAIOwner()->GetPawn());
    
	// Check if both the AI pawn and the blackboard component are valid.
	if (AIPawn != nullptr && BlackboardComp != nullptr)
	{
		// Set the new behavior for the AI pawn using the string value from the blackboard.
		AIPawn->SetSwitchString(BlackboardComp->GetValueAsString("StringBehavior"));
        
		// Return success if the behavior was set successfully.
		return EBTNodeResult::Succeeded;
	}
    
	// Return failure if the AI pawn or the blackboard component is invalid.
	return EBTNodeResult::Failed;
}
