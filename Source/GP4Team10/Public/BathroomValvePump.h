// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interactable.h"
#include "BathroomValvePump.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLeakFixableStateChange, bool, bCanFixLeaks);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFlowValueChange, float, NewFlow);

class UStaticMeshComponent;
class UNetworkIDComponent;
class UAudioComponent;
class USoundBase;

UCLASS()
class GP4TEAM10_API ABathroomValvePump : public AActor, public IInteractable
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABathroomValvePump();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	bool IsInteractableBy_Implementation(int PlayerID) override;
	void Interact_Implementation(bool bIsInteracting, int PlayerID) override;
	float GetProgress_Implementation() override;
	bool GetHoveredOver_Implementation() override;

	UPROPERTY(BlueprintAssignable)
	FOnLeakFixableStateChange OnLeakFixableStateChange;

	UPROPERTY(BlueprintAssignable)
	FOnFlowValueChange OnFlowValueChange;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TObjectPtr<UStaticMeshComponent> PumpMeshPlayerOne;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TObjectPtr<UStaticMeshComponent> PumpMeshPlayerTwo;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UNetworkIDComponent> NetworkIDComponent;

	UPROPERTY(VisibleAnywhere, ReplicatedUsing=OnRep_CurrentFlow, BlueprintReadOnly)
	float CurrentFlow = 0.0f;

	//This function is called whenever CurrentFlow replicates
	UFUNCTION()
	void OnRep_CurrentFlow();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FlowPerPump = 0.15f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FlowDecayPerSecond = 0.05f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FlowDecayFrequency = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector2D GoodFlowRange = FVector2D(0.4f, 0.8f);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float TimeSinceLastDecay;

	UPROPERTY()
	bool bLastLeakFixableBroadcast = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UAudioComponent> PumpAudioComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UAudioComponent> OngoingAudioComponent;

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayAudio(USoundBase* Sound, UAudioComponent* Source);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<USoundBase> SinglePumpSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<USoundBase> OngoingGoodFlowSound;

};
