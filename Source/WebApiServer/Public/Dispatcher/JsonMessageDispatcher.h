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
typedef TFunction<void (const TSharedPtr<FJsonValue>&)> TJsonRequestCompletionCallback;
typedef TFunction<void (const FString&)> TJsonRequestErrorCallback;
typedef TFunction<TSharedPtr<FJsonValue> (const TSharedPtr<FJsonValue>&)> TJsonRequestHandlerLambda;
typedef TFunction<TSharedPtr<FJsonValue> (const TArray<TSharedPtr<FJsonValue>>&)> TJsonRequestHandlerStructuredArrayLambda;

USTRUCT()
struct FJsonRequestHandler
{
    GENERATED_BODY()

    virtual ~FJsonRequestHandler() {}

    virtual void HandleRequest(
        const TSharedPtr<FJsonValue>& Param,
        const TJsonRequestCompletionCallback& Completion,
        const TJsonRequestErrorCallback& Error)
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
        const TJsonRequestCompletionCallback& Completion,
        const TJsonRequestErrorCallback& Error
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

/** RequestHandler using Lambda */
USTRUCT()
struct FJsonRequestHandlerWithLambda : public FJsonRequestHandler
{
    GENERATED_BODY()

    virtual ~FJsonRequestHandlerWithLambda() override {}

    virtual void HandleRequest(
        const TSharedPtr<FJsonValue>& Param,
        const TJsonRequestCompletionCallback& Completion,
        const TJsonRequestErrorCallback& Error
    ) override
    {
        try
        {
            TSharedPtr<FJsonValue> Result = Lambda(Param);
            Completion(Result);
        }
        catch (std::exception& e)
        {
            Error(e.what());
        }
    }

    TJsonRequestHandlerLambda Lambda;
};

/** RequestHandler using Lambda from structured params */
USTRUCT()
struct FJsonRequestHandlerWithStructuredArrayLambda : public FJsonRequestHandler
{
    GENERATED_BODY()

    virtual ~FJsonRequestHandlerWithStructuredArrayLambda() override {}

    virtual void HandleRequest(
        const TSharedPtr<FJsonValue>& Param,
        const TJsonRequestCompletionCallback& Completion,
        const TJsonRequestErrorCallback& Error
    ) override;

    TArray<EJson> ExpectedTypes;
    TJsonRequestHandlerStructuredArrayLambda Lambda;
};


typedef TFunction<void(const TSharedPtr<FJsonValue>&)> TJsonNotificationHandlerLambda;
typedef TFunction<void(const TArray<TSharedPtr<FJsonValue>>&)> TJsonNotificationHandlerStructuredArrayLambda;

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

/** RequestHandler using Lambda */
USTRUCT()
struct FJsonNotificationHandlerWithLambda : public FJsonNotificationHandler
{
    GENERATED_BODY()

    virtual ~FJsonNotificationHandlerWithLambda() override {}

    virtual void HandleNotification(const TSharedPtr<FJsonValue>& Param) override
    {
        Lambda(Param);
    }

    TJsonNotificationHandlerLambda Lambda;
};

/** RequestHandler using structured array Lambda */
USTRUCT()
struct FJsonNotificationHandlerWithStructuredArrayLambda : public FJsonNotificationHandler
{
    GENERATED_BODY()

    virtual ~FJsonNotificationHandlerWithStructuredArrayLambda() override {}

    virtual void HandleNotification(const TSharedPtr<FJsonValue>& Param) override;

    TArray<EJson> ExpectedTypes;
    TJsonNotificationHandlerStructuredArrayLambda Lambda;
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
    bool RegisterRequestHandler(const FString& Method, const TJsonRequestHandlerLambda& Handler, bool bOverride = false);
    bool RegisterRequestHandler(const FString& Method, const TArray<EJson>& ExpectedTypes, const TJsonRequestHandlerStructuredArrayLambda& Handler, bool bOverride = false);

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

    UFUNCTION(BlueprintCallable) // TODO: Create MessageSender and add it as parameter to be used for returning result
    void HandleMessage(const FString& Message, TScriptInterface<IMessageSender> MessageSender);

    UFUNCTION(BlueprintCallable)
    void HandleJsonMessage(const FJsonObjectWrapper& JsonMessage, TScriptInterface<IMessageSender> MessageSender);

    void HandleJsonMessage(const TSharedPtr<FJsonObject>& JsonMessage, TScriptInterface<IMessageSender> MessageSender);

private:

    TMap<FString, TSharedPtr<FJsonRequestHandler>> RequestHandlers;
    TMap<FString, TArray<TSharedPtr<FJsonNotificationHandler>>> NotificationHandlers;

};
