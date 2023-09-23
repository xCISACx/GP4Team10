// Fill out your copyright notice in the Description page of Project Settings.


#include "KitchenSinkTaskStation.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Components/BoxComponent.h"
#include "HelpMeGameState.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"
#include "Components/StaticMeshComponent.h"


// Sets default values
AKitchenSinkTaskStation::AKitchenSinkTaskStation()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	bAlwaysRelevant = true;

	BloodMakingMachineMesh = CreateDefaultSubobject<UStaticMeshComponent>(FName("BloodMakingMachine"));
	SetRootComponent(BloodMakingMachineMesh);

	BodyPartDepositBox = CreateDefaultSubobject<UBoxComponent>(FName("BodyPartDepositBox"));
	BodyPartDepositBox->SetupAttachment(RootComponent);

	AudioComponent = CreateDefaultSubobject<UAudioComponent>(FName("AudioComponent"));
	AudioComponent->SetupAttachment(RootComponent);
	AudioComponent->SetComponentTickEnabled(false);

	FSoundAttenuationSettings Settings;
	Settings.FalloffDistance = 1600.0f;
	AudioComponent->bOverrideAttenuation = true;
	AudioComponent->AttenuationOverrides = Settings;

}

void AKitchenSinkTaskStation::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AKitchenSinkTaskStation, CurrentBodyParts);
}

// Called when the game starts or when spawned
void AKitchenSinkTaskStation::BeginPlay()
{
	Super::BeginPlay();

	if (GetNetMode() != ENetMode::NM_ListenServer)
	{
		AudioComponent->SetRelativeLocation(FVector(-3000, 0, 0));
	}
}

// Called every frame
void AKitchenSinkTaskStation::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//Only run logic below on ListenServer
	if (GetNetMode() != ENetMode::NM_ListenServer) return;

	TimeSinceLastBodyPartCheck += DeltaTime;
	if (TimeSinceLastBodyPartCheck >= BodyPartCheckFrequency)
	{
		CheckForBodyParts();
	}
}

void AKitchenSinkTaskStation::CheckForBodyParts()
{
	//Can only be called on server
	if (GetNetMode() != ENetMode::NM_ListenServer) return;

	TArray<AActor*> FoundBodyParts;
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_Visibility));
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_WorldDynamic));


	UKismetSystemLibrary::BoxOverlapActors(
		this,
		BodyPartDepositBox->GetComponentLocation(),
		BodyPartDepositBox->GetScaledBoxExtent(),
		ObjectTypes,
		BodypartBlueprintType,
		TArray<AActor*>(),
		FoundBodyParts
	);
	int PreviousBodyParts = CurrentBodyParts;
	for (AActor* BodyPart : FoundBodyParts)
	{
		if (CurrentBodyParts >= BodyPartsGoal)
			break;

		GetWorld()->DestroyActor(BodyPart);
		CurrentBodyParts++;
	}
	if (PreviousBodyParts != CurrentBodyParts)
	{
		Multicast_BroadcastScoreChange(CurrentBodyParts, PreviousBodyParts);
		Multicast_PlayAudio(GrindingSound);
	}

	//The task was not completed, but now it is
	if (PreviousBodyParts < BodyPartsGoal && CurrentBodyParts >= BodyPartsGoal)
	{
		AHelpMeGameState* GameState = GetWorld()->GetGameState<AHelpMeGameState>();
		if (GameState)
			GameState->ChangeTaskCompleted(ETaskType::TT_KITCHENSINK, true);
		
		FTimerHandle TimerHandle;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &AKitchenSinkTaskStation::PlayFinishedSound, GrindingSound->GetDuration(), false);
	}
}

void AKitchenSinkTaskStation::PlayFinishedSound()
{
	Multicast_PlayAudio(TaskCompleteSound);
}

float AKitchenSinkTaskStation::GetProgress()
{
	return FMath::Clamp((float)(CurrentBodyParts) / (float)(BodyPartsGoal), 0.0f, 1.0f);
}

void AKitchenSinkTaskStation::DoMonsterInterference(int BodyPartsRemoved)
{
	//Can only be called on server
	if (GetNetMode() != ENetMode::NM_ListenServer) return;

	int PreviousBodyParts = CurrentBodyParts;
	CurrentBodyParts -= BodyPartsRemoved;
	if (CurrentBodyParts < 0)
		CurrentBodyParts = 0;

	if (PreviousBodyParts != CurrentBodyParts)
		Multicast_PlayAudio(MonsterInterferenceSound);

	//The task was completed, now becomes failed
	if (PreviousBodyParts >= BodyPartsGoal && CurrentBodyParts < BodyPartsGoal)
	{
		AHelpMeGameState* GameState = GetWorld()->GetGameState<AHelpMeGameState>();
		if (GameState)
			GameState->ChangeTaskCompleted(ETaskType::TT_KITCHENSINK, false);
	}

	Multicast_BroadcastScoreChange(CurrentBodyParts, PreviousBodyParts);
}

void AKitchenSinkTaskStation::Multicast_BroadcastScoreChange_Implementation(int NewScore, int OldScore)
{
	OnTaskScoreChange.Broadcast(NewScore, OldScore);
}

void AKitchenSinkTaskStation::Multicast_PlayAudio_Implementation(USoundBase* Sound)
{
	if (!Sound) return;

	AudioComponent->SetSound(Sound);
	AudioComponent->Play();
}