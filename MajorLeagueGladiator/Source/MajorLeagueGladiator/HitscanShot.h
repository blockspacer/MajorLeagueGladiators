// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "MlgProjectile.h"

#include "HitscanShot.generated.h"

UCLASS()
class MAJORLEAGUEGLADIATOR_API AHitscanShot : public AMlgProjectile
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AHitscanShot();

	virtual void FireProjectile(FVector Location, FVector DirectionVector, AActor* ProjectileOwner, AController* ProjectileInstigator) const override;

	FHitResult Trace(UWorld* world, FVector Location, FVector DirectionVector, const TArray<TWeakObjectPtr<AActor>>& IngnoredActors) const;

private:
	float range;
};