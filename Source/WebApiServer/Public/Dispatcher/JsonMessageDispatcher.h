// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Serialization/JsonTypes.h"
#include "JsonObjectWrapper.h"
#include "JsonMessageDispatcher.generated.h"

DECLARE_DYNAMIC_DELEGATE_TwoParams(FJsonRequestHandler, FJsonObjectWrapper, Params, FJsonObjectWrapper&, Result);
DECLARE_DYNAMIC_DELEGATE_OneParam(FJsonNotificationCallback, FJsonObjectWrapper, Params);

/**
 * 
 */
UCLASS(BlueprintType)
class WEBAPISERVER_API UJsonMessageDispatcher : public UObject
{
	GENERATED_BODY()

public:

    UFUNCTION(BlueprintCallable)
    void AddRequestHandler(const FString& Method, const FJsonRequestHandler& Handler);

    UFUNCTION(BlueprintCallable)
    void RemoveRequestHandler(const FString& Method, const FJsonRequestHandler& Handler);

    UFUNCTION(BlueprintCallable)
    void AddNotificationCallback(const FString& Method, const FJsonNotificationCallback& Callback);

    UFUNCTION(BlueprintCallable)
    void RemoveNotificationCallback(const FString& Method, const FJsonNotificationCallback& Callback);


    /** Message handling */

    UFUNCTION(BlueprintCallable) // TODO: Create MessageSender and add it as parameter to be used for returning result
    void HandleMessage(const FString& Message);

    UFUNCTION(BlueprintCallable)
    void HandleJsonMessage(const FJsonObjectWrapper& JsonMessage);

private:

    UPROPERTY()
    TMap<FString, FJsonRequestHandler> RequestHandlers;

    UPROPERTY()
    TMap<FString, FJsonNotificationCallback> NotificationCallbacks;

};
