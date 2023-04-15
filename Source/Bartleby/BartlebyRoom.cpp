// Fill out your copyright notice in the Description page of Project Settings.


#include "Bartleby/BartlebyRoom.h"
#include "Bartleby/BartlebyObject.h"
#include "Components/BoxComponent.h"

// Sets default values
ABartlebyRoom::ABartlebyRoom()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Box = CreateDefaultSubobject<UBoxComponent>(TEXT("Box"));
	Box->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

}

// Called when the game starts or when spawned
void ABartlebyRoom::BeginPlay()
{
	Super::BeginPlay();

	
}

bool ABartlebyRoom::IsInside(const FVector& pt) const
{
	auto bx = Box->GetCollisionShape().Box;
	FVector ext(bx.HalfExtentX, bx.HalfExtentY, bx.HalfExtentZ);
	FVector center = Box->GetComponentLocation();
	return FBox(center - ext, center + ext).IsInside(pt);
}
