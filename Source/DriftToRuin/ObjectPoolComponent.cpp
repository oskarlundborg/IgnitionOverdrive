#include "ObjectPoolComponent.h"

UObjectPoolComponent::UObjectPoolComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UObjectPoolComponent::BeginPlay()
{
	Super::BeginPlay();
	if( ensureMsgf(PoolSize <= 0, TEXT("Pool size is 0.")) ) { return; }
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	for (uint64 Index = 0; Index <= PoolSize; Index++)
	{
		Return(Pool.EmplaceAt_GetRef(Index, GetWorld()->SpawnActor(ObjectClass, &Storage, SpawnParams)));
	}
}

AActor* UObjectPoolComponent::Next()
{
	if( ensureMsgf(PoolSize <= 0, TEXT("Pool size is 0.")) ) { return nullptr; }
	if (NextIndex >= Pool.Num() - 1) { NextIndex = 0; }
	return Pool[NextIndex++];
}

void UObjectPoolComponent::Return(AActor* Object, const TFunction<void()>& OnReturn) const
{
	ensureMsgf(Object != nullptr, TEXT("Object was nullptr."));
	Object->SetActorHiddenInGame(true);
	Object->SetActorTransform(Storage);
	Object->SetActorTickEnabled(false);
	if (OnReturn != nullptr) { OnReturn(); };
}
