// Fill out your copyright notice in the Description page of Project Settings.


#include "BartlebyInput.h"


FText UBartlebyInput::GetInputText() const
{
	return InputText;
}


void UBartlebyInput::SetInputText(const FText& text)
{
	InputText = text;
}


