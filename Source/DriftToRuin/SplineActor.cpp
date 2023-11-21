// Fill out your copyright notice in the Description page of Project Settings.


#include "SplineActor.h"

#include "Components/BoxComponent.h"
#include "Components/SplineComponent.h"

// Sets default values
ASplineActor::ASplineActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Create the root component (in this case, a USceneComponent)
	USceneComponent* NewRootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	SetRootComponent(NewRootComponent);

	// Create the spline component
	SplineComponent = CreateDefaultSubobject<USplineComponent>(TEXT("SplineComponent"));
	SplineComponent->SetupAttachment(NewRootComponent); // Attach the spline component to the root

}

// Called when the game starts or when spawned
void ASplineActor::BeginPlay()
{
	Super::BeginPlay();
	
	/*for (int32 PointIndex = 0; PointIndex < SplineComponent->GetNumberOfSplinePoints(); ++PointIndex)
	{
		// Get the world space location of the spline point
		const FVector SplinePointLocation = SplineComponent->GetLocationAtSplinePoint(
			PointIndex, ESplineCoordinateSpace::World);

		// Create a collision box component
		UBoxComponent* CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox: "));
		CollisionBox->SetWorldLocation(SplinePointLocation);
		CollisionBox->SetupAttachment(RootComponent); // Attach the collision box to the root

		// Customize collision box properties if needed
		CollisionBox->SetBoxExtent(FVector(200.0f, 50.0f, 50.0f));
		//set collision profile
		CollisionBox->SetCollisionProfileName(FName("OverlapAllDynamic"));
		//projectile 
		//CollisionBox->SetCollisionResponseToChannel(ECC_Pro, ECR_Ignore);

		if (PointIndex < SplineComponent->GetNumberOfSplinePoints() - 1)
		{
			FVector Tangent = SplineComponent->GetTangentAtSplinePoint(PointIndex, ESplineCoordinateSpace::World);
			FRotator Rotation = Tangent.Rotation();
			CollisionBox->SetWorldRotation(Rotation);
		}
	}*/
}

// Called every frame
void ASplineActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
