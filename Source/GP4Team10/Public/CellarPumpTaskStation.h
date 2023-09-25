// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interactable.h"
#include "CellarPumpTaskStation.generated.h"

class UStaticMeshComponent;
class UNetworkIDComponent;
class UAudioComponent;
class USoundBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTaskProgressChange, float, NewProgress, float, OldProgress);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSuccessfulUseBy, int, PumpID);

UCLASS()
class GP4TEAM10_API ACellarPumpTaskStation : public AActor, public IInteractable
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACellarPumpTaskStation();
	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	bool IsInteractableBy_Implementation(int PlayerID) override;
	void Interact_Implementation(bool bIsInteracting, int PlayerID) override;
	float GetProgress_Implementation() override;
	bool GetHoveredOver_Implementation() override;

	UFUNCTION(BlueprintCallable)
	UNetworkIDComponent* GetNetworkIDComponent() { return NetworkIDComponent; }

	UFUNCTION(BlueprintCallable)
	void DoMonsterInterference(float ProgressRemoved);

	UPROPERTY(BlueprintAssignable)
	FOnTaskProgressChange OnTaskProgressChange;

	UPROPERTY(BlueprintAssignable)
	FOnSuccessfulUseBy OnSuccessfulUseBy;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UNetworkIDComponent> NetworkIDComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UStaticMeshComponent> PlayerOnePumpOneMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UStaticMeshComponent> PlayerTwoPumpOneMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UStaticMeshComponent> PlayerOnePumpTwoMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UStaticMeshComponent> PlayerTwoPumpTwoMesh;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PumpUseCooldown = 3.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float TimeSincePumpOneUse = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float TimeSincePumpTwoUse = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ProgressOnUse = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DecayMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DecayFrequency = 0.1f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float TimeSinceDecay = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float CurrentProgress = 0.0f;

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_ResetPumpUseTime(int PumpID);

	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_BroadcastProgressChange(float NewProgress, float OldProgress);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UAudioComponent> PumpOneAudioComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UAudioComponent> PumpTwoAudioComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UAudioComponent> MainPumpAudioComponent;

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayAudio(USoundBase* Sound, UAudioComponent* Source);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<USoundBase> SinglePumpSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<USoundBase> PumpResetSound;

	UPROPERTY()
	bool bShouldPlayResetSoundPumpOne = false;

	UPROPERTY()
	bool bShouldPlayResetSoundPumpTwo = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<USoundBase> TaskCompleteSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<USoundBase> TaskOngoingSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<USoundBase> MonsterInterferenceSound;

};
