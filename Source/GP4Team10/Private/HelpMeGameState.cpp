// Fill out your copyright notice in the Description page of Project Settings.


#include "HelpMeGameState.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/KismetSystemLibrary.h"


void AHelpMeGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AHelpMeGameState, CompletedTaskFlags);
}


void AHelpMeGameState::ChangeTaskCompleted(ETaskType Type, bool bNewStatus)
{
	
	uint8 OldFlags = CompletedTaskFlags;
	if (bNewStatus)
	{
		CompletedTaskFlags |= (uint8)(Type);
	}
	else
	{
		CompletedTaskFlags &= ~(uint8)(Type);
	}
	if (OldFlags != CompletedTaskFlags)
	{
		Multicast_UpdateTaskState(Type, bNewStatus);
		OnRep_CompletedTaskFlags();
	}
}

bool AHelpMeGameState::IsTaskCompleted(ETaskType Type)
{
	return ((uint8)(Type) & CompletedTaskFlags);
}

void AHelpMeGameState::Multicast_UpdateTaskState_Implementation(ETaskType Type, bool bNewStatus)
{
	OnTaskCompletionChange.Broadcast(Type, bNewStatus);
}

int AHelpMeGameState::NumberOfCompletedTasks()
{
	uint8 Flags = CompletedTaskFlags;
	int Count = 0;
	do {
		int has = Flags & 1;
		if (has == 1)
			Count++;

	} while ((Flags >>= 1) != 0);
	return Count;
}

void AHelpMeGameState::OnRep_CompletedTaskFlags()
{
	if (NumberOfCompletedTasks() == 4)
		OnGameComplete.Broadcast();
}