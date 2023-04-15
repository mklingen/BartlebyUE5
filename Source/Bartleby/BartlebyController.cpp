// Fill out your copyright notice in the Description page of Project Settings.


#include "Bartleby/BartlebyController.h"
#include "Bartleby/BartlebySystem.h"
#include "Bartleby/BartlebyRoom.h"
#include "Bartleby/BartlebyObject.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"

void ABartlebyController::BeginPlay()
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

	if (!GetCharacter())
	{
		UE_LOG(LogTemp, Error,  TEXT("No character?"));
		return;
	}

	if (System)
	{
		CurrentRoom = System->GetRoomAtOrNull(GetCharacter()->GetActorLocation());
		System->Controller = this;
	}

	OwnerCharacter = Cast<ACharacter>(GetCharacter());
}

void ABartlebyController::Tick(float dt)
{
	Super::Tick(dt);
	if (!OwnerCharacter)
	{
		return;
	}
	if (IsWaitingForScriptedEvent)
	{
		StopMovement();
		return;
	}

	if (System && System->IsWaitingOnInput)
	{
		StopMovement();
		return;
	}

	switch (state)
	{
		case State::GoingToRoom:
		{
			GetCharacter()->GetCharacterMovement()->bOrientRotationToMovement = true;
			ClearFocus(EAIFocusPriority::Gameplay);
			MoveToActor(CurrentRoom, 100.0f);
			// If in the room, select a random target.
			if (CurrentRoom && FVector::Dist2D(CurrentRoom->GetActorLocation(), GetCharacter()->GetActorLocation()) < 150.0f)
			{
				System->AppendMsg("action_result: You travelled to " + CurrentRoom->Id);
				state = State::WaitForPlayerToGetNear;
			}
			break;
		}
		case State::GoingToObject :
		{
			GetCharacter()->GetCharacterMovement()->bOrientRotationToMovement = true;
			ClearFocus(EAIFocusPriority::Gameplay);
			// Wait for the player to get near after going to the object.
			if (TargetActor)
			{
				SetFocus(TargetActor);
				MoveToActor(TargetActor, 200.0f);
				if (FVector::Dist2D(TargetActor->GetActorLocation(), GetCharacter()->GetActorLocation()) < 250.0f)
				{
					state = State::WaitForPlayerToGetNear;
				}
			}
			
			break;
		}
		case State::WaitForPlayerToGetNear :
		{
			GetCharacter()->GetCharacterMovement()->bOrientRotationToMovement = false;
			APlayerController* playerController = GetWorld()->GetFirstPlayerController();
			if (playerController)
			{
				if (TargetActor)
				{
					SetFocus(TargetActor);
				}
				else
				{
					SetFocus(playerController->GetCharacter(), EAIFocusPriority::Gameplay);
				}
				FVector actorPos = playerController->GetCharacter()->GetActorLocation();
				// If the player is near, start the next round of ai stuff.
				if (FVector::Dist2D(actorPos, GetCharacter()->GetActorLocation()) < 300.0f)
				{
					state = State::WaitingForAI;
				}
			}
			break;
		}
		case State::TalkingOrThinking:
		{
			// Wait until we're not saying anything more.
			if (IsWaitingForScriptedEvent)
			{
				return;
			}
			CurrentObject = nullptr;
			// Done talking, so start waiting for the AI.
			state = State::WaitingForAI;
			break;
		}
		case State::WaitingForAI:
		{
			GetCharacter()->GetCharacterMovement()->bOrientRotationToMovement = false;
			APlayerController* playerController = GetWorld()->GetFirstPlayerController();
			if (playerController)
			{
				SetFocus(playerController->GetCharacter(), EAIFocusPriority::Gameplay);
			}
			// If already waiting on the AI, do nothing.
			if (System->IsWaitingOnOpenAI) 
			{
				return;
			}
			if (!System->LastThingOpenAISaid.IsEmpty())
			{
				OnOpenAICallback(System->LastThingOpenAISaid);
				System->LastThingOpenAISaid = "";
				return;
			}
			// Otherwise, kick off a new AI round.
			System->StartOpenAICall();
			break;
		}
	}
}



void ABartlebyController::OnOpenAICallback(const FString& command)
{
	FString error;
	if (!TryDo(command, error))
	{
		UE_LOG(LogTemp, Error,  TEXT("%s"), *error);
		System->AppendMsg(error);
	}
	System->LastThingPlayerSaid = "";
}

