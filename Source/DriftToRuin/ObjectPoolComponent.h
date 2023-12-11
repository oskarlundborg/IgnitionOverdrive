// Fill out your copyright notice in the Description page of Project Settings.
/**
 * @author Gin Lindelöw
 *	Object pool that can handle any type of AActor derived object.
 **/

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h" 
#include "ObjectPoolComponent.generated.h"

UCLASS()
class UObjectPoolComponent : public UActorComponent
{
	GENERATED_BODY()
public:	
	UObjectPoolComponent();

	template <typename Type>
	FORCEINLINE Type* Fetch(const FVector& SpawnLocation, const FRotator& SpawnRotation, const TFunction<void()>& OnFetch = nullptr)
	{
		if (AActor* Object = Next())
		{
			//Object->Reset();
			Object->SetActorHiddenInGame(false);
			Object->SetActorLocation(SpawnLocation);
			Object->SetActorRotation(SpawnRotation);
			Object->SetActorTickEnabled(true);
			if (OnFetch != nullptr) { OnFetch(); };
			return static_cast<Type*>(Object);
		}
		return nullptr;
	}
	
	void Return(AActor* Object, const TFunction<void()>& OnReturn = nullptr) const;
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AActor> ObjectClass;

protected:
	virtual void BeginPlay() override;
	
private:
	AActor* Next();
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess=true))
	TArray<AActor*> Pool {};
	
	UPROPERTY(EditDefaultsOnly)
	uint64 PoolSize = 100;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta=(AllowPrivateAccess=true))
	FTransform Storage = FTransform();

	UPROPERTY(VisibleAnywhere)
	uint64 NextIndex = 0;
};
