// Fill out your copyright notice in the Description page of Project Settings.


#include "CellarPumpTaskStation.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "HelpMeGameState.h"
#include "HelpMeGameMode.h"
#include "HelpMeCharacter.h"
#include "HelpMePlayerController.h"
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

	PlayerOnePumpOneMesh = CreateDefaultSubobject<UStaticMeshComponent>(FName("PlayerOnePumpOne"));
	PlayerOnePumpOneMesh->SetupAttachment(RootComponent);

	PlayerTwoPumpOneMesh = CreateDefaultSubobject<UStaticMeshComponent>(FName("PlayerTwoPumpOne"));
	PlayerTwoPumpOneMesh->SetupAttachment(RootComponent);
	PlayerTwoPumpOneMesh->SetRelativeLocation(FVector(-3000, 0, 0));

	PlayerOnePumpTwoMesh = CreateDefaultSubobject<UStaticMeshComponent>(FName("PlayerOnePumpTwo"));
	PlayerOnePumpTwoMesh->SetupAttachment(RootComponent);

	PlayerTwoPumpTwoMesh = CreateDefaultSubobject<UStaticMeshComponent>(FName("PlayerTwoPumpTwo"));
	PlayerTwoPumpTwoMesh->SetupAttachment(RootComponent);
	PlayerTwoPumpTwoMesh->SetRelativeLocation(FVector(-3000, 0, 0));

	NetworkIDComponent = CreateDefaultSubobject<UNetworkIDComponent>(FName("NetworkIDComponent"));

	PumpOneAudioComponent = CreateDefaultSubobject<UAudioComponent>(FName("PlayerOneAudioComponent"));
	PumpOneAudioComponent->SetupAttachment(RootComponent);
	PumpOneAudioComponent->SetComponentTickEnabled(false);

	PumpTwoAudioComponent = CreateDefaultSubobject<UAudioComponent>(FName("PlayerTwoAudioComponent"));
	PumpTwoAudioComponent->SetupAttachment(RootComponent);
	PumpTwoAudioComponent->SetComponentTickEnabled(false);

	MainPumpAudioComponent = CreateDefaultSubobject<UAudioComponent>(FName("MainPumpAudioComponent"));
	MainPumpAudioComponent->SetupAttachment(RootComponent);
	MainPumpAudioComponent->SetComponentTickEnabled(false);

	FSoundAttenuationSettings Settings;
	Settings.FalloffDistance = 1600.0f;

	PumpOneAudioComponent->bOverrideAttenuation = true;
	PumpOneAudioComponent->AttenuationOverrides = Settings;
	PumpTwoAudioComponent->bOverrideAttenuation = true;
	PumpTwoAudioComponent->AttenuationOverrides = Settings;
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
		PumpOneAudioComponent->SetRelativeLocation(PlayerOnePumpOneMesh->GetRelativeLocation());
		PumpTwoAudioComponent->SetRelativeLocation(PlayerOnePumpTwoMesh->GetRelativeLocation());
		MainPumpAudioComponent->SetRelativeLocation((PlayerOnePumpOneMesh->GetRelativeLocation() + PlayerOnePumpTwoMesh->GetRelativeLocation()) / 2.0f);
	}
	else
	{
		PumpOneAudioComponent->SetRelativeLocation(PlayerTwoPumpOneMesh->GetRelativeLocation());
		PumpTwoAudioComponent->SetRelativeLocation(PlayerTwoPumpTwoMesh->GetRelativeLocation());
		MainPumpAudioComponent->SetRelativeLocation((PlayerTwoPumpOneMesh->GetRelativeLocation() + PlayerTwoPumpTwoMesh->GetRelativeLocation()) / 2.0f);
	}

}

// Called every frame
void ACellarPumpTaskStation::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	TimeSincePumpOneUse += DeltaTime;
	TimeSincePumpTwoUse += DeltaTime;

	//Only run logic below on ListenServer
	if (GetNetMode() != ENetMode::NM_ListenServer) return;


	if (bShouldPlayResetSoundPumpOne && TimeSincePumpOneUse >= PumpUseCooldown)
	{
		if (CurrentProgress < 1.0f)
			Multicast_PlayAudio(PumpResetSound, PumpOneAudioComponent);
		bShouldPlayResetSoundPumpOne = false;
	}
	if (bShouldPlayResetSoundPumpTwo && TimeSincePumpTwoUse >= PumpUseCooldown)
	{
		if (CurrentProgress < 1.0f)
			Multicast_PlayAudio(PumpResetSound, PumpTwoAudioComponent);
		bShouldPlayResetSoundPumpTwo = false;
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

	return true;
}

void ACellarPumpTaskStation::Interact_Implementation(bool bIsInteracting, int PlayerID)
{
	//Don't run if we are ending the interaction, or if we're not allowed to interact.
	if (!bIsInteracting || !IsInteractableBy_Implementation(PlayerID)) return;
	if (CurrentProgress >= 1.0f) return;

	//Get linetrace
	AHelpMeGameMode* GameMode = Cast<AHelpMeGameMode>(UGameplayStatics::GetGameMode(this));
	AHelpMePlayerController* Controller = Cast<AHelpMePlayerController>(GameMode->GetControllerFromID(PlayerID));
	AHelpMeCharacter* Character = Cast<AHelpMeCharacter>(Controller->GetCharacter());
	FHitResult LineTraceResult = Character->TickHitResult;
	UPrimitiveComponent* HitComponent = LineTraceResult.GetComponent();
	if (HitComponent == PlayerOnePumpOneMesh || HitComponent == PlayerTwoPumpOneMesh)
	{
		Multicast_PlayAudio(SinglePumpSound, PumpOneAudioComponent);
		bShouldPlayResetSoundPumpOne = true;
		UKismetSystemLibrary::PrintString(this, FString("Pump One: ") + FString::FromInt(PlayerID));
		Multicast_ResetPumpUseTime(0);
	} 
	else if (HitComponent == PlayerOnePumpTwoMesh || HitComponent == PlayerTwoPumpTwoMesh)
	{
		Multicast_PlayAudio(SinglePumpSound, PumpTwoAudioComponent);
		bShouldPlayResetSoundPumpTwo = true;
		UKismetSystemLibrary::PrintString(this, FString("Pump Two: ") + FString::FromInt(PlayerID));
		Multicast_ResetPumpUseTime(1);
	}

	float PreviousProgress = CurrentProgress;
	CurrentProgress = FMath::Min(1.0f, CurrentProgress + ProgressOnUse);

	//The task was not completed, but now it is
	if (PreviousProgress < 1.0f && CurrentProgress >= 1.0f)
	{
		AHelpMeGameState* GameState = GetWorld()->GetGameState<AHelpMeGameState>();
		if (GameState)
			GameState->ChangeTaskCompleted(ETaskType::TT_CELLARPUMP, true);
		Multicast_PlayAudio(TaskCompleteSound, MainPumpAudioComponent);
	}
	Multicast_BroadcastProgressChange(CurrentProgress, PreviousProgress);
	
}

void ACellarPumpTaskStation::Multicast_ResetPumpUseTime_Implementation(int PumpID)
{
	OnSuccessfulUseBy.Broadcast(PumpID);
	if (PumpID == 0)
	{
		TimeSincePumpOneUse = 0.0f;
	}
	if (PumpID == 1)
	{
		TimeSincePumpTwoUse = 0.0f;
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