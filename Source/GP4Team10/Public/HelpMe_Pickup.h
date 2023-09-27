// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interactable.h"
#include "GameFramework/Actor.h"
#include "NetworkIDComponent.h"
#include "HelpMe_Pickup.generated.h"

class AHelpMeCharacter;
class UAudioComponent;
class USoundBase;

UCLASS()
class GP4TEAM10_API AHelpMe_Pickup : public AActor, public IInteractable
{
	GENERATED_BODY()
	
public:
	// Sets default values for this actor's properties
	AHelpMe_Pickup();
	void Interact_Implementation(bool bIsInteracting, int PlayerID) override;
	bool IsInteractableBy_Implementation(int PlayerID) override;


	UFUNCTION()
	UNetworkIDComponent* GetNetworkIDComponent() {
		return NetworkIDComponent;
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UStaticMeshComponent> PlayerOneMeshComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UStaticMeshComponent> PlayerTwoMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool BeingHeld;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<AHelpMeCharacter> CurrentHolder; 

	UPROPERTY(Replicated)
	int CurrentHolderID = -1;

	UFUNCTION()
	void PlaceAt(FVector Location, int PlayerID);

	UFUNCTION(BlueprintCallable)
	void MakeOnlyVisibleForPlayer(int PlayerID);

	UFUNCTION(NetMulticast, Reliable, BlueprintCallable)
	void Multicast_MakeVisibleForBothPlayers();

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
	bool bIsTeleportable = true;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UNetworkIDComponent> NetworkIDComponent; 

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UAudioComponent> AudioComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UAudioComponent> OngoingAudioComponent;

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayAudio(USoundBase* Sound, UAudioComponent* Source);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<USoundBase> PickupSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<USoundBase> PutDownSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<USoundBase> WhileHeldSound;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
