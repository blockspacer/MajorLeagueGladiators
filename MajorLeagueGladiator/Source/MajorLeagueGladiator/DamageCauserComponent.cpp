// Fill out your copyright notice in the Description page of Project Settings.

#include "MajorLeagueGladiator.h"
#include "DamageCauserComponent.h"


UDamageCauserComponent::UDamageCauserComponent()
	: damageAppliedOnHit(0)
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UDamageCauserComponent::BeginPlay()
{
	Super::BeginPlay();
	GetOwner()->OnActorBeginOverlap.AddDynamic(this, &UDamageCauserComponent::OnBeginOverlap);
}

void UDamageCauserComponent::OnBeginOverlap(AActor* OverlappedActor, AActor* OtherActor)
{
	UGameplayStatics::ApplyDamage(OtherActor, damageAppliedOnHit, nullptr, OverlappedActor, damageType);
}