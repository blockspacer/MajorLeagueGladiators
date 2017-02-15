// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "MlgGrippableStaticMeshActor.h"
#include "HitScanGunActor.generated.h"

class UGripMotionControllerComponent;
class UAmmoComponent;
class UTextWidget;
class UWidgetComponent;
struct FBPActorGripInformation;

UCLASS()
class MAJORLEAGUEGLADIATOR_API AHitScanGunActor : public AMlgGrippableStaticMeshActor
{
	GENERATED_BODY()
	
public:
	AHitScanGunActor(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void OnUsed() override;
	virtual void OnGrip(UGripMotionControllerComponent* GrippingController, const FBPActorGripInformation& GripInformation) override;
	
private:
	void shoot();

	UFUNCTION(NetMulticast, Reliable)
	void playShotEffect_NetMulticast();

	//UPROPERTY(EditAnywhere) // Removed until we upgrade to 4.15
	//UChildActorComponent* boltAction;

	UPROPERTY(EditAnywhere)
	USceneCaptureComponent2D* sceneCapture;

	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* scopeMesh;

	UPROPERTY(EditAnywhere)
	UAudioComponent* shotAudioComponent;

	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* laserMesh;

	UPROPERTY(EditAnywhere)
	UAmmoComponent* ammoComponent;

	UPROPERTY(EditAnywhere)
	UWidgetComponent* ammoCountWidget;
	
	UTextWidget* textWidget;

	UPROPERTY(EditAnywhere)
	float shotRange;

	UPROPERTY(EditAnywhere)
	float damage;

	UPROPERTY(EditAnywhere)
	float recoilAnimBackDuration;

	UPROPERTY(EditAnywhere)
	float recoilAnimForwardDuration;

	float currentAnimDuration;

	float elapsedAnimTime;

	float recoilOrigin;

	// How far the gun should move back after firing.
	UPROPERTY(EditAnywhere)
	float recoilDistance;

	UStaticMeshSocket* shotOriginSocket;
	
	UGripMotionControllerComponent* grippingController;
	
	FBPActorGripInformation gripInfo;

	bool bApplyingRecoil;
};
