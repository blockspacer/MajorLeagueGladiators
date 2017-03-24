// Fill out your copyright notice in the Description page of Project Settings.

#include "MajorLeagueGladiator.h"
#include "HitscanProjectile.h"
#include "DamageTypes/PlayerDamage.h"
#include "MlgGameplayStatics.h"
#include "ParticleSystemManagerActor.h"
#include "MlgGameState.h"
#include "ShieldActor.h"
#include "CollisionStatics.h"

AHitscanProjectile::AHitscanProjectile()
	: range(1000000.f)
{
	
}

void AHitscanProjectile::FireProjectile(FVector Location, FVector DirectionVector, AActor* ProjectileOwner, AController* ProjectileInstigator) const
{
	if (Role < ROLE_Authority)
	{
		UE_LOG(LogTemp, Warning, TEXT("Fire Projectile should not be called on client"));
		return;
	}

	FHitResult hitresult = Trace(ProjectileOwner->GetWorld(), Location, DirectionVector, { ProjectileOwner });


	AActor* hitActor = hitresult.GetActor();

	UGameplayStatics::SpawnEmitterAtLocation(ProjectileOwner->GetWorld(), beamParticleSystem, FTransform(DirectionVector.Rotation().Quaternion(), Location));

	FTransform transf = FTransform(DirectionVector.Rotation().Quaternion(), Location);
	
	/*UParticleSystemComponent* psc = UGameplayStatics::SpawnEmitterAtLocation(ProjectileOwner->GetWorld(), beamParticleSystem, transf);
	psc->SetIsReplicated(true);*/
	ProjectileOwner->GetWorld()->GetGameState<AMlgGameState>()->GetParticleSystemManager()->CreateParticleSystemMain(beamParticleSystem, transf, true);
	//ProjectileOwner->GetWorld()->GetGameState<AMlgGameState>()->GetParticleSystemManager()->SpawnParticleSystemAtLocation(EParticleSystem::HitscanBeam, transf);


	if (hitActor == nullptr)
	{
		return;
	}

	if (AShieldActor* interactable = Cast<AShieldActor>(hitActor))
	{
		interactable->OnHitInteractable(this);
	}
	else if (UMlgGameplayStatics::CanDealDamageTo(ProjectileInstigator, hitActor))
	{
		UGameplayStatics::ApplyPointDamage(hitActor, damage, DirectionVector, hitresult, ProjectileInstigator, ProjectileOwner, UPlayerDamage::StaticClass());
	}

	
}


FHitResult AHitscanProjectile::Trace(UWorld* world, FVector Location, FVector DirectionVector, const TArray<TWeakObjectPtr<AActor>>& IngnoredActors) const
{
	FVector end = Location + DirectionVector * range;

	FHitResult result;

	FCollisionQueryParams CollisionParams("HitscanShot", true, Instigator);
	CollisionParams.AddIgnoredActors(IngnoredActors);

	ECollisionChannel hitScanChannel = CollisionStatics::GetCollsionChannelByName(CollisionStatics::HITSCAN_TRACE_CHANNEL_NAME);

	world->LineTraceSingleByChannel(result, Location, end, hitScanChannel, CollisionParams);

	//DrawDebugDirectionalArrow(world, Location, end, 100.f, FColor::Purple, true, 2.f);
	//world->LineTraceSingleByObjectType(result, Location, end, queryTypes, CollisionParams);
	
	return result;
}