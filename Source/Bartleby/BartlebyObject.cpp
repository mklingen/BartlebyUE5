// Fill out your copyright notice in the Description page of Project Settings.


#include "Bartleby/BartlebyObject.h"
#include "Kismet/GameplayStatics.h"

// Sets default values for this component's properties
UBartlebyObject::UBartlebyObject()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

}


// Called when the game starts
void UBartlebyObject::BeginPlay()
{
	Super::BeginPlay();

	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABartlebySystem::StaticClass(), FoundActors);

	for (AActor* actor : FoundActors)
	{
		System = Cast<ABartlebySystem>(actor);
		if (System)
		{
			break;
		}
	}

	if (System)
	{
		auto room = System->GetRoomAtOrNull(GetOwner()->GetActorLocation());
		if (room)
		{
			room->Objects.Add(this);
		}
	}
}

