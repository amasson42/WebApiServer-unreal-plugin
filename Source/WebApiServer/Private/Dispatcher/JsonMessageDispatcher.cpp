// Fill out your copyright notice in the Description page of Project Settings.


#include "Dispatcher/JsonMessageDispatcher.h"

#include "Async/JsonPromise.h"


bool UJsonMessageDispatcher::HaveValidRequestHandler(const FString& Method) const
{
    auto* CurrentHandlerPtr = RequestHandlers.Find(Method);
    if (CurrentHandlerPtr == nullptr)
        return false;

    return CurrentHandlerPtr->IsValid();
}

bool UJsonMessageDispatcher::RegisterRequestHandler(const FString& Method, const FJsonRequestHandlerDelegate& Handler, UObject* Owner, bool bOverride)
{
    return RegisterRequestHandler(Method, [Handler](const TSharedPtr<FJsonValue>& Params)
    {
        FJsonObjectWrapper Result;
        EJsonObjectWrapperType ResultType = EJsonObjectWrapperType::JOWT_Object;
        FString Error;
        bool bSuccess;
        Handler.ExecuteIfBound(ToJsonWrapper(Params), Result, ResultType, Error, bSuccess);
        return FromJsonWrapper(Result, ResultType);
    }, Owner, bOverride);
}

bool UJsonMessageDispatcher::RegisterRequestHandler(const FString& Method, const TJsonRequestHandlerLambda& Handler, UObject* Owner, bool bOverride)
{
    if (!bOverride && HaveValidRequestHandler(Method))
        return false;

    auto NewHandler = MakeShared<FJsonRequestHandler>();
    NewHandler->Owner = Owner;
    NewHandler->Action = [Handler](const TSharedPtr<FJsonValue>& Param,
        const TJsonRequestCompletionCallback& CompletionCallback,
        const TJsonRequestErrorCallback& FailureCallback)
    {
        try
        {
            TSharedPtr<FJsonValue> Result = Handler(Param);
            CompletionCallback(Result);
        }
        catch (FString& e)
        {
            FailureCallback(e);
        }
        catch (std::exception& e)
        {
            FailureCallback(e.what());
        }
    };

    RequestHandlers.Add(Method, NewHandler);
    return true;
}

FString ToString(EJson Type)
{
    switch (Type)
    {
    case EJson::None:
        return TEXT("None");
    case EJson::Null:
        return TEXT("Null");
    case EJson::String:
        return TEXT("String");
    case EJson::Number:
        return TEXT("Number");
    case EJson::Boolean:
        return TEXT("Boolean");
    case EJson::Array:
        return TEXT("Array");
    case EJson::Object:
        return TEXT("Object");
    default:
        return TEXT("Unknown");
    }
}

bool ValidateParamsArray(const TArray<TSharedPtr<FJsonValue>> &Parameters, const TArray<EJson>& ExpectedTypes, FString *ErrorMessage)
{
    int32 ExpectedCount = ExpectedTypes.Num();

    if (Parameters.Num() != ExpectedCount)
    {
        if (ErrorMessage)
            *ErrorMessage = FString::Printf(TEXT("Wrong number of parameters. Expected %d, got %d"), ExpectedCount, Parameters.Num());
        return false;
    }

    for (int32 i = 0; i < Parameters.Num(); i++)
    {
        if (!Parameters[i].IsValid() || Parameters[i]->Type != ExpectedTypes[i])
        {
            if (ErrorMessage)
                *ErrorMessage = FString::Printf(TEXT("Invalid type received. params[%d] expected %s, got %s"), i, *ToString(ExpectedTypes[i]), *ToString(Parameters[i]->Type));
            return false;
        }
    }

    return true;
}

bool UJsonMessageDispatcher::RegisterRequestHandler(const FString& Method, const TArray<EJson>& ExpectedTypes, const TJsonRequestHandlerStructuredArrayLambda& Handler, UObject* Owner, bool bOverride)
{
    return RegisterRequestHandler(
        Method,
        [Handler, ExpectedTypes](const TSharedPtr<FJsonValue>& Params)
        {
            if (Params && Params->Type != EJson::Array)
                throw FString(TEXT("Invalid parameters (not an array)"));

            TArray<TSharedPtr<FJsonValue>> Parameters = Params ? Params->AsArray() : TArray<TSharedPtr<FJsonValue>>();
            FString ErrorMessage;
            if (!ValidateParamsArray(Parameters, ExpectedTypes, &ErrorMessage))
                throw ErrorMessage;

            return Handler(Parameters);
        },
        Owner,
        bOverride
    );
}

