// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputActionValue.h"
#include "Interactable.h"
#include "GameFramework/Character.h"
#include "HelpMeCharacter.generated.h"

class UInputMappingContext;
class UInputAction;
class USceneComponent;

UCLASS()
class GP4TEAM10_API AHelpMeCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AHelpMeCharacter();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
	bool bCanMove = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category ="Gameplay")
	float InteractionRange = 1000.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bGameHasStarted = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LookInputMultiplier = 0.5f;

	UFUNCTION(Client, Reliable)
	void Client_StartGame();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void GameIsOver();

	UPROPERTY(EditAnywhere)
	float PlaceObjectMinDotProduct = 0.75f;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void Move(const FInputActionValue& InputActionValue);

	// Called for looking input 
	void Look(const FInputActionValue& Value);
	void Interact();

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	void ResetMovementVector();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	bool LookingAtInteractable;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FHitResult TickHitResult;

	//Not replicated only usable locally
	UPROPERTY(EditAnywhere, Category = Input)
	AActor* CurrentInteractableActor;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	FString HeldObjectNetworkID;

	UPROPERTY()
	AActor* LastInteractedObject;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<USceneComponent> HandSocket;

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SetHeldObjectNetID(const FString& NewID);

private:
	
	UPROPERTY(EditAnywhere, Category = Input)
	UInputMappingContext* InputMappingContext;

	UPROPERTY(EditAnywhere,  Category = Input)
	UInputAction* MoveAction;

	UPROPERTY(EditAnywhere,  Category = Input)
	UInputAction* LookAction;

	UPROPERTY(EditAnywhere,  Category = Input)
	UInputAction* InteractAction;

	UPROPERTY(EditAnywhere, Category = Input)
	FVector2D MoveAxisVector;

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_ShareHitResult(FHitResult HitResult);
	
};
