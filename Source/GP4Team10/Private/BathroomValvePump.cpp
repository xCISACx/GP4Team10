// Fill out your copyright notice in the Description page of Project Settings.


#include "BathroomValvePump.h"
#include "NetworkIDComponent.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"
#include "Sound/SoundAttenuation.h"
#include "HelpMeGameState.h"
#include "Components/StaticMeshComponent.h"

// Sets default values
ABathroomValvePump::ABathroomValvePump()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	USceneComponent* RootComp = CreateDefaultSubobject<USceneComponent>(FName("RootComponent"));
	SetRootComponent(RootComp);

	PumpMeshPlayerOne = CreateDefaultSubobject<UStaticMeshComponent>(FName("PumpMeshPlayerOne"));
	PumpMeshPlayerOne->SetupAttachment(RootComponent);

	PumpMeshPlayerTwo = CreateDefaultSubobject<UStaticMeshComponent>(FName("PumpMeshPlayerTwo"));
	PumpMeshPlayerTwo->SetupAttachment(RootComponent);

	NetworkIDComponent = CreateDefaultSubobject<UNetworkIDComponent>(FName("NetworkIDComponent"));

	PumpAudioComponent = CreateDefaultSubobject<UAudioComponent>(FName("PumpAudioComponent"));
	PumpAudioComponent->SetupAttachment(RootComponent);
	PumpAudioComponent->SetComponentTickEnabled(false);

	OngoingAudioComponent = CreateDefaultSubobject<UAudioComponent>(FName("OngoingAudioComponent"));
	OngoingAudioComponent->SetupAttachment(RootComponent);
	OngoingAudioComponent->SetComponentTickEnabled(false);

	FSoundAttenuationSettings Settings;
	Settings.FalloffDistance = 1600.0f;

	PumpAudioComponent->bOverrideAttenuation = true;
	PumpAudioComponent->AttenuationOverrides = Settings;
	OngoingAudioComponent->bOverrideAttenuation = true;
	OngoingAudioComponent->AttenuationOverrides = Settings;

}


void ABathroomValvePump::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ABathroomValvePump, CurrentFlow);
	DOREPLIFETIME(ABathroomValvePump, bValveIsOpen);
}

// Called when the game starts or when spawned
void ABathroomValvePump::BeginPlay()
{
	Super::BeginPlay();

	if (GetNetMode() != ENetMode::NM_ListenServer)
	{
		PumpAudioComponent->SetRelativeLocation(PumpMeshPlayerTwo->GetRelativeLocation());
		OngoingAudioComponent->SetRelativeLocation(PumpMeshPlayerTwo->GetRelativeLocation());
	}
	else
	{
		PumpAudioComponent->SetRelativeLocation(PumpMeshPlayerOne->GetRelativeLocation());
		OngoingAudioComponent->SetRelativeLocation(PumpMeshPlayerOne->GetRelativeLocation());
	}
	
}

// Called every frame
void ABathroomValvePump::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);


	if (GetNetMode() == ENetMode::NM_ListenServer)
	{
		TimeSinceLastDecay += DeltaTime;
		if (TimeSinceLastDecay >= FlowChangeFrequency)
		{
			float PreviousFlow = CurrentFlow;
			if (bValveIsOpen)
			{
				CurrentFlow = CurrentFlow + FlowChangeFrequency * FlowGainPerSecond * (TimeSinceLastDecay / FlowChangeFrequency);
			}
			else
			{
				CurrentFlow = CurrentFlow - FlowChangeFrequency * FlowDecayPerSecond * (TimeSinceLastDecay / FlowChangeFrequency);
			}
			CurrentFlow = FMath::Clamp(CurrentFlow, 0.0f, 1.0f);
			if (GetNetMode() == ENetMode::NM_ListenServer && PreviousFlow != CurrentFlow)
				OnRep_CurrentFlow();
			TimeSinceLastDecay = 0.0f;
		}
	}

}

bool ABathroomValvePump::IsInteractableBy_Implementation(int PlayerID)
{
	AHelpMeGameState* GameState = GetWorld()->GetGameState<AHelpMeGameState>();
	if (GameState && 
		(!GameState->IsTaskCompleted(ETaskType::TT_FINAL) || 
			GameState->IsTaskCompleted(ETaskType::TT_CELLARPUMP)))
		return false;

	return true;
}
void ABathroomValvePump::Interact_Implementation(bool bIsInteracting, int PlayerID)
{
	if (!IsInteractableBy_Implementation(PlayerID)) return;

	bValveIsOpen = !bValveIsOpen;
	OnRep_ValveIsOpen();
	bValveIsOpen ?
		Multicast_PlayAudio(OpenValveSound, PumpAudioComponent) : 
		Multicast_PlayAudio(CloseValveSound, PumpAudioComponent);
}
float ABathroomValvePump::GetProgress_Implementation()
{
	return CurrentFlow;
}
bool ABathroomValvePump::GetHoveredOver_Implementation()
{
	return false;
}

void ABathroomValvePump::OnRep_CurrentFlow()
{
	OnFlowValueChange.Broadcast(CurrentFlow);
	if (CurrentFlow >= GoodFlowRange.X && CurrentFlow <= GoodFlowRange.Y)
	{
		if (!bLastLeakFixableBroadcast)
		{
			OnLeakFixableStateChange.Broadcast(true);
			bLastLeakFixableBroadcast = true;
			OngoingAudioComponent->SetSound(OngoingGoodFlowSound);
			OngoingAudioComponent->Play();
			UKismetSystemLibrary::PrintString(this, FString("Good Flow"));
		}
	}
	else
	{
		if (bLastLeakFixableBroadcast)
		{
			OnLeakFixableStateChange.Broadcast(false);
			bLastLeakFixableBroadcast = false;
			OngoingAudioComponent->Stop();
			UKismetSystemLibrary::PrintString(this, FString("Bad Flow"));
		}
	}
	
}

void ABathroomValvePump::Multicast_PlayAudio_Implementation(USoundBase* Sound, UAudioComponent* Source)
{
	if (!Sound) return;

	Source->SetSound(Sound);
	Source->Play();
}

void ABathroomValvePump::OnRep_ValveIsOpen()
{
	OnValveStateChange.Broadcast(bValveIsOpen);
}