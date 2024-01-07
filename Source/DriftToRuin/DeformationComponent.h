// Fill out your copyright notice in the Description page of Project Settings.
/**
 * @author Gin Lindelöw
 *	Handler for skeletal mesh deformation at runtime.
 **/

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DeformationComponent.generated.h"

USTRUCT()
struct FVertexPositions
{
	GENERATED_BODY()

	UPROPERTY()
	FVector Initial;
	
	UPROPERTY()
	FVector Active;
};

USTRUCT()
struct FVertex
{
	GENERATED_BODY()

	UPROPERTY()
	uint32		Id;
	UPROPERTY()
	float		Influence;
	UPROPERTY()
	FVector3f	InitPosition;
};

USTRUCT()
struct FPoint
{
	GENERATED_BODY()
	
	UPROPERTY()
	FVertexPositions Position;

	TMap<USkeletalMeshComponent*, TArray<FVertex>>	VertexInfluences;
};

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
	float MaxDeform = 40.f;

	UPROPERTY(EditAnywhere, Category=Deformation)
	TArray<FString> BoneIgnoreFilter;
	
	UPROPERTY(EditAnywhere)
	bool bDrawDebug = false;

	UPROPERTY(EditAnywhere, Category=Grid)
	float PointDensity = 30.f;

	UPROPERTY(EditAnywhere, Category=Grid)
	float PointInfluenceMaxDistance = 50.f;

	UPROPERTY(EditAnywhere, Category=Grid)
	int32 PointInfluenceMaxNum = 10;
	
protected:
	virtual void BeginPlay() override;
	
private:
	void BuildGrid();

	void SetupInfluences();

	void UpdateRenderData();

	TArray<FPoint> Grid;
	
	UPROPERTY()
	TArray<USkeletalMeshComponent*> SkeletalMeshComponents;
};
