// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "UsableItem.h"
#include "HealthPack.generated.h"

UCLASS()
class MAJORLEAGUEGLADIATOR_API AHealthPack : public AUsableItem
{
	GENERATED_BODY()
	
public:
	AHealthPack();
	
	UFUNCTION()
	virtual void Use(AActor* User, TriggerType Type) override;

private:
	UPROPERTY(EditAnywhere)
	float amountToRefill;
};
