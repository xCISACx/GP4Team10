// Fill out your copyright notice in the Description page of Project Settings.


#include "HelpMeCharacter.h"
#include "Components/InputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "HelpMePlayerController.h"
#include "Interactable.h"
#include "TaskStation.h"
#include "Components/SceneComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "HelpMeGameState.h"
#include "Net/UnrealNetwork.h"

class IInteractable;
// Sets default values
AHelpMeCharacter::AHelpMeCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	bAlwaysRelevant = true;

	HandSocket = CreateDefaultSubobject<USceneComponent>(FName("HandComponent"));
	HandSocket->SetupAttachment(RootComponent);

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->bOrientRotationToMovement = true;
		Movement->RotationRate = FRotator(0.0f, 360.0f, 0.0f);
	}

}

/*void AHelpMeCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AHelpMeCharacter, TickHitResult);
}*/

// Called when the game starts or when spawned
void AHelpMeCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->ClearAllMappings();

			Subsystem->AddMappingContext(InputMappingContext, 0);
		}
	}
	
}

void AHelpMeCharacter::Client_StartGame_Implementation()
{
	bGameHasStarted = true;
	AHelpMeGameState* GameState = Cast<AHelpMeGameState>(UGameplayStatics::GetGameState(this));
	if (GameState)
	{
		GameState->OnGameComplete.AddDynamic(this, &AHelpMeCharacter::GameIsOver);
	}


}

void AHelpMeCharacter::GameIsOver()
{
	bGameHasStarted = false;
}

// Called every frame
void AHelpMeCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!IsLocallyControlled()) return;
	//if (!HasAuthority()) return;
	if (!bGameHasStarted) return;


	// Get the player controller
    APlayerController* PlayerController = Cast<APlayerController>(GetController());
     
     if (PlayerController)
     {
        FVector StartTrace = PlayerController->PlayerCameraManager->GetCameraLocation();
        FVector EndTrace = StartTrace + (PlayerController->GetControlRotation().Vector() * InteractionRange);
         
        FCollisionQueryParams CollisionParams;
        CollisionParams.AddIgnoredActor(this);

     	LookingAtInteractable = false;

     	// Perform a line trace to detect what the player is looking at
     	if (GetWorld()->LineTraceSingleByChannel(TickHitResult, StartTrace, EndTrace, ECC_Visibility, CollisionParams))
     	{
     		AActor* HitActor = TickHitResult.GetActor();
     		if (HitActor)
     		{
     			const FString ActorName = HitActor->GetName();
     			//UE_LOG(LogTemp, Warning, TEXT("Hit Actor Name: %s"), *ActorName);
     		}
     		
     		// Check if the hit actor implements the IInteractable interface
     		if (TickHitResult.GetActor()->Implements<UInteractable>())
     		{
				//UE_LOG(LogTemp, Warning, TEXT("implements interface"));
				// Set CurrentInteractable to the hit object
				//CurrentInteractable.SetInterface(Interactable);
				CurrentInteractableActor = HitActor;
				LookingAtInteractable = true;
				const FString ActorName = CurrentInteractableActor->GetName();
				//UE_LOG(LogTemp, Warning, TEXT("%s is looking at interactable: %s"), *GetName(),*ActorName);
     			
     		}
			else
			{
				//UE_LOG(LogTemp, Warning, TEXT("does not implement interface"));
				LookingAtInteractable = false;
				//CurrentInteractable.SetInterface(nullptr); // Object doesn't implement IInteractable
			}
     	}
     	else
     	{
     		//UE_LOG(LogTemp, Warning, TEXT("nothing hit"));
     		LookingAtInteractable = false;
     		// If nothing is hit, set CurrentInteractable to null
     		//CurrentInteractable.SetInterface(nullptr);
     	}
     }
}

// Called to bind functionality to input
void AHelpMeCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		//Movement Bindings
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AHelpMeCharacter::Move);
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Completed, this, &AHelpMeCharacter::ResetMovementVector);
		
		//Looking Bindings
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AHelpMeCharacter::Look);
		
		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Completed, this, &AHelpMeCharacter::Interact);
	}

}

