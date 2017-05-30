// Fill out your copyright notice in the Description page of Project Settings.

#include "MajorLeagueGladiator.h"
#include "PreGameGameMode.h"

#include "Characters/MlgPlayerCharacter.h"
#include "MlgPlayerState.h"

namespace 
{
	const FString GAME_MAP("/Game/ScaleRef?game=/Game/BluePrints/MlgGameModeBP");
}

APreGameGameMode::APreGameGameMode(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bUseSeamlessTravel = true;

	GameStateClass = AMlgGameState::StaticClass();

	ConstructorHelpers::FClassFinder<AMlgPlayerCharacter> ranged(TEXT("/Game/BluePrints/Characters/RangedPlayerCharacterBP"));
	dpsClass = ranged.Class;

	ConstructorHelpers::FClassFinder<AMlgPlayerCharacter> melee(TEXT("/Game/BluePrints/Characters/MeleePlayerCharacterBP"));
	tankClass = melee.Class;
}

UClass* APreGameGameMode::GetDefaultPawnClassForController_Implementation(AController* InController)
{
	// Handle only Player Controllers for now, as AI is spawned differently
	check(InController->IsPlayerController());

	// For now this just assigns the Server the DPS class and the Client the Tank Class.
	// Game Mode is only on the Server. If the controller is locally controlled it is the
	// server's controller
	if (InController->IsLocalPlayerController())
	{
		if (dpsClass.Get() == nullptr)
		{
			UE_LOG(DebugLog, Warning, TEXT("DPS Class not set. Using Default Pawn Class"));
			return Super::GetDefaultPawnClassForController_Implementation(InController);
		}

		return dpsClass.Get();
	}
	else
	{
		if (tankClass.Get() == nullptr)
		{
			UE_LOG(DebugLog, Warning, TEXT("Tank Class not set. Using Default Pawn Class"));
			return Super::GetDefaultPawnClassForController_Implementation(InController);
		}

		return tankClass.Get();
	}
}

void APreGameGameMode::BeginPlay()
{
	GetWorld()->GetTimerManager().SetTimer(timerHandle, this, &APreGameGameMode::StartGame, 5.f);
}

void APreGameGameMode::StartGame()
{
	if (NumPlayers >= 2)
	{
		GetWorld()->GetTimerManager().ClearTimer(timerHandle);
		GetWorld()->ServerTravel(GAME_MAP, true);
	}
}

void APreGameGameMode::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
	Super::HandleStartingNewPlayer_Implementation(NewPlayer);
	if (NumPlayers >= 2)
	{
		GetWorld()->GetTimerManager().SetTimer(timerHandle, this, &APreGameGameMode::StartGame, 10.f, true);
	}
}
