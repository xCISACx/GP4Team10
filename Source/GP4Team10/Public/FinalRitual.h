// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FinalRitual.generated.h"

class UNetworkIDComponent;
class ABathroomValvePump;
class UAudioComponent;
class USoundBase;
class UBoxComponent;
class AHelpMe_Pickup;

UCLASS()
class GP4TEAM10_API AFinalRitual : public AActor
{
	GENERATED_BODY()
	
public:	
	AFinalRitual();
	virtual void Tick(float DeltaTime) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TObjectPtr<UStaticMeshComponent> RitualMeshPlayerOne;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TObjectPtr<UStaticMeshComponent> RitualMeshPlayerTwo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UAudioComponent> RitualAudioComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UAudioComponent> OngoingAudioComponent;

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayAudio(USoundBase* Sound);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<USoundBase> CompleteRitualSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<USoundBase> AddComponentSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<USoundBase> RemoveComponentSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<USoundBase> OngoingSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TObjectPtr<UBoxComponent> RitualBox1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TObjectPtr<UBoxComponent> RitualBox2;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TObjectPtr<UBoxComponent> RitualBox3;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TObjectPtr<UBoxComponent> RitualBox4;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AHelpMe_Pickup> RitualComponent1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AHelpMe_Pickup> RitualComponent2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AHelpMe_Pickup> RitualComponent3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AHelpMe_Pickup> RitualComponent4;

	//This keeps track of whether the right ritual component has been put
	//into each BoxComponent
	UPROPERTY()
	TMap<UBoxComponent*, bool> CompletedMap;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int CompleteParts = 0;

	UPROPERTY()
	float TimeSinceLastRitualCheck;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RitualCheckFrequency = 1.0f;

	UFUNCTION()
	void CheckForRitualComponent(UBoxComponent* Box, TSubclassOf<AActor> ComponentType);

};
