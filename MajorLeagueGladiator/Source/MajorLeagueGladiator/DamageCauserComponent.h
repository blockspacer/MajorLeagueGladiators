// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/ActorComponent.h"
#include "DamageCauserComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MAJORLEAGUEGLADIATOR_API UDamageCauserComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UDamageCauserComponent();
	virtual void BeginPlay() override;	

	UPROPERTY(EditAnywhere, Category = "Damage")
	TSubclassOf<UDamageType> DamageType;

protected:
	UFUNCTION()
	virtual void OnBeginOverlap(AActor* OverlappedActor, AActor* OtherActor);

	UPROPERTY(EditAnywhere, Category = "Damage")
	float damageAppliedOnHit;

	UPROPERTY(EditAnywhere, Category = "Damage")
	float cooldownBetweenDamage;

	FTimerHandle cooldownTimer;
};
