// Fill out your copyright notice in the Description page of Project Settings.


#include "HelpMePlayerController.h"
#include "HelpMeGameMode.h"
#include "TaskStation.h"
#include "Interactable.h"
#include "NetworkIDComponent.h"
#include "Kismet/GameplayStatics.h"
#include "HelpMe_Pickup.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"

void AHelpMePlayerController::Multicast_SetPlayerID_Implementation(int ID)
{
	if (ID >= 0 && ID <= 1)
		HelpMePlayerID = ID;
}

void AHelpMePlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	DOREPLIFETIME(AHelpMePlayerController, HelpMePlayerID);
}

void AHelpMePlayerController::InteractWith(AActor* Interactable, bool bIsInteracting)
{
	//const FString Name = FString::FromInt(Interactable->GetUniqueID());
	FString Name = UNetworkIDComponent::GetIDOfActor(Interactable);
	UKismetSystemLibrary::PrintString(this, FString("InteractWithPC") + Name);
	if (!Name.IsEmpty())
		Server_Interact(Name, bIsInteracting, HelpMePlayerID);
}

void AHelpMePlayerController::Server_Interact_Implementation(const FString& Name, bool bIsInteracting, int PlayerID)
{
	UKismetSystemLibrary::PrintString(this, FString("ServerInteract"));
	UNetworkIDComponent* NetComponent = Cast<AHelpMeGameMode>(UGameplayStatics::GetGameMode(this))->GetNetworkComponentFromID(Name);
	AActor* Actor = NetComponent->GetOwner();
	//ATaskStation* Station = Cast<AHelpMeGameMode>(UGameplayStatics::GetGameMode(this))->GetStation(Name);
	
	if(Actor->Implements<UInteractable>())
		IInteractable::Execute_Interact(Actor, bIsInteracting, PlayerID);
}

bool AHelpMePlayerController::Server_Interact_Validate(const FString& Name, bool bIsInteracting, int PlayerID)
{
	return true;
}

void AHelpMePlayerController::Server_PlaceHeldObject_Implementation(const FString& NetworkID, int PlayerID, const FVector& Location)
{
	UKismetSystemLibrary::PrintString(this, FString("ServerPlace"));
	UNetworkIDComponent* NetComponent = Cast<AHelpMeGameMode>(UGameplayStatics::GetGameMode(this))->GetNetworkComponentFromID(NetworkID);
	AActor* Actor = NetComponent->GetOwner();
	Cast<AHelpMe_Pickup>(Actor)->PlaceAt(Location, PlayerID);
}

bool AHelpMePlayerController::Server_PlaceHeldObject_Validate(const FString& NetworkID, int PlayerID, const FVector& Location)
{
	return true;
}