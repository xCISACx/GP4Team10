// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interactable.h"
#include "TaskStation.generated.h"

UENUM()
enum class ETaskState : uint8
{
	TS_NULL			UMETA(Hidden),
	TS_None			UMETA(DisplayName = "None"),
	TS_Preparing	UMETA(DisplayName = "Preparing"),
	TS_Prepared		UMETA(DisplayName = "Prepared"),
	TS_Performing	UMETA(DisplayName = "Performing"),
	TS_Performed	UMETA(DisplayName = "Performed"),
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInteract, int, PlayerID, bool, bIsInteracting);

class UStaticMeshComponent;
class AHelpMeGameState;
class UAudioComponent;
class USoundBase;
class UNetworkIDComponent;

UCLASS()
class GP4TEAM10_API ATaskStation : public AActor, public IInteractable
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATaskStation();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
	void SetOwningPlayerID(int PlayerID) { OwnerID = PlayerID; }

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SetOwningPlayer(int PlayerID);

	UFUNCTION(BlueprintCallable)
	int GetOwningPlayerID() { return OwnerID; }

	UFUNCTION(BlueprintCallable)
	ETaskState GetCurrentState() { return CurrentState; }

	//IInteractable virtual functions
	bool IsInteractableBy_Implementation(int PlayerID) override;
	void Interact_Implementation(bool bIsInteracting, int PlayerID) override;
	float GetProgress_Implementation() override;
	bool GetHoveredOver_Implementation() override;

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_Interact(bool bIsInteracting, int PlayerID);

	UFUNCTION()
		UNetworkIDComponent* GetNetworkIDComponent() {
		return NetworkIDComponent;
	}

	UPROPERTY(BlueprintAssignable)
	FOnInteract OnInteractServerOnly;


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION()
	void SetState(ETaskState NewState);

	UPROPERTY(VisibleAnywhere, Replicated)
	int OwnerID;

	UPROPERTY(VisibleAnywhere, Replicated)
	ETaskState CurrentState;

	UPROPERTY(EditAnywhere)
	bool bFirstPlayerIsOwner = true;

	//The time it takes to prepare a task
	UPROPERTY(EditAnywhere)
	float TaskPrepareTime = 5.0f;

	//The time it takes to perform a task
	UPROPERTY(EditAnywhere)
	float TaskPerformTime = 10.0f;

	UPROPERTY(EditAnywhere)
	float TaskResetTime = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = -1.0f, ClampMax = 1.0f))
	float TaskPerformScore = 0.25f;

	//Here we store the time that the current interaction has lasted. 
	//Once it reaches TaskPrepTime or TaskPerformTime the task prepares/completes
	UPROPERTY(VisibleAnywhere, Replicated)
	float CurrentInteractDuration;

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UStaticMeshComponent> OwnerMeshComponent;

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UStaticMeshComponent> NonOwnerMeshComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UAudioComponent> AudioComponent;

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayAudio(USoundBase* Sound);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<USoundBase> PreparingSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<USoundBase> PreparedSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<USoundBase> PerformingSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<USoundBase> PerformedSound;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UNetworkIDComponent> NetworkIDComponent;

};
