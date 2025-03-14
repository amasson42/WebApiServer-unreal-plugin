// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Serialization/JsonTypes.h"
#include "JsonObjectWrapper.h"
#include "Messaging/MessageSender.h"
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

    /** Register a request handler. Request handlers are unique per methods. */
    UFUNCTION(BlueprintCallable, Category = "Handler|Request")
    bool RegisterRequestHandler(const FString& Method, const FJsonRequestHandler& Handler, bool bOverride = false);

    /** Check if a request handler is registered */
    UFUNCTION(BlueprintCallable, Category = "Handler|Request")
    bool IsRequestHandlerRegistered(const FString& Method, const FJsonRequestHandler& Handler) const;

    /** Unregister a request handler */
    UFUNCTION(BlueprintCallable, Category = "Handler|Request")
    bool UnregisterRequestHandler(const FString& Method, const FJsonRequestHandler& Handler);

    /** Register a notification callback. Notification callbacks are not unique per methods. */
    UFUNCTION(BlueprintCallable, Category = "Handler|Notification")
    void RegisterNotificationCallback(const FString& Method, const FJsonNotificationCallback& Callback);

    /** Check if a notification callback is registered */
    UFUNCTION(BlueprintCallable, Category = "Handler|Notification")
    bool IsNotificationCallbackRegistered(const FString& Method, const FJsonNotificationCallback& Callback) const;

    /** Unregister a notification h */
    UFUNCTION(BlueprintCallable, Category = "Handler|Notification")
    void UnregisterNotificationCallback(const FString& Method, const FJsonNotificationCallback& Callback);


    /** Message handling */

    UFUNCTION(BlueprintCallable) // TODO: Create MessageSender and add it as parameter to be used for returning result
    void HandleMessage(const FString& Message, TScriptInterface<IMessageSender> MessageSender);

    UFUNCTION(BlueprintCallable)
    void HandleJsonMessage(const FJsonObjectWrapper& JsonMessage, TScriptInterface<IMessageSender> MessageSender);

private:

    UPROPERTY()
    TMap<FString, FJsonRequestHandler> RequestHandlers;

    UPROPERTY()
    TMap<FString, FJsonNotificationCallback> NotificationCallbacks;

};
