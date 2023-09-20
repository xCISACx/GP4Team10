// Fill out your copyright notice in the Description page of Project Settings.


#include "TaskStation.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "HelpMeGameMode.h"
#include "HelpMeGameState.h"
#include "HelpMePlayerController.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "NetworkIDComponent.h"
#include "Math/Vector.h"
#include "Net/UnrealNetwork.h"



// Sets default values
ATaskStation::ATaskStation()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;
	bAlwaysRelevant = true;

	USceneComponent* NewRoot = CreateDefaultSubobject<USceneComponent>(FName("RootComponent"));
	SetRootComponent(NewRoot);

	OwnerMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(FName("OwnerVisibleMesh"));
	OwnerMeshComponent->bOnlyOwnerSee = true;
	OwnerMeshComponent->SetupAttachment(RootComponent);
	OwnerMeshComponent->SetCastShadow(false);
	NonOwnerMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(FName("NonOwnerVisibleMesh"));
	NonOwnerMeshComponent->SetupAttachment(RootComponent);
	NonOwnerMeshComponent->bOwnerNoSee = true;
	NonOwnerMeshComponent->SetCastShadow(false);

	AudioComponent = CreateDefaultSubobject<UAudioComponent>(FName("AudioComponent"));
	AudioComponent->SetupAttachment(RootComponent);
	AudioComponent->SetComponentTickEnabled(false);


	NetworkIDComponent = CreateDefaultSubobject<UNetworkIDComponent>(FName("NetworkIDComponent"));

}

void ATaskStation::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	DOREPLIFETIME(ATaskStation, CurrentState);
	DOREPLIFETIME(ATaskStation, CurrentInteractDuration);
	DOREPLIFETIME(ATaskStation, OwnerID);
}

// Called when the game starts or when spawned
void ATaskStation::BeginPlay()
{
	Super::BeginPlay();

	CurrentState = ETaskState::TS_None;
	
	AGameModeBase* GameMode = UGameplayStatics::GetGameMode(this);
	if (GameMode != nullptr)
	{
		AHelpMeGameMode* HelpMeGameMode = Cast<AHelpMeGameMode>(GameMode);
		if (HelpMeGameMode != nullptr)
		{
			HelpMeGameMode->RegisterTaskStation(this, bFirstPlayerIsOwner);
		}
	}

}

// Called every frame
void ATaskStation::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (CurrentState == ETaskState::TS_Performing)
	{
		CurrentInteractDuration += DeltaTime;
		if (CurrentInteractDuration >= TaskPerformTime)
		{
			SetState(ETaskState::TS_Performed);
		}
	}

	if (CurrentState == ETaskState::TS_Preparing)
	{
		CurrentInteractDuration += DeltaTime;
		if (CurrentInteractDuration >= TaskPrepareTime)
		{
			SetState(ETaskState::TS_Prepared);
		}
	}

	if (CurrentState == ETaskState::TS_Performed)
	{
		CurrentInteractDuration += DeltaTime;
		if (CurrentInteractDuration >= TaskResetTime)
		{
			SetState(ETaskState::TS_None);
		}
	}

}

void ATaskStation::SetState(ETaskState NewState)
{
	if (NewState == CurrentState) return;

	UE_LOG(LogTemp, Warning, TEXT("%s changed to state %i"), *GetName(), (int)CurrentState);

	CurrentState = NewState;
	if (NewState == ETaskState::TS_Performed)
	{
		//GetWorld()->GetGameState<AHelpMeGameState>()->GainMotivation(TaskPerformScore);
	}

	if (NewState == ETaskState::TS_None ||
		NewState == ETaskState::TS_Prepared ||
		NewState == ETaskState::TS_Performed)
	{
		CurrentInteractDuration = 0.0f;
	}

	switch (NewState)
	{
	case ETaskState::TS_Performing:
		Multicast_PlayAudio(PerformingSound);
		break;
	case ETaskState::TS_Performed:
		Multicast_PlayAudio(PerformedSound);
		break;
	case ETaskState::TS_Preparing:
		Multicast_PlayAudio(PreparingSound);
		break;
	case ETaskState::TS_Prepared:
		Multicast_PlayAudio(PreparedSound);
		break;

	}

}

bool ATaskStation::IsInteractableBy_Implementation(int PlayerID)
{
	switch (CurrentState)
	{
		case ETaskState::TS_NULL:
		case ETaskState::TS_Performed:
			return false;
		case ETaskState::TS_None:
		case ETaskState::TS_Preparing:
			return PlayerID != OwnerID;
		case ETaskState::TS_Prepared:
		case ETaskState::TS_Performing:
			return PlayerID == OwnerID;
		default: 
			return false;
	}
}

void ATaskStation::Interact_Implementation(bool bIsInteracting, int PlayerID)
{
	Server_Interact(bIsInteracting, PlayerID);
}

float ATaskStation::GetProgress_Implementation() 
{
	if (CurrentState == ETaskState::TS_Preparing || 
		CurrentState == ETaskState::TS_Prepared)
		return FMath::Clamp(CurrentInteractDuration / TaskPrepareTime, 0.0f, 1.0f);
	if (CurrentState == ETaskState::TS_Performing ||
		CurrentState == ETaskState::TS_Performed)
		return FMath::Clamp(CurrentInteractDuration / TaskPerformTime, 0.0f, 1.0f);
	return 0.0f;
}

bool ATaskStation::GetHoveredOver_Implementation()
{
	return bHoveredOver;
}

void ATaskStation::Server_Interact_Implementation(bool bIsInteracting, int PlayerID)
{

	if (!IsInteractableBy_Implementation(PlayerID)) return;
	
	
	
	if (PlayerID == OwnerID)
	{
		if (bIsInteracting)
		{
			SetState(ETaskState::TS_Performing);
		}
		else
		{
			SetState(ETaskState::TS_Prepared);
		}
	}
	else
	{
		if (bIsInteracting)
		{
			SetState(ETaskState::TS_Preparing);
		}
		else
		{
			SetState(ETaskState::TS_None);
		}
	}

	OnInteractServerOnly.Broadcast(PlayerID, bIsInteracting);
}

bool ATaskStation::Server_Interact_Validate(bool bIsInteracting, int PlayerID)
{
	return true;
}

void ATaskStation::Multicast_SetOwningPlayer_Implementation(int PlayerID)
{
	OwnerID = PlayerID;

	AudioComponent->SetRelativeLocation(NonOwnerMeshComponent->GetRelativeLocation());

	for (FConstPlayerControllerIterator iter = GetWorld()->GetPlayerControllerIterator(); iter; ++iter)
	{
		AHelpMePlayerController* PC = Cast<AHelpMePlayerController>(*iter);
		if (PC && PC->GetPlayerID() == PlayerID)
		{
			SetOwner(PC->GetPawn());
			AudioComponent->SetRelativeLocation(OwnerMeshComponent->GetRelativeLocation());
		}
	}
	
}

void ATaskStation::Multicast_PlayAudio_Implementation(USoundBase* Sound)
{
	if (!Sound) return;

	AudioComponent->SetSound(Sound);
	AudioComponent->Play();
}