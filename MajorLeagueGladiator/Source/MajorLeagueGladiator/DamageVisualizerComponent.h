// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/ActorComponent.h"
#include "DamageVisualizerComponent.generated.h"

// On Damage, instead of setting class members, add another FDamageVisual
// In Tick, process them all
// In OnPointDamage, check if we have a SkeletalMesh, if yes, use the skinned submesh

USTRUCT()
struct FDamageVisual
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, Category="DamageVisual")
	UParticleSystemComponent* ParticleSystem;

	UPROPERTY(EditAnywhere, Category = "DamageVisual")
	UMeshComponent* AffectedMesh;

	UPROPERTY(EditAnywhere, Category = "DamageVisual")
	UMaterialInstanceDynamic* MatInstance;

	UPROPERTY(EditAnywhere, Category = "DamageVisual")
	FName DamageValParamName;

	UPROPERTY(EditAnywhere, Category = "DamageVisual")
	float MatVisDuration;
	
	UPROPERTY(EditAnywhere, Category = "DamageVisual")
	float CurrentMatVisDuration;

	FDamageVisual()
		: ParticleSystem(nullptr)
		, AffectedMesh(nullptr)
		, MatInstance(nullptr)
		, DamageValParamName(FName("DamageValue"))
		, MatVisDuration(0.3f)
		, CurrentMatVisDuration(0.f)
	{}
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MAJORLEAGUEGLADIATOR_API UDamageVisualizerComponent : public UActorComponent
{
	GENERATED_BODY()


public:	
	UDamageVisualizerComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION()
	void onPointDamageReceived(AActor* DamagedActor, const FVector& HitLocation, const FVector& OriginDirection);

	UFUNCTION()
	void onDamageReceived(AActor* DamagedActor, const UDamageType* DamageType);

	UFUNCTION()
	void onPointDamageReceived(AActor* DamagedActor, const FVector& HitLocation, const FVector& OriginDirection, const UDamageType* DamageType);
	
	UFUNCTION(NetMulticast, reliable)
	void AddVisual_NetMulticast(UMeshComponent* affectedMesh, bool bSpawnParticles, const FTransform& particleTrafo = FTransform());

private:
	TArray<FDamageVisual> visuals;

	UPROPERTY(EditAnywhere, Category= "Damage Visualizer")
	UParticleSystem* particleTemplate;
};
