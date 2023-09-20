// Fill out your copyright notice in the Description page of Project Settings.


#include "FinalRitual.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "NetworkIDComponent.h"
#include "HelpMeGameState.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"
#include "Sound/SoundAttenuation.h"
#include "Net/UnrealNetwork.h"
#include "HelpMe_Pickup.h"

// Sets default values
AFinalRitual::AFinalRitual()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	USceneComponent* RootComp = CreateDefaultSubobject<USceneComponent>(FName("RootComponent"));
	SetRootComponent(RootComp);

	RitualMeshPlayerOne = CreateDefaultSubobject<UStaticMeshComponent>(FName("RitualMeshP1"));
	RitualMeshPlayerOne->SetupAttachment(RootComponent);

	RitualMeshPlayerTwo = CreateDefaultSubobject<UStaticMeshComponent>(FName("RitualMeshP2"));
	RitualMeshPlayerTwo->SetupAttachment(RootComponent);
	RitualMeshPlayerTwo->SetRelativeLocation(FVector(-3000, 0, 0));

	USceneComponent* RitualBoxesRoot = CreateDefaultSubobject<USceneComponent>(FName("RitualBoxes"));
	RitualBoxesRoot->SetupAttachment(RootComponent);

	RitualBox1 = CreateDefaultSubobject<UBoxComponent>(FName("RitualBox1"));
	RitualBox1->SetupAttachment(RitualBoxesRoot);
	RitualBox2 = CreateDefaultSubobject<UBoxComponent>(FName("RitualBox2"));
	RitualBox2->SetupAttachment(RitualBoxesRoot);
	RitualBox3 = CreateDefaultSubobject<UBoxComponent>(FName("RitualBox3"));
	RitualBox3->SetupAttachment(RitualBoxesRoot);
	RitualBox4 = CreateDefaultSubobject<UBoxComponent>(FName("RitualBox4"));
	RitualBox4->SetupAttachment(RitualBoxesRoot);

	RitualAudioComponent = CreateDefaultSubobject<UAudioComponent>(FName("RitualAudioComponent"));
	RitualAudioComponent->SetupAttachment(RootComponent);
	RitualAudioComponent->SetComponentTickEnabled(false);
	OngoingAudioComponent = CreateDefaultSubobject<UAudioComponent>(FName("OngoingAudioComponent"));
	OngoingAudioComponent->SetupAttachment(RootComponent);
	OngoingAudioComponent->SetComponentTickEnabled(false);

	FSoundAttenuationSettings Settings;
	Settings.FalloffDistance = 1600.0f;

	RitualAudioComponent->bOverrideAttenuation = true;
	RitualAudioComponent->AttenuationOverrides = Settings;
	OngoingAudioComponent->bOverrideAttenuation = true;
	OngoingAudioComponent->AttenuationOverrides = Settings;
}

/*
void ABathroomValveTaskStation::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ABathroomValveTaskStation, bLeakIsFixable);
}*/

// Called when the game starts or when spawned
void AFinalRitual::BeginPlay()
{
	Super::BeginPlay();
	
	if (GetNetMode() == ENetMode::NM_ListenServer)
	{
		RitualAudioComponent->SetRelativeLocation(RitualMeshPlayerOne->GetRelativeLocation());
		OngoingAudioComponent->SetRelativeLocation(RitualMeshPlayerOne->GetRelativeLocation());
		CompletedMap.Add(RitualBox1, false);
		CompletedMap.Add(RitualBox2, false);
		CompletedMap.Add(RitualBox3, false);
		CompletedMap.Add(RitualBox4, false);
	}
	else
	{
		RitualAudioComponent->SetRelativeLocation(RitualMeshPlayerTwo->GetRelativeLocation());
		OngoingAudioComponent->SetRelativeLocation(RitualMeshPlayerTwo->GetRelativeLocation());
	}

}

// Called every frame
void AFinalRitual::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//Only run logic below on ListenServer
	if (GetNetMode() != ENetMode::NM_ListenServer) return;

	TimeSinceLastRitualCheck += DeltaTime;
	if (TimeSinceLastRitualCheck >= RitualCheckFrequency)
	{
		CheckForRitualComponent(RitualBox1, RitualComponent1);
		CheckForRitualComponent(RitualBox2, RitualComponent2);
		CheckForRitualComponent(RitualBox3, RitualComponent3);
		CheckForRitualComponent(RitualBox4, RitualComponent4);
	}

}

void AFinalRitual::CheckForRitualComponent(UBoxComponent* Box, TSubclassOf<AActor> ComponentType)
{
	TArray<AActor*> FoundComponents;
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_Visibility));

	UKismetSystemLibrary::BoxOverlapActors(
		this,
		Box->GetComponentLocation(),
		Box->GetScaledBoxExtent(),
		ObjectTypes,
		ComponentType,
		TArray<AActor*>(),
		FoundComponents
	);

	bool bHasComponent = !FoundComponents.IsEmpty();

	if (bHasComponent)
	{
		for (AActor* Comp : FoundComponents)
		{
			AHelpMe_Pickup* Pickup = Cast<AHelpMe_Pickup>(Comp);
			if (Pickup)
			{
				Pickup->Multicast_MakeVisibleForBothPlayers();
			}
		}
	}

	if (bHasComponent != CompletedMap[Box])
	{
		int PreviousCompleteParts = CompleteParts;
		CompletedMap[Box] = bHasComponent;
		CompleteParts += bHasComponent ? 1 : -1;
		UKismetSystemLibrary::PrintString(this, FString("Ritual parts in place: ") + FString::FromInt(CompleteParts));
		if (CompleteParts == 4)
		{	
			AHelpMeGameState* GameState = GetWorld()->GetGameState<AHelpMeGameState>();
			if (GameState)
				GameState->ChangeTaskCompleted(ETaskType::TT_FINAL, true);
		}
		else if (PreviousCompleteParts == 4)
		{
			AHelpMeGameState* GameState = GetWorld()->GetGameState<AHelpMeGameState>();
			if (GameState)
				GameState->ChangeTaskCompleted(ETaskType::TT_FINAL, false);
		}
		Multicast_PlayAudio(bHasComponent ? AddComponentSound : RemoveComponentSound);
	}

}

void AFinalRitual::Multicast_PlayAudio_Implementation(USoundBase* Sound)
{
	if (!Sound) return;

	RitualAudioComponent->SetSound(Sound);
	RitualAudioComponent->Play();
}