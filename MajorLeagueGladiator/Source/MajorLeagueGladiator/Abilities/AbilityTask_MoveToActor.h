// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "AbilityTask_MoveToActor.generated.h"

DECLARE_MULTICAST_DELEGATE(FMoveToLocationReached);

UCLASS()
class MAJORLEAGUEGLADIATOR_API AAbilityTask_MoveToActor : public AActor
{
	GENERATED_BODY()
	
public:	
	AAbilityTask_MoveToActor();
	virtual void Tick(float DeltaTime) override;

	UPROPERTY()
	FVector TargetLocation;
	
	UPROPERTY()
	float MoveSpeed;
	
	UPROPERTY()
	ACharacter* MovingCharacter;

	FMoveToLocationReached OnLocationReached;

private:
	float MinDistanceThreshold;
};