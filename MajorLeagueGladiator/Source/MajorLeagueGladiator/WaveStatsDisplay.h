// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "WaveStatsDisplay.generated.h"

class UWidgetComponent;

UCLASS()
class MAJORLEAGUEGLADIATOR_API AWaveStatsDisplay : public AActor
{
	GENERATED_BODY()
	
public:	
	AWaveStatsDisplay(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void BeginPlay() override;	

public:	

	UFUNCTION(BlueprintNativeEvent)
	void UpdateWaveNumber(int NewWaveCount);

	UFUNCTION(BlueprintNativeEvent)
	void UpdateEnemyCount(int NewEnemyCount);
private:

	void OnWaveNumberChanged(int NewWaveCount, int OldWaveNumber);
	void OnEnemyCountChanged(int NewCount, int OldCount);	
};
