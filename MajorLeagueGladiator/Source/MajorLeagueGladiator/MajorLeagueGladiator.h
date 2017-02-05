// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine.h"

// PROTOTYPE
#include "VRExpansion/VRBPDatatypes.h"
#include "VRExpansion/GripMotionControllerComponent.h"
#include "VRExpansion/VRExpansionFunctionLibrary.h"
#include "VRExpansion/ReplicatedVRCameraComponent.h"
#include "VRExpansion/ParentRelativeAttachmentComponent.h"
#include "VRExpansion/VRRootComponent.h"
#include "VRExpansion/VRCharacterMovementComponent.h"
#include "VRExpansion/VRCharacter.h"
#include "VRExpansion/VRPathFollowingComponent.h"
#include "VRExpansion/VRPlayerController.h"
#include "VRExpansion/VRGripInterface.h"

// UNREAL
#include "PhysXIncludes.h"
#include "UnrealNetwork.h"
#include "MessageEndpointBuilder.h"
#include "Runtime/Launch/Resources/Version.h"
#include "Runtime/Engine/Private/EnginePrivate.h"
#include "Runtime/Engine/Classes/Kismet/KismetMathLibrary.h"
#include "Runtime/UMG/Public/UMG.h"
#include "Runtime/UMG/Public/UMGStyle.h"
#include "Runtime/UMG/Public/Slate/SObjectWidget.h"
#include "Runtime/UMG/Public/IUMGModule.h"
#include "Runtime/UMG/Public/Blueprint/UserWidget.h"

// STL
#include <algorithm>
#include <memory>

// LOG
DECLARE_LOG_CATEGORY_EXTERN(DebugLog, Log, All);