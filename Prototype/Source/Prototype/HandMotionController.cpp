#include "Prototype.h"
#include "HandMotionController.h"
#include "PlayerCharacter.h"

HandMotionController::HandMotionController(APlayerCharacter* playerCharacter)
{
	IModularFeatures::Get().RegisterModularFeature(IMotionController::GetModularFeatureName(), this);
	playerChar = playerCharacter;
}

HandMotionController::~HandMotionController()
{
	IModularFeatures::Get().UnregisterModularFeature(IMotionController::GetModularFeatureName(), this);
}

bool HandMotionController::GetControllerOrientationAndPosition(int32 controllerIndex, EControllerHand deviceHand, FRotator& outOrientation, FVector& outPosition) const
{
	if (deviceHand != EControllerHand::Left && deviceHand != EControllerHand::Right)
	{
		UE_LOG(LogTemp, Error, TEXT("HandMotionController was queried with neither left or right"));
		return false;
	}

	auto trafo = playerChar->GetActorTransform();

	FVector offset(0, 25, -75);
	offset.Y *= deviceHand == EControllerHand::Right ? -1.f : 1.f;
	trafo.AddToTranslation(offset);

	outPosition = trafo.GetLocation();
	outOrientation = trafo.Rotator();

	return true;
}

ETrackingStatus HandMotionController::GetControllerTrackingStatus(int32 controllerIndex, EControllerHand deviceHand) const
{
	return ETrackingStatus::Tracked;
}

