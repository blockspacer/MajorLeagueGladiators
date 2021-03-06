#include "MajorLeagueGladiator.h"
#include "ActorSpawn.h"

AActorSpawn::AActorSpawn(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, spawnRadius(0.0f)
	, minSpawnTime(0.2f)
	, maxSpawnTime(5.0f)
{	
	SetRootComponent(ObjectInitializer.CreateDefaultSubobject<USceneComponent>(this, TEXT("RootComponet")));
}

void AActorSpawn::SetRespawnTimer(float IntervalInSeconds)
{
	if (Role < ROLE_Authority)
	{
		return;
	}

	FTimerManager& timer = GetWorldTimerManager();
	timer.SetTimer(spawnTimerHandle, this, &AActorSpawn::spawnActor, IntervalInSeconds, false);
}

void AActorSpawn::BeginPlay()
{
	Super::BeginPlay();
	randomStream.Initialize(randomSeed == 0 ? FMath::Rand() : randomSeed);
	SetRespawnTimer(generateRandomSpawnTime());
}

void AActorSpawn::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	for (AActor* actor : spawnedActors)
	{
		actor->OnEndPlay.RemoveDynamic(this, &AActorSpawn::OnSpawnedActorEndPlay);
	}

	spawnedActors.Empty();
}

bool AActorSpawn::HasSpawnLimit() const
{
	return maxSpawnActors != 0;
}

bool AActorSpawn::IsAllowedToSpawn() const
{
	return spawnedActors.Num() < maxSpawnActors || !HasSpawnLimit();
}

void AActorSpawn::spawnActor()
{
	SetRespawnTimer(generateRandomSpawnTime());

	if (!IsAllowedToSpawn())
	{
		return;
	}

	UWorld* world = GetWorld();		
	
	AActor* actor = world->SpawnActor<AActor>(actorToSpawn,
		generateRandomSpawnLocation(),
		generateRandomRotator()
		);

	if (!actor)
	{
		return;
	}

	spawnedActors.Add(actor);
	actor->OnEndPlay.AddDynamic(this, &AActorSpawn::OnSpawnedActorEndPlay);

	actor->SetReplicates(true);
	actor->SetReplicateMovement(true);
}

float AActorSpawn::generateRandomSpawnTime()
{
	return randomStream.FRandRange(minSpawnTime, maxSpawnTime);
}

void AActorSpawn::OnSpawnedActorEndPlay(AActor* Actor, EEndPlayReason::Type EndPlayReason)
{
	Actor->OnEndPlay.RemoveDynamic(this, &AActorSpawn::OnSpawnedActorEndPlay);
	spawnedActors.RemoveSingleSwap(Actor);
}

FRotator AActorSpawn::generateRandomRotator()
{
	FRotator rotation = randomStream.GetUnitVector().Rotation();
	rotation.Pitch = 0;
	rotation.Roll = 0;
	return rotation;
}

FVector AActorSpawn::generateRandomSpawnLocation()
{
	const float horizontalDistance = spawnRadius * randomStream.GetFraction();
	FVector randVec = randomStream.GetUnitVector() * horizontalDistance;
	randVec.Z = 0;

	return RootComponent->GetComponentLocation() + randVec;		
}