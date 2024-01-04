// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseProjectile.h"

#include "NiagaraComponent.h"
#include "Components/AudioComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "PhysicsEngine/RadialForceComponent.h"

ABaseProjectile::ABaseProjectile()
{
	PrimaryActorTick.bCanEverTick = true;
	
	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Projectile Mesh"));
	SetRootComponent(ProjectileMesh);

	ProjectileVfxNiagaraComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("ProjectileNiagaraComponent"));
	ProjectileVfxNiagaraComponent->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform, TEXT("VFX"));

	RadialForceComponent = CreateDefaultSubobject<URadialForceComponent>(TEXT("Radial force"));
	RadialForceComponent->SetupAttachment(RootComponent);

	WhizzingAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("Whizzing Audio Component"));
	WhizzingAudioComponent->SetupAttachment(RootComponent);
	
	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Projectile Movement Component"));
	ProjectileMovementComponent->MaxSpeed = 8000.f;
	ProjectileMovementComponent->InitialSpeed = 8000.f;
}

/*Getter for the projectile movement component*/
UProjectileMovementComponent* ABaseProjectile::GetProjectileMovementComponent()
{
	return ProjectileMovementComponent;
}

/*Virtual callback function for block collision. Bound to OnComponentHit delegate. Holds logic executed when projectiles hit all non-vehicle objects*/
void ABaseProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	FVector NormalImpusle, const FHitResult& Hit)
{
	ProjectileImpactHitResult(Hit);
}

/*Virtual callback function for overlap collision. Bound to OnComponentBeginOverlap delegate. Holds logic executed when projectiles hit vehicles (applying damage)*/
void ABaseProjectile::OnOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
                                int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	auto ProjectileOwner = GetOwner();
	if(!ProjectileOwner) return;
	if(OtherActor == ProjectileOwner) return;
	ProjectileImpactSweepResult(SweepResult);
}

void ABaseProjectile::BeginPlay()
{
	Super::BeginPlay();
	ProjectileMesh->OnComponentHit.AddDynamic(this, &ABaseProjectile::OnHit);
	ProjectileMesh->OnComponentBeginOverlap.AddDynamic(this, &ABaseProjectile::OnOverlap);

	if(ProjectileVfxNiagaraComponent)
	{
		ProjectileVfxNiagaraComponent->SetAsset(ProjectileVfxNiagaraSystem);
		//ProjectileVfxNiagaraComponent->Deactivate();
	}
}

/*Initializes projectile whizzing audio*/
void ABaseProjectile::InitializeAudio()
{
	if(WhizzingAudioComponent)
	{
		WhizzingAudioComponent->Play();
	}
}

// Called every frame
void ABaseProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
