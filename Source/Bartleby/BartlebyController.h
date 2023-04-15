/*MIT License

Copyright (c) 2023 Matthew Klingensmith

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/


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
