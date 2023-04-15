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



#include "Bartleby/BartlebySystem.h"
#include "GameFramework/Character.h"
#include "Bartleby/BartlebyRoom.h"
#include "Bartleby/BartlebyObject.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/UserWidget.h"
#include "HttpModule.h"
#include "Bartleby/BartlebyInput.h"
#include "JsonObjectConverter.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Bartleby/BartlebyController.h"


ABartlebySystem::ABartlebySystem()
{
	PrimaryActorTick.bCanEverTick = true;
}


void ABartlebySystem::BeginPlay()
{
	Super::BeginPlay();
	// Get all the rooms.
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABartlebyRoom::StaticClass(), FoundActors);

	for (AActor* actor : FoundActors)
	{
		Rooms.Add(Cast<ABartlebyRoom>(actor));
	}

	// Creat the input widget and start it hidden.
	inputWidget = CreateWidget<UBartlebyInput>(GetWorld(), InputWidgetClass);
	if (inputWidget)
	{
		inputWidget->AddToViewport(0);
		inputWidget->SetVisibility(ESlateVisibility::Hidden);
	}

}

void ABartlebySystem::CollectInput()
{
	// If we have an input widget, turn it on and enable mouse input.
	if (inputWidget)
	{
		IsWaitingOnInput = true;
		inputWidget->InputText = FText::FromString("");
		inputWidget->CancelButtonPressed = false;
		inputWidget->SayButtonPressed = false;
		inputWidget->SetVisibility(ESlateVisibility::Visible);
		APlayerController* player = GetWorld()->GetFirstPlayerController();
		if (player)
		{
			player->SetInputMode(FInputModeUIOnly());
			player->SetShowMouseCursor(true);
		}
	}
}

// Called every frame.
void ABartlebySystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// If we're waiting on input, try to get the text that was said.
	if (IsWaitingOnInput)
	{
		LastThingPlayerSaid = inputWidget->InputText.ToString();
		// If the user cancelled, exit.
		if (inputWidget->CancelButtonPressed)
		{
			LastThingPlayerSaid = "";
			IsWaitingOnInput = false;
		}
		// If the user told us to say something, then say it.
		if (inputWidget->SayButtonPressed)
		{
			IsWaitingOnInput = false;
		}

		// Turn off the input widget.
		if (!IsWaitingOnInput)
		{
			APlayerController* player = GetWorld()->GetFirstPlayerController();
			if (player)
			{
				player->SetInputMode(FInputModeGameOnly());
			}
			inputWidget->SetVisibility(ESlateVisibility::Hidden);
		}
	}

}


ABartlebyRoom* ABartlebySystem::GetRoomOrNull(const FString& id)
{
	// Use lower case.
	FString lower = id.ToLower();
	for (ABartlebyRoom* room : Rooms)
	{
		// The AI sometimes abbreviates room names, so abbreviations are valid?
		if (room && room->Id == id || room->Id.ToLower().Contains(lower))
		{
			return room;
		}
	}
	return nullptr;
}


TArray<FDoor> ABartlebySystem::GetDoorsAt(const FString& roomId)
{
	TArray<FDoor> out;
	// Any door connecting this room to another will be added.
	for (const FDoor& door : Doors)
	{
		if (door.Room1 == roomId || door.Room2 == roomId)
		{
			out.Add(door);
		}
	}
	return out;
}


ABartlebyRoom* ABartlebySystem::GetRoomAtOrNull(const FVector& pos)
{
	// Get the first room containing this position.
	for (ABartlebyRoom* room : Rooms)
	{
		if (room->IsInside(pos))
		{
			return room;
		}
	}
	return nullptr;
}

void ABartlebySystem::Say(AActor* actor, const FString& title, const FString& text)
{
	// Implement in your game.
	UE_LOG(LogTemp, Display, TEXT("Implement this part in your game."));
	OnSay(actor, title, text);
	OnSayCompleted();
}

void ABartlebySystem::OnSayCompleted()
{
	CollectInput();
}

TArray<UBartlebyObject*> ABartlebySystem::GetObjectsAt(const FString& roomId)
{
	TArray<UBartlebyObject*> objects;
	auto room = GetRoomOrNull(roomId);
	if (!room)
	{
		return objects;
	}
	return room->Objects;
}

