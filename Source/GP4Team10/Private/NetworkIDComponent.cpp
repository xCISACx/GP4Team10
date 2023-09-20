// Fill out your copyright notice in the Description page of Project Settings.


#include "NetworkIDComponent.h"
#include "HelpMeGameMode.h"
#include "Net/UnrealNetwork.h"
#include "Misc/DateTime.h"
#include "Kismet/GameplayStatics.h"

// Sets default values for this component's properties
UNetworkIDComponent::UNetworkIDComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);

	// ...
	GenerateID();
}


// Called when the game starts
void UNetworkIDComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
	//Only run logic below on ListenServer
	if (GetNetMode() != ENetMode::NM_ListenServer) return;

	GenerateID();

	AGameModeBase* GameMode = UGameplayStatics::GetGameMode(this);
	if (GameMode)
	{
		AHelpMeGameMode* CastGameMode = Cast<AHelpMeGameMode>(GameMode);
		if (CastGameMode)
			CastGameMode->RegisterNetworkIDComponent(this);
	}
}

void UNetworkIDComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UNetworkIDComponent, NetworkID);
}


void UNetworkIDComponent::GenerateID()
{
	NetworkID = FString::FromInt(FDateTime::Now().ToUnixTimestamp() + FMath::RandRange(0, 100000));
	this->MarkPackageDirty();
}

// Called every frame
void UNetworkIDComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

FString UNetworkIDComponent::GetIDOfActor(AActor* Actor)
{
	UNetworkIDComponent* Component = Actor->GetComponentByClass<UNetworkIDComponent>();
	if (Component)
	{
		return Component->GetID();
	}
	return FString();
}
