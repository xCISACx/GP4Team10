// Fill out your copyright notice in the Description page of Project Settings.


#include "CellarPumpTaskStation.h"
#include "Kismet/KismetSystemLibrary.h"
#include "HelpMeGameState.h"
#include "Net/UnrealNetwork.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"
#include "NetworkIDComponent.h"
#include "Components/SceneComponent.h"
#include "Sound/SoundAttenuation.h"
#include "Components/StaticMeshComponent.h"

// Sets default values
ACellarPumpTaskStation::ACellarPumpTaskStation()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	bAlwaysRelevant = true;

	USceneComponent* NewRoot = CreateDefaultSubobject<USceneComponent>(FName("RootComponent"));
	SetRootComponent(NewRoot);

	PlayerOnePumpMesh = CreateDefaultSubobject<UStaticMeshComponent>(FName("PlayerOnePump"));
	PlayerOnePumpMesh->SetupAttachment(RootComponent);

	PlayerTwoPumpMesh = CreateDefaultSubobject<UStaticMeshComponent>(FName("PlayerTwoPump"));
	PlayerTwoPumpMesh->SetupAttachment(RootComponent);
	PlayerTwoPumpMesh->SetRelativeLocation(FVector(-3000, 0, 0));

	NetworkIDComponent = CreateDefaultSubobject<UNetworkIDComponent>(FName("NetworkIDComponent"));

	PlayerOnePumpAudioComponent = CreateDefaultSubobject<UAudioComponent>(FName("PlayerOneAudioComponent"));
	PlayerOnePumpAudioComponent->SetupAttachment(RootComponent);
	PlayerOnePumpAudioComponent->SetComponentTickEnabled(false);

	PlayerTwoPumpAudioComponent = CreateDefaultSubobject<UAudioComponent>(FName("PlayerTwoAudioComponent"));
	PlayerTwoPumpAudioComponent->SetupAttachment(RootComponent);
	PlayerTwoPumpAudioComponent->SetComponentTickEnabled(false);

	MainPumpAudioComponent = CreateDefaultSubobject<UAudioComponent>(FName("MainPumpAudioComponent"));
	MainPumpAudioComponent->SetupAttachment(RootComponent);
	MainPumpAudioComponent->SetComponentTickEnabled(false);

	FSoundAttenuationSettings Settings;
	Settings.FalloffDistance = 1600.0f;

	PlayerOnePumpAudioComponent->bOverrideAttenuation = true;
	PlayerOnePumpAudioComponent->AttenuationOverrides = Settings;
	PlayerTwoPumpAudioComponent->bOverrideAttenuation = true;
	PlayerTwoPumpAudioComponent->AttenuationOverrides = Settings;
	MainPumpAudioComponent->bOverrideAttenuation = true;
	MainPumpAudioComponent->AttenuationOverrides = Settings;
}
/*
void ACellarPumpTaskStation::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ACellarPumpTaskStation, CurrentProgress);
}*/

// Called when the game starts or when spawned
void ACellarPumpTaskStation::BeginPlay()
{
	Super::BeginPlay();

	if (GetNetMode() == ENetMode::NM_ListenServer)
	{
		PlayerOnePumpAudioComponent->SetRelativeLocation(PlayerOnePumpMesh->GetRelativeLocation());
		PlayerTwoPumpAudioComponent->SetRelativeLocation(PlayerOnePumpMesh->GetRelativeLocation());
		MainPumpAudioComponent->SetRelativeLocation(PlayerOnePumpMesh->GetRelativeLocation());
	}
	else
	{
		PlayerOnePumpAudioComponent->SetRelativeLocation(PlayerTwoPumpMesh->GetRelativeLocation());
		PlayerTwoPumpAudioComponent->SetRelativeLocation(PlayerTwoPumpMesh->GetRelativeLocation());
		MainPumpAudioComponent->SetRelativeLocation(PlayerTwoPumpMesh->GetRelativeLocation());
	}

}

// Called every frame
void ACellarPumpTaskStation::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	TimeSincePlayerOneUse += DeltaTime;
	TimeSincePlayerTwoUse += DeltaTime;

	//Only run logic below on ListenServer
	if (GetNetMode() != ENetMode::NM_ListenServer) return;


	if (bShouldPlayResetSoundPlayerOne && TimeSincePlayerOneUse >= PumpUseCooldown)
	{
		if (CurrentProgress < 1.0f)
			Multicast_PlayAudio(PumpResetSound, PlayerOnePumpAudioComponent);
		bShouldPlayResetSoundPlayerOne = false;
	}
	if (bShouldPlayResetSoundPlayerTwo && TimeSincePlayerTwoUse >= PumpUseCooldown)
	{
		if (CurrentProgress < 1.0f)
			Multicast_PlayAudio(PumpResetSound, PlayerTwoPumpAudioComponent);
		bShouldPlayResetSoundPlayerTwo = false;
	}

	//Decay only happens if we haven't completed the task
	TimeSinceDecay += DeltaTime;
	if (TimeSinceDecay >= DecayFrequency && CurrentProgress < 1.0f)
	{
		float PreviousProgress = CurrentProgress;
		CurrentProgress = FMath::Max(
			0.0f,
			CurrentProgress - 
			(TimeSinceDecay / DecayFrequency) * DecayMultiplier * DecayFrequency * ProgressOnUse / PumpUseCooldown
		);
		TimeSinceDecay = 0.0f;
		Multicast_BroadcastProgressChange(CurrentProgress, PreviousProgress);
	}

}

