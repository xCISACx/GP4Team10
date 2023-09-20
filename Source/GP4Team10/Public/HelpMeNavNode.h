// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HelpMeNavNode.generated.h"


UCLASS()
class GP4TEAM10_API AHelpMeNavNode : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AHelpMeNavNode();

	UFUNCTION(BlueprintCallable)
	static TArray<AHelpMeNavNode*> FindPathTo(AHelpMeNavNode* From, AHelpMeNavNode* To);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere)
	TArray<AHelpMeNavNode*> NeighbouringNodes;

	UFUNCTION(CallInEditor)
	void BakeAllNavigation();

	UFUNCTION()
	void BakeThisNavigation(TArray<AHelpMeNavNode*>& AllNodes);

	UFUNCTION()
	bool HasLineOfSightTo(AHelpMeNavNode* Node);



public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
