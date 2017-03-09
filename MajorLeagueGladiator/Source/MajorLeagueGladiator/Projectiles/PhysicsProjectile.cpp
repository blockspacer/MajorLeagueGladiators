// Fill out your copyright notice in the Description page of Project Settings.

#include "MajorLeagueGladiator.h"
#include "PhysicsProjectile.h"
#include "MlgPlayerController.h"

#include "MlgGameplayStatics.h"

#include "ShieldActor.h" //Replace with interface when ready

APhysicsProjectile::APhysicsProjectile(const FObjectInitializer& ObjectInitializer)
	: projectileMovementComponent(ObjectInitializer.CreateDefaultSubobject<UProjectileMovementComponent>(this, TEXT("ProjectileMovementComponent")))
{
	bReplicates = true;
	bReplicateMovement = true;
	bStaticMeshReplicateMovement = true;
	GetStaticMeshComponent()->SetCollisionProfileName("Projectile");
	GetStaticMeshComponent()->Mobility = EComponentMobility::Movable;

	UPrimitiveComponent* primRoot = CastChecked<UPrimitiveComponent>(RootComponent);

	projectileMovementComponent->InitialSpeed = 1000.f;
	projectileMovementComponent->bShouldBounce = false;
	projectileMovementComponent->ProjectileGravityScale = 0;
}

void APhysicsProjectile::FireProjectile(FVector Location, FVector DirectionVector, AActor* ProjectileOwner, AController* ProjectileInstigator) const
{
	FTransform projectileTransform(DirectionVector.ToOrientationRotator(), Location);
	APhysicsProjectile* spawnedActor = ProjectileOwner->GetWorld()->SpawnActorDeferred<APhysicsProjectile>(GetClass(), projectileTransform, ProjectileOwner, ProjectileInstigator->GetPawn());
	UPrimitiveComponent* spawnedRootComponent = CastChecked<UPrimitiveComponent>(spawnedActor->GetRootComponent());

	//Don't collide with shield again
	spawnedRootComponent->MoveIgnoreActors.Add(ProjectileOwner);
	
	spawnedActor->SetLifeSpan(5.f);

	spawnedActor->projectileMovementComponent->OnProjectileStop.AddDynamic(spawnedActor, &APhysicsProjectile::OnProjectileStop);
	spawnedActor->FinishSpawning(projectileTransform);
}

void APhysicsProjectile::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);

	if (Role < ROLE_Authority || IsIgnoredActor(OtherActor))
	{
		return;
	}

	if (AShieldActor* interactable = Cast<AShieldActor>(OtherActor))
	{
		interactable->OnHitInteractable(this);
	}
	else if (UMlgGameplayStatics::CanDealDamageTo(this, OtherActor))
	{
		DealDamage(OtherActor);
	}	
	//Destroy even if no damage is applied
	Destroy();
}

void APhysicsProjectile::DealDamage(AActor* OtherActor)
{
	FVector travelingDir = GetRootComponent()->GetComponentVelocity().GetSafeNormal();

	UGameplayStatics::ApplyPointDamage(OtherActor, damage, travelingDir, FHitResult{}, Instigator->Controller, this, DamageType);	
}

bool APhysicsProjectile::IsIgnoredActor(const AActor* Actor) const
{
	return GetOwner() == Actor;
}

void APhysicsProjectile::OnProjectileStop(const FHitResult& ImpactResult)
{
	Destroy();
}
