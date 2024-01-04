//Daniel Olsson - set a behavoir for the AI to use in their tick


#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTT_SetBehaviorString.generated.h"
/**
 * 
 */
UCLASS()
class DRIFTTORUIN_API UBTT_ShootPlayer : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTT_ShootPlayer();
	EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory);
};
