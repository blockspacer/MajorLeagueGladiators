// Fill out your copyright notice in the Description page of Project Settings.

#include "MajorLeagueGladiator.h"
#include "HitScanGunActor.h"
#include "AmmoComponent.h"
#include "WidgetComponent.h"
#include "TextWidget.h"
#include "ChargeShotComponent.h"
#include "PlayerHudWidget.h"
#include "MlgGameplayStatics.h"
#include "Characters/MlgPlayerCharacter.h"

namespace
{
	const char* NO_COLLISION_PROFILE_NAME = "NoCollision";
	const FName PROJECTILE_SPAWN_SOCKET_NAME("ProjectileSpawn");
}

AHitScanGunActor::AHitScanGunActor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, recoilAnimBackDuration(0.5f)
	, recoilAnimForwardDuration(0.5f)
	, elapsedAnimTime(0.f)
	, recoilOrigin(0.f)
	, recoilDistance(-30.f)
	, MaxGlow(100.f)
	, MinGlow(0.f)
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	SetReplicates(true);

	ConstructorHelpers::FObjectFinder<USoundCue> shotSoundCueFinder(TEXT("SoundCue'/Game/MVRCFPS_Assets/Sounds/hitscan_shot_Cue.hitscan_shot_Cue'"));
	shotSoundCue = shotSoundCueFinder.Object;

	ConstructorHelpers::FObjectFinder<USoundCue> shootEmptyCueFinder (TEXT("SoundCue'/Game/MVRCFPS_Assets/Sounds/hitscan_shot_empty_cue.hitscan_shot_empty_cue'"));
	shootEmptyCue = shootEmptyCueFinder.Object;

	MeshComponent->SetSimulatePhysics(false);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> GunStaticMesh(TEXT("StaticMesh'/Game/MVRCFPS_Assets/MultiTool/gun_mount_static.gun_mount_static'"));
	UStaticMeshComponent* staticMeshComp = Cast<UStaticMeshComponent>(MeshComponent);
	if (GunStaticMesh.Succeeded() && staticMeshComp)
	{
		staticMeshComp->SetStaticMesh(GunStaticMesh.Object);
	}

	laserMesh = ObjectInitializer.CreateDefaultSubobject<UStaticMeshComponent>(this, TEXT("LaserMeshComponent"));
	laserMesh->SetupAttachment(MeshComponent, PROJECTILE_SPAWN_SOCKET_NAME);
	laserMesh->SetCollisionProfileName(NO_COLLISION_PROFILE_NAME);

	sceneCapture = ObjectInitializer.CreateDefaultSubobject<USceneCaptureComponent2D>(this, TEXT("SceneCapture"));
	sceneCapture->SetupAttachment(GetRootComponent());

	scopeMesh = ObjectInitializer.CreateDefaultSubobject<UStaticMeshComponent>(this, TEXT("ScopeMesh"));
	scopeMesh->SetIsReplicated(true);
	scopeMesh->SetupAttachment(GetRootComponent(), FName("UI"));
	scopeMesh->SetCollisionProfileName(NO_COLLISION_PROFILE_NAME);
	
	ammoComponent = ObjectInitializer.CreateDefaultSubobject<UAmmoComponent>(this, TEXT("AmmoComponent"));

	ammoCountWidget = ObjectInitializer.CreateDefaultSubobject<UWidgetComponent>(this, TEXT("AmmoCounterWidget"));
	ammoCountWidget->SetupAttachment(GetRootComponent(), FName(TEXT("UI")));
	ammoCountWidget->SetIsReplicated(true);
	ammoCountWidget->SetCollisionProfileName(NO_COLLISION_PROFILE_NAME);

	chargeShotHud = ObjectInitializer.CreateDefaultSubobject<UWidgetComponent>(this, TEXT("ChargeShotHUD"));
	chargeShotHud->SetupAttachment(GetRootComponent(), FName(TEXT("UI")));
	chargeShotHud->SetIsReplicated(true);
	chargeShotHud->SetCollisionProfileName(NO_COLLISION_PROFILE_NAME);

	chargeShot = ObjectInitializer.CreateDefaultSubobject<UChargeShotComponent>(this, TEXT("ChargeShot"));
	chargeShot->SetupAttachment(MeshComponent, PROJECTILE_SPAWN_SOCKET_NAME);
} 

