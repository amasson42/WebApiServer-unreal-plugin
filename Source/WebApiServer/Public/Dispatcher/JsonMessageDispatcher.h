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
#define JSONRPC_ID_MAX 10000000

class UJsonPromise;

/* Request Handlers */

DECLARE_DYNAMIC_DELEGATE_FiveParams(FJsonRequestHandlerDelegate, FJsonObjectWrapper, Params, FJsonObjectWrapper&, Result, EJsonObjectWrapperType&, ResultType, FString&, Error, bool&, Success);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FJsonRequestHandlerAsyncDelegate, FJsonObjectWrapper, Params, UJsonPromise*, Promise);

typedef TFunction<void (const TSharedPtr<FJsonValue>&)> TJsonRequestCompletionCallback;
typedef TFunction<void (const FString&)> TJsonRequestErrorCallback;
typedef TFunction<TSharedPtr<FJsonValue> (const TSharedPtr<FJsonValue>&)> TJsonRequestHandlerLambda;
typedef TFunction<TSharedPtr<FJsonValue> (const TArray<TSharedPtr<FJsonValue>>&)> TJsonRequestHandlerStructuredArrayLambda;

USTRUCT()
struct FJsonRequestHandler
{
    GENERATED_BODY()

    UPROPERTY()
    TWeakObjectPtr<UObject> Owner;

    TFunction<void (const TSharedPtr<FJsonValue>&, const TJsonRequestCompletionCallback&, const TJsonRequestErrorCallback&)> Action;

};

/* Notification Handlers */

DECLARE_DYNAMIC_DELEGATE_OneParam(FJsonNotificationHandlerDelegate, FJsonObjectWrapper, Params);

typedef TFunction<void(const TSharedPtr<FJsonValue>&)> TJsonNotificationHandlerLambda;
typedef TFunction<void(const TArray<TSharedPtr<FJsonValue>>&)> TJsonNotificationHandlerStructuredArrayLambda;

USTRUCT()
struct FJsonNotificationHandler
{
    GENERATED_BODY()

    UPROPERTY()
    TWeakObjectPtr<UObject> Owner;

    TJsonNotificationHandlerLambda Action;
};

/* Response Handlers */

DECLARE_DYNAMIC_DELEGATE_ThreeParams(FJsonResponseHandlerDelegate, bool, bSuccess, const FJsonObjectWrapper&, Result, const FString&, Error);
DECLARE_DYNAMIC_DELEGATE_OneParam(FJsonResponseSuccessHandlerDelegate, const FJsonObjectWrapper&, Result);
DECLARE_DYNAMIC_DELEGATE_OneParam(FJsonResponseFailureHandlerDelegate, const FString&, Error);

typedef TFunction<void(bool, const TSharedPtr<FJsonValue>&, const FString&)> TJsonResponseHandlerLambda;
typedef TFunction<void(const TSharedPtr<FJsonValue>&)> TJsonResponseSuccessHandlerLambda;
typedef TFunction<void(const FString&)> TJsonResponseFailureHandlerLambda;

USTRUCT()
struct FJsonResponseHandler
{
    GENERATED_BODY()

    TJsonResponseHandlerLambda CompletionHandler;
    
    FDateTime Timeout;
};

/**
 * 
 */
UCLASS(BlueprintType)
class WEBAPISERVER_API UJsonMessageDispatcher : public UObject
{
	GENERATED_BODY()

public:

    /** Register a request handler. Request handlers are unique per methods. */
    UFUNCTION(BlueprintCallable, Category = "Handler|Request", meta = (DefaultToSelf = "Identifier"))
    bool RegisterRequestHandler(const FString& Method, const FJsonRequestHandlerDelegate& Handler, UObject* Owner = nullptr, bool bOverride = false);

    bool RegisterRequestHandler(const FString& Method, const TJsonRequestHandlerLambda& Handler, UObject* Owner = nullptr, bool bOverride = false);

    bool RegisterRequestHandler(const FString& Method, const TArray<EJson>& ExpectedTypes, const TJsonRequestHandlerStructuredArrayLambda& Handler, UObject* Owner = nullptr, bool bOverride = false);

    UFUNCTION(BlueprintCallable, Category = "Handler|Request", meta = (DefaultToSelf = "Owner"))
    bool RegisterRequestAsyncHandler(const FString& Method, const FJsonRequestHandlerAsyncDelegate& Handler, UObject* Owner = nullptr, bool bOverride = false);

    /** Check if a request handler is registered */
    UFUNCTION(BlueprintCallable, Category = "Handler|Request")
    bool IsRequestHandlerRegistered(const FString& Method, UObject* Owner = nullptr) const;

    /** Unregister a request handler */
    UFUNCTION(BlueprintCallable, Category = "Handler|Request")
    bool UnregisterRequestHandler(const FString& Method, UObject* Owner = nullptr);

    UFUNCTION(BlueprintCallable, Category = "Handler|Request")
    void UnregisterRequestHandlersFromOwner(UObject* Owner);