bool UJsonMessageDispatcher::RegisterRequestAsyncHandler(const FString& Method, const FJsonRequestHandlerAsyncDelegate& Handler, UObject* Owner, bool bOverride)
{
    if (!bOverride && HaveValidRequestHandler(Method))
        return false;

    TWeakObjectPtr<UObject> PromiseOuter = this;
    auto NewHandler = MakeShared<FJsonRequestHandler>();
    NewHandler->Owner = Owner;
    NewHandler->Action = [Handler, PromiseOuter](
        const TSharedPtr<FJsonValue>& Params,
        const TJsonRequestCompletionCallback& CompletionCallback,
        const TJsonRequestErrorCallback& FailureCallback)
    {
        UJsonPromise* Promise = NewObject<UJsonPromise>(PromiseOuter.Get());
        Promise->GetOnResolve().AddLambda(CompletionCallback);
        Promise->GetOnReject().AddLambda(FailureCallback);
        Handler.ExecuteIfBound(ToJsonWrapper(Params), Promise);
    };

    RequestHandlers.Add(Method, NewHandler);
    return false;
}


bool UJsonMessageDispatcher::IsRequestHandlerRegistered(const FString& Method, UObject* Owner) const
{
    auto CurrentHandlerPtr = RequestHandlers.Find(Method);
    if (CurrentHandlerPtr == nullptr || !CurrentHandlerPtr->IsValid())
        return false;

    if (!IsValid(Owner))
        return true;

    return CurrentHandlerPtr->Get()->Owner == Owner;
}

bool UJsonMessageDispatcher::UnregisterRequestHandler(const FString& Method, UObject* Owner)
{
    if (IsRequestHandlerRegistered(Method, Owner))
    {
        RequestHandlers.Remove(Method);
        return true;
    }
    return false;
}

void UJsonMessageDispatcher::UnregisterRequestHandlersFromOwner(UObject* Owner)
{
    TArray<FString> KeysToRemove;
    for (const auto& Pair : RequestHandlers)
    {
        if (Pair.Value->Owner == Owner)
            KeysToRemove.Add(Pair.Key);
    }
    for (const auto& Key : KeysToRemove)
    {
        RequestHandlers.Remove(Key);
    }
}

void UJsonMessageDispatcher::RegisterNotificationHandler(const FString& Method, const FJsonNotificationHandlerDelegate& Handler, UObject* Owner)
{
    RegisterNotificationHandler(
        Method,
        [Handler](const TSharedPtr<FJsonValue>& Params)
        {
            FJsonObjectWrapper Result;
            EJsonObjectWrapperType ResultType = EJsonObjectWrapperType::JOWT_Object;
            FString Error;
            bool bSuccess = true;
            Handler.ExecuteIfBound(ToJsonWrapper(Params));
        }, Owner);
}

void UJsonMessageDispatcher::RegisterNotificationHandler(const FString& Method, const TJsonNotificationHandlerLambda& Handler, UObject* Owner)
{
    auto NewHandler = MakeShared<FJsonNotificationHandler>();
    NewHandler->Owner = Owner;
    NewHandler->Action = Handler;

    if (TArray<TSharedPtr<FJsonNotificationHandler>>* MethodHandlers = NotificationHandlers.Find(Method))
        MethodHandlers->Add(NewHandler);
    else
        NotificationHandlers.Add(Method, {NewHandler});
}

void UJsonMessageDispatcher::RegisterNotificationHandler(const FString& Method, const TArray<EJson>& ExpectedTypes, const TJsonNotificationHandlerStructuredArrayLambda& Handler, UObject* Owner)
{
    RegisterNotificationHandler(
        Method,
        [Handler, ExpectedTypes](const TSharedPtr<FJsonValue>& Params)
        {
            if (Params && Params->Type == EJson::Array)
                return;

            TArray<TSharedPtr<FJsonValue>> Parameters = Params->AsArray();
            if (!ValidateParamsArray(Parameters, ExpectedTypes, nullptr))
                return;

            Handler(Parameters);
        }, Owner);
}

