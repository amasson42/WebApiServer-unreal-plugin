// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Dispatcher/JsonMessageDispatcher.h"
#include "JsonHandlers.generated.h"


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
        EJsonObjectWrapperType ResultType = EJsonObjectWrapperType::JOWT_Object;
        FJsonObjectWrapper ResultWrapper;
        FString ErrorMessage;
        bool Success = false;
        Delegate.ExecuteIfBound(ToJsonWrapper(Param), ResultWrapper, ResultType, ErrorMessage, Success);
        if (Success)
            Completion(FromJsonWrapper(ResultWrapper, ResultType));
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

/** RequestHandler using Promise */
USTRUCT()
struct FJsonRequestHandlerWithJsonPromise : public FJsonRequestHandler
{
    GENERATED_BODY()

    virtual ~FJsonRequestHandlerWithJsonPromise() override {}

    virtual void HandleRequest(
        const TSharedPtr<FJsonValue>& Param,
        const TJsonRequestCompletionCallback& Completion,
        const TJsonRequestErrorCallback& Error) override;

    TWeakObjectPtr<UObject> Owner;
    FJsonRequestHandlerAsyncDelegate Delegate;
};


/** Notification Handlers */

USTRUCT()
struct FJsonNotificationHandler
{
    GENERATED_BODY()

    virtual ~FJsonNotificationHandler() {}

    virtual void HandleNotification(const TSharedPtr<FJsonValue>& Param) {}
};

/** NotificationHandler using Delegate */
USTRUCT()
struct FJsonNotificationHandlerWithDelegate : public FJsonNotificationHandler
{
    GENERATED_BODY()

    virtual ~FJsonNotificationHandlerWithDelegate() override {}

    virtual void HandleNotification(const TSharedPtr<FJsonValue>& Param) override
    {
        Delegate.ExecuteIfBound(ToJsonWrapper(Param));
    }

    FJsonNotificationHandlerDelegate Delegate;
};

/** NotificationHandler using Lambda */
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

/** NotificationHandler using structured array Lambda */
USTRUCT()
struct FJsonNotificationHandlerWithStructuredArrayLambda : public FJsonNotificationHandler
{
    GENERATED_BODY()

    virtual ~FJsonNotificationHandlerWithStructuredArrayLambda() override {}

    virtual void HandleNotification(const TSharedPtr<FJsonValue>& Param) override;

    TArray<EJson> ExpectedTypes;
    TJsonNotificationHandlerStructuredArrayLambda Lambda;
};