void AHitScanGunActor::BeginPlay()
{
	Super::BeginPlay();
			
	if (!(MeshComponent->DoesSocketExist(PROJECTILE_SPAWN_SOCKET_NAME)))
	{
		UE_LOG(DebugLog, Warning, TEXT("HitscanGun Mesh does not have a 'ProjectileSpawn' socket, shots will originate from incorrect position"));
	}	

	if (!sceneCapture->TextureTarget)
	{
		sceneCapture->TextureTarget = UCanvasRenderTarget2D::CreateCanvasRenderTarget2D(GetWorld(), UCanvasRenderTarget2D::StaticClass(),512,512);
		auto instance = UMaterialInstanceDynamic::Create(scopeMesh->GetMaterial(0), scopeMesh);
		instance->SetTextureParameterValue(FName("ScopeTex"), sceneCapture->TextureTarget);
		scopeMesh->SetMaterial(0, instance);
	}

	textWidget = CastChecked<UTextWidget>(ammoCountWidget->GetUserWidgetObject());
	textWidget->SetText(FString::FromInt(ammoComponent->GetAmmoCount()));

	ammoComponent->OnAmmoChanged.AddLambda([this](int32 newCount)
	{
		textWidget->SetText(FString::FromInt(newCount));
	});

	chargeWidget = CastChecked<UPlayerHudWidget>(chargeShotHud->GetUserWidgetObject());
	chargeWidget->SetCurrentPercentage(1.0f, 1.0f);

	laserMeshMaterial = laserMesh->CreateAndSetMaterialInstanceDynamic(0);
	gunMeshMaterial = MeshComponent->CreateAndSetMaterialInstanceDynamic(0);

	chargeShot->OnChargeValueChangedPercentage.AddLambda([this](float newValue)
	{
		chargeWidget->SetCurrentPercentage(newValue, newValue);

		float currentGlow = FMath::Lerp(MinGlow, MaxGlow, newValue);
		gunMeshMaterial->SetScalarParameterValue(FName("Glow"), currentGlow);
		laserMeshMaterial->SetScalarParameterValue(FName("Glow"), currentGlow);
	});
}

void AHitScanGunActor::OnUsed_Implementation()
{
	if (bIsApplyingRecoil) // Gun hasn't reset yet.
		return;

	if (ammoComponent->GetAmmoCount() <= 0)
	{
		playEmptyEffect();
		return;
	}
	
	ammoComponent->ConsumeAmmo();
	chargeShot->FireHitscanShot();

	playShotEffect_NetMulticast(chargeShot->GetValuePercentage());

	AMlgPlayerCharacter* player = CastChecked<AMlgPlayerCharacter>(GetOwner());
	player->PlayRumbleRight();
}

void AHitScanGunActor::playEmptyEffect()
{
	FSoundParams soundParams(shootEmptyCue, GetActorLocation());
	UMlgGameplayStatics::PlaySoundNetworkedPredicted(Cast<APawn>(GetOwner()), soundParams);
}

void AHitScanGunActor::playShotEffect_NetMulticast_Implementation(float Charge)
{
	currentAnimDuration = recoilAnimBackDuration;
	adjustedRecoilDistance = recoilDistance * Charge;
	bIsApplyingRecoil = true;

	FSoundParams soundParams(shotSoundCue, GetActorLocation());
	UMlgGameplayStatics::PlaySoundNetworkedPredicted(Cast<APawn>(GetOwner()), soundParams);
}

void AHitScanGunActor::OnGrip_Implementation(UGripMotionControllerComponent* GrippingController, const FBPActorGripInformation& GripInformation) 
{
	//Super::OnGrip(GrippingController, GripInformation);

	grippingController = GrippingController;
	gripInfo = GripInformation;
}

void AHitScanGunActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsApplyingRecoil)
	{
		elapsedAnimTime += DeltaTime;

		FVector recoil = FMath::Lerp(FVector(0, recoilOrigin, 0), FVector(0, adjustedRecoilDistance, 0), elapsedAnimTime / currentAnimDuration);

		// NOTE(Phil): Temporarily disabled, since the attachments are now different.
		//EBPVRResultSwitch result;
		//FTransform addTrafo;
		//addTrafo.AddToTranslation(recoil);
		//grippingController->SetGripAdditionTransform(gripInfo, result, addTrafo, true);

		if (elapsedAnimTime >= currentAnimDuration)
		{
			elapsedAnimTime = 0.f;

			if (adjustedRecoilDistance < 0.f) // Gun finished moving back
			{
				recoilOrigin = adjustedRecoilDistance;
				adjustedRecoilDistance = 0.f;
				currentAnimDuration = recoilAnimForwardDuration;
			}
			else // Gun is back at original position 
			{
				recoilOrigin = 0.f;
				bIsApplyingRecoil = false;
			}
		}
	}
}






