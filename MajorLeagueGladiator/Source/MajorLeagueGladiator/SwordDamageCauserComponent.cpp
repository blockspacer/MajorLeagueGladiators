// Fill out your copyright notice in the Description page of Project Settings.

#include "MajorLeagueGladiator.h"
#include "SwordDamageCauserComponent.h"
#include "MlgGrippableStaticMeshActor.h"
#include "MlgPlayerController.h"

USwordDamageCauserComponent::USwordDamageCauserComponent()
	: oldSwingSpeed(FVector::ZeroVector)
	, threshholdDoDamageSquared(16)
	, slashVelocityLearnRate(0.1f)
	, canDealDamage(false)
{
	PrimaryComponentTick.bCanEverTick = true;

	static ConstructorHelpers::FObjectFinder<UMaterial> SwordMaterialRed(TEXT("Material'/Game/MVRCFPS_Assets/Blade_HeroSword22/M_Blade_HeroSword_Red.M_Blade_HeroSword_Red'"));

	if (SwordMaterialRed.Object != NULL)
	{
		materialRedObject = CastChecked<UMaterial>(SwordMaterialRed.Object);
	}
	else 
	{
		UE_LOG(LogTemp, Warning, TEXT("M_Blade_HeroSword_Red not found."));
	}
	
	static ConstructorHelpers::FObjectFinder<UMaterial> SwordMaterial(TEXT("Material'/Game/MVRCFPS_Assets/Blade_HeroSword22/M_Blade_HeroSword22.M_Blade_HeroSword22'"));

	if (SwordMaterial.Object != NULL)
	{
		materialObject = CastChecked<UMaterial>(SwordMaterial.Object);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("M_Blade_HeroSword22 not found."));
	}
}

void USwordDamageCauserComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	FVector newSwordState = GetOwner()->GetVelocity();
	oldSwingSpeed = oldSwingSpeed * (1.0f - slashVelocityLearnRate) + (DeltaTime * newSwordState) * slashVelocityLearnRate;

	bool newCanDealDamage = oldSwingSpeed.SizeSquared() > threshholdDoDamageSquared;

	if (newCanDealDamage && !canDealDamage)
	{
		onStartSlash();
	}
	else if (!newCanDealDamage && canDealDamage)
	{
		onEndSlash();
	}

	canDealDamage = newCanDealDamage;
}

void USwordDamageCauserComponent::onStartSlash()
{
	damageAllOverlappingActors();

	if (materialRedObject_Dyn == nullptr)
	{
		materialRedObject_Dyn = UMaterialInstanceDynamic::Create(materialRedObject, this);
	}

	setMaterialOfOwnerMesh(materialRedObject_Dyn);
}

void USwordDamageCauserComponent::onEndSlash()
{
	if (!materialObject_Dyn)
	{
		materialObject_Dyn = UMaterialInstanceDynamic::Create(materialObject, this);
	}

	setMaterialOfOwnerMesh(materialObject_Dyn);
}

void USwordDamageCauserComponent::damageAllOverlappingActors()
{
	TArray <AActor*> overlappingActors;
	AActor* owner = GetOwner();
	owner->GetOverlappingActors(overlappingActors);

	int32 overlaps = overlappingActors.Num();

	if (overlaps != 0 && (overlaps > 1 || overlappingActors[0] != owner)) // Either more than one or the one is not us
	{
		doRumbleRight(GetOwner());
	}

	for (AActor* actor : overlappingActors)
	{
		UGameplayStatics::ApplyDamage(actor, damageAppliedOnHit, owner->GetInstigatorController(), owner, damageType);
	}
}

void USwordDamageCauserComponent::setMaterialOfOwnerMesh(UMaterialInstanceDynamic* material_Dyn) 
{
	AMlgGrippableStaticMeshActor* owner = Cast<AMlgGrippableStaticMeshActor>(GetOwner());
	
	if (owner != nullptr)
	{
		auto* mesh = owner->GetStaticMeshComponent();
		mesh->SetMaterial(0, material_Dyn);
	}
}

void USwordDamageCauserComponent::OnBeginOverlap(AActor* OverlappedActor, AActor* OtherActor)
{
	if (canDealDamage)
	{
		UGameplayStatics::ApplyDamage(OtherActor, damageAppliedOnHit, OverlappedActor->GetInstigatorController(), OverlappedActor, damageType);
		doRumbleRight(OtherActor);
	}
}

void USwordDamageCauserComponent::doRumbleRight(AActor* OtherActor)
{
	AMlgGrippableStaticMeshActor* owner = Cast<AMlgGrippableStaticMeshActor>(GetOwner());
	if (owner != nullptr)
	{
		AMlgPlayerController* controller = owner->GetMlgPlayerController();
		if (controller != nullptr)
		{
			//check if hit object is part of myself, then we do not rumble
			APawn* otherPlayer = Cast<APawn>(OtherActor);
			if (otherPlayer != nullptr)
			{
				if (otherPlayer->Controller == controller)
				{
					return;
				}
			}
			controller->ClientPlayForceFeedback(controller->GetRumbleShortRight(), false, FName("rumbleRight"));
		}
	}
}