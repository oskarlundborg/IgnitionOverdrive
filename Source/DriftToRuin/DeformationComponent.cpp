#include "DeformationComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/StaticMesh.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Rendering/SkeletalMeshModel.h"
#include "Rendering/SkeletalMeshRenderData.h"
#include "Rendering/StaticMeshVertexBuffer.h"
#include "Rendering/SkeletalMeshLODModel.h"
#include "StaticMeshResources.h"
#include "Components/LineBatchComponent.h"
#include "Engine/SkinnedAssetCommon.h"
#if WITH_EDITOR
#include "DrawDebugHelpers.h"
#endif

UDeformationComponent::UDeformationComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UDeformationComponent::AddMesh(USkeletalMeshComponent* Mesh)
{
	SkeletalMeshComponents.Add(Mesh);
}

void UDeformationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* TickFunction)
{
	if (bDrawDebug)
	{
		for (FPoint Point : Grid)
		{
			GetWorld()->LineBatcher.Get()->DrawSphere(
				GetOwner()->GetTransform().TransformPosition(Point.InitialPosition + Point.ActivePosition),
				10,
				3,
				FColor::Cyan,
				false,
				-1,
				1
			);
		}
	}
}

void UDeformationComponent::BeginPlay()
{
	Super::BeginPlay();
	GetOwner()->OnActorHit.AddDynamic(this, &UDeformationComponent::OnHit);
	BuildGrid();
	SetupInfluences();
}

void UDeformationComponent::OnHit(AActor* Self, AActor* Other, FVector NormalImpulse, const FHitResult& Hit)
{
	const FVector RelativeLocation = GetOwner()->GetActorTransform().InverseTransformPosition(Hit.Location);
	FVector RelativeNormal = GetOwner()->GetActorTransform().InverseTransformVector(NormalImpulse)
		* -FMath::Sign(FVector::DotProduct(RelativeNormal.GetSafeNormal(), RelativeLocation.GetSafeNormal()));

	float Mass = 100.f;
	if (Hit.Component->Mobility == EComponentMobility::Movable)
	{
		Mass = Hit.Component->GetMass();
	}
	const float ImpactForce = ((Other->GetVelocity() + Self->GetVelocity()).Size() * Mass) / 100;
	
	DeformMesh(RelativeLocation, (RelativeNormal * 0.1).GetClampedToMaxSize(MaxDeform));
}

void UDeformationComponent::BuildGrid()
{
	if (SkeletalMeshComponents.IsEmpty()) { return; }
	
	FBoxSphereBounds Bounds = FBoxSphereBounds(FVector::ZeroVector, FVector::ZeroVector, 0.f);
	for (const USkeletalMeshComponent* Mesh : SkeletalMeshComponents)
	{
		Bounds = Bounds + Mesh->GetSkeletalMeshAsset()->GetImportedBounds();
	}
	const FVector Points {
		FMath::Max(Bounds.BoxExtent.X / PointDensity, 1),
		FMath::Max(Bounds.BoxExtent.Y / PointDensity, 1),
		FMath::Max(Bounds.BoxExtent.Z / PointDensity, 1)
	};
	const FVector Distance {
		Bounds.BoxExtent.X * 2 / (Points.X),
		Bounds.BoxExtent.Y * 2 / (Points.Y),
		Bounds.BoxExtent.Z * 2 / (Points.Z)
	};
	const FVector BoundsStart = Bounds.Origin - Bounds.BoxExtent + 0.5 * FVector(Distance.X, Distance.Y, Distance.Z);

	for (int32 x = 0; x < Points.X; x++)
	{
		for (int32 y = 0; y < Points.Y; y++)
		{
			for (int32 z = 0; z < Points.Z; z++)
			{
				Grid.Add(FPoint(BoundsStart + FVector(x * Distance.X, y * Distance.Y, z * Distance.Z)));
			}
		}
	}
}

