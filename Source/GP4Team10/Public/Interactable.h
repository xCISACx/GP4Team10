// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Interactable.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI, BlueprintType)
class UInteractable : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class GP4TEAM10_API IInteractable
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool IsInteractableBy(int PlayerID); // 0 or 1

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void Interact(bool bIsInteracting, int PlayerID);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void Interact_Blueprint(bool bIsInteracting, int PlayerID);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	float GetProgress();

	bool bHoveredOver = false;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool GetHoveredOver();

};
