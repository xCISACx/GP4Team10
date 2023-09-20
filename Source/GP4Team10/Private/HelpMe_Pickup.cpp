// Fill out your copyright notice in the Description page of Project Settings.


#include "HelpMe_Pickup.h"

#include "HelpMeCharacter.h"
#include "HelpMeGameMode.h"
#include "Components/SceneComponent.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"
#include "HelpMePlayerController.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AHelpMe_Pickup::AHelpMe_Pickup()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	bIsTeleportable = false;
	bReplicates = true;
	SetReplicateMovement(true);
	bAlwaysRelevant = true;
	NetworkIDComponent = CreateDefaultSubobject<UNetworkIDComponent>(FName("NetworkIDComponent"));

	USceneComponent* NewRoot = CreateDefaultSubobject<USceneComponent>(FName("RootComponent"));
	SetRootComponent(NewRoot);

	PlayerOneMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(FName("PlayerOneMesh"));
	PlayerOneMeshComponent->SetIsReplicated(true);
	PlayerOneMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	PlayerOneMeshComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	PlayerOneMeshComponent->SetEnableGravity(false);
	PlayerOneMeshComponent->SetSimulatePhysics(false);

	PlayerTwoMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(FName("PlayerTwoMesh"));
	PlayerTwoMeshComponent->SetIsReplicated(true);
	PlayerTwoMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	PlayerTwoMeshComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	PlayerTwoMeshComponent->SetEnableGravity(false);
	PlayerTwoMeshComponent->SetSimulatePhysics(false);

	PlayerOneMeshComponent->SetupAttachment(RootComponent);
	PlayerTwoMeshComponent->SetupAttachment(RootComponent);
	PlayerTwoMeshComponent->SetRelativeLocation(FVector(-3000, 0, 0));

	PlayerTwoMeshComponent->SetVisibility(false);

	AudioComponent = CreateDefaultSubobject<UAudioComponent>(FName("AudioComponent"));
	AudioComponent->SetupAttachment(RootComponent);
	AudioComponent->SetComponentTickEnabled(false);

	FSoundAttenuationSettings Settings;
	Settings.FalloffDistance = 1600.0f;

	AudioComponent->bOverrideAttenuation = true;
	AudioComponent->AttenuationOverrides = Settings;

}

void AHelpMe_Pickup::Interact_Implementation(bool bIsInteracting, int PlayerID)
{
	AHelpMePlayerController* Controller = Cast<AHelpMeGameMode>(UGameplayStatics::GetGameMode(GetWorld()))->GetControllerFromID(PlayerID);
	AHelpMeCharacter* Character = Cast<AHelpMeCharacter>(Controller->GetCharacter());
	//These two pointers ^^ are correct, both client and server.

	SetOwner(Character);
	Character->Multicast_SetHeldObjectNetID(NetworkIDComponent->GetID());
	CurrentHolder = Character;
	
	AttachToActor(Character, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	FVector AttachmentLocation = Character->HandSocket->GetRelativeLocation();
	if (PlayerID == 1)
		AttachmentLocation += FVector(3000, 0, 0);
	SetActorRelativeLocation(AttachmentLocation);

	MakeOnlyVisibleForPlayer(PlayerID);

	BeingHeld = true;
	Multicast_PlayAudio(PickupSound);
}

// Called when the game starts or when spawned
void AHelpMe_Pickup::BeginPlay()
{
	Super::BeginPlay();
	
	if (GetNetMode() == ENetMode::NM_ListenServer)
	{
		AudioComponent->SetRelativeLocation(PlayerOneMeshComponent->GetRelativeLocation());
	}
	else
	{
		AudioComponent->SetRelativeLocation(PlayerTwoMeshComponent->GetRelativeLocation());
	}

}

// Called every frame
void AHelpMe_Pickup::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

bool AHelpMe_Pickup::IsInteractableBy_Implementation(int PlayerID)
{
	return !BeingHeld;
}

void AHelpMe_Pickup::PlaceAt(FVector Location, int PlayerID)
{
	DetachFromActor(FDetachmentTransformRules::KeepRelativeTransform);
	FVector TargetLocation = Location;
	if (PlayerID == 1)
		TargetLocation -= FVector(-3000, 0, 0);
	SetActorLocation(TargetLocation);
	BeingHeld = false;
	CurrentHolder = nullptr;
	Multicast_PlayAudio(PutDownSound);
	MakeOnlyVisibleForPlayer(PlayerID);
	bIsTeleportable = true;
}

void AHelpMe_Pickup::MakeOnlyVisibleForPlayer(int PlayerID)
{
	if (PlayerID == 0)
	{
		PlayerOneMeshComponent->SetVisibility(true);
		PlayerOneMeshComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
		PlayerTwoMeshComponent->SetVisibility(false);
		PlayerTwoMeshComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Ignore);
	}
	else
	{
		PlayerOneMeshComponent->SetVisibility(false);
		PlayerOneMeshComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Ignore);
		PlayerTwoMeshComponent->SetVisibility(true);
		PlayerTwoMeshComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	}
}

void AHelpMe_Pickup::Multicast_MakeVisibleForBothPlayers_Implementation()
{
	PlayerOneMeshComponent->SetVisibility(true);
	PlayerOneMeshComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	PlayerTwoMeshComponent->SetVisibility(true);
	PlayerTwoMeshComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
}

void AHelpMe_Pickup::Multicast_PlayAudio_Implementation(USoundBase* Sound)
{
	if (!Sound) return;

	AudioComponent->SetSound(Sound);
	AudioComponent->Play();
}