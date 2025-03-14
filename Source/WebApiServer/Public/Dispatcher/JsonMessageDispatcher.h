// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Serialization/JsonTypes.h"
#include "JsonObjectWrapper.h"
#include "Messaging/MessageSender.h"
#include "JsonMessageDispatcher.generated.h"

#define JSONRPC_ID "id"
#define JSONRPC_METHOD "method"
#define JSONRPC_PARAMS "params"
#define JSONRPC_RESULT "result"
#define JSONRPC_ERROR "error"

DECLARE_DYNAMIC_DELEGATE_FourParams(FJsonRequestHandlerDelegate, FJsonObjectWrapper, Params, FJsonObjectWrapper&, Result, FString&, Error, bool&, Success);
DECLARE_DYNAMIC_DELEGATE_OneParam(FJsonNotificationHandlerDelegate, FJsonObjectWrapper, Params);

// TODO: Put Handlers structures in private files

/** Abstract RequestHandler class */
typedef TFunction<void (const TSharedPtr<FJsonValue>&)> JsonRequestCompletionCallback;
typedef TFunction<void (const FString&)> JsonRequestErrorCallback;

USTRUCT()
struct FJsonRequestHandler
{
    GENERATED_BODY()

    virtual ~FJsonRequestHandler() {}

    virtual void HandleRequest(
        const TSharedPtr<FJsonValue>& Param,
        const JsonRequestCompletionCallback& Completion,
        const JsonRequestErrorCallback& Error)
    {}
};

/** RequestHandler using Delegate */
USTRUCT()
struct FJsonRequestHandlerWithDelegate : public FJsonRequestHandler
{
    GENERATED_BODY()

    virtual ~FJsonRequestHandlerWithDelegate() override {}

    virtual void HandleRequest(
        const TSharedPtr<FJsonValue>& Param,
        const JsonRequestCompletionCallback& Completion,
        const JsonRequestErrorCallback& Error
    ) override
    {
        FJsonObjectWrapper ParamWrapper;
        if (Param.IsValid())
            ParamWrapper.JsonObject = Param->AsObject();
        FJsonObjectWrapper ResultWrapper;
        FString ErrorMessage;
        bool Success;
        Delegate.ExecuteIfBound(ParamWrapper, ResultWrapper, ErrorMessage, Success);
        if (Success)
            Completion(MakeShared<FJsonValueObject>(ResultWrapper.JsonObject));
        else
            Error(ErrorMessage);
    }

    FJsonRequestHandlerDelegate Delegate;
};


USTRUCT()
struct FJsonNotificationHandler
{
    GENERATED_BODY()

    virtual ~FJsonNotificationHandler() {}

    virtual void HandleNotification(const TSharedPtr<FJsonValue>& Param) {}
};

/** RequestHandler using Delegate */
USTRUCT()
struct FJsonNotificationHandlerWithDelegate : public FJsonNotificationHandler
{
    GENERATED_BODY()

    virtual ~FJsonNotificationHandlerWithDelegate() override {}

    virtual void HandleNotification(const TSharedPtr<FJsonValue>& Param) override
    {
        FJsonObjectWrapper ParamWrapper;
        if (Param.IsValid())
            ParamWrapper.JsonObject = Param->AsObject();
        (void)Delegate.ExecuteIfBound(ParamWrapper);
    }

    FJsonNotificationHandlerDelegate Delegate;
};


/** </Handlers> */

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

    /** Check if a request handler is registered */
    UFUNCTION(BlueprintCallable, Category = "Handler|Request")
    bool IsRequestHandlerRegistered(const FString& Method, const FJsonRequestHandlerDelegate& Handler) const;

    /** Unregister a request handler */
    UFUNCTION(BlueprintCallable, Category = "Handler|Request")
    bool UnregisterRequestHandler(const FString& Method, const FJsonRequestHandlerDelegate& Handler);

    /** Register a notification callback. Notification callbacks are not unique per methods. */
    UFUNCTION(BlueprintCallable, Category = "Handler|Notification")
    void RegisterNotificationCallback(const FString& Method, const FJsonNotificationHandlerDelegate& Callback);

    /** Unregister a notification h */
    UFUNCTION(BlueprintCallable, Category = "Handler|Notification")
    void UnregisterNotificationCallback(const FString& Method, const FJsonNotificationHandlerDelegate& Callback);


    /** Message handling */

    UFUNCTION(BlueprintCallable) // TODO: Create MessageSender and add it as parameter to be used for returning result
    void HandleMessage(const FString& Message, TScriptInterface<IMessageSender> MessageSender);

    UFUNCTION(BlueprintCallable)
    void HandleJsonMessage(const FJsonObjectWrapper& JsonMessage, TScriptInterface<IMessageSender> MessageSender);

    void HandleJsonMessage(const TSharedPtr<FJsonObject>& JsonMessage, TScriptInterface<IMessageSender> MessageSender);

private:

    TMap<FString, TSharedPtr<FJsonRequestHandler>> RequestHandlers;
    TMap<FString, TArray<TSharedPtr<FJsonNotificationHandler>>> NotificationHandlers;

};
