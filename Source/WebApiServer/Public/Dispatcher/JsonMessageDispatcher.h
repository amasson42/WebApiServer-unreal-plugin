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

DECLARE_DYNAMIC_DELEGATE_FiveParams(FJsonRpcRequestHandlerDelegate, FJsonObjectWrapper, Params, FJsonObjectWrapper&, Result, EJsonObjectWrapperType&, ResultType, FString&, Error, bool&, Success);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FJsonRpcRequestHandlerAsyncDelegate, FJsonObjectWrapper, Params, UJsonPromise*, Promise);

typedef TFunction<void (const TSharedPtr<FJsonValue>&)> FJsonRpcRequestCompletionCallback;
typedef TFunction<void (const FString&)> FJsonRpcRequestErrorCallback;
typedef TFunction<TSharedPtr<FJsonValue> (const TSharedPtr<FJsonValue>&)> FJsonRpcRequestHandlerLambda;
typedef TFunction<TSharedPtr<FJsonValue> (const TArray<TSharedPtr<FJsonValue>>&)> FJsonRpcRequestHandlerStructuredArrayLambda;
typedef TFunction<TSharedPtr<FJsonValue> (const TSharedPtr<FJsonObject>&)> FJsonRpcRequestHandlerStructuredObjectLambda;

USTRUCT()
struct FJsonRpcRequestHandler
{
    GENERATED_BODY()

    UPROPERTY()
    TWeakObjectPtr<UObject> Owner;

    TFunction<void (const TSharedPtr<FJsonValue>&, const FJsonRpcRequestCompletionCallback&, const FJsonRpcRequestErrorCallback&)> Action;

};

/* Notification Handlers */

DECLARE_DYNAMIC_DELEGATE_OneParam(FJsonRpcNotificationHandlerDelegate, FJsonObjectWrapper, Params);

typedef TFunction<void(const TSharedPtr<FJsonValue>&)> FJsonRpcNotificationHandlerLambda;
typedef TFunction<void(const TArray<TSharedPtr<FJsonValue>>&)> FJsonRpcNotificationHandlerStructuredArrayLambda;
typedef TFunction<void(const TSharedPtr<FJsonObject>&)> FJsonRpcNotificationHandlerStructuredObjectLambda;


USTRUCT()
struct FJsonRpcNotificationHandler
{
    GENERATED_BODY()

    UPROPERTY()
    TWeakObjectPtr<UObject> Owner;

    FJsonRpcNotificationHandlerLambda Action;
};

/* Response Handlers */

DECLARE_DYNAMIC_DELEGATE_ThreeParams(FJsonRpcResponseHandlerDelegate, bool, bSuccess, const FJsonObjectWrapper&, Result, const FString&, Error);
DECLARE_DYNAMIC_DELEGATE_OneParam(FJsonRpcResponseSuccessHandlerDelegate, const FJsonObjectWrapper&, Result);
DECLARE_DYNAMIC_DELEGATE_OneParam(FJsonRpcResponseFailureHandlerDelegate, const FString&, Error);

typedef TFunction<void(bool, const TSharedPtr<FJsonValue>&, const FString&)> FJsonRpcResponseHandlerLambda;
typedef TFunction<void(const TSharedPtr<FJsonValue>&)> FJsonRpcResponseSuccessHandlerLambda;
typedef TFunction<void(const FString&)> FJsonRpcResponseFailureHandlerLambda;

USTRUCT()
struct FJsonRpcResponseHandler
{
    GENERATED_BODY()

    FJsonRpcResponseHandlerLambda CompletionHandler;
    
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
    bool RegisterRequestHandler(const FString& Method, const FJsonRpcRequestHandlerDelegate& Handler, UObject* Owner = nullptr, bool bOverride = false);

    bool RegisterRequestHandler(const FString& Method, const FJsonRpcRequestHandlerLambda& Handler, UObject* Owner = nullptr, bool bOverride = false);

    bool RegisterRequestHandler(const FString& Method, const TArray<EJson>& ExpectedTypes, const FJsonRpcRequestHandlerStructuredArrayLambda& Handler, UObject* Owner = nullptr, bool bOverride = false);

    bool RegisterRequestHandler(const FString& Method, const TMap<FString, EJson>& ExpectedTypes, const FJsonRpcRequestHandlerStructuredObjectLambda& Handler, UObject* Owner = nullptr, bool bOverride = false);

    UFUNCTION(BlueprintCallable, Category = "Handler|Request", meta = (DefaultToSelf = "Owner"))
    bool RegisterRequestAsyncHandler(const FString& Method, const FJsonRpcRequestHandlerAsyncDelegate& Handler, UObject* Owner = nullptr, bool bOverride = false);

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
    void RegisterNotificationHandler(const FString& Method, const FJsonRpcNotificationHandlerDelegate& Handler, UObject* Owner = nullptr);

    void RegisterNotificationHandler(const FString& Method, const FJsonRpcNotificationHandlerLambda& Handler, UObject* Owner = nullptr);

    void RegisterNotificationHandler(const FString& Method, const TArray<EJson>& ExpectedTypes, const FJsonRpcNotificationHandlerStructuredArrayLambda& Handler, UObject* Owner = nullptr);

    void RegisterNotificationHandler(const FString& Method, const TMap<FString, EJson>& ExpectedTypes, const FJsonRpcNotificationHandlerStructuredObjectLambda& Handler, UObject* Owner = nullptr);

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
    void SendRequestWithCompletion(const TScriptInterface<IMessageSender>& MessageSender, const FString& Method, const FJsonObjectWrapper& Params, EJsonObjectWrapperType ParamsType, const FJsonRpcResponseHandlerDelegate& CompletionHandler, float Timeout = 5.0f);

    UFUNCTION(BlueprintCallable, Category = "Send|Request")
    void SendRequestWithSuccessOrFailure(const TScriptInterface<IMessageSender>& MessageSender, const FString& Method, const FJsonObjectWrapper& Params, EJsonObjectWrapperType ParamsType, const FJsonRpcResponseSuccessHandlerDelegate& SuccessHandler, const FJsonRpcResponseFailureHandlerDelegate& FailureHandler, float Timeout = 5.0f);

    UFUNCTION(BlueprintCallable, Category = "Send|Request")
    void SendRequestWithPromise(const TScriptInterface<IMessageSender>& MessageSender, const FString& Method, const FJsonObjectWrapper& Params, EJsonObjectWrapperType ParamsType, UJsonPromise* Promise, float Timeout = 5.0f);

    void SendRequest(const TScriptInterface<IMessageSender>& MessageSender, const FString& Method, const TSharedPtr<FJsonValue>& Params, const FJsonRpcResponseHandlerLambda& CompletionHandler, float Timeout = 5.0f);

    void SendRequest(const TScriptInterface<IMessageSender>& MessageSender, const FString& Method, const TSharedPtr<FJsonValue>& Params, const FJsonRpcResponseSuccessHandlerLambda& SuccessHandler, const FJsonRpcResponseFailureHandlerLambda& FailureHandler, float Timeout = 5.0f);

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

    TMap<FString, TSharedPtr<FJsonRpcRequestHandler>> RequestHandlers;
    TMap<FString, TArray<TSharedPtr<FJsonRpcNotificationHandler>>> NotificationHandlers;
    TMap<int32, TSharedPtr<FJsonRpcResponseHandler>> ResponseHandlers;
    int32 LastRequestId = 0;

};
