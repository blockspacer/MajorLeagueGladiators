// Fill out your copyright notice in the Description page of Project Settings.

#include "MajorLeagueGladiator.h"
#include "VRControllerComponent.h"
#include "MlgPlayerController.h"

#include "CollisionStatics.h"

#include "PackMovementComponent.h"
#include "Characters/MlgPlayerCharacter.h"
#include "BasePack.h"

#include "MlgGrippableActor.h"


namespace
{
	const FName PACK_SOCKET_NAME("Pack");
}

UVRControllerComponent::UVRControllerComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, grabRadius(32)
{}


bool UVRControllerComponent::GrabNearestActor()
{	
	if (GrippedObjects.Num() > 0)
		return false;

	auto grabData = getNearestGrabableActor();
	
	if (!grabData.pActorToGrip)
	{
		return false;
	}

	GrabActorImpl(grabData);
	
	return true;
}

void UVRControllerComponent::DropAllGrips()
{
	for (int i = GrippedObjects.Num()-1; i >= 0; --i)
	{
		DropGrip(GrippedObjects[i], true);
	}
}

void UVRControllerComponent::DropNonInteractGrips()
{
	for (int i = GrippedObjects.Num() - 1; i >= 0; --i)
	{
		IVRGripInterface* vrGrip;
		vrGrip = Cast<IVRGripInterface>(CastChecked<AActor>(GrippedObjects[i].GrippedObject));

		if (!vrGrip) 
		{
			vrGrip = Cast<IVRGripInterface>(CastChecked<AActor>(GrippedObjects[i].GrippedObject)->GetRootComponent());
		}

		if (!vrGrip->IsInteractible_Implementation())
		{
			DropGrip(GrippedObjects[i], true);
		}
	}
}

UVRControllerComponent::ActorGrabData UVRControllerComponent::getNearestGrabableActor() const
{
	TArray<FHitResult> hitresults;

	ECollisionChannel gripScanChannel = CollisionStatics::GetCollsionChannelByName(CollisionStatics::GRIPSCAN_TRACE_CHANNEL_NAME);

	GetWorld()->SweepMultiByChannel(hitresults, GetComponentLocation(), GetComponentLocation(),
		FQuat::Identity, gripScanChannel, FCollisionShape::MakeSphere(grabRadius));

	hitresults = hitresults.FilterByPredicate([](const FHitResult& hitresult)
	{
		AActor* actor = hitresult.GetActor();
		// Covers two cases: We have an Actor whose RootComponent is of IVRGripInterface or we have an Actor
		// who is derived from another grippable Actor, such as AGrippableStaticMeshActor.
		return actor && (Cast<IVRGripInterface>(actor->GetRootComponent()) != nullptr || Cast<IVRGripInterface>(actor) != nullptr);
	});

	if (hitresults.Num() == 0)
	{
		return {};
	}

	const FHitResult& closest = std::min(*hitresults.GetData(), hitresults.Last(), [this](const FHitResult& a, const FHitResult& b)
	{
		AActor* actorA = a.GetActor();
		AActor* actorB = b.GetActor();
		return FVector::DistSquared(actorA->GetRootComponent()->GetComponentLocation(), GetComponentLocation())
			 < FVector::DistSquared(actorB->GetRootComponent()->GetComponentLocation(), GetComponentLocation());
	});

	ActorGrabData ret;
	ret.pActorToGrip = closest.GetActor();

	ret.pIVRGrip = Cast<IVRGripInterface>(ret.pActorToGrip->GetRootComponent());
	if (!ret.pIVRGrip)
	{
		ret.pIVRGrip = Cast<IVRGripInterface>(ret.pActorToGrip);
	}
	
	return ret;
}

void UVRControllerComponent::UseGrippedActors()
{
	for (auto& grip : GrippedObjects)
	{
		auto gripActor = Cast<IVRGripInterface>(CastChecked<AActor>(grip.GrippedObject));

		if (gripActor)
		{
			gripActor->OnUsed();
		}
	}
	for (auto& grip : LocallyGrippedObjects)
	{
		auto gripActor = Cast<IVRGripInterface>(CastChecked<AActor>(grip.GrippedObject));

		if (gripActor)
		{
			gripActor->OnUsed();
		}
	}
}

void UVRControllerComponent::EndUseGrippedActors()
{
	for (auto& grip : GrippedObjects)
	{
		auto gripActor = Cast<IVRGripInterface>(CastChecked<AActor>(grip.GrippedObject));

		if (gripActor)
		{
			gripActor->OnEndUsed();
		}
	}
	for (auto& grip : LocallyGrippedObjects)
	{
		auto gripActor = Cast<IVRGripInterface>(CastChecked<AActor>(grip.GrippedObject));

		if (gripActor)
		{
			gripActor->OnEndUsed();
		}
	}
}

bool UVRControllerComponent::HasGrip() const
{
	return GrippedObjects.Num() > 0 || LocallyGrippedObjects.Num() > 0;
}

AMlgGrippableActor* UVRControllerComponent::GetGrippedActor() const
{
	if (GrippedObjects.Num() == 0)
	{
		return nullptr;
	}

	AActor* grippedActor = CastChecked<AActor>(GrippedObjects[0].GrippedObject);
	return  CastChecked<AMlgGrippableActor>(grippedActor);
}

