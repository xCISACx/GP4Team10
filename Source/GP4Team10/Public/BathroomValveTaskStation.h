// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interactable.h"
#include "BathroomValveTaskStation.generated.h"

class UNiagaraSystem;
class UNiagaraComponent;
class UNetworkIDComponent;
class ABathroomValvePump;

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

	void DoMonsterInterference(float Interference);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<AActor> LeakEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TObjectPtr<UStaticMeshComponent> LeakingPipeMeshPlayerOne;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TObjectPtr<UStaticMeshComponent> LeakingPipeMeshPlayerTwo;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UNetworkIDComponent> NetworkIDComponent;

	UPROPERTY(VisibleAnywhere)
	TArray<AActor*> ActiveLeaks;

	UFUNCTION(BlueprintCallable)
	void TEST_SpawnLeak();

	UFUNCTION()
	void StartGame();

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SpawnLeak();

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

	UPROPERTY()
	bool bLeakIsFixable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int MaxLeaks = 5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LeakRespawnRate = 5.0f;

	UPROPERTY()
	FTimerHandle LeakRespawnTimerHandle;
};
