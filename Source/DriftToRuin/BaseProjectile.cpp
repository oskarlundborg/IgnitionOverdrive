// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseProjectile.h"

#include "NiagaraComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"

ABaseProjectile::ABaseProjectile()
{
	PrimaryActorTick.bCanEverTick = true;
	
	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Projectile Mesh"));
	SetRootComponent(ProjectileMesh);

	ProjectileVfxNiagaraComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("ProjectileNiagaraComponent"));
	ProjectileVfxNiagaraComponent->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform, TEXT("VFX"));
	
	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Projectile Movement Component"));
	ProjectileMovementComponent->MaxSpeed = 8000.f;
	ProjectileMovementComponent->InitialSpeed = 8000.f;
}

UProjectileMovementComponent* ABaseProjectile::GetProjectileMovementComponent()
{
	return ProjectileMovementComponent;
}

void ABaseProjectile::OnOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
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

// Called every frame
void ABaseProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
