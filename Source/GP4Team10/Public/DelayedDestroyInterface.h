// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "DelayedDestroyInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI, BlueprintType)
class UDelayedDestroyInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class GP4TEAM10_API IDelayedDestroyInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void DestroyWithDelay(float Duration);
	
};
