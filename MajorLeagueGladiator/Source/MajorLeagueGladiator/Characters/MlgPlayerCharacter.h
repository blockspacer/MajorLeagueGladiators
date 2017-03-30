#pragma once

#include "VRExpansion/VRSimpleCharacter.h"
#include "HandMotionController.h"
#include "ChaperoneBounds.h"
#include "AbilitySystemInterface.h"
#include "MlgPlayerCharacter.generated.h"

class UHealthComponent;
class AMlgPlayerController;
class UWidgetComponent;
class UDamageReceiverComponent;
class UAbilitySystemComponent;
class UGameplayAbilitySet;
class UMlgAbilitySet;
class UVRControllerComponent;
class AMlgGrippableStaticMeshActor;
class USteamVRChaperoneComponent;

UCLASS()
class MAJORLEAGUEGLADIATOR_API AMlgPlayerCharacter : public AVRSimpleCharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()
public:
	AMlgPlayerCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void PossessedBy(AController* NewController) override;

	void MoveForward(float Value);
	void MoveRight(float Value);
	void OnLeftTriggerClicked();
	void OnLeftTriggerReleased();
	void OnRightTriggerClicked();
	void OnRightTriggerReleased();
	void OnSideGripButtonLeft();
	void OnSideGripButtonRight();

	UFUNCTION(NetMulticast, reliable)
	void EnableActorCollison_NetMulticast(bool bNewActorEnableCollision);

	UFUNCTION(NetMulticast, reliable)
	void LaunchCharacter_NetMulticast(FVector LaunchVelocity, bool bXYOverride, bool bZOverride);

	UFUNCTION(NetMulticast, reliable)
	void StopMovementImmediately_NetMulticast();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UStaticMeshComponent* GetMotionControllerMesh(EControllerHand Hand);
	UVRControllerComponent* GetMotionController(EControllerHand Hand);

	FVector CalcFeetPosition() const;

protected:
	UPROPERTY(EditAnywhere, Category = "Mesh")
	UStaticMeshComponent* leftMesh;

	UPROPERTY(EditAnywhere, Category = "Mesh")
	UStaticMeshComponent* rightMesh;

	UPROPERTY(EditAnywhere, Category = "Mesh")
	UMaterialInterface* multiToolMaterial;

private:
	std::unique_ptr<HandMotionController> pHandMotionController;
	std::unique_ptr<ChaperoneBounds> pChaperoneBounds;

	UPROPERTY(EditAnywhere)
	USteamVRChaperoneComponent* chaperone;

	UPROPERTY(EditAnywhere)
	UHealthComponent* healthComp;

	UPROPERTY(EditAnywhere)
	UDamageReceiverComponent* dmgReceiverComp;

	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* bodyMesh;

	UPROPERTY(EditAnywhere)
	TSubclassOf<AMlgGrippableStaticMeshActor> startWeaponClass;

	UPROPERTY(Transient, ReplicatedUsing = OnRep_AttachedWeapon)
	AMlgGrippableStaticMeshActor* attachedWeapon;

	UPROPERTY(EditAnywhere)
	UWidgetComponent* hudHealth;

	UPROPERTY(EditAnywhere)
	UAbilitySystemComponent* abilitySystemComponent;

	UPROPERTY(EditAnywhere)
	TAssetSubclassOf<UMlgAbilitySet> abilitySetClass;

	UPROPERTY(Transient)
	const UMlgAbilitySet* cachedAbilitySet;

	const UMlgAbilitySet* GetOrLoadAbilitySet();

	UFUNCTION(Server, WithValidation, reliable)
	void leftHandGrab_Server();

	UFUNCTION(Server, WithValidation, reliable)
	void leftHandRelease_Server();

	UFUNCTION(Server, WithValidation, reliable)
	void leftHandDrop_Server();

	UFUNCTION(Server, WithValidation, reliable)
	void rightHandGrab_Server();

	UFUNCTION(Server, WithValidation, reliable)
	void rightHandRelease_Server();

	UFUNCTION(Server, WithValidation, reliable)
	void rightHandDrop_Server();

	void SpawnWeapon();

	UFUNCTION()
	void OnRep_AttachedWeapon();

	void OnAttachedWeaponSet();

	virtual void BecomeViewTarget(APlayerController* PC) override;

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const { return abilitySystemComponent; }
	
	UFUNCTION()
	void OnLand(const FHitResult& hit);
};
