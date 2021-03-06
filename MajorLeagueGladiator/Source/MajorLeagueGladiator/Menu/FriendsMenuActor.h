// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FriendWidget.h"
#include "Online.h"
#include "FriendsMenuActor.generated.h"

class FOnlineFriend;
class UWidgetComponent;

UENUM(BlueprintType)
namespace EFriendsMenuLocation
{
	enum Type
	{
		MainMenu UMETA(DisplayName = "MainMenu"),
		Lobby	 UMETA(DisplayName = "Lobby")
	};
}

UCLASS(Blueprintable)
class MAJORLEAGUEGLADIATOR_API AFriendsMenuActor : public AActor
{
	GENERATED_BODY()
	
public:	
	AFriendsMenuActor(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void BeginPlay() override;
	
protected:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void OnFriendsInfoLoaded(const TArray<FFriendState>& friends);

	void OnFriendsInfoLoaded_Implementation(const TArray<FFriendState>& friends);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UWidgetComponent* widgetComponent;

	UFUNCTION()
	void OnJoinFriendRequest(int32 friendIndex);

	UFUNCTION()
	void OnInviteFriendRequest(int32 friendIndex);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<UFriendWidget*> friendWidgets;

	// Decides wether the invite button is shown or not.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TEnumAsByte<EFriendsMenuLocation::Type> menuLocation;
private:
	void OnFriendlistLoaded(const TArray<TSharedRef<FOnlineFriend>>& friendlist);
	void MockFriendsListLoadedEditor();

	UFUNCTION()
	void OnFriendsRefreshRequested();

	UFUNCTION(BlueprintCallable)
	void BindToFriendWidget(UFriendWidget* Widget);

	UFUNCTION(BlueprintCallable)
	void HideUnusedWidgets(int32 LastUsedIndex);

	// To be called after widgets have been constructed to run dependent inits (e.g. Friend Avatars).
	UFUNCTION(BlueprintCallable)
	void PostWidgetsConstructed();

	void FindFriendSession(TSharedRef<FOnlineFriend> Friend);
	void OnFindFriendSessionComplete(int32 LocalUserNum, bool bWasSuccessful, const TArray<FOnlineSessionSearchResult>& SearchResult);

	FOnFindFriendSessionCompleteDelegate onFindFriendSessionCompleteDelegate;
	FDelegateHandle onFindFriendSessionCompleteDelegateHandle;

	void UpdateAvatarTextureCount(int32 CurrentNumFriends);

	UFUNCTION()
	void OnAvatarDownloaded(int32 FriendIndex);

	UPROPERTY()
	TArray<UTexture2D*> avatarTextures;
};
