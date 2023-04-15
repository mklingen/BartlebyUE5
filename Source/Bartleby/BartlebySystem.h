// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <deque>

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BartlebySystem.generated.h"

class UBartlebyInput;
DECLARE_DELEGATE_OneParam(FOnOpenAICompleteDelegate, const FString&);

// Connects two rooms.
USTRUCT(Blueprintable)
struct FDoor {
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		FString Room1;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		FString Room2;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		FString Description = "door";
};
class ABartlebyRoom;

// Implements the Bartleby system. Keeps track of a single AI, a collection of rooms, and a collection of objects.
UCLASS()
class BARTLEBY_API ABartlebySystem : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABartlebySystem();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	class ABartlebyController* Controller = nullptr;
	
	// Causes the actor to say the given text, with the given title text.
	UFUNCTION(BlueprintCallable)
		void Say(AActor* actor, const FString& title, const FString& text);

	// Implement this callback to make your actor say something.
	UFUNCTION(BlueprintImplementableEvent)
		void OnSay(AActor* actor, const FString& title, const FString& text);

	// Called to collect input from the UI widget.
	UFUNCTION()
		void CollectInput();

	// List of rooms the system knows about.
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Bartleby")
		TArray<ABartlebyRoom*> Rooms;

	// List of doors the system knows about.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Bartleby")
		TArray<FDoor> Doors;

	// Gets the room with the given ID, or null otherwise.
	UFUNCTION(BlueprintCallable)
		ABartlebyRoom* GetRoomOrNull(const FString& id);

	// Gets the room containing the given position, or null otherwise.
	UFUNCTION(BlueprintCallable)
		ABartlebyRoom* GetRoomAtOrNull(const FVector& pos);

	// Gets the doors adjacent to the given room ID.
	UFUNCTION(BlueprintCallable)
		TArray<FDoor> GetDoorsAt(const FString& roomId);

	// Gets the objects in the given room ID.
	UFUNCTION(BlueprintCallable)
		TArray<class UBartlebyObject*> GetObjectsAt(const FString& roomId);

	// Prototype class for the input UI widget to use for player input.
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		TSubclassOf<UBartlebyInput> InputWidgetClass;

	// If true, the system is waiting on user input.
	UPROPERTY(BlueprintReadWrite, VisibleInstanceOnly)
		bool IsWaitingOnInput = false;

	// If true, the system is waiting on OpenAI to say something.
	UPROPERTY(BlueprintReadWrite, VisibleInstanceOnly)
		bool IsWaitingOnOpenAI = false;

	// Stores the last thing that the OpenAI system returned.
	UPROPERTY(BlueprintReadWrite, VisibleInstanceOnly)
		FString LastThingOpenAISaid = "";

	// Stores the last thing that the player inputted.
	UPROPERTY(BlueprintReadWrite, VisibleInstanceOnly)
		FString LastThingPlayerSaid;

	// Called when the "Say" function is done.
	UFUNCTION()
		void OnSayCompleted();

	// If true, the system needs to output the help string to the AI.
	UPROPERTY()
		bool NeedsHelpString = true;

	// If true, the API to OpenAI is enabled.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "API")
		bool IsEnabled = true;

	// The default "help" prompt to give the AI. This is triggered once per session, and is always at
	// the front of the message queue.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Prompt")
		FString HelpPrompt = "BARTLEBY API:\n"
		"* say(Phrase) # says the given phrase to the guest. Keep phrases short and pithy.\n"
		"Example:\nsay(hello I am Bartleby)\n"
		"* go(Room_ID) # goes to the room from the current room.\n"
		"Example:\ngo(entry_hall)\n"
		"* examine(Object_ID) # examines the object in the room. It's important to examine something before making things up.\n"
		"Example:\nexamine(sunglasses)\n"
		"* think(Thought) # causes Bartleby to think something.\n"
		"Example:\nthink(I must tell a compelling story to this guest!)";

	// The prompt to give to the AI on every single iteration.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Prompt")
		FString GroundingPrompt = "INFO:\n"
		"You control Bartleby, a helpful and erudite british tour guide. "
		"Guide guests through the museum and make up a compelling story about what is in it.\n"
		"Do this using the Bartleby API, one action at a time.\n";

	// Tells the AI that a player is near.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Prompt")
		FString SeeGuestPrompt = "A guest is here. ";

	// Tells the AI what the player said.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Prompt")
		FString GuestSaidPrompt = "The guest said:";

	// URL to the AI.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "API")
		FString URL = "https://api.openai.com/v1/chat/completions";

	// Your secret private API key. Be warned that this costs money.
	// Get one here: https://platform.openai.com/account/api-keys
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "API")
		FString OpenAiKey = "ENTER_YOUR_OPENAI_KEY_HERE";

	// Stores the last prompt that was given to the AI.
	UPROPERTY(BlueprintReadWrite, VisibleInstanceOnly, Category = "Prompt")
		FString LastFullPrompt = "";

	// Temperature parameter. Higher numbers are noisier and funnier. Lower numbers are more predictable and likely helpful.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "API")
		double Temperature = 0.4;

	// The actual model to use. gpt-3.5-turbo is the default ChatGPT.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "API")
		FString Model = "gpt-3.5-turbo";

	// Kicks off a call to the OpenAI system in the background.
	void StartOpenAICall();

	// Type of data to send to OpenAI.
	enum class BartlebyLogType
	{
		Prompt, // Prompt to the AI.
		Output // Re-submits whatever the AI said.
	};
	// The Chat API involves sending a series of "log elements", which are either things the user said, or things the
	// AI said.
	struct BartlebyLogElement
	{
		BartlebyLogType Type;
		FString Content;
	};
	// Keep around this many log elements as "memory". Can't be much higher, because of the token limit of ChatGPT.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "API")
		int32 MaxNumLogElements = 8;

	// Appends the given user message to the list of messages. This is used for feedback for actions, for example. Exactly
	// one message will be generated by these, so this is just string concat.
	UFUNCTION(BlueprintCallable)
		void AppendMsg(const FString& lastMsg);

private:
	// Creates the "Help" text that is sent to the AI.
	FString GenerateHelpString();
	// Creates the "status" text that is sent to the AI.
	FString GenerateStatusString();
	// Generates a prompt to send to the AI.
	FString GeneratePrompt(bool askForHelp);
	// Generates a list of things for the AI to see.
	FString GenerateYouSeeString();
	// Generates a list of nearby doors.
	FString GenerateDoorsString();
	// Generates a list of recent places the AI has been.
	FString GenerateRecentPlacesString();
	// Adds the given log message to the list.
	void AddLog(const FString& log);
	// A circular buffer of log messages. Log messages are removed from the front of the list
	// when we exceed MaxNumLogElements.
	std::deque<BartlebyLogElement> Log;
	// List of recent things the AI was thinking.
	TArray<FString> Thoughts;
	// Current dump of strings that we are going to send the AI on the next iteartion.
	FString appendedMsg;
private:
	// Pointer to the input widget that the user will see.
	UPROPERTY()
		UBartlebyInput* inputWidget = nullptr;
};
