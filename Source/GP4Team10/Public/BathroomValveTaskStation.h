// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interactable.h"
#include "BathroomValveTaskStation.generated.h"

class USceneComponent;
class UNiagaraSystem;
class UNiagaraComponent;
class UNetworkIDComponent;
class ABathroomValvePump;
class UAudioComponent;
class USoundBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLeakStateChange, bool, bNewState);

UCLASS()
class GP4TEAM10_API ABathroomValveTaskStation : public AActor, public IInteractable
{
	GENERATED_BODY()
	
public:	
	ABathroomValveTaskStation();
	virtual void Tick(float DeltaTime) override;

	//Interactable interface overrides
	bool IsInteractableBy_Implementation(int PlayerID) override;
	void Interact_Implementation(bool bIsInteracting, int PlayerID) override;
	float GetProgress_Implementation() override;
	bool GetHoveredOver_Implementation() override;

	UFUNCTION(BlueprintCallable)
	void DoMonsterInterference(float Interference);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<AActor> LeakEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TObjectPtr<UStaticMeshComponent> LeakingPipeMeshPlayerOne;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TObjectPtr<USceneComponent> SpawnPointsStart;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TObjectPtr<USceneComponent> SpawnPointsEnd;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TObjectPtr<UStaticMeshComponent> LeakingPipeMeshPlayerTwo;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UNetworkIDComponent> NetworkIDComponent;

	UPROPERTY(VisibleAnywhere)
	TArray<AActor*> ActiveLeaks;

	UPROPERTY(VisibleAnywhere)
	TArray<AActor*> DestroyLeaksQueue;

	UFUNCTION()
	void DestroyLeakTimer();

	UFUNCTION(BlueprintCallable)
	void TEST_SpawnLeak();

	UFUNCTION()
	void StartGame();

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SpawnLeak();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SpawnLeak(FVector Location);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_DestroyLeak(int Index);

	UFUNCTION(BlueprintCallable)
	bool TryFixLeakAt(FVector Location, int PlayerID);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxLeakFixDistance = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<ABathroomValvePump> ConnectedPump;

	//This is connected to a delegate in the ConnectedPump. 
	//Do not call manually
	UFUNCTION()
	void ChangeFixability(bool bIsFixable);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bLeakIsFixable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int MaxLeaks = 5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LeakRespawnRate = 5.0f;

	UPROPERTY()
	FTimerHandle LeakRespawnTimerHandle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UAudioComponent> AudioComponent;

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayAudio(USoundBase* Sound, UAudioComponent* Source);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<USoundBase> FixLeakSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<USoundBase> TaskCompleteSound;

	UPROPERTY(BlueprintAssignable)
	FOnLeakStateChange OnLeakStateChange;
};
