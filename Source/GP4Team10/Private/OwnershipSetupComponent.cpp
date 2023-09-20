// Fill out your copyright notice in the Description page of Project Settings.


#include "OwnershipSetupComponent.h"
#include "HelpMeGameMode.h"
#include "Kismet/GameplayStatics.h"

UOwnershipSetupComponent::UOwnershipSetupComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UOwnershipSetupComponent::BeginPlay()
{
	Super::BeginPlay();

	//If we're the host, register this component in the game mode
	//So it can assign ownership properly.
	AGameModeBase* GameMode = UGameplayStatics::GetGameMode(this);
	if (GameMode != nullptr)
	{
		AHelpMeGameMode* HelpMeGameMode = Cast<AHelpMeGameMode>(GameMode);
		if (HelpMeGameMode != nullptr)
		{
			HelpMeGameMode->RegisterOwnershipComponent(this, bFirstPlayerIsOwner);
		}
	}
	
}

void UOwnershipSetupComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