void UDeformationComponent::SetupInfluences()
{
	const float PointInfluenceMaxDistSquared = FMath::Square(PointInfluenceMaxDistance);

	// __________________
	// Add Skeletal Meshes.
	for (const auto& Mesh : SkeletalMeshComponents)
	{
		USkeletalMesh* NewMesh = Mesh->GetSkeletalMeshAsset();
		NewMesh->GetLODInfo(0)->bAllowCPUAccess = true;

		FSkeletalMeshLODRenderData& LODRenderData = Mesh->GetSkeletalMeshAsset()->GetResourceForRendering()->LODRenderData[0];
		FPositionVertexBuffer& Positions = Mesh->GetSkeletalMeshAsset()->GetResourceForRendering()->LODRenderData[0].StaticVertexBuffers.PositionVertexBuffer;

		TArray<int32> IgnoredBones;
		TArray<FName> BoneNames;
		Mesh->GetBoneNames(BoneNames);
		for ( int Index = 0; FName Name : BoneNames)
		{
			if( BoneIgnoreFilter.Contains(Name) )
			{
				IgnoredBones.Add(Index);
			}
			Index++;
		}
		
		
		// __________________
		// Add Vertices.
		for (uint32 i = 0; i < Positions.GetNumVertices(); ++i)
		{
			uint8 MaxWeight = 0;
			int32 BoneIndex = 0; 
			for (int32 WeightIdx = 0; WeightIdx < MAX_TOTAL_INFLUENCES; WeightIdx++)
			{
				const uint8 Weight = LODRenderData.SkinWeightVertexBuffer.GetBoneWeight(i, WeightIdx);

				if (Weight > MaxWeight)
				{
					MaxWeight = Weight;
					BoneIndex = LODRenderData.SkinWeightVertexBuffer.GetBoneIndex(i, WeightIdx);
				}
			}
			int32 SectionIndex;
			int32 SectionVertexIndex;
			LODRenderData.GetSectionFromVertexIndex(i, SectionIndex, SectionVertexIndex);
			BoneIndex = LODRenderData.RenderSections[SectionIndex].BoneMap[BoneIndex];

			if (IgnoredBones.Contains(BoneIndex)) { continue; }

			TMap<int32, float> InfluencePoints;
			for (int32 PointID = 0; PointID < Grid.Num(); ++PointID)
			{
				const float PointDistSquared = FVector::DistSquared(FVector(Positions.VertexPosition(i)), Grid[PointID].InitialPosition);
				if (PointDistSquared < PointInfluenceMaxDistSquared)
				{
					InfluencePoints.Add(PointID, PointDistSquared);
				}
			}
			InfluencePoints.ValueSort([](const float& A, const float& B)
			{
				return A < B;
			});
			for (int32 InfPointID = 0; const TTuple<int32, float> InfluencePoint : InfluencePoints)
			{
				Grid[InfluencePoint.Key].SkeletalVertexInfluences.FindOrAdd(Mesh).Add( FPoint::Vertex {
					.Id = i,
					.Influence = 1 - FMath::Sqrt(InfluencePoint.Value) / PointInfluenceMaxDistance}
				);
				InfPointID++;
				if (InfPointID >= PointInfluenceMaxNum) break;
			}
		}
	}
	for (USkeletalMeshComponent* Mesh : SkeletalMeshComponents)
	{
		const TObjectPtr<USkeletalMesh> SourceSkeletalMesh = Mesh->GetSkeletalMeshAsset();
		TObjectPtr<USkeletalMesh> TargetSkeletalMesh = NewObject<USkeletalMesh>();

		auto& SourceLODRenderData = SourceSkeletalMesh->GetResourceForRendering()->LODRenderData[0];
		SourceSkeletalMesh->GetLODInfo(0)->bAllowCPUAccess = true;

		TargetSkeletalMesh->SetRefSkeleton(SourceSkeletalMesh->GetRefSkeleton());
		TargetSkeletalMesh->SetSkeleton(SourceSkeletalMesh->GetSkeleton());
		TargetSkeletalMesh->SetPhysicsAsset(SourceSkeletalMesh->GetPhysicsAsset());

		// __________________
		// Allocate Resources.
		TargetSkeletalMesh->AllocateResourceForRendering();
		FSkeletalMeshRenderData* MeshRenderData = TargetSkeletalMesh->GetResourceForRendering();
		FSkeletalMeshLODRenderData* LODMeshRenderData = new FSkeletalMeshLODRenderData;
		MeshRenderData->LODRenderData.Add(LODMeshRenderData);

		// __________________
		// Add LOD Data.
		TargetSkeletalMesh->ResetLODInfo();
		FSkeletalMeshLODInfo& MeshLodInfo = TargetSkeletalMesh->AddLODInfo();
		MeshLodInfo.LODHysteresis = 0.02;
		MeshLodInfo.ScreenSize = 1.0;
		MeshLodInfo.bAllowCPUAccess = true;

		// __________________
		// Set Imported Bounds.
		TargetSkeletalMesh->SetImportedBounds(SourceSkeletalMesh->GetImportedBounds());

#if WITH_EDITORONLY_DATA
		FSkeletalMeshLODModel* NewSkeletalMeshLODModel = new FSkeletalMeshLODModel();
		NewSkeletalMeshLODModel->CopyStructure(NewSkeletalMeshLODModel, &SourceSkeletalMesh->GetImportedModel()->LODModels[0]);
		TargetSkeletalMesh->GetImportedModel()->LODModels.Add(NewSkeletalMeshLODModel);
#endif

		LODMeshRenderData->RenderSections = SourceLODRenderData.RenderSections;

		TArray<uint32> IndexBuffer;
		SourceLODRenderData.MultiSizeIndexContainer.GetIndexBuffer(IndexBuffer);
		LODMeshRenderData->MultiSizeIndexContainer.RebuildIndexBuffer(SourceLODRenderData.MultiSizeIndexContainer.GetDataTypeSize(), IndexBuffer);

		// __________________
		// Build Position Vertex Buffer.
		const uint32 VertexCount = SourceLODRenderData.StaticVertexBuffers.PositionVertexBuffer.GetNumVertices();
		LODMeshRenderData->StaticVertexBuffers.PositionVertexBuffer.Init(VertexCount, true);
		for (uint32 i = 0; i < VertexCount; ++i)
		{
			LODMeshRenderData->StaticVertexBuffers.PositionVertexBuffer.VertexPosition(i) = SourceLODRenderData.StaticVertexBuffers.PositionVertexBuffer.VertexPosition(i);
		}

		// __________________
		// Build Static Mesh Vertex Buffer.
		LODMeshRenderData->StaticVertexBuffers.StaticMeshVertexBuffer.Init(
			VertexCount,
			SourceLODRenderData.StaticVertexBuffers.StaticMeshVertexBuffer.GetNumTexCoords(),
			true
		);

		// __________________
		// Copy the vertices into the buffer.
		for (uint32 VertexIndex = 0; VertexIndex < VertexCount; VertexIndex++)
		{
			LODMeshRenderData->StaticVertexBuffers.StaticMeshVertexBuffer.SetVertexTangents(
				VertexIndex,
				SourceLODRenderData.StaticVertexBuffers.StaticMeshVertexBuffer.VertexTangentX(VertexIndex),
				SourceLODRenderData.StaticVertexBuffers.StaticMeshVertexBuffer.VertexTangentY(VertexIndex),
				SourceLODRenderData.StaticVertexBuffers.StaticMeshVertexBuffer.VertexTangentZ(VertexIndex)
			);

			for (uint32 UVIndex = 0; UVIndex < SourceLODRenderData.StaticVertexBuffers.StaticMeshVertexBuffer.GetNumTexCoords(); UVIndex++)
			{
				LODMeshRenderData->StaticVertexBuffers.StaticMeshVertexBuffer.SetVertexUV(
					VertexIndex,
					UVIndex,
					SourceLODRenderData.StaticVertexBuffers.StaticMeshVertexBuffer.GetVertexUV(VertexIndex, UVIndex)
				);
			}
		}

		// __________________
		// Build Skin Weight Buffer.
		LODMeshRenderData->SkinWeightVertexBuffer.SetMaxBoneInfluences(SourceLODRenderData.SkinWeightVertexBuffer.GetMaxBoneInfluences());
		LODMeshRenderData->SkinWeightVertexBuffer.SetUse16BitBoneIndex(SourceLODRenderData.SkinWeightVertexBuffer.Use16BitBoneIndex());
		TArray<FSkinWeightInfo> Weights;
		SourceLODRenderData.SkinWeightVertexBuffer.GetSkinWeights(Weights);
		LODMeshRenderData->SkinWeightVertexBuffer = Weights;

		// __________________
		// Set the default Material.
		TargetSkeletalMesh->SetMaterials(SourceSkeletalMesh->GetMaterials());
		
		// __________________
		// Set Bone data.
		LODMeshRenderData->RequiredBones = SourceLODRenderData.RequiredBones;
		LODMeshRenderData->ActiveBoneIndices = SourceLODRenderData.ActiveBoneIndices;

		// __________________
		// TODO: Figure out how to use UMirrorDataTable for this as that is the new way.
		//MirrorDataTable->EmptyTable();
		//MirrorDataTable->MirrorAxis = EAxis::Type::None;
		TargetSkeletalMesh->SkelMirrorTable.Empty();
		TargetSkeletalMesh->SkelMirrorAxis = EAxis::Type::None;
		TargetSkeletalMesh->SkelMirrorFlipAxis = EAxis::Type::None;
		TargetSkeletalMesh->GetRefBasesInvMatrix().Empty();
		TargetSkeletalMesh->CalculateInvRefMatrices();
		
		FlushRenderingCommands();
		TargetSkeletalMesh->PostLoad();
#if WITH_EDITOR
		TargetSkeletalMesh->StackPostEditChange();
#endif
		Mesh->SetSkeletalMesh(TargetSkeletalMesh);
	}
}

