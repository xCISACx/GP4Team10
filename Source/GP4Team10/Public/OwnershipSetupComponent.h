// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "OwnershipSetupComponent.generated.h"

/* This component can be attached to an actor
* to assign ownership to the correct player
* on game start.
*/
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GP4TEAM10_API UOwnershipSetupComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UOwnershipSetupComponent();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere)
	bool bFirstPlayerIsOwner;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	


};
