// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/Subsystem.h"
#include "Delegates/Delegate.h"
#include "GameFramework/GameStateBase.h"
#include "HelpMeGameState.generated.h"


UENUM(BlueprintType, meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class ETaskType : uint8
{
	TT_NULL				= 0 UMETA(Hidden),
	TT_KITCHENSINK		= 1 << 0 UMETA(DisplayName = "Kitchen Sink"),
	TT_CELLARPUMP		= 1 << 1 UMETA(DisplayName = "Cellar Pump"),
	TT_BATHROOMVALVE	= 1 << 2 UMETA(DisplayName = "Bathroom Valve"),
	TT_FINAL			= 1 << 3 UMETA(DisplayName = "Final")
};
ENUM_CLASS_FLAGS(ETaskType);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTaskCompletionChange, ETaskType, Task, bool, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGameComplete);

/**
 * 
 */
UCLASS()
class GP4TEAM10_API AHelpMeGameState : public AGameStateBase
{
public:

	GENERATED_BODY()

	UPROPERTY(BlueprintAssignable)
	FOnTaskCompletionChange OnTaskCompletionChange;

	UPROPERTY(BlueprintAssignable)
	FOnGameComplete OnGameComplete;

	UPROPERTY(EditAnywhere, ReplicatedUsing = OnRep_CompletedTaskFlags, BlueprintReadWrite, Category = "Gameplay", meta = (Bitmask, BitmaskEnum = "/Script/GP4Team10.ETaskType"))
	uint8 CompletedTaskFlags;

	//This function is called whenever CompletedTaskFlags replicates
	UFUNCTION()
	void OnRep_CompletedTaskFlags();

	UFUNCTION()
	int NumberOfCompletedTasks();

	UFUNCTION(BlueprintCallable)
	void ChangeTaskCompleted(ETaskType Type, bool bNewStatus);

	UFUNCTION(BlueprintCallable)
	bool IsTaskCompleted(ETaskType Type);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_UpdateTaskState(ETaskType Type, bool bNewStatus);
	
};