bool UJsonMessageDispatcher::IsNotificationHandlerRegistered(const FString& Method, UObject* Owner) const
{
    const TArray<TSharedPtr<FJsonNotificationHandler>>* HandlersPtr = NotificationHandlers.Find(Method);
    if (HandlersPtr == nullptr)
        return false;

    if (Owner == nullptr)
        return !HandlersPtr->IsEmpty();

    for (const auto& Handler : *HandlersPtr)
    {
        if (Handler->Owner == Owner)
            return true;
    }

    return false;
}

void UJsonMessageDispatcher::UnregisterNotificationHandler(const FString& Method, UObject* Owner)
{
    TArray<TSharedPtr<FJsonNotificationHandler>>* HandlersPtr = NotificationHandlers.Find(Method);
    if (HandlersPtr == nullptr)
        return;

    if (Owner == nullptr)
    {
        NotificationHandlers.Remove(Method);
        return;
    }

    for (int i = HandlersPtr->Num() - 1; i >= 0; --i)
    {
        if ((*HandlersPtr)[i]->Owner == Owner)
            HandlersPtr->RemoveAt(i);
    }
    if (HandlersPtr->IsEmpty())
        NotificationHandlers.Remove(Method);
}

void UJsonMessageDispatcher::UnregisterNotificationHandlersFromOwner(UObject* Owner)
{
    TArray<FString> Keys;
    NotificationHandlers.GetKeys(Keys);
    for (const FString& Key : Keys)
        UnregisterNotificationHandler(Key, Owner);
}

void UJsonMessageDispatcher::UnregisterHandlersFromOwner(UObject* Owner)
{
    UnregisterRequestHandlersFromOwner(Owner);
    UnregisterNotificationHandlersFromOwner(Owner);
}


/** Message Sending */

bool SendMessageIfBound(const TScriptInterface<IMessageSender>& MessageSender, const FString& Message)
{
    UObject* Object = MessageSender.GetObject();
    if (!IsValid(Object))
        return false;

    return IMessageSender::Execute_SendMessage(Object, Message);
}

void SendJsonMessage(const TScriptInterface<IMessageSender>& MessageSender, const TSharedPtr<FJsonObject>& JsonResponse)
{
    FString StringResponse;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&StringResponse);

    if (!FJsonSerializer::Serialize(JsonResponse.ToSharedRef(), Writer))
    {
        SendMessageIfBound(MessageSender, TEXT("internal_serialization_error"));
        return;
    }

    SendMessageIfBound(MessageSender, StringResponse);
}

void UJsonMessageDispatcher::SendRequestWithCompletion(const TScriptInterface<IMessageSender>& MessageSender, const FString& Method, const FJsonObjectWrapper& Params, EJsonObjectWrapperType ParamsType, const FJsonResponseHandlerDelegate& CompletionHandler, float Timeout)
{
    SendRequest(MessageSender, Method, FromJsonWrapper(Params, ParamsType),
    [CompletionHandler](bool bSuccess, const TSharedPtr<FJsonValue>& Result, const FString& Error)
    {
        CompletionHandler.ExecuteIfBound(bSuccess, ToJsonWrapper(Result), Error);
    }, Timeout);
}

void UJsonMessageDispatcher::SendRequestWithSuccessOrFailure(const TScriptInterface<IMessageSender>& MessageSender, const FString& Method, const FJsonObjectWrapper& Params, EJsonObjectWrapperType ParamsType, const FJsonResponseSuccessHandlerDelegate& SuccessHandler, const FJsonResponseFailureHandlerDelegate& FailureHandler, float Timeout)
{
    SendRequest(MessageSender, Method, FromJsonWrapper(Params, ParamsType),
        [SuccessHandler](const TSharedPtr<FJsonValue>& Result)
        {
            SuccessHandler.ExecuteIfBound(ToJsonWrapper(Result));
        },
        [FailureHandler](const FString& Error)
        {
            FailureHandler.ExecuteIfBound(Error);
        }, Timeout);
}

