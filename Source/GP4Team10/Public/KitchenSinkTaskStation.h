// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Delegates/Delegate.h"
#include "KitchenSinkTaskStation.generated.h"

class UBoxComponent;
class UStaticMeshComponent;
class UAudioComponent;
class USoundBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTaskScoreChange, int, NewScore, int, OldScore);

UCLASS()
class GP4TEAM10_API AKitchenSinkTaskStation : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AKitchenSinkTaskStation();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	//This is called on both clients when the server determines a score change.
	UPROPERTY(BlueprintAssignable)
	FOnTaskScoreChange OnTaskScoreChange;

	UFUNCTION(BlueprintCallable)
	int GetCurrentBodyParts() { return CurrentBodyParts; }

	UFUNCTION(BlueprintCallable)
	int GetBodyPartsGoal() { return BodyPartsGoal; }

	//Progress from 0.0f to 1.0f
	UFUNCTION(BlueprintCallable)
	float GetProgress();

	UFUNCTION(BlueprintCallable)
	void DoMonsterInterference(int BodyPartsRemoved);
	


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UStaticMeshComponent> BloodMakingMachineMesh;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UBoxComponent> BodyPartDepositBox;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AActor> BodypartBlueprintType;

	UFUNCTION()
	void CheckForBodyParts();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float TimeSinceLastBodyPartCheck = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BodyPartCheckFrequency = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int BodyPartsGoal = 4;

	UPROPERTY(VisibleAnywhere, Replicated)
	int CurrentBodyParts = 0;

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_BroadcastScoreChange(int NewScore, int OldScore);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UAudioComponent> AudioComponent;

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayAudio(USoundBase* Sound);

	UFUNCTION()
	void PlayFinishedSound();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<USoundBase> GrindingSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<USoundBase> TaskCompleteSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<USoundBase> MonsterInterferenceSound;

};
