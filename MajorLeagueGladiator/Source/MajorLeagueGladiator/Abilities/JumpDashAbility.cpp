// Fill out your copyright notice in the Description page of Project Settings.

#include "MajorLeagueGladiator.h"
#include "JumpDashAbility.h"

#include "AbilityTask_WaitTargetData.h"
#include "Abilities/GameplayAbilityTargetActor_Raycast.h"
#include "Abilities/GameplayAbilityTargetActor_Cone.h"

#include "Characters/MlgPlayerCharacter.h"

#include "MlgGameplayStatics.h"

#include "AI/MlgAICharacter.h"

namespace
{
	const char* AIM_SOCKET_NAME = "Aim";
}

UJumpDashAbility::UJumpDashAbility()
	: launchVelocity(0,0,1300)
	, minmalJumpHightBeforeDash(500)
	, dashSpeed(1750)
	, maxDashRange(50'000)
	, effectDistance(400)
	, halfEffectAngleDegrees(45)
	, stunRadius(400)
	, stunTimeSeconds(2.0f)
{
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerExecution;
}

void UJumpDashAbility::InputPressed(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
	if (waitTargetDataTask)
	{
		waitTargetDataTask->ExternalConfirm(true);
		waitTargetDataTask = nullptr;
	}
}

void UJumpDashAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!ActorInfo->AvatarActor.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid Avatar"));
		return;
	}

	cachedCharacter = CastChecked<AMlgPlayerCharacter>(ActorInfo->AvatarActor.Get());
	cachedMoveComp = CastChecked<UCharacterMovementComponent>(cachedCharacter->GetMovementComponent());

	if (cachedMoveComp->MovementMode != MOVE_Walking)
	{
		EndAbility(Handle,ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		UE_LOG(LogTemp, Warning, TEXT("Tried to activate ability with insufficient resources"));
		return;
	}

	cachedCharacter->SetAbilityMoveTargetLocation(cachedCharacter->GetActorLocation());
	cachedCharacter->LaunchCharacter(launchVelocity, true, true);
	cachedCharacter->OnActorHit.AddDynamic(this, &UJumpDashAbility::OnCollidedWithWorld);
	BeginFindingActorsToLaunch();

	const float targetingDelay = minmalJumpHightBeforeDash / launchVelocity.Z;

	FTimerManager& timeManager = cachedCharacter->GetWorldTimerManager();
	timeManager.SetTimer(timerHandle, this, &UJumpDashAbility::BeginTargeting,
		targetingDelay, false);
}

void UJumpDashAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

	if (bWasCancelled)
	{
		return;
	}

	cachedMoveComp->SetMovementMode(MOVE_Falling);
	cachedMoveComp->StopMovementImmediately();

	FTimerManager& timeManager = cachedCharacter->GetWorldTimerManager();
	timeManager.ClearTimer(timerHandle);
	timerHandle.Invalidate();

	cachedCharacter->OnActorHit.RemoveDynamic(this, &UJumpDashAbility::OnCollidedWithWorld);
	
	if (waitTargetDataTask)
	{
		waitTargetDataTask->EndTask();
		waitTargetDataTask = nullptr;
	}

	cachedCharacter->InvalidateAbilityMoveTargetLocation();
}

void UJumpDashAbility::OnCollidedWithWorld(AActor* SelfActor, AActor* OtherActor, FVector NormalImpulse, const FHitResult& Hit)
{	
	BeginFindingEnemiesInArea();
}

void UJumpDashAbility::BeginFindingActorsToLaunch()
{
	UAbilityTask_WaitTargetData* findActorsToLaunchTask = UAbilityTask_WaitTargetData::WaitTargetData(this, "findActorsToLaunchTask",
		EGameplayTargetingConfirmation::Instant, AGameplayAbilityTargetActor_Cone::StaticClass());

	findActorsToLaunchTask->ValidData.AddDynamic(this, &UJumpDashAbility::LaunchFoundActorsUpwards);

	AGameplayAbilityTargetActor* spawnedTargetingActor = nullptr;

	if (!findActorsToLaunchTask->BeginSpawningActor(this, AGameplayAbilityTargetActor_Cone::StaticClass(), spawnedTargetingActor))
	{
		return;
	}

	AGameplayAbilityTargetActor_Cone* targetingConeActor = CastChecked<AGameplayAbilityTargetActor_Cone>(spawnedTargetingActor);
	targetingConeActor->Distance = effectDistance;
	targetingConeActor->HalfAngleDegrees = halfEffectAngleDegrees;
	targetingConeActor->StartLocation = MakeTargetLocationInfoFromOwnerActor();

	findActorsToLaunchTask->FinishSpawningActor(this, spawnedTargetingActor);
}

void UJumpDashAbility::LaunchFoundActorsUpwards(const FGameplayAbilityTargetDataHandle& Data)
{
	const FGameplayAbilityTargetData* targetData = Data.Get(0);
	for (const auto& actor : targetData->GetActors())
	{
		if (actor.IsValid())
		{
			ACharacter* characterToLaunch = CastChecked<ACharacter>(actor.Get());
			if (CanLaunchCharacter(characterToLaunch))
			{
				characterToLaunch->LaunchCharacter(launchVelocity, true, true);
				affectedCharacters.Add(characterToLaunch);				
			}		
		}
	}
}

