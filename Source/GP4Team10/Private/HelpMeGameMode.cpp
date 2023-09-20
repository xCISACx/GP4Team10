// Fill out your copyright notice in the Description page of Project Settings.


#include "HelpMeGameMode.h"
#include "OwnershipSetupComponent.h"
#include "TaskStation.h"
#include "HelpMePlayerController.h"
#include "HelpMeCharacter.h"
#include "GameFramework/PlayerStart.h"
#include "NetworkIDComponent.h"
#include "EngineUtils.h"
#include "HelpMeGameState.h"
#include "Kismet/KismetSystemLibrary.h"

void AHelpMeGameMode::RegisterOwnershipComponent(UOwnershipSetupComponent* Component, bool bFirstPlayerIsOwner)
{
	if (bFirstPlayerIsOwner)
	{
		FirstPlayerRegisteredComponents.AddUnique(Component);
	}
	else 
	{
		SecondPlayerRegisteredComponents.AddUnique(Component);
	}
}

void AHelpMeGameMode::RegisterTaskStation(ATaskStation* Station, bool bFirstPlayerIsOwner)
{
	if (bFirstPlayerIsOwner)
	{
		FirstPlayerTaskStations.AddUnique(Station);
	}
	else
	{
		SecondPlayerTaskStations.AddUnique(Station);
	}
	StationNames.Add(UNetworkIDComponent::GetIDOfActor(Station), Station);
}

bool AHelpMeGameMode::AssignOwnershipToRegisteredComponents()
{
	if (PlayerOne == nullptr || PlayerTwo == nullptr) return false;

	for (UOwnershipSetupComponent* Component : FirstPlayerRegisteredComponents)
	{
		Component->GetOwner()->SetOwner(PlayerOne->GetPawn());
	}
	for (UOwnershipSetupComponent* Component : SecondPlayerRegisteredComponents)
	{		
		Component->GetOwner()->SetOwner(PlayerTwo->GetPawn());
	}
	for (ATaskStation* Station : FirstPlayerTaskStations)
	{
		Station->SetOwner(PlayerOne->GetPawn());
		Station->Multicast_SetOwningPlayer(0);
	}
	for (ATaskStation* Station : SecondPlayerTaskStations)
	{
		Station->SetOwner(PlayerTwo->GetPawn());
		Station->Multicast_SetOwningPlayer(1);
	}

	return true;
}

void AHelpMeGameMode::OnPostLogin(AController* NewPlayer)
{
	//First player to log in is assigned to PlayerOne
	//Second player as PlayerTwo, which also starts the game.
	AHelpMePlayerController* CastNewPlayer = Cast<AHelpMePlayerController>(NewPlayer);
	if (CastNewPlayer == nullptr) return;
	if (PlayerOne == nullptr)
	{
		PlayerOne = CastNewPlayer;
		CastNewPlayer->Multicast_SetPlayerID(0);
	}
	else if (PlayerTwo == nullptr)
	{
		PlayerTwo = CastNewPlayer;
		CastNewPlayer->Multicast_SetPlayerID(1);
		FTimerHandle TimerHandle;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &AHelpMeGameMode::StartGame, 1.0f, false);
	}
}

void AHelpMeGameMode::StartGame()
{
	AssignOwnershipToRegisteredComponents();

	/*
	FTimerHandle MotivationDrainTimerHandle;
	// Set a timer with a lambda function
	GetWorldTimerManager().SetTimer(MotivationDrainTimerHandle, [this]()
	{
		LoseMotivation(MotivationDrainAmount);
	}, MotivationDrainRate, true);*/

	Cast<AHelpMeCharacter>(PlayerOne->GetCharacter())->Client_StartGame();
	Cast<AHelpMeCharacter>(PlayerTwo->GetCharacter())->Client_StartGame();

	OnGameStart.Broadcast();

}

AActor* AHelpMeGameMode::ChoosePlayerStart_Implementation(AController* Player)
{
	AActor* ChosenSpawn = nullptr;

	for (TActorIterator<APlayerStart> It(GetWorld()); It; ++It)
	{
		APlayerStart* PlayerStart = *It;
		if (PlayerStart->PlayerStartTag.ToString().Contains(FString::FromInt(NextPlayerNumberToSpawn)))
		{
			ChosenSpawn = PlayerStart;
			break;
		}
	}
	NextPlayerNumberToSpawn++;


	if (!ChosenSpawn)
		ChosenSpawn = Super::ChoosePlayerStart_Implementation(Player);

	return ChosenSpawn;
}

void AHelpMeGameMode::RegisterNetworkIDComponent(UNetworkIDComponent* Component)
{
	NetworkIDComponents.Add(Component->GetID(), Component);
}

UNetworkIDComponent* AHelpMeGameMode::GetNetworkComponentFromID(FString ID)
{
	if (NetworkIDComponents.Contains(ID))
	{
		return NetworkIDComponents[ID];
	}
	return nullptr;
}