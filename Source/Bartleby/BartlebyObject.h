// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Bartleby/BartlebySystem.h"
#include "Bartleby/BartlebyRoom.h"
#include "BartlebyObject.generated.h"

// Attach to an AActor to give it a description and let the AI know about it.
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BARTLEBY_API UBartlebyObject : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UBartlebyObject();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		FString Id;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		FString Description;
	UPROPERTY()
		class ABartlebySystem* System = nullptr;

};