bool UJumpDashAbility::CanLaunchCharacter(ACharacter* Character) const
{
	return UMlgGameplayStatics::CanDealDamageTo(cachedCharacter, Character);
}

void UJumpDashAbility::BeginTargeting()
{
	waitTargetDataTask = UAbilityTask_WaitTargetData::WaitTargetData(this, "WaitTargetDataTask",
		EGameplayTargetingConfirmation::Custom, AGameplayAbilityTargetActor_Raycast::StaticClass());

	waitTargetDataTask->ValidData.AddDynamic(this, &UJumpDashAbility::OnTargetingSuccess);
	waitTargetDataTask->Cancelled.AddDynamic(this, &UJumpDashAbility::OnTargetingFailed);

	AGameplayAbilityTargetActor* spawnedTargetingActor = nullptr;

	if (!waitTargetDataTask->BeginSpawningActor(this, AGameplayAbilityTargetActor_Raycast::StaticClass(), spawnedTargetingActor))
	{
		return;
	}

	AGameplayAbilityTargetActor_Raycast* targetingRayCastActor = CastChecked<AGameplayAbilityTargetActor_Raycast>(spawnedTargetingActor);
	
	auto gripController = cachedCharacter->GetMotionControllerMesh(EControllerHand::Left);
	targetingRayCastActor->StartLocation.SourceComponent = gripController;
	targetingRayCastActor->StartLocation.LocationType = EGameplayAbilityTargetingLocationType::SocketTransform;
	targetingRayCastActor->StartLocation.SourceSocketName = AIM_SOCKET_NAME;

	targetingRayCastActor->aimDirection = ERaycastTargetDirection::ComponentRotation;
	targetingRayCastActor->IgnoredActors.Add(cachedCharacter);
	targetingRayCastActor->MaxRange = maxDashRange;

	targetingRayCastActor->EvalTargetFunc = [](const FHitResult& result)
	{
		auto isNotVertical = FVector::DotProduct(result.ImpactNormal, FVector(0, 0, 1)) > 0.7f;
		auto hitSurface = result.bBlockingHit > 0;
		return isNotVertical && hitSurface;
	};

	waitTargetDataTask->FinishSpawningActor(this, spawnedTargetingActor);
}

void UJumpDashAbility::OnTargetingSuccess(const FGameplayAbilityTargetDataHandle& Data)
{
	const FVector targetLocation = Data.Data[0]->GetHitResult()->Location;
	const FVector actorFeetLocation = cachedCharacter->CalcFeetPosition();
	const FVector feetToTargetVector = targetLocation - actorFeetLocation;
	const FVector direction = feetToTargetVector.GetUnsafeNormal();
	const FVector zNormalizedDirection = direction / std::abs(direction.Z);
	const FVector velocity = zNormalizedDirection * dashSpeed;
	
	cachedCharacter->SetAbilityMoveTargetLocation(targetLocation);
	BeginDashing(velocity);
}

void UJumpDashAbility::OnTargetingFailed(const FGameplayAbilityTargetDataHandle& Data)
{
	BeginDashing(FVector(0, 0, -dashSpeed));
}

void UJumpDashAbility::BeginDashing(const FVector& Velocity)
{
	cachedMoveComp->StopMovementImmediately();
	cachedMoveComp->SetMovementMode(MOVE_Flying);
	cachedMoveComp->AddImpulse(Velocity, true);
}

void UJumpDashAbility::BeginFindingEnemiesInArea()
{
	UAbilityTask_WaitTargetData* findActorsToStunTask = UAbilityTask_WaitTargetData::WaitTargetData(this, "findActorsToStunTask",
		EGameplayTargetingConfirmation::Instant, AGameplayAbilityTargetActor_Cone::StaticClass());

	findActorsToStunTask->ValidData.AddDynamic(this, &UJumpDashAbility::StunFoundEnemies);

	AGameplayAbilityTargetActor* spawnedTargetingActor = nullptr;

	if (!findActorsToStunTask->BeginSpawningActor(this, AGameplayAbilityTargetActor_Cone::StaticClass(), spawnedTargetingActor))
	{
		return;
	}

	AGameplayAbilityTargetActor_Cone* targetingConeActor = CastChecked<AGameplayAbilityTargetActor_Cone>(spawnedTargetingActor);
	targetingConeActor->Distance = stunRadius;
	targetingConeActor->HalfAngleDegrees = 360; // Target around me. 180 Would be enough, but just in case.
	targetingConeActor->StartLocation = MakeTargetLocationInfoFromOwnerActor();

	findActorsToStunTask->FinishSpawningActor(this, spawnedTargetingActor);

	// TODO: Call Particle Affect
}


void UJumpDashAbility::StunFoundEnemies(const FGameplayAbilityTargetDataHandle& Data)
{
	const FGameplayAbilityTargetData* targetData = Data.Get(0);
	for (const auto& actor : targetData->GetActors())
	{
		if (actor.IsValid())
		{
			if (AMlgAICharacter* characterToStun = Cast<AMlgAICharacter>(actor.Get()))
			{
				characterToStun->ApplyStagger(stunTimeSeconds);
			}
		}
	}
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}
