// Fill out your copyright notice in the Description page of Project Settings.


#include "HelpMeNavNode.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

// Sets default values
AHelpMeNavNode::AHelpMeNavNode()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AHelpMeNavNode::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AHelpMeNavNode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AHelpMeNavNode::BakeAllNavigation()
{
	TArray<AActor*> OutActors;
	UGameplayStatics::GetAllActorsOfClass(this, StaticClass(), OutActors);
	TArray<AHelpMeNavNode*> AllNodes;
	for (AActor* Actor : OutActors)
	{
		AHelpMeNavNode* Node = Cast<AHelpMeNavNode>(Actor);
		if (Node)
			AllNodes.Add(Node);
	}

	for (AHelpMeNavNode* Node : AllNodes)
	{
		Node->BakeThisNavigation(AllNodes);
	}
}

void AHelpMeNavNode::BakeThisNavigation(TArray<AHelpMeNavNode*> &AllNodes)
{
	NeighbouringNodes.Empty();
	for (AHelpMeNavNode* Node : AllNodes)
	{
		if (Node && Node != this && HasLineOfSightTo(Node))
		{
			NeighbouringNodes.Add(Node);
			UKismetSystemLibrary::DrawDebugLine(
				this, 
				GetActorLocation(), 
				Node->GetActorLocation(), 
				FLinearColor::Red, 
				15.0f);
		}
	}

}

bool AHelpMeNavNode::HasLineOfSightTo(AHelpMeNavNode* Node)
{
	FHitResult OutHit;

	return !GetWorld()->LineTraceSingleByChannel(
		OutHit,
		GetActorLocation(),
		Node->GetActorLocation(),
		ECollisionChannel::ECC_Visibility);
}

TArray<AHelpMeNavNode*> AHelpMeNavNode::FindPathTo(AHelpMeNavNode* StartNode, AHelpMeNavNode* TargetNode, bool bIncludeNonRandomNodes)
{
	if (!StartNode || !TargetNode || StartNode == TargetNode) return TArray<AHelpMeNavNode*>();

    // Create a map to store the cost to reach each node.
    TMap<AHelpMeNavNode*, float> Distance;

    // Create a map to store the previous node on the path.
    TMap<AHelpMeNavNode*, AHelpMeNavNode*> PreviousNode;

    // Create a set to store unvisited nodes.
    TSet<AHelpMeNavNode*> OpenList;
    OpenList.Add(StartNode);

    Distance.Add(StartNode);
    Distance[StartNode] = 0.0f;

    while (!OpenList.IsEmpty())
    {
        // Find the node with the smallest distance value.
        AHelpMeNavNode* CurrentNode = nullptr;
        for (AHelpMeNavNode* Node : OpenList)
        {
            if (!CurrentNode || Distance[Node] < Distance[CurrentNode])
            {
                CurrentNode = Node;
            }
        }

        // If the target node has been reached, reconstruct the path and return.
        if (CurrentNode == TargetNode)
        {
            TArray<AHelpMeNavNode*> Path;
            while (CurrentNode)
            {
                Path.Add(CurrentNode);
                if (!PreviousNode.Contains(CurrentNode))
                    break;
                CurrentNode = PreviousNode[CurrentNode];
            }
            Algo::Reverse(Path);
            return Path;
        }

        OpenList.Remove(CurrentNode);

        // Relax the neighbors of the current node.
        for (AHelpMeNavNode* Neighbor : CurrentNode->NeighbouringNodes)
        {
            if (!bIncludeNonRandomNodes && !Neighbor->IsRandomFindable()) continue;

            float AltDistance = Distance[CurrentNode] + FVector::Distance(CurrentNode->GetActorLocation(), Neighbor->GetActorLocation());
            if (!Distance.Contains(Neighbor)) 
            {
                Distance.Add(Neighbor, MAX_FLT);
            }
            if (!PreviousNode.Contains(Neighbor))
                PreviousNode.Add(Neighbor);
                
            if (AltDistance < Distance[Neighbor])
            {
                Distance[Neighbor] = AltDistance;
                PreviousNode[Neighbor] = CurrentNode;
                OpenList.Add(Neighbor);
            }
        }
    }
	
	TArray<AHelpMeNavNode*> Path;
	return Path;
}