bool ACellarPumpTaskStation::IsInteractableBy_Implementation(int PlayerID)
{
	AHelpMeGameState* GameState = GetWorld()->GetGameState<AHelpMeGameState>();
	if (GameState && !GameState->IsTaskCompleted(ETaskType::TT_FINAL))
		return false;

	return (PlayerID == 0) ? (TimeSincePlayerOneUse >= PumpUseCooldown) : (TimeSincePlayerTwoUse >= PumpUseCooldown);
}

void ACellarPumpTaskStation::Interact_Implementation(bool bIsInteracting, int PlayerID)
{
	//Don't run if we are ending the interaction, or if we're not allowed to interact.
	if (!bIsInteracting || !IsInteractableBy_Implementation(PlayerID)) return;
	
	if (CurrentProgress >= 1.0f) return;

	float PreviousProgress = CurrentProgress;
	CurrentProgress = FMath::Min(1.0f, CurrentProgress + ProgressOnUse);

	if (PlayerID == 0)
	{
		Multicast_PlayAudio(SinglePumpSound, PlayerOnePumpAudioComponent);
		bShouldPlayResetSoundPlayerOne = true;
	}
	else
	{
		Multicast_PlayAudio(SinglePumpSound, PlayerTwoPumpAudioComponent);
		bShouldPlayResetSoundPlayerTwo = true;
	}

	//The task was not completed, but now it is
	if (PreviousProgress < 1.0f && CurrentProgress >= 1.0f)
	{
		AHelpMeGameState* GameState = GetWorld()->GetGameState<AHelpMeGameState>();
		if (GameState)
			GameState->ChangeTaskCompleted(ETaskType::TT_CELLARPUMP, true);
		Multicast_PlayAudio(TaskCompleteSound, MainPumpAudioComponent);
	}
	Multicast_BroadcastProgressChange(CurrentProgress, PreviousProgress);
	Multicast_ResetPlayerUseTime(PlayerID);
}

void ACellarPumpTaskStation::Multicast_ResetPlayerUseTime_Implementation(int PlayerID)
{
	OnSuccessfulUseBy.Broadcast(PlayerID);
	if (PlayerID == 0)
	{
		TimeSincePlayerOneUse = 0.0f;
	}
	if (PlayerID == 1)
	{
		TimeSincePlayerTwoUse = 0.0f;
	}
}

float ACellarPumpTaskStation::GetProgress_Implementation()
{
	return FMath::Clamp(CurrentProgress, 0.0f, 1.0f);
}

bool ACellarPumpTaskStation::GetHoveredOver_Implementation()
{
	return false;
}

void ACellarPumpTaskStation::DoMonsterInterference(float ProgressRemoved)
{
	//Can only be called on server
	if (GetNetMode() != ENetMode::NM_ListenServer) return;

	float PreviousProgress = CurrentProgress;
	CurrentProgress = FMath::Max(0.0f, CurrentProgress - ProgressRemoved);

	if (PreviousProgress != CurrentProgress)
		Multicast_PlayAudio(MonsterInterferenceSound, MainPumpAudioComponent);

	if (PreviousProgress >= 1.0f && CurrentProgress < 1.0f)
	{
		AHelpMeGameState* GameState = GetWorld()->GetGameState<AHelpMeGameState>();
		if (GameState)
			GameState->ChangeTaskCompleted(ETaskType::TT_CELLARPUMP, false);
	}
	Multicast_BroadcastProgressChange(CurrentProgress, PreviousProgress);
}

void ACellarPumpTaskStation::Multicast_BroadcastProgressChange_Implementation(float NewProgress, float OldProgress)
{
	if (NewProgress == OldProgress) return;

	OnTaskProgressChange.Broadcast(NewProgress, OldProgress);
}


void ACellarPumpTaskStation::Multicast_PlayAudio_Implementation(USoundBase* Sound, UAudioComponent* Source)
{
	if (!Sound) return;

	Source->SetSound(Sound);
	Source->Play();
}