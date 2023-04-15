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
