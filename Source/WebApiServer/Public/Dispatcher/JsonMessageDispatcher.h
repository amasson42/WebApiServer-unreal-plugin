// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Serialization/JsonTypes.h"
#include "Json/JsonObjectWrapperType.h"
#include "Messaging/MessageSender.h"
#include "JsonMessageDispatcher.generated.h"

#define JSONRPC_ID "id"
#define JSONRPC_METHOD "method"
#define JSONRPC_PARAMS "params"
#define JSONRPC_RESULT "result"
#define JSONRPC_ERROR "error"

class UJsonPromise;

DECLARE_DYNAMIC_DELEGATE_FiveParams(FJsonRequestHandlerDelegate, FJsonObjectWrapper, Params, FJsonObjectWrapper&, Result, EJsonObjectWrapperType&, ResultType, FString&, Error, bool&, Success);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FJsonRequestHandlerAsyncDelegate, FJsonObjectWrapper, Params, UJsonPromise*, Promise);

/** Abstract RequestHandler class */
typedef TFunction<void (const TSharedPtr<FJsonValue>&)> TJsonRequestCompletionCallback;
typedef TFunction<void (const FString&)> TJsonRequestErrorCallback;
typedef TFunction<TSharedPtr<FJsonValue> (const TSharedPtr<FJsonValue>&)> TJsonRequestHandlerLambda;
typedef TFunction<TSharedPtr<FJsonValue> (const TArray<TSharedPtr<FJsonValue>>&)> TJsonRequestHandlerStructuredArrayLambda;

struct FJsonRequestHandler;

DECLARE_DYNAMIC_DELEGATE_OneParam(FJsonNotificationHandlerDelegate, FJsonObjectWrapper, Params);

typedef TFunction<void(const TSharedPtr<FJsonValue>&)> TJsonNotificationHandlerLambda;
typedef TFunction<void(const TArray<TSharedPtr<FJsonValue>>&)> TJsonNotificationHandlerStructuredArrayLambda;

struct FJsonNotificationHandler;

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
    bool RegisterRequestHandler(const FString& Method, const FJsonRequestHandlerDelegate& Handler, bool bOverride = false);
    bool RegisterRequestHandler(const FString& Method, const TJsonRequestHandlerLambda& Handler, bool bOverride = false);
    bool RegisterRequestHandler(const FString& Method, const TArray<EJson>& ExpectedTypes, const TJsonRequestHandlerStructuredArrayLambda& Handler, bool bOverride = false);

    UFUNCTION(BlueprintCallable, Category = "Handler|Request")
    bool RegisterRequestAsyncHandler(const FString& Method, const FJsonRequestHandlerAsyncDelegate& Handler, bool bOverride = false);

    /** Check if a request handler is registered */
    UFUNCTION(BlueprintCallable, Category = "Handler|Request")
    bool IsRequestHandlerRegistered(const FString& Method, const FJsonRequestHandlerDelegate& Handler) const;

    /** Unregister a request handler */
    UFUNCTION(BlueprintCallable, Category = "Handler|Request")
    bool UnregisterRequestHandler(const FString& Method, const FJsonRequestHandlerDelegate& Handler);

    /** Register a notification callback. Notification callbacks are not unique per methods. */
    UFUNCTION(BlueprintCallable, Category = "Handler|Notification")
    void RegisterNotificationCallback(const FString& Method, const FJsonNotificationHandlerDelegate& Callback);
    void RegisterNotificationCallback(const FString& Method, const TJsonNotificationHandlerLambda& Callback);
    void RegisterNotificationCallback(const FString& Method, const TArray<EJson>& ExpectedTypes, const TJsonNotificationHandlerStructuredArrayLambda& Callback);

    /** Unregister a notification h */
    UFUNCTION(BlueprintCallable, Category = "Handler|Notification")
    void UnregisterNotificationCallback(const FString& Method, const FJsonNotificationHandlerDelegate& Callback);


    /** Message handling */

    UFUNCTION(BlueprintCallable)
    void HandleMessage(const FString& Message, TScriptInterface<IMessageSender> MessageSender);

    UFUNCTION(BlueprintCallable)
    void HandleJsonMessage(const FJsonObjectWrapper& JsonMessage, TScriptInterface<IMessageSender> MessageSender);

    void HandleJsonMessage(const TSharedPtr<FJsonObject>& JsonMessage, TScriptInterface<IMessageSender> MessageSender);

private:

    bool HaveValidRequestHandler(const FString& Method) const;

    void HandleRequest(int32 Id, const FString& Method, const TSharedPtr<FJsonValue>& Params, TScriptInterface<IMessageSender> MessageSender);
    void HandleNotification(const FString& Method, const TSharedPtr<FJsonValue>& Params);


    TMap<FString, TSharedPtr<FJsonRequestHandler>> RequestHandlers;
    TMap<FString, TArray<TSharedPtr<FJsonNotificationHandler>>> NotificationHandlers;

};
