// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "MessageSender.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UMessageSender : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class WEBAPISERVER_API IMessageSender
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Message")
	bool SendMessage(const FString& Message);
	virtual bool SendMessage_Implementation(const FString& Message);

};