bool ABartlebyController::GoTo(const FString& LocationID, FString& errorMessage)
{
	UE_LOG(LogTemp, Display,  TEXT("Bartleby GoTo %s"), *LocationID);
	if (!System)
	{
		errorMessage = "No sys";
		return false;
	}
	ABartlebyRoom* room = System->GetRoomOrNull(LocationID);
	if (!room)
	{
		UE_LOG(LogTemp, Error,  TEXT("No Room called \"%s\""), *LocationID);
		errorMessage = "Cannot go to that room from here.";
		return false;
	}
	state = State::GoingToRoom;
	CurrentRoom = room;
	TargetActor = room;
	MoveToActor(CurrentRoom, 100.0f);
	if (!RecentPlaces.Contains(LocationID))
	{
		RecentPlaces.Add(LocationID);
		if (RecentPlaces.Num() > 5)
		{
			RecentPlaces.Remove(LocationID);
		}
	}
	return true;
}

void ABartlebyController::Say(const FString& Phrase)
{
	UE_LOG(LogTemp, Display,  TEXT("Bartleby Say %s"), *Phrase);
	state = State::TalkingOrThinking;
	if (System)
	{
		System->Say(GetCharacter(), CharacterName, Phrase);
	}
}

void ABartlebyController::Think(const FString& Phrase)
{
	UE_LOG(LogTemp, Display,  TEXT("Bartleby Think %s"), *Phrase);
	state = State::TalkingOrThinking;
	if (System)
	{
		System->Say(GetCharacter(), CharacterName + " (Thinking)", Phrase);
	}
}

bool ABartlebyController::Examine(const FString& Target, FString& errorMessage)
{
	UE_LOG(LogTemp, Display,  TEXT("Bartleby Examine %s"), *Target);
	if (!CurrentRoom)
	{
		UE_LOG(LogTemp, Error,  TEXT("NO room."));
		errorMessage = "No room";
		return false;
	}
	TArray<UBartlebyObject*> objects = CurrentRoom->Objects;
	UBartlebyObject* targetObject = nullptr;
	FString lower = Target.ToLower();
	for (UBartlebyObject* obj : objects)
	{
		if (obj->Id == Target || obj->Id.ToLower().Contains(lower))
		{
			targetObject = obj;
			break;
		}
	}
	CurrentObject = targetObject;
	if (targetObject == nullptr)
	{
		UE_LOG(LogTemp, Error,  TEXT("Unable to examine %s as it does not exist in the room."), *Target);
		errorMessage = "action_result: Error. could not find the object in the current room.";
		return false;
	}
	System->AppendMsg("action_result: " + CurrentObject->Description);
	TargetActor = targetObject->GetOwner();
	state = State::GoingToObject;
	return true;
	
}


bool ParseString(const FString& inputString, FString& command, FString& argument)
{
	int openBracketIndex = inputString.Find(TEXT("(")); // Find the index of the opening bracket
	int closeBracketIndex = inputString.Find(TEXT(")")); // Find the index of the closing bracket

	if (openBracketIndex != INDEX_NONE && closeBracketIndex != INDEX_NONE) // Make sure both brackets are present
	{
		command = inputString.Mid(0, openBracketIndex); // Extract the command before the opening bracket
		argument = inputString.Mid(openBracketIndex + 1, closeBracketIndex - openBracketIndex - 1); // Extract the argument between the brackets
		// Possibly chop off the quotes, because the AI loves quotes (python I guess).
		if (argument.Len() > 0)
		{
			if (argument[0] == '"' || argument[0] == '\'')
			{
				argument = argument.Mid(1, argument.Len() - 1);
			}
			if (argument[argument.Len() - 1] == '"' || argument[argument.Len() - 1] == '\'')
			{
				argument = argument.Mid(0, argument.Len() - 1);
			}
		}
		return true;
	}
	else // If the string is not in the correct format, set the output strings to empty
	{
		command = "";
		argument = "";
		return false;
	}
}


bool ABartlebyController::TryDo(const FString& Command, FString& errorMessage)
{
	FString commandVerb;
	FString commandNoun;
	if (!ParseString(Command, commandVerb, commandNoun))
	{
		errorMessage = "action_result: Malformed command. Expected exactly 2 parentheses.";
		return false;
	}
	if (commandVerb == "say")
	{
		Say(commandNoun);
		return true;
	}
	else if (commandVerb == "go")
	{
		if (!GoTo(commandNoun, errorMessage))
		{
			// AI is dumb and thinks GO = examine.
			return Examine(commandNoun, errorMessage);
		}
		return true;
	}
	else if (commandVerb == "examine")
	{
		return Examine(commandNoun, errorMessage);
	}
	else if (commandVerb == "think")
	{
		Think(commandNoun);
		return true;
	}
	else
	{
		errorMessage = "action_result: Unrecognized command " + commandVerb;
		return false;
	}
}