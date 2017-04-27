// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "DamageFeedbackComponent.h"
#include "MeleeDamageFeedbackComponent.generated.h"

UCLASS(Blueprintable, meta = (BlueprintSpawnableComponent))
class MAJORLEAGUEGLADIATOR_API UMeleeDamageFeedbackComponent : public UDamageFeedbackComponent
{
	GENERATED_BODY()
	
public:

	UMeleeDamageFeedbackComponent();

	UFUNCTION(NetMulticast, reliable)
	virtual void DoParticleSystemVisualization_NetMulticast(const FVector& HitLocation, const FVector& OriginDirection, TSubclassOf<UDamageType> DamageType = nullptr) override;

	UFUNCTION(NetMulticast, reliable)
	virtual void DoWeakpointParticleSystemVisualization_NetMulticast(const FVector& HitLocation, const FVector& OriginDirection, TSubclassOf<UDamageType> DamageType = nullptr) override;

private:

	/*
		Reasoning: although these member variables are CURRENTLY the same as in the melee class,
		the spawn behaviour can be different. Furthermore, some may not be needed. Do not move
		those in the common parent base class. They E.g. the smoke emitted from a hitscan
		projectile hit rises up, the sparks emitted from a sword slash jump in the direction of
		the sword.
	*/

	UPROPERTY(EditAnywhere, Category = "Feedback (Melee)")
	TArray<UParticleSystem*> hitscanDamageParticleSystems;

	UPROPERTY(EditAnywhere, Category = "Feedback (Melee)")
	TArray<UParticleSystem*> swordDamageParticleSystems;

	UPROPERTY(EditAnywhere, Category = "Feedback (Melee)")
	TArray<UParticleSystem*> hitscanWeakpointDamageParticleSystems;

	UPROPERTY(EditAnywhere, Category = "Feedback (Melee)")
	TArray<UParticleSystem*> swordWeakpointDamageParticleSystems;

	UPROPERTY(EditAnywhere, Category = "Feedback (Melee)")
	TArray<USoundBase*> hitscanSounds;

	UPROPERTY(EditAnywhere, Category = "Feedback (Melee)")
	TArray<USoundBase*> swordSounds;
	
};
