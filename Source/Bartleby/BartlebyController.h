// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"

#include "BartlebyController.generated.h"

// Implements an AI controller used by the Bartleby system.
UCLASS()
class BARTLEBY_API ABartlebyController : public AAIController
{
	GENERATED_BODY()
public:
	virtual void Tick(float dt) override;
	virtual void BeginPlay() override;

	enum class State
	{
		WaitForPlayerToGetNear,
		GoingToRoom,
		GoingToObject,
		TalkingOrThinking,
		WaitingForAI
	};

	UPROPERTY()
		class ABartlebySystem* System = nullptr;

	UPROPERTY()
		class ABartlebyRoom* CurrentRoom = nullptr;

	UPROPERTY()
		class ACharacter* OwnerCharacter = nullptr;

	UFUNCTION(BlueprintCallable)
		bool GoTo(const FString& LocationID, FString& errorMessage);

	UFUNCTION(BlueprintCallable)
		void Say(const FString& Phrase);

	UFUNCTION(BlueprintCallable)
		void Think(const FString& Phrase);

	UFUNCTION(BlueprintCallable)
		bool Examine(const FString& Target, FString& errorMessage);

	UFUNCTION(BlueprintCallable)
		bool TryDo(const FString& Command, FString& errorMessage);

	UFUNCTION(BlueprintCallable)
		void OnOpenAICallback(const FString& command);

	State state = State::GoingToRoom;
	UPROPERTY()
		AActor* TargetActor = nullptr;

	UPROPERTY()
		class UBartlebyObject* CurrentObject = nullptr;


	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		FString CharacterName = "Bartleby";

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TArray<FString> RecentPlaces;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool IsWaitingForScriptedEvent = false;

};
