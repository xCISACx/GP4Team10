// Fill out your copyright notice in the Description page of Project Settings.


#include "BathroomValveTaskStation.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "NetworkIDComponent.h"
#include "BathroomValvePump.h"
#include "HelpMePlayerController.h"
#include "HelpMeCharacter.h"
#include "HelpMeGameMode.h"
#include "HelpMeGameState.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Net/UnrealNetwork.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"
#include "Sound/SoundAttenuation.h"
#include "DelayedDestroyInterface.h"

//UNiagaraFunctionLibrary::SpawnSystemAtLocation

// Sets default values
ABathroomValveTaskStation::ABathroomValveTaskStation()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	USceneComponent* RootComp = CreateDefaultSubobject<USceneComponent>(FName("RootComponent"));
	SetRootComponent(RootComp);

	LeakingPipeMeshPlayerOne = CreateDefaultSubobject<UStaticMeshComponent>(FName("LeakingPipeMeshP1"));
	LeakingPipeMeshPlayerOne->SetupAttachment(RootComponent);

	SpawnPointsStart = CreateDefaultSubobject<USceneComponent>(FName("LeakSpawnStart"));
	SpawnPointsStart->SetupAttachment(LeakingPipeMeshPlayerOne);
	SpawnPointsEnd = CreateDefaultSubobject<USceneComponent>(FName("LeakSpawnEnd"));
	SpawnPointsEnd->SetupAttachment(LeakingPipeMeshPlayerOne);

	LeakingPipeMeshPlayerTwo = CreateDefaultSubobject<UStaticMeshComponent>(FName("LeakingPipeMeshP2"));
	LeakingPipeMeshPlayerTwo->SetupAttachment(RootComponent);
	LeakingPipeMeshPlayerTwo->SetRelativeLocation(FVector(-3000, 0, 0));

	NetworkIDComponent = CreateDefaultSubobject<UNetworkIDComponent>(FName("NetworkIDComponent"));

	AudioComponent = CreateDefaultSubobject<UAudioComponent>(FName("AudioComponent"));
	AudioComponent->SetupAttachment(RootComponent);
	AudioComponent->SetComponentTickEnabled(false);

	FSoundAttenuationSettings Settings;
	Settings.FalloffDistance = 1600.0f;
	AudioComponent->bOverrideAttenuation = true;
	AudioComponent->AttenuationOverrides = Settings;
}
/*
void ABathroomValveTaskStation::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ABathroomValveTaskStation, bLeakIsFixable);
}*/


// Called when the game starts or when spawned
void ABathroomValveTaskStation::BeginPlay()
{
	Super::BeginPlay();
	if (ConnectedPump)
	{
		ConnectedPump->OnLeakFixableStateChange.AddDynamic(this, &ABathroomValveTaskStation::ChangeFixability);
	}

	AHelpMeGameMode* GameMode = Cast<AHelpMeGameMode>(UGameplayStatics::GetGameMode(this));
	if (GameMode)
	{
		GameMode->OnGameStart.AddDynamic(this, &ABathroomValveTaskStation::StartGame);
	}

	if (GetNetMode() == ENetMode::NM_ListenServer)
	{
		AudioComponent->SetRelativeLocation(LeakingPipeMeshPlayerOne->GetRelativeLocation());
	}
	else
	{
		AudioComponent->SetRelativeLocation(LeakingPipeMeshPlayerTwo->GetRelativeLocation());
	}

}

// Called every frame
void ABathroomValveTaskStation::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ABathroomValveTaskStation::StartGame()
{
	while (ActiveLeaks.Num() < MaxLeaks)
	{
		Server_SpawnLeak();
	}
	GetWorld()->GetTimerManager().SetTimer(LeakRespawnTimerHandle, this, &ABathroomValveTaskStation::Server_SpawnLeak, LeakRespawnRate, true);
}


bool ABathroomValveTaskStation::IsInteractableBy_Implementation(int PlayerID)
{
	return true;
}
void ABathroomValveTaskStation::Interact_Implementation(bool bIsInteracting, int PlayerID)
{
	AHelpMeGameMode* GameMode = Cast<AHelpMeGameMode>(UGameplayStatics::GetGameMode(this));
	AHelpMePlayerController* Controller = Cast<AHelpMePlayerController>(GameMode->GetControllerFromID(PlayerID));
	AHelpMeCharacter* Character = Cast<AHelpMeCharacter>(Controller->GetCharacter());
	FHitResult LineTraceResult = Character->TickHitResult;
	if (TryFixLeakAt(LineTraceResult.Location, PlayerID))
	{
		
	}
}
float ABathroomValveTaskStation::GetProgress_Implementation()
{
	return (float)(ActiveLeaks.Num()) / (float)(MaxLeaks);
}

bool ABathroomValveTaskStation::GetHoveredOver_Implementation()
{
	return false;
}

void ABathroomValveTaskStation::TEST_SpawnLeak()
{
	Server_SpawnLeak();
}