FString ABartlebySystem::GenerateYouSeeString()
{
	if (!Controller)
	{
		UE_LOG(LogTemp, Error, TEXT("NO controller"));
		return "";
	}
	// Get any nearby objects.
	TArray<UBartlebyObject*> objects = GetObjectsAt(Controller->CurrentRoom->Id);
	FVector pos = Controller->GetCharacter()->GetActorLocation();
	// Sort the objects by distance to the AI.
	Algo::Sort(objects, [&pos](const UBartlebyObject* a, const UBartlebyObject* b)
		{
			return FVector::Dist(a->GetOwner()->GetActorLocation(), pos) < FVector::Dist(b->GetOwner()->GetActorLocation(), pos);
		});
	// No objects, empty list.
	if (objects.Num() == 0)
	{
		return "[]";
	}

	//  Make a formatted list of objects.
	FString S = "[";
	for (int32 i = 0; i < objects.Num(); i++)
	{
		S += objects[i]->Id;
		if (i < objects.Num() - 1)
		{
			S += ",";
		}
	}
	S += "]";
	return S;
	
}

FString ABartlebySystem::GenerateDoorsString()
{
	if (!Controller)
	{
		UE_LOG(LogTemp, Error, TEXT("NO controller"));
		return "";
	}
	// Get all the doors as a list.
	const auto& roomId = Controller->CurrentRoom->Id;
	auto doors = GetDoorsAt(roomId);

	// No doors, empty list.
	if (doors.Num() == 0)
	{
		return "[]";
	}

	// Formatted list of doors.
	FString S = "[";
	for (int32 i = 0; i < doors.Num(); i++)
	{
		if (doors[i].Room1 == roomId)
		{
			S += doors[i].Room2;
		}
		else
		{
			S += doors[i].Room1;
		}
		if (i < doors.Num() - 1)
		{
			S += ",";
		}
	}
	S += "]";
	return S;
}

FString ABartlebySystem::GenerateRecentPlacesString()
{
	if (!Controller)
	{
		UE_LOG(LogTemp, Error, TEXT("NO controller"));
		return "";
	}
	// Get the list of recent places, make an empty or formatted list of them.
	auto places = Controller->RecentPlaces;
	if (places.Num() == 0)
	{
		return "[]";
	}

	FString S = "[";
	for (int32 i = 0; i < places.Num(); i++)
	{
		S += places[i];
		if (i < places.Num() - 1)
		{
			S += ",";
		}
	}
	S += "]";
	return S;
}

FString ABartlebySystem::GenerateStatusString()
{
	if (!Controller)
	{
		UE_LOG(LogTemp, Error, TEXT("NO controller"));
		return "";
	}
	FString guestString;
	if (LastThingPlayerSaid != "")
	{
		guestString = GuestSaidPrompt + " \"" + LastThingPlayerSaid + "\"";
	}
	// Big ol' concatenation.
	return "You are in room_id=\"" + Controller->CurrentRoom->Id + "\"."
		"\nroom_description=\"" + Controller->CurrentRoom->Description + "\""
		"\nnearby_object_ids=" + GenerateYouSeeString() +
		"\nadjacent_rooms=" + GenerateDoorsString() +
		"\nrecent_rooms=" + GenerateRecentPlacesString() + 
		"\n" + SeeGuestPrompt + guestString;
}

FString ABartlebySystem::GeneratePrompt(bool askForHelp)
{
	FString helpString;
	if (askForHelp)
	{
		helpString = GenerateHelpString() + "\n";
	}
	// Big ol' concatenation.
	return GroundingPrompt +
		helpString +
		"STATUS:\n" + GenerateStatusString() + "\n" +
		"Enter exactly one action now:\n"; // This is super important for making the AI actually emit just one action.
}

FString ABartlebySystem::GenerateHelpString()
{
	return HelpPrompt;
}

