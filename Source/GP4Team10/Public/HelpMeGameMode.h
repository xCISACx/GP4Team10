// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Delegates/Delegate.h"
#include "HelpMeGameMode.generated.h"

class UOwnershipSetupComponent;
class ATaskStation;
class UNetworkIDComponent;
class AHelpMePlayerController;
class APlayerStart;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGameStart);

/**
 * 
 */
UCLASS()
class GP4TEAM10_API AHelpMeGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintAssignable)
	FOnGameStart OnGameStart;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	float MotivationDrainRate = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	float MotivationDrainAmount = 0.005f;
	
	UFUNCTION(BlueprintCallable)
	void RegisterOwnershipComponent(UOwnershipSetupComponent* Component, bool bFirstPlayerIsOwner);

	UFUNCTION()
	void RegisterTaskStation(ATaskStation* Station, bool bFirstPlayerIsOwner);

	UFUNCTION()
	ATaskStation* GetStation(FString Name) { return StationNames[Name]; }

	UFUNCTION()
	void RegisterNetworkIDComponent(UNetworkIDComponent* Component);

	UFUNCTION()
	UNetworkIDComponent* GetNetworkComponentFromID(FString ID);

	UFUNCTION()
	AHelpMePlayerController* GetControllerFromID(int PlayerID)
	{
		return PlayerID == 0 ? PlayerOne : PlayerTwo;
	}

protected:

	UPROPERTY()
	TObjectPtr<AHelpMePlayerController> PlayerOne;

	UPROPERTY()
	TObjectPtr<AHelpMePlayerController> PlayerTwo;

	UPROPERTY()
	int NextPlayerNumberToSpawn = 1;

	UPROPERTY()
	TArray<UOwnershipSetupComponent*> FirstPlayerRegisteredComponents;

	UPROPERTY()
	TArray<UOwnershipSetupComponent*> SecondPlayerRegisteredComponents;

	UPROPERTY()
	TArray<ATaskStation*> FirstPlayerTaskStations;

	UPROPERTY()
	TArray<ATaskStation*> SecondPlayerTaskStations;

	UPROPERTY()
	TMap<FString, ATaskStation*> StationNames;

	UPROPERTY()
	TMap<FString, UNetworkIDComponent*> NetworkIDComponents;

	UFUNCTION()
	void StartGame();
	
	//This is called when the game starts, and registers ownership to the correct Pawn
	//for each component that called the RegisterOwnershipComponent function prior to game start.
	UFUNCTION()
	bool AssignOwnershipToRegisteredComponents();

	//This one is called when a player logs in, called by built-in Unreal stuff.
	void OnPostLogin(AController* NewPlayer) override;
	void LoseMotivation(float Value);

	AActor* ChoosePlayerStart_Implementation(AController* Player) override;
};