void ABathroomValveTaskStation::Server_SpawnLeak_Implementation()
{
	if (ActiveLeaks.Num() >= MaxLeaks) return;

	//Generate leak on random spot between the two spawn points
	FVector RandomLocation = FMath::Lerp(SpawnPointsStart->GetComponentLocation(), SpawnPointsEnd->GetComponentLocation(), FMath::FRand());

	Multicast_SpawnLeak(RandomLocation);

	
	if (ActiveLeaks.Num() == 1)
	{
		AHelpMeGameState* GameState = GetWorld()->GetGameState<AHelpMeGameState>();
		if (GameState)
		{
			GameState->ChangeTaskCompleted(ETaskType::TT_BATHROOMVALVE, false);
			GetWorld()->GetTimerManager().SetTimer(LeakRespawnTimerHandle, this, &ABathroomValveTaskStation::Server_SpawnLeak, LeakRespawnRate, true);
		}
	}

}
bool ABathroomValveTaskStation::Server_SpawnLeak_Validate() { return true; }

void ABathroomValveTaskStation::Multicast_SpawnLeak_Implementation(FVector Location)
{
	FActorSpawnParameters SpawnParams;
	AActor* NewLeak = GetWorld()->SpawnActor<AActor>(
		LeakEffect,
		Location,
		FRotator::ZeroRotator,
		SpawnParams
		);

	ActiveLeaks.Add(NewLeak);
}

bool ABathroomValveTaskStation::TryFixLeakAt(FVector Location, int PlayerID)
{
	if (ActiveLeaks.Num() == 0 || !bLeakIsFixable)
	{
		return false;
	}

	if (PlayerID == 1)
	{
		Location += FVector(3000, 0, 0);
	}

	int NearestLeakIndex = 0;
	for (int i = 1; i < ActiveLeaks.Num(); i++)
	{
		if (FVector::Distance(Location, ActiveLeaks[i]->GetActorLocation()) < FVector::Distance(Location, ActiveLeaks[NearestLeakIndex]->GetActorLocation()))
			NearestLeakIndex = i;
	}

	if (FVector::Distance(Location, ActiveLeaks[NearestLeakIndex]->GetActorLocation()) > MaxLeakFixDistance)
	{
		return false;
	}

	Multicast_DestroyLeak(NearestLeakIndex);
	

	return true;
}

void ABathroomValveTaskStation::Multicast_DestroyLeak_Implementation(int Index)
{
	AActor* RemoveLeak = ActiveLeaks[Index];
	ActiveLeaks.RemoveAt(Index);
	DestroyLeaksQueue.Add(RemoveLeak);

	if (RemoveLeak->Implements<UDelayedDestroyInterface>())
	{
		IDelayedDestroyInterface::Execute_DestroyWithDelay(RemoveLeak, 3.0f);
	}
	else
	{
		UKismetSystemLibrary::PrintString(this, FString("FailedToImplementInterface"));
		FTimerHandle Handle;
		GetWorld()->GetTimerManager().SetTimer(
			Handle,
			this,
			&ABathroomValveTaskStation::DestroyLeakTimer,
			3.0f,
			false);
	}

	if (GetNetMode() == ENetMode::NM_ListenServer)
	{
		if (ActiveLeaks.Num() == 0)
		{
			AHelpMeGameState* GameState = GetWorld()->GetGameState<AHelpMeGameState>();
			if (GameState)
			{
				GameState->ChangeTaskCompleted(ETaskType::TT_BATHROOMVALVE, true);
				Multicast_PlayAudio(TaskCompleteSound, AudioComponent);
				GetWorld()->GetTimerManager().ClearTimer(LeakRespawnTimerHandle);
			}
		}
		else
		{
			Multicast_PlayAudio(FixLeakSound, AudioComponent);
		}
	}
	
}

void ABathroomValveTaskStation::DestroyLeakTimer()
{
	if (DestroyLeaksQueue.IsEmpty()) return;

	AActor* DestroyActor = DestroyLeaksQueue[0];
	DestroyLeaksQueue.RemoveAt(0);
	GetWorld()->DestroyActor(DestroyActor);

}

void ABathroomValveTaskStation::ChangeFixability(bool bIsFixable)
{
	if (bLeakIsFixable == bIsFixable) return;
	//if (GetNetMode() != ENetMode::NM_ListenServer) return;

	bLeakIsFixable = bIsFixable;
	OnLeakStateChange.Broadcast(bLeakIsFixable);
}

void ABathroomValveTaskStation::DoMonsterInterference(float Interference)
{
	float InterferencePerLeak = Interference / MaxLeaks;

	while (Interference > 0)
	{
		float RandomFloat = FMath::RandRange(0.0f, InterferencePerLeak);
		if (RandomFloat <= Interference)
			Server_SpawnLeak();

		Interference -= InterferencePerLeak;
	}

	AHelpMeGameState* GameState = GetWorld()->GetGameState<AHelpMeGameState>();
	if (GameState)
	{
		GameState->ChangeTaskCompleted(ETaskType::TT_BATHROOMVALVE, false);
	}

}

void ABathroomValveTaskStation::Multicast_PlayAudio_Implementation(USoundBase* Sound, UAudioComponent* Source)
{
	if (!Sound) return;

	Source->SetSound(Sound);
	Source->Play();
}