void UJsonMessageDispatcher::SendRequestWithPromise(const TScriptInterface<IMessageSender>& MessageSender, const FString& Method, const FJsonObjectWrapper& Params, EJsonObjectWrapperType ParamsType, UJsonPromise* Promise, float Timeout)
{
    SendRequest(MessageSender, Method, FromJsonWrapper(Params, ParamsType), Promise, Timeout);
}


void UJsonMessageDispatcher::SendRequest(const TScriptInterface<IMessageSender>& MessageSender, const FString& Method, const TSharedPtr<FJsonValue>& Params, const TJsonResponseHandlerLambda& CompletionHandler, float Timeout)
{
    int32 RequestId = ++LastRequestId;

    if (LastRequestId >= JSONRPC_ID_MAX)
        LastRequestId = 0;

    TSharedPtr<FJsonResponseHandler> NewHandler = MakeShared<FJsonResponseHandler>();
    NewHandler->CompletionHandler = CompletionHandler;
    NewHandler->Timeout = FDateTime::UtcNow() + FTimespan::FromSeconds(Timeout);
    ResponseHandlers.Add(RequestId, NewHandler);

    TSharedPtr<FJsonObject> Request = MakeShared<FJsonObject>();

    Request->SetNumberField(TEXT(JSONRPC_ID), RequestId);
    Request->SetStringField(TEXT(JSONRPC_METHOD), Method);
    if (Params.IsValid())
        Request->SetField(TEXT(JSONRPC_PARAMS), Params);

    SendJsonMessage(MessageSender, Request);
}

void UJsonMessageDispatcher::SendRequest(const TScriptInterface<IMessageSender>& MessageSender, const FString& Method, const TSharedPtr<FJsonValue>& Params, const TJsonResponseSuccessHandlerLambda& SuccessHandler, const TJsonResponseFailureHandlerLambda& FailureHandler, float Timeout)
{
    SendRequest(MessageSender, Method, Params,
        [SuccessHandler, FailureHandler](bool bSuccess, const TSharedPtr<FJsonValue>& Result, const FString& Error)
        {
        if (bSuccess)
            SuccessHandler(Result);
        else
            FailureHandler(Error);
        }, Timeout);
}

void UJsonMessageDispatcher::SendRequest(const TScriptInterface<IMessageSender>& MessageSender, const FString& Method, const TSharedPtr<FJsonValue>& Params, UJsonPromise* Promise, float Timeout)
{
    SendRequest(MessageSender, Method, Params,
        [Promise](bool bSuccess, const TSharedPtr<FJsonValue>& Result, const FString& Error)
        {
            if (bSuccess)
                Promise->ResolveWithValue(Result);
            else
                Promise->Reject(Error);
        }, Timeout);
}


void UJsonMessageDispatcher::SendNotification(const TScriptInterface<IMessageSender>& MessageSender, const FString& Method, const FJsonObjectWrapper& Params, EJsonObjectWrapperType ParamsType)
{
    SendNotification(MessageSender, Method, FromJsonWrapper(Params, ParamsType));
}

void UJsonMessageDispatcher::SendNotification(const TScriptInterface<IMessageSender>& MessageSender, const FString& Method, const TSharedPtr<FJsonValue>& Params)
{
    TSharedPtr<FJsonObject> Request = MakeShared<FJsonObject>();

    Request->SetStringField(TEXT(JSONRPC_METHOD), Method);
    if (Params.IsValid())
        Request->SetField(TEXT(JSONRPC_PARAMS), Params);

    SendJsonMessage(MessageSender, Request);
}

/** Message handling */

void UJsonMessageDispatcher::HandleMessage(const FString& Message, TScriptInterface<IMessageSender> MessageSender)
{
    TSharedPtr<FJsonObject> JsonMessage;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Message);

    if (!FJsonSerializer::Deserialize(Reader, JsonMessage) || !JsonMessage.IsValid())
    {
        SendMessageIfBound(MessageSender, TEXT("invalid_json"));
        return;
    }

    HandleJsonMessage(JsonMessage, MessageSender);
}

void UJsonMessageDispatcher::HandleJsonMessage(const FJsonObjectWrapper& JsonMessage, TScriptInterface<IMessageSender> MessageSender)
{
    HandleJsonMessage(JsonMessage.JsonObject, MessageSender);
}

