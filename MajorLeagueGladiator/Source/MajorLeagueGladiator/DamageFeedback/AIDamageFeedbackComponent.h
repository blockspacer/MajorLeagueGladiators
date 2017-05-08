// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "DamageFeedbackComponent.h"
#include "AIDamageFeedbackComponent.generated.h"


UCLASS(Blueprintable, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class MAJORLEAGUEGLADIATOR_API UAIDamageFeedbackComponent : public UDamageFeedbackComponent
{
	GENERATED_BODY()
public:

	UAIDamageFeedbackComponent();

	UFUNCTION(NetMulticast, reliable)
	virtual void DoParticleSystemVisualization_NetMulticast(const FVector& HitLocation, const FVector& OriginDirection, TSubclassOf<UDamageType> DamageType = nullptr) override;

	UFUNCTION(NetMulticast, reliable)
	virtual void DoWeakpointParticleSystemVisualization_NetMulticast(const FVector& HitLocation, const FVector& OriginDirection, TSubclassOf<UDamageType> DamageType = nullptr) override;
	
	UFUNCTION(NetMulticast, reliable)
	virtual void PlaySound_NetMulticast(const FVector& location, const FVector& OriginDirection, TSubclassOf<UDamageType> DamageType = nullptr) override;
	
protected:

	UPROPERTY(EditAnywhere, Category = "Feedback (AI)")
	TArray<UParticleSystem*> hitscanDamageParticleSystems;

	UPROPERTY(EditAnywhere, Category = "Feedback (AI)")
	TArray<UParticleSystem*> swordDamageParticleSystems;

	UPROPERTY(EditAnywhere, Category = "Feedback (AI)")
	TArray<UParticleSystem*> hitscanWeakpointDamageParticleSystems;

	UPROPERTY(EditAnywhere, Category = "Feedback (AI)")
	TArray<UParticleSystem*> swordWeakpointDamageParticleSystems;

	UPROPERTY(EditAnywhere, Category = "Feedback (AI)")
	TArray<USoundBase*> hitscanSounds;

	UPROPERTY(EditAnywhere, Category = "Feedback (AI)")
	TArray<USoundBase*> swordSounds;

	void playSwordHitSound(const FVector& location);
};
