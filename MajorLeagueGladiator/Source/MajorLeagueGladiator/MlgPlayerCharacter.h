#pragma once

#include "VRExpansion/VRSimpleCharacter.h"
#include "HandMotionController.h"
#include "AbilitySystemInterface.h"
#include "MlgPlayerCharacter.generated.h"

class UTeleportComponent;
class UHealthComponent;
class AMlgPlayerController;
class UWidgetComponent;
class UDamageReceiverComponent;
class UAbilitySystemComponent;
class UGameplayAbilitySet;
class UMlgAbilitySet;

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
	void OnTeleportPressedLeft();
	void OnTeleportPressedRight();
	void OnTeleportReleased();
	void OnSideGripButtonLeft();
	void OnSideGripButtonRight();

private:
	std::unique_ptr<HandMotionController> pHandMotionController;

	UPROPERTY(EditAnywhere, Category = "Mesh")
	UStaticMeshComponent* leftMesh;

	UPROPERTY(EditAnywhere, Category = "Mesh")
	UStaticMeshComponent* rightMesh;

	UPROPERTY(EditAnywhere)
	UTeleportComponent* teleportComp;

	UPROPERTY(EditAnywhere)
	UHealthComponent* healthComp;

	UPROPERTY(EditAnywhere)
	UDamageReceiverComponent* dmgReceiverComp;

	UPROPERTY(EditAnywhere)
	USphereComponent* leftGrabSphere;

	UPROPERTY(EditAnywhere)
	USphereComponent* rightGrabSphere;

	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* bodyMesh;

	UPROPERTY(EditAnywhere)
	TSubclassOf<AActor> healthTriggerClass;

	UPROPERTY(EditAnywhere)
	UWidgetComponent* hudHealth;

	UPROPERTY(EditAnywhere)
	UWidgetComponent* hudTeleportCD;

	UPROPERTY(EditAnywhere)
	UAbilitySystemComponent* abilitySystemComponent;

	UPROPERTY(EditAnywhere)
	TAssetSubclassOf<UMlgAbilitySet> abilitySetClass;

	//Store ability Set reference
	UPROPERTY(Transient)
	const UMlgAbilitySet* abilitySet;

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

	UFUNCTION(Server, WithValidation, reliable)
	void requestTeleport_Server(FVector Location, FRotator Rotation);
	
	UFUNCTION(NetMulticast, reliable)
	void performTeleport_NetMulticast(FVector Location, FRotator Rotation);

	virtual void BecomeViewTarget(APlayerController* PC) override;

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const { return abilitySystemComponent; }
};
