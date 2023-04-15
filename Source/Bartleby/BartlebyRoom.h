// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BartlebyRoom.generated.h"

UCLASS()
class BARTLEBY_API ABartlebyRoom : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABartlebyRoom();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		FString Id;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		FString Description;

	UFUNCTION()
		bool IsInside(const FVector& pt) const;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
		class UBoxComponent* Box = nullptr;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
		TArray<class UBartlebyObject*> Objects;
};