TSharedPtr<FJsonObject> MakeErrorJson(int32 Id, const FString &Error)
{
    TSharedPtr<FJsonObject> ErrorJson = MakeShared<FJsonObject>();

    ErrorJson->SetNumberField(TEXT(JSONRPC_ID), Id);
    ErrorJson->SetStringField(TEXT(JSONRPC_ERROR), Error);
    return ErrorJson;
}

void UJsonMessageDispatcher::HandleJsonMessage(const TSharedPtr<FJsonObject>& JsonMessage, TScriptInterface<IMessageSender> MessageSender)
{
    int32 Id;
    bool bHasId = JsonMessage->TryGetNumberField(TEXT(JSONRPC_ID), Id);

    FString Method;
    bool bHasMethod = JsonMessage->TryGetStringField(TEXT(JSONRPC_METHOD), Method);

    if (bHasMethod)
    {
        if (bHasId)
            HandleRequest(Id, Method, JsonMessage->TryGetField(TEXT(JSONRPC_PARAMS)), MessageSender);
        else
            HandleNotification(Method, JsonMessage->TryGetField(TEXT(JSONRPC_PARAMS)));
    }
    else
    {
        if (bHasId)
            HandleResponse(Id, JsonMessage->TryGetField(TEXT(JSONRPC_RESULT)), JsonMessage->TryGetField(TEXT(JSONRPC_ERROR)));
    }
}

void UJsonMessageDispatcher::HandleRequest(int32 Id,const FString& Method, const TSharedPtr<FJsonValue>& Params, TScriptInterface<IMessageSender> MessageSender)
{
    TSharedPtr<FJsonRequestHandler>* Handler = RequestHandlers.Find(Method);
    if (Handler == nullptr || !Handler->IsValid())
        return SendJsonMessage(MessageSender, MakeErrorJson(Id, FString::Printf(TEXT("no handlers for method %s"), *Method)));

    (*Handler)->Action(Params,
        [Id, MessageSender](const TSharedPtr<FJsonValue>& Result)
        {
            TSharedPtr<FJsonObject> JsonResponse = MakeShared<FJsonObject>();
            JsonResponse->SetNumberField(TEXT(JSONRPC_ID), Id);
            JsonResponse->SetField(TEXT(JSONRPC_RESULT), Result != nullptr ? Result : MakeShared<FJsonValueNull>());
            SendJsonMessage(MessageSender, JsonResponse);
        }, [Id, MessageSender](const FString& Error)
        {
            SendJsonMessage(MessageSender, MakeErrorJson(Id, Error));
        });
}

void UJsonMessageDispatcher::HandleNotification(const FString& Method, const TSharedPtr<FJsonValue>& Params)
{
    auto* Handlers = NotificationHandlers.Find(Method);
    if (!Handlers)
        return;

    for (const auto& Handler : *Handlers)
    {
        Handler->Action(Params);
    }
}

void UJsonMessageDispatcher::HandleResponse(int32 Id, const TSharedPtr<FJsonValue>& Result, const TSharedPtr<FJsonValue>& Error)
{
    TSharedPtr<FJsonResponseHandler> Handler;

    if (!ResponseHandlers.RemoveAndCopyValue(Id, Handler))
        return;
    
    if (!Handler.IsValid())
        return;

    if (Error.IsValid() && !Error->IsNull())
    {
        FString ErrorString;
        if (Error->TryGetString(ErrorString))
            Handler->CompletionHandler(false, nullptr, ErrorString);
        return;
    }

    Handler->CompletionHandler(true, Result, TEXT(""));
}

void UJsonMessageDispatcher::Cleanup()
{
    for (auto It = ResponseHandlers.CreateIterator(); It; ++It)
    {
        if (!It->Value.IsValid())
        {
            It.RemoveCurrent();
            continue;
        }
        if (It->Value->Timeout < FDateTime::UtcNow())
        {
            It->Value->CompletionHandler(false, nullptr, TEXT("timeout"));
            It.RemoveCurrent();
        }
    }
    for (auto It = NotificationHandlers.CreateIterator(); It; ++It)
    {
        if (!It->Value.IsEmpty())
            It.RemoveCurrent();
    }
}