    /** Register a notification callback. Notification callbacks are not unique per methods. */
    UFUNCTION(BlueprintCallable, Category = "Handler|Notification")
    void RegisterNotificationHandler(const FString& Method, const FJsonNotificationHandlerDelegate& Handler, UObject* Owner = nullptr);

    void RegisterNotificationHandler(const FString& Method, const TJsonNotificationHandlerLambda& Handler, UObject* Owner = nullptr);

    void RegisterNotificationHandler(const FString& Method, const TArray<EJson>& ExpectedTypes, const TJsonNotificationHandlerStructuredArrayLambda& Handler, UObject* Owner = nullptr);

    /** Check if a notification handler is registered */
    UFUNCTION(BlueprintCallable, Category = "Handler|Notification")
    bool IsNotificationHandlerRegistered(const FString& Method, UObject* Owner = nullptr) const;

    /** Unregister a notification handler */
    UFUNCTION(BlueprintCallable, Category = "Handler|Notification")
    void UnregisterNotificationHandler(const FString& Method, UObject* Owner = nullptr);

    UFUNCTION(BlueprintCallable, Category = "Handler|Notification")
    void UnregisterNotificationHandlersFromOwner(UObject* Owner);

    /** Handlers */
    
    UFUNCTION(BlueprintCallable, Category = "Handler")
    void UnregisterHandlersFromOwner(UObject* Owner);

    /** Send Messages */

    UFUNCTION(BlueprintCallable, Category = "Send|Request")
    void SendRequestWithCompletion(const TScriptInterface<IMessageSender>& MessageSender, const FString& Method, const FJsonObjectWrapper& Params, EJsonObjectWrapperType ParamsType, const FJsonResponseHandlerDelegate& CompletionHandler, float Timeout = 5.0f);

    UFUNCTION(BlueprintCallable, Category = "Send|Request")
    void SendRequestWithSuccessOrFailure(const TScriptInterface<IMessageSender>& MessageSender, const FString& Method, const FJsonObjectWrapper& Params, EJsonObjectWrapperType ParamsType, const FJsonResponseSuccessHandlerDelegate& SuccessHandler, const FJsonResponseFailureHandlerDelegate& FailureHandler, float Timeout = 5.0f);

    UFUNCTION(BlueprintCallable, Category = "Send|Request")
    void SendRequestWithPromise(const TScriptInterface<IMessageSender>& MessageSender, const FString& Method, const FJsonObjectWrapper& Params, EJsonObjectWrapperType ParamsType, UJsonPromise* Promise, float Timeout = 5.0f);

    void SendRequest(const TScriptInterface<IMessageSender>& MessageSender, const FString& Method, const TSharedPtr<FJsonValue>& Params, const TJsonResponseHandlerLambda& CompletionHandler, float Timeout = 5.0f);

    void SendRequest(const TScriptInterface<IMessageSender>& MessageSender, const FString& Method, const TSharedPtr<FJsonValue>& Params, const TJsonResponseSuccessHandlerLambda& SuccessHandler, const TJsonResponseFailureHandlerLambda& FailureHandler, float Timeout = 5.0f);

    void SendRequest(const TScriptInterface<IMessageSender>& MessageSender, const FString& Method, const TSharedPtr<FJsonValue>& Params, UJsonPromise* Promise, float Timeout = 5.0f);

    UFUNCTION(BlueprintCallable, Category = "Send|Notification")
    void SendNotification(const TScriptInterface<IMessageSender>& MessageSender, const FString& Method, const FJsonObjectWrapper& Params, EJsonObjectWrapperType ParamsType);

    void SendNotification(const TScriptInterface<IMessageSender>& MessageSender, const FString& Method, const TSharedPtr<FJsonValue>& Params);

    /** Message handling */

    UFUNCTION(BlueprintCallable)
    void HandleMessage(const FString& Message, TScriptInterface<IMessageSender> MessageSender);

    UFUNCTION(BlueprintCallable)
    void HandleJsonMessage(const FJsonObjectWrapper& JsonMessage, TScriptInterface<IMessageSender> MessageSender);

    void HandleJsonMessage(const TSharedPtr<FJsonObject>& JsonMessage, TScriptInterface<IMessageSender> MessageSender);

    /** Cleanup */

    UFUNCTION(BlueprintCallable)
    void Cleanup();

private:

    bool HaveValidRequestHandler(const FString& Method) const;

    void HandleRequest(int32 Id, const FString& Method, const TSharedPtr<FJsonValue>& Params, TScriptInterface<IMessageSender> MessageSender);
    void HandleNotification(const FString& Method, const TSharedPtr<FJsonValue>& Params);
    void HandleResponse(int32 Id, const TSharedPtr<FJsonValue>& Result, const TSharedPtr<FJsonValue>& Error);

    TMap<FString, TSharedPtr<FJsonRequestHandler>> RequestHandlers;
    TMap<FString, TArray<TSharedPtr<FJsonNotificationHandler>>> NotificationHandlers;
    TMap<int32, TSharedPtr<FJsonResponseHandler>> ResponseHandlers;
    int32 LastRequestId = 0;

};