void UDeformationComponent::DeformMesh(const FVector Location, const FVector Normal)
{
	const float Size = Normal.Size() * 3;
	const float SizeSquared = FMath::Square(Size);

	for (FPoint& Point : Grid)
	{
		const float DistSquared = FVector::DistSquared(Point.InitialPosition + Point.ActivePosition, Location);
		if (DistSquared > SizeSquared) { continue; }

		const float Percent = 1 - FMath::Clamp(FMath::Sqrt(DistSquared) / Size, 0.f, 1.f);
		const FVector PreviousPos = Point.ActivePosition;
		Point.ActivePosition = (Point.ActivePosition + Normal * FVector(Percent)).GetClampedToMaxSize(MaxDeform);
		const FVector Movement = Point.ActivePosition - PreviousPos;

		// __________________
		// Move Vertices.
		for (const auto& SkeletalVertexInfluence : Point.SkeletalVertexInfluences)
		{
			if (!SkeletalVertexInfluence.Key) { continue; }

			FPositionVertexBuffer& Positions = SkeletalVertexInfluence.Key->GetSkeletalMeshAsset()->GetResourceForRendering()->LODRenderData[0].StaticVertexBuffers.PositionVertexBuffer;
			FColorVertexBuffer& ColorVertexBuffer = SkeletalVertexInfluence.Key->GetSkeletalMeshAsset()->GetResourceForRendering()->LODRenderData[0].StaticVertexBuffers.ColorVertexBuffer;

			for (const FPoint::Vertex& Vertex : SkeletalVertexInfluence.Value)
			{
				const FVector VertexDelta = Movement * FVector(Vertex.Influence);
				Positions.VertexPosition(Vertex.Id) += FVector3f(VertexDelta);

				if (ColorVertexBuffer.GetNumVertices() > Vertex.Id)
				{
					ColorVertexBuffer.VertexColor(Vertex.Id).R = FMath::Min(255 - VertexDelta.Size() / MaxDeform * 255, ColorVertexBuffer.VertexColor(Vertex.Id).R);
				}
			}
		}
		for (const auto& StaticVertexInfluence : Point.StaticVertexInfluences)
		{
			if (!StaticVertexInfluence.Key) { continue; }
			
			FStaticMeshLODResources& LODResources = StaticVertexInfluence.Key->GetStaticMesh()->GetRenderData()->LODResources[0];
			FPositionVertexBuffer& Positions = LODResources.VertexBuffers.PositionVertexBuffer;
			FColorVertexBuffer& ColorVertexBuffer = LODResources.VertexBuffers.ColorVertexBuffer;

			for (const FPoint::Vertex& Vertex : StaticVertexInfluence.Value)
			{
				const FVector VertexDelta = Movement * FVector(Vertex.Influence);
				Positions.VertexPosition(Vertex.Id) += FVector3f(VertexDelta);
				if (ColorVertexBuffer.GetNumVertices() > Vertex.Id)
				{
					ColorVertexBuffer.VertexColor(Vertex.Id).R = FMath::Min(255 - VertexDelta.Size() / MaxDeform * 255, ColorVertexBuffer.VertexColor(Vertex.Id).R);
				}
			}
		}
		// __________________
		// Move Attached Components.
		for (const TTuple<USceneComponent*, float> InfluenceComponent : Point.ComponentInfluences)
		{
			InfluenceComponent.Key->AddLocalOffset(Movement * FVector(InfluenceComponent.Value));
		}
	}
	// __________________
	// Update the render data.
	ENQUEUE_RENDER_COMMAND(UpdateSkeletalMeshRenderData)([&](FRHICommandListImmediate& RHICmdList)
	{
		for (const auto& Mesh : SkeletalMeshComponents)
		{
			FSkeletalMeshLODRenderData& LODRenderData = Mesh->GetSkeletalMeshAsset()->GetResourceForRendering()->LODRenderData[0];
			LODRenderData.StaticVertexBuffers.PositionVertexBuffer.UpdateRHI(RHICmdList);
			LODRenderData.StaticVertexBuffers.StaticMeshVertexBuffer.UpdateRHI(RHICmdList);
			LODRenderData.StaticVertexBuffers.ColorVertexBuffer.UpdateRHI(RHICmdList);
		}
	});
	// __________________
	// Notify change in render data to apply update.
	for (const auto& Mesh : SkeletalMeshComponents)
	{
		Mesh->MarkRenderStateDirty();
		if (Mesh->GetCPUSkinningEnabled())
		{
			Mesh->MarkRenderDynamicDataDirty();
		}
	}
}

void UDeformationComponent::ResetMesh()
{
	for (FPoint& Point : Grid)
	{
		// __________________
		// Move Vertices.
		for (const auto& SkeletalVertexInfluence : Point.SkeletalVertexInfluences)
		{
			if (!SkeletalVertexInfluence.Key) { continue; }

			FPositionVertexBuffer& Positions = SkeletalVertexInfluence.Key->GetSkeletalMeshAsset()->GetResourceForRendering()->LODRenderData[0].StaticVertexBuffers.PositionVertexBuffer;
			FColorVertexBuffer& ColorVertexBuffer = SkeletalVertexInfluence.Key->GetSkeletalMeshAsset()->GetResourceForRendering()->LODRenderData[0].StaticVertexBuffers.ColorVertexBuffer;

			for (const FPoint::Vertex& Vertex : SkeletalVertexInfluence.Value)
			{
				Positions.VertexPosition(Vertex.Id) = FVector3f(Point.InitialPosition);

				if (ColorVertexBuffer.GetNumVertices() > Vertex.Id)
				{
					ColorVertexBuffer.VertexColor(Vertex.Id).R = FMath::Min(255 - FVector3f(Point.InitialPosition).Size() / MaxDeform * 255, ColorVertexBuffer.VertexColor(Vertex.Id).R);
				}
			}
		}
		for (const auto& StaticVertexInfluence : Point.StaticVertexInfluences)
		{
			if (!StaticVertexInfluence.Key) { continue; }
			
			FStaticMeshLODResources& LODResources = StaticVertexInfluence.Key->GetStaticMesh()->GetRenderData()->LODResources[0];
			FPositionVertexBuffer& Positions = LODResources.VertexBuffers.PositionVertexBuffer;
			FColorVertexBuffer& ColorVertexBuffer = LODResources.VertexBuffers.ColorVertexBuffer;

			for (const FPoint::Vertex& Vertex : StaticVertexInfluence.Value)
			{
				Positions.VertexPosition(Vertex.Id) = FVector3f(Point.InitialPosition);
				if (ColorVertexBuffer.GetNumVertices() > Vertex.Id)
				{
					ColorVertexBuffer.VertexColor(Vertex.Id).R = FMath::Min(255 - FVector3f(Point.InitialPosition).Size() / MaxDeform * 255, ColorVertexBuffer.VertexColor(Vertex.Id).R);
				}
			}
		}
		// __________________
		// Move Attached Components.
		for (const TTuple<USceneComponent*, float> InfluenceComponent : Point.ComponentInfluences)
		{
			InfluenceComponent.Key->AddLocalOffset(FVector(Point.InitialPosition) * FVector(InfluenceComponent.Value));
		}
	}
	// __________________
	// Update the render data.
	ENQUEUE_RENDER_COMMAND(UpdateSkeletalMeshRenderData)([&](FRHICommandListImmediate& RHICmdList)
	{
		for (const auto& Mesh : SkeletalMeshComponents)
		{
			FSkeletalMeshLODRenderData& LODRenderData = Mesh->GetSkeletalMeshAsset()->GetResourceForRendering()->LODRenderData[0];
			LODRenderData.StaticVertexBuffers.PositionVertexBuffer.UpdateRHI(RHICmdList);
			LODRenderData.StaticVertexBuffers.StaticMeshVertexBuffer.UpdateRHI(RHICmdList);
			LODRenderData.StaticVertexBuffers.ColorVertexBuffer.UpdateRHI(RHICmdList);
		}
	});
	// __________________
	// Notify change in render data to apply update.
	for (const auto& Mesh : SkeletalMeshComponents)
	{
		Mesh->MarkRenderStateDirty();
		if (Mesh->GetCPUSkinningEnabled())
		{
			Mesh->MarkRenderDynamicDataDirty();
		}
	}
}
