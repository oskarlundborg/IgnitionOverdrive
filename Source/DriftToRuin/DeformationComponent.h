/*
 * @author Gin Lindelöw
 *		
 */

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DeformationComponent.generated.h"

UCLASS()
class DRIFTTORUIN_API UDeformationComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UDeformationComponent();
	
	UFUNCTION(BlueprintCallable, Category=Deformation)
	void DeformMesh(const FVector Location, const FVector Normal);

	UFUNCTION(BlueprintCallable, Category=Deformation)
	void ResetMesh();

	UFUNCTION(BlueprintCallable, Category=Deformation)
	void AddMesh(USkeletalMeshComponent* Mesh);

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	UFUNCTION()
	void OnHit(AActor* Self, AActor* Other, FVector NormalImpulse, const FHitResult& Hit);
	
	UPROPERTY(EditAnywhere, Category=Deformation)
	float MaxDeform = 33.f;

	UPROPERTY(EditAnywhere, Category=Deformation)
	TArray<FString> BoneIgnoreFilter;
	
	UPROPERTY(EditAnywhere)
	bool bDrawDebug = false;

	UPROPERTY(EditAnywhere, Category=Grid)
	float PointDensity = 30.f;

	UPROPERTY(EditAnywhere, Category=Grid)
	float PointInfluenceMaxDistance = 60.f;

	UPROPERTY(EditAnywhere, Category=Grid)
	int32 PointInfluenceMaxNum = 10;
	
protected:
	virtual void BeginPlay() override;
	
	void BuildGrid();

	void SetupInfluences();

	struct FPoint
	{
		struct Vertex
		{
			uint32 Id;
			float Influence;
		};

		FVector InitialPosition;
		FVector ActivePosition;
	
		TMap<USkeletalMeshComponent*, TArray<Vertex>> SkeletalVertexInfluences;
		TMap<UStaticMeshComponent*, TArray<Vertex>> StaticVertexInfluences;
	
		TMap<USceneComponent*, float> ComponentInfluences;
	};

	TArray<FPoint> Grid;
	
	UPROPERTY()
	TArray<USkeletalMeshComponent*> SkeletalMeshComponents;
};
