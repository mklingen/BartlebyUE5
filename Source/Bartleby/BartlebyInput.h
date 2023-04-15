// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BartlebyInput.generated.h"

UCLASS()
class BARTLEBY_API UBartlebyInput : public UUserWidget
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Text")
		FText InputText;

	UFUNCTION(BlueprintPure, Category = "Widgets|Text")
		FText GetInputText() const;

	UFUNCTION(BlueprintCallable, Category = "Widgets|Text")
		void SetInputText(const FText& text);
	

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
		bool SayButtonPressed = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
		bool CancelButtonPressed = false;
};
