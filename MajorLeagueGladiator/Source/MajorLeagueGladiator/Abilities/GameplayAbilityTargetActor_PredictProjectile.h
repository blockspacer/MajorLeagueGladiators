#pragma once 

#include "GameplayAbilityTargetActor.h"
#include "GameplayAbilityTargetActor_PredictProjectile.generated.h"

UENUM(BlueprintType)
namespace EPickMoveLocationTargeting
{
	enum Type
	{
		FromController				UMETA(DisplayName = "FromController"),
		FromPlayerCapsule			UMETA(DisplayName = "FromPlayerCapsule"),
		FromSourceComponent			UMETA(DisplayName = "FromSourceComponent") // Requires you to set the SourceComponent manually.
	};
}

class AMlgPlayerCharacter;
class UVRControllerComponent;
class UTargetingSplineMeshComponent;
class UPlayAreaMeshComponent;

UCLASS()
class MAJORLEAGUEGLADIATOR_API AGameplayAbilityTargetActor_PredictProjectile : public AGameplayAbilityTargetActor
{
	GENERATED_BODY()

public:
	AGameplayAbilityTargetActor_PredictProjectile(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	virtual void Tick(float DeltaSeconds) override;
	virtual bool IsConfirmTargetingAllowed() override;
	virtual void StartTargeting(UGameplayAbility* Ability) override;
	virtual void ConfirmTargetingAndContinue() override;
	
	float TargetProjectileSpeed;
	float TargetProjectileFlightTime;
	
	TArray<AActor*> IgnoredActors;
	
	EPickMoveLocationTargeting::Type targetingType;

	void SetTargetHitColor(FLinearColor Color);
	void SetTargetMissColor(FLinearColor Color);
	
	static FVector GetVelocityFromTargetDataHandle(const FGameplayAbilityTargetDataHandle& Data);

	UPROPERTY(EditAnywhere)
	UTargetingSplineMeshComponent* TargetingSplineMesh;

	UPROPERTY(EditAnywhere)
	UPlayAreaMeshComponent* PlayAreaMesh;

	float OverrideGravityZ;

	bool bShowPlayArea;
	
private:
	FGameplayAbilityTargetDataHandle makeDataHandle();
	
	void GetPlayerCapsuleFromAbility(AMlgPlayerCharacter* owner);
	void GetVrControllerFromAbility(AMlgPlayerCharacter* owner);
	bool PickTarget();
	
	UPROPERTY(Transient)
	UCapsuleComponent* playerCapsule;

	UPROPERTY(Transient)
	UVRControllerComponent* vrController;
	
	FPredictProjectilePathResult predictResult;
	FVector launchVelocity;
};