void ABartlebySystem::StartOpenAICall()
{
	if (!IsEnabled)
	{
		return;
	}

	// Set up our HTTP request.
	FHttpModule& httpModule = FHttpModule::Get();
	FHttpRequestRef request = httpModule.CreateRequest();
	request->SetURL(URL);
	request->SetVerb(TEXT("POST"));
	request->SetHeader(TEXT("Content-type"), TEXT("application/json"));
	request->SetHeader(TEXT("Authorization"), TEXT("Bearer " + OpenAiKey));
	// Create a new JSON object
	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);

	// Add the model name, messages, and temperature to the object
	JsonObject->SetStringField(TEXT("model"), Model);

	LastFullPrompt = "";
	if (!appendedMsg.IsEmpty())
	{
		AddLog(appendedMsg);
		LastFullPrompt += appendedMsg + "\n";
		appendedMsg = "";
	}
	FString nextPrompt = GeneratePrompt(false);
	LastFullPrompt += nextPrompt;
	AddLog(nextPrompt);
	NeedsHelpString = false; // TODO, when the AI fails, give it another help string?
	TArray<TSharedPtr<FJsonValue>> MessagesArray;
	// Construct the messages array.
	{
		// Always generate the help string.
		TSharedPtr<FJsonObject> MessageObject = MakeShareable(new FJsonObject);
		MessageObject->SetStringField(TEXT("role"), TEXT("user"));
		MessageObject->SetStringField(TEXT("content"), GenerateHelpString());
		MessagesArray.Add(MakeShareable(new FJsonValueObject(MessageObject)));
	}
	// Add a bunch of messages.
	for (const auto& log_element : Log)
	{
		TSharedPtr<FJsonObject> MessageObject = MakeShareable(new FJsonObject);
		if (log_element.Type == BartlebyLogType::Prompt)
		{
			MessageObject->SetStringField(TEXT("role"), TEXT("user"));
		}
		else
		{
			MessageObject->SetStringField(TEXT("role"), TEXT("assistant"));
		}
		MessageObject->SetStringField(TEXT("content"), log_element.Content);
		MessagesArray.Add(MakeShareable(new FJsonValueObject(MessageObject)));
	}
	JsonObject->SetArrayField(TEXT("messages"), MessagesArray);

	JsonObject->SetNumberField(TEXT("temperature"), Temperature);

	// Convert the JSON object to a string.
	FString JsonString;
	TSharedRef<TJsonWriter<TCHAR>> JsonWriter = TJsonWriterFactory<>::Create(&JsonString);
	FJsonSerializer::Serialize(JsonObject.ToSharedRef(), JsonWriter);
	request->SetContentAsString(JsonString);
	IsWaitingOnOpenAI = true;
	auto completionCallback = [&](
		FHttpRequestPtr pRequest,
		FHttpResponsePtr pResponse,
		bool connectedSuccessfully)
	{
		IsWaitingOnOpenAI = false;
		if (connectedSuccessfully) {

			// We should have a JSON response - attempt to process it.
			IsWaitingOnOpenAI = false;
			// Validate http called us back on the Game Thread...
			check(IsInGameThread());
			UE_LOG(LogTemp, Display, TEXT("%s"), *(pResponse->GetContentAsString()));
			TSharedRef<TJsonReader<TCHAR>> JsonReader = 
				TJsonReaderFactory<TCHAR>::Create(pResponse->GetContentAsString());
			TSharedPtr<FJsonObject> jsonObject;
			bool wasParsingError = false;
			// Deserialize the json into an object.
			if (FJsonSerializer::Deserialize(JsonReader, jsonObject))
			{
				// If that was successful..
				if (jsonObject)
				{
					// AI should have generated a list of "choices". We just want the first one.
					if (jsonObject->HasField("choices"))
					{
						TArray<TSharedPtr<FJsonValue>> ChoicesArray = jsonObject->GetArrayField("choices");
						if (ChoicesArray.Num() > 0)
						{
							TSharedPtr<FJsonObject> ChoiceObject = ChoicesArray[0]->AsObject();
							// We got an object, figure out what the AI said.
							if (ChoiceObject)
							{
								TSharedPtr<FJsonObject> MessageObject = ChoiceObject->GetObjectField("message");
								FString Role = MessageObject->GetStringField("role");
								FString Content = MessageObject->GetStringField("content");
								LastThingOpenAISaid = Content;
								TArray<FString> Lines;
								// AI sometimes says a lot of things. Infer each line to be exactly one command and ignore
								// all but the first.
								LastThingOpenAISaid.ParseIntoArrayLines(Lines, true);
								if (Lines.Num() > 0)
								{
									LastThingOpenAISaid = Lines[0];
								}
								Log.push_back(BartlebyLogElement{ BartlebyLogType::Output, LastThingOpenAISaid });
							}
						}
						else
						{
							wasParsingError = true;
						}
					}
					else
					{
						wasParsingError = true;
					}
				}
				else
				{
					wasParsingError = true;
				}
				
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("Failed to deserialize from json reader!"));
				wasParsingError = true;
			}

			if (wasParsingError)
			{
				UE_LOG(LogTemp, Warning, TEXT("Clearing the log, openAI failed."));
				Log.clear();
			}
		}
		else 
		{
			// Sometimes the internet fails us.
			switch (pRequest->GetStatus()) 
			{
			case EHttpRequestStatus::Failed_ConnectionError:
				UE_LOG(LogTemp, Error, TEXT("Connection failed."));
				break;
			default:
				UE_LOG(LogTemp, Error, TEXT("Request failed."));
				break;
			}
		}
		};
	// When done, the completion callback will be called.
	request->OnProcessRequestComplete().BindLambda(completionCallback);

	// Finally, submit the request for processing
	request->ProcessRequest();
}

void ABartlebySystem::AppendMsg(const FString& append)
{
	appendedMsg += append;
}

void ABartlebySystem::AddLog(const FString& log)
{
	Log.push_back(BartlebyLogElement{ BartlebyLogType::Prompt, log });
	// Remove the first element whenever we have too many!
	if (Log.size() > MaxNumLogElements)
	{
		Log.pop_front();
	}
}