AMlgPlayerController* UVRControllerComponent::GetMlgPlayerController()
{
	AActor* owner = GetOwner();
	APawn* ownerPawn = CastChecked<APawn>(owner);
	AController* controller = ownerPawn->GetController();
	return CastChecked<AMlgPlayerController>(controller,ECastCheckedType::NullAllowed);
}


bool UVRControllerComponent::TryGrabActor(AActor* Actor)
{
	if (GrippedObjects.Num() > 0)
		return false;

	IVRGripInterface* gripInterface = Cast<IVRGripInterface>(Actor->GetRootComponent());
	if (!gripInterface)
	{
		gripInterface = Cast<IVRGripInterface>(Actor);
		if (gripInterface == nullptr)
		{
			return false;
		}
	}

	GrabActorImpl({ Actor, gripInterface });
	return true;
}

/*
NOTE(Phil)
We now try to find a primary socket in range for gripping first. If we dont find a
socket, we simply attach the Actor as it is at the time of the overlap.
VRExpansion supports gripping sockets, however, the sockets need to be
named either VRGripP (Primary) or VRGripS(Secondary).
IVRGripInterface implementers can be queried using Closest[Primary|Secondary]SlotinRange.
*/
void UVRControllerComponent::GrabActorImpl(ActorGrabData GrabData)
{
	check(GrabData.pActorToGrip);
	check(GrabData.pIVRGrip);
	check(GrippedObjects.Num() == 0);

	if (auto* moveComp = GrabData.pActorToGrip->FindComponentByClass<UPackMovementComponent>())
	{
		moveComp->StopSimulating();
	}

	bool foundSlot;
	bool bFindPrimarySlot = true;
	FTransform slotTrafo;


	IVRGripInterface::Execute_ClosestGripSlotInRange(GrabData.pActorToGrip, GetComponentLocation(), bFindPrimarySlot, foundSlot, slotTrafo, nullptr, NAME_None);
	FTransform actorTransform = GrabData.pActorToGrip->GetTransform();

	if (foundSlot)
	{
		slotTrafo = UKismetMathLibrary::ConvertTransformToRelative(slotTrafo, actorTransform);
		slotTrafo.SetScale3D(actorTransform.GetScale3D());


		GripActor(GrabData.pActorToGrip, slotTrafo, true, NAME_None, IVRGripInterface::Execute_GetPrimaryGripType(GrabData.pActorToGrip, foundSlot));
	}
	else
	{
		const FVector gripLocation = GrabData.pActorToGrip->IsA(ABasePack::StaticClass())
			? GetChildSocketTransform(PACK_SOCKET_NAME).GetLocation()
			: GetComponentLocation();

		const FVector actorLocation = actorTransform.GetLocation();
		const FVector actorCenter = GrabData.pActorToGrip->GetComponentsBoundingBox(false).GetCenter();
		FVector locationToCenterOffset = actorCenter - actorLocation;

		actorTransform.SetLocation(gripLocation - locationToCenterOffset);
		GripActor(GrabData.pActorToGrip, actorTransform, false, NAME_None, IVRGripInterface::Execute_GetPrimaryGripType(GrabData.pActorToGrip, foundSlot));
	}
}

bool UVRControllerComponent::LaunchActor(FVector Velocity, bool IgnoreWeight)
{
	if(GrippedObjects.Num() == 0)
	{
		UE_LOG(DebugLog, Warning, TEXT("Tried to launch Actor but not actor was present"));
		return false;
	}

	AMlgGrippableActor* grippedActor = GetGrippedActor();
	UPrimitiveComponent* rootComp = CastChecked<UPrimitiveComponent>(grippedActor->GetRootComponent());

	//Temporarily disable Collision with this Actor so I don't shoot against myself
	rootComp->MoveIgnoreActors.Add(GetOwner());

	FTimerManager& timer = GetWorld()->GetTimerManager();
	FTimerHandle timerhandle;

	const float TimeUntilClearMoveIgnore = 0.5f;
	timer.SetTimer(timerhandle, rootComp, &UPrimitiveComponent::ClearMoveIgnoreActors, TimeUntilClearMoveIgnore, false);

	auto& grippedActorInfo = GrippedObjects[0];

	DropGrip(grippedActorInfo, grippedActor->SimulateOnDrop());
	rootComp->SetPhysicsLinearVelocity(FVector::ZeroVector);

	if (UPackMovementComponent* moveComp = grippedActor->FindComponentByClass<UPackMovementComponent>())
	{
		moveComp->SetVelocity(Velocity);
	}
	else
	{
		rootComp->AddImpulse(Velocity, NAME_None, IgnoreWeight);
	}
	
	return true;
}

FTransform UVRControllerComponent::GetChildSocketTransform(FName SocketName) const
{
	for (const auto& childComp : GetAttachChildren())
	{
		if (childComp->DoesSocketExist(SocketName))
		{
			return childComp->GetSocketTransform(PACK_SOCKET_NAME);
		}		
	}
	UE_LOG(DebugLog, Warning, TEXT("UVRControllerComponent::GetChildSocketTransform: socket with name \"%s\" not found"), *PACK_SOCKET_NAME.ToString());
	return GetComponentTransform();
}