void AHelpMeCharacter::Move(const FInputActionValue& Value)
{
	MoveAxisVector = Value.Get<FVector2D>();

	if (!bCanMove) return;
	if (!bGameHasStarted) return;

	if (GetController())
	{
		const FRotator ControlRotation = GetControlRotation();
		const FRotator YawRotation(0.0f, ControlRotation.Yaw, 0.0f);

		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		
		AddMovementInput(ForwardDirection, MoveAxisVector.Y);
		AddMovementInput(RightDirection, MoveAxisVector.X);
	}
}

void AHelpMeCharacter::ResetMovementVector()
{
	MoveAxisVector = FVector2D::ZeroVector;
}

void AHelpMeCharacter::Look(const FInputActionValue& Value)
{
	if (!bGameHasStarted) return;
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (GetController())
	{
		AddControllerYawInput(LookAxisVector.X * LookInputMultiplier);
		AddControllerPitchInput(LookAxisVector.Y * LookInputMultiplier);
	}
}

void AHelpMeCharacter::Interact()
{
	if (!bGameHasStarted) return;
	// Get the player controller
	APlayerController* PlayerController = Cast<APlayerController>(GetController());

	if (PlayerController)
	{
		//Only interact if we're not holdings something.
		if (HeldObjectNetworkID.IsEmpty() || HeldObjectNetworkID.Len() == 0)
		{
			FVector StartTrace = PlayerController->PlayerCameraManager->GetCameraLocation();
			FVector EndTrace = StartTrace + (PlayerController->GetControlRotation().Vector() * InteractionRange);

			FHitResult HitResult;

			// Perform the raycast
			FCollisionQueryParams CollisionParams;
			CollisionParams.AddIgnoredActor(this); // Ignore the player character

			if (GetWorld()->LineTraceSingleByChannel(HitResult, StartTrace, EndTrace, ECC_Visibility, CollisionParams))
			{
				FString HitActorName = HitResult.GetActor() ? HitResult.GetActor()->GetName() : TEXT("None");


				if (HitResult.GetActor()->Implements<UInteractable>() && HitResult.GetComponent()->IsVisible())
				{
					Server_ShareHitResult(HitResult);
					GetController<AHelpMePlayerController>()->InteractWith(HitResult.GetActor(), true);
					FString InteractableName = HitResult.GetActor()->GetName();

					LastInteractedObject = HitResult.GetActor();

					UE_LOG(LogTemp, Warning, TEXT("%s: I interacted with interactable: %s"), *GetOwner()->GetName(), *InteractableName);
				}

			}

			//DrawDebugLine(GetWorld(), StartTrace, EndTrace, FColor::Green, false, 1, 0, 1);
		}
		//We are holding something, put it down
		else {
			FVector StartTrace = PlayerController->PlayerCameraManager->GetCameraLocation();
			FVector EndTrace = StartTrace + (PlayerController->GetControlRotation().Vector() * InteractionRange);

			FHitResult HitResult;

			FCollisionQueryParams CollisionParams;
			CollisionParams.AddIgnoredActor(this); // Ignore the player character
			if (LastInteractedObject)
				CollisionParams.AddIgnoredActor(LastInteractedObject);

			if (GetWorld()->LineTraceSingleByChannel(HitResult, StartTrace, EndTrace, ECC_Visibility, CollisionParams))
			{
				//Verify that the surface is somewhat flat
				if (HitResult.ImpactNormal.Dot(FVector::UpVector) > PlaceObjectMinDotProduct)
				{
					AHelpMePlayerController* PC = GetController<AHelpMePlayerController>();
					PC->Server_PlaceHeldObject(HeldObjectNetworkID, PC->GetPlayerID(), HitResult.Location);
					HeldObjectNetworkID.Empty();
				}				
			}

			//DrawDebugLine(GetWorld(), StartTrace, EndTrace, FColor::Green, false, 1, 0, 1);
		}

		
	}
}

void AHelpMeCharacter::Multicast_SetHeldObjectNetID_Implementation(const FString& NewID)
{
	HeldObjectNetworkID = NewID;
}

void AHelpMeCharacter::Server_ShareHitResult_Implementation(FHitResult HitResult)
{
	TickHitResult = HitResult;
}

bool AHelpMeCharacter::Server_ShareHitResult_Validate(FHitResult HitResult)
{
	return true;
}