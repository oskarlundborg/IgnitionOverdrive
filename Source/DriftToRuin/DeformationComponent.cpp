#include "DeformationComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Components/SkeletalMeshComponent.h"
#include "Rendering/SkeletalMeshModel.h"
#include "Rendering/SkeletalMeshRenderData.h"
#include "Rendering/StaticMeshVertexBuffer.h"
#include "Rendering/SkeletalMeshLODModel.h"
#include "StaticMeshResources.h"
#include "Engine/SkinnedAssetCommon.h"
#include "DrawDebugHelpers.h"

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
			DrawDebugSphere(
				GetWorld(),
				GetOwner()->GetTransform().TransformPosition(FVector(Point.Position.Initial) + FVector(Point.Position.Active)),
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
	DeformMesh(
		GetOwner()->GetActorTransform().InverseTransformPosition(Hit.Location),
		GetOwner()->GetActorTransform().InverseTransformVector(NormalImpulse).GetClampedToMaxSize(MaxDeform) * 3
	);
}

void UDeformationComponent::BuildGrid()
{
	if (SkeletalMeshComponents.IsEmpty()) { return; }

	FBoxSphereBounds Bounds;
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
				Grid.Add(FPoint({UE::Math::TVector<float>(BoundsStart) + UE::Math::TVector<float>(x * Distance.X, y * Distance.Y, z * Distance.Z)}));
			}
		}
	}
}

void UDeformationComponent::SetupInfluences()
{
	// __________________
	// Add Skeletal Meshes.
	for (const auto& Mesh : SkeletalMeshComponents)
	{
		if(Mesh->GetNumBones() <= 0)
		{
			TMap<int32, float> InfluencePoints;
			for (int32 Index = 0; Index < Grid.Num(); ++Index)
			{
				const float DistanceSquared = FVector::DistSquared(GetOwner()->GetTransform().InverseTransformPosition(Mesh->GetComponentLocation()), FVector(Grid[Index].Position.Initial));
				if (DistanceSquared < 15.f)
				{
					InfluencePoints.Add(Index, DistanceSquared);
				}
			}
		}
		USkeletalMesh* NewMesh = Mesh->GetSkeletalMeshAsset();
		NewMesh->GetLODInfo(0)->bAllowCPUAccess = true;

		FSkeletalMeshLODRenderData& LODRenderData = Mesh->GetSkeletalMeshAsset()->GetResourceForRendering()->LODRenderData[0];
		FPositionVertexBuffer& Positions = Mesh->GetSkeletalMeshAsset()->GetResourceForRendering()->LODRenderData[0].StaticVertexBuffers.PositionVertexBuffer;

		TArray<int32> IgnoredBones;
		TArray<FName> BoneNames;
		Mesh->GetBoneNames(BoneNames);
		for (int Index = 0; FName Name : BoneNames)
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
				const float PointDistSquared = FVector::DistSquared(FVector(Positions.VertexPosition(i)), FVector(Grid[PointID].Position.Initial));
				if (PointDistSquared < FMath::Square(PointInfluenceMaxDistance))
				{
					InfluencePoints.Add(PointID, PointDistSquared);
				}
			}
			InfluencePoints.ValueSort([](const float& A, const float& B) { return A < B; });
			for (int32 InfPointID = 0; const TTuple<int32, float> InfluencePoint : InfluencePoints)
			{
				Grid[InfluencePoint.Key].VertexInfluences.FindOrAdd(Mesh).Add(
					FPoint::FVertex {
						.Id				= i,
						.Influence		= 1 - FMath::Sqrt(InfluencePoint.Value) / PointInfluenceMaxDistance,
						.InitPosition	= LODRenderData.StaticVertexBuffers.PositionVertexBuffer.VertexPosition(i)
					}
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
		// Build Static Mesh Vertex Buffer and copy the vertices into buffer.
		LODMeshRenderData->StaticVertexBuffers.StaticMeshVertexBuffer.Init(
			VertexCount,
			SourceLODRenderData.StaticVertexBuffers.StaticMeshVertexBuffer.GetNumTexCoords(),
			true
		);
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
	const float Size = Normal.Size();

	for (FPoint& Point : Grid)
	{
		const float DistSquared = FVector::DistSquared(FVector(Point.Position.Initial) + FVector(Point.Position.Active), Location);
		if (DistSquared > FMath::Square(Size)) { continue; }

		const float Percent = 1 - FMath::Clamp(FMath::Sqrt(DistSquared) / Size, 0.f, 1.f);
		const FVector PreviousPos = FVector(Point.Position.Active);
		Point.Position.Active = (Point.Position.Active + UE::Math::TVector<float>(Normal) * Percent).GetClampedToMaxSize(MaxDeform);
		const FVector Movement = FVector(Point.Position.Active) - PreviousPos;

		// __________________
		// Move Vertices.
		for (const auto& VertexInfluence : Point.VertexInfluences)
		{
			if (!VertexInfluence.Key) { continue; }
			
			FPositionVertexBuffer& PositionVertexBuffer = VertexInfluence.Key
				->GetSkeletalMeshAsset()
				->GetResourceForRendering()
				->LODRenderData[0].StaticVertexBuffers.PositionVertexBuffer;

			for (const FPoint::FVertex& Vertex : VertexInfluence.Value)
			{
				const FVector VertexDelta = Movement * FVector(Vertex.Influence);
				PositionVertexBuffer.VertexPosition(Vertex.Id) += FVector3f(VertexDelta);
			}
		}
	}
	UpdateRenderData();
}

void UDeformationComponent::ResetMesh()
{
	for (FPoint& Point : Grid)
	{
		for (const auto& VertexInfluence : Point.VertexInfluences)
		{
			if (!VertexInfluence.Key) { continue; }
			FPositionVertexBuffer& PositionVertexBuffer = VertexInfluence.Key
				->GetSkeletalMeshAsset()
				->GetResourceForRendering()
				->LODRenderData[0].StaticVertexBuffers.PositionVertexBuffer;

			for (const FPoint::FVertex& Vertex : VertexInfluence.Value)
			{
				PositionVertexBuffer.VertexPosition(Vertex.Id) = Vertex.InitPosition;
			}
		}
		Point.Position.Active = Point.Position.Initial;
	}
	Grid.Empty();
	BuildGrid();
	SetupInfluences();
	UpdateRenderData();
}

void UDeformationComponent::UpdateRenderData()
{
	// __________________
	// Enqueue update of the render data to the gpu.
	ENQUEUE_RENDER_COMMAND(UpdateSkeletalMeshRenderData)([&](FRHICommandListImmediate& RHICmdList)
	{
		for (const auto& Mesh : SkeletalMeshComponents)
		{
			FSkeletalMeshLODRenderData& LODRenderData = Mesh->GetSkeletalMeshAsset()->GetResourceForRendering()->LODRenderData[0];
			LODRenderData.StaticVertexBuffers.PositionVertexBuffer.UpdateRHI(RHICmdList);
			LODRenderData.StaticVertexBuffers.StaticMeshVertexBuffer.UpdateRHI(RHICmdList);
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
