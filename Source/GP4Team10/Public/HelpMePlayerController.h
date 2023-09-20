// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Interactable.h"
#include "HelpMePlayerController.generated.h"


/**
 * 
 */
UCLASS()
class GP4TEAM10_API AHelpMePlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:

	UFUNCTION(BlueprintCallable)
	int GetPlayerID() { return HelpMePlayerID; }
	
	UFUNCTION(Client, Reliable)
	void Multicast_SetPlayerID(int ID);

	UFUNCTION()
	void InteractWith(AActor* Interactable, bool bIsInteracting);
	
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_Interact(const FString& Name, bool bIsInteracting, int PlayerID);
	
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_PlaceHeldObject(const FString& NetworkID, int PlayerID, const FVector& Location);

protected:
	UPROPERTY(VisibleAnywhere, Replicated)
	int HelpMePlayerID;
};
