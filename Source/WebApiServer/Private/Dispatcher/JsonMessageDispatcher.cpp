// Fill out your copyright notice in the Description page of Project Settings.


#include "Dispatcher/JsonMessageDispatcher.h"

bool UJsonMessageDispatcher::RegisterRequestHandler(const FString& Method, const FJsonRequestHandlerDelegate& Handler, bool bOverride)
{
    if (!bOverride)
    {
        auto* CurrentHandlerPtr = RequestHandlers.Find(Method);
        if (CurrentHandlerPtr && CurrentHandlerPtr->IsValid())
            return false;
    }

    auto NewHandler = MakeShared<FJsonRequestHandlerWithDelegate>();
    NewHandler->Delegate = Handler;

    RequestHandlers.Add(Method, NewHandler);
    return true;
}

bool UJsonMessageDispatcher::RegisterRequestHandler(const FString& Method, const TJsonRequestHandlerLambda& Handler, bool bOverride)
{
    if (!bOverride)
    {
        auto* CurrentHandlerPtr = RequestHandlers.Find(Method);
        if (CurrentHandlerPtr && CurrentHandlerPtr->IsValid())
            return false;
    }

    auto NewHandler = MakeShared<FJsonRequestHandlerWithLambda>();
    NewHandler->Lambda = Handler;

    RequestHandlers.Add(Method, NewHandler);
    return true;
}

bool UJsonMessageDispatcher::RegisterRequestHandler(const FString& Method, const TArray<EJson>& ExpectedTypes, const TJsonRequestHandlerStructuredArrayLambda& Handler, bool bOverride)
{
    if (!bOverride)
    {
        auto* CurrentHandlerPtr = RequestHandlers.Find(Method);
        if (CurrentHandlerPtr && CurrentHandlerPtr->IsValid())
            return false;
    }

    auto NewHandler = MakeShared<FJsonRequestHandlerWithStructuredArrayLambda>();
    NewHandler->ExpectedTypes = ExpectedTypes;
    NewHandler->Lambda = Handler;

    RequestHandlers.Add(Method, NewHandler);
    return true;
}

bool UJsonMessageDispatcher::IsRequestHandlerRegistered(const FString& Method, const FJsonRequestHandlerDelegate& Handler) const
{
    // TODO: Check if of correct type and correct handler
    return false;
}

bool UJsonMessageDispatcher::UnregisterRequestHandler(const FString& Method, const FJsonRequestHandlerDelegate& Handler)
{
    if (IsRequestHandlerRegistered(Method, Handler))
    {
        RequestHandlers.Remove(Method);
        return true;
    }
    return false;
}

void UJsonMessageDispatcher::RegisterNotificationCallback(const FString& Method, const FJsonNotificationHandlerDelegate& Callback)
{
    auto NewHandler = MakeShared<FJsonNotificationHandlerWithDelegate>();
    NewHandler->Delegate = Callback;

    if (TArray<TSharedPtr<FJsonNotificationHandler>>* MethodHandlers = NotificationHandlers.Find(Method))
        MethodHandlers->Add(NewHandler);
    else
        NotificationHandlers.Add(Method, {NewHandler});
}

void UJsonMessageDispatcher::RegisterNotificationCallback(const FString& Method, const TJsonNotificationHandlerLambda& Callback)
{
    auto NewHandler = MakeShared<FJsonNotificationHandlerWithLambda>();
    NewHandler->Lambda = Callback;

    if (TArray<TSharedPtr<FJsonNotificationHandler>>* MethodHandlers = NotificationHandlers.Find(Method))
        MethodHandlers->Add(NewHandler);
    else
        NotificationHandlers.Add(Method, {NewHandler});
}

void UJsonMessageDispatcher::RegisterNotificationCallback(const FString& Method, const TArray<EJson>& ExpectedTypes, const TJsonNotificationHandlerStructuredArrayLambda& Callback)
{
    auto NewHandler = MakeShared<FJsonNotificationHandlerWithStructuredArrayLambda>();
    NewHandler->ExpectedTypes = ExpectedTypes;
    NewHandler->Lambda = Callback;

    if (TArray<TSharedPtr<FJsonNotificationHandler>>* MethodHandlers = NotificationHandlers.Find(Method))
        MethodHandlers->Add(NewHandler);
    else
        NotificationHandlers.Add(Method, {NewHandler});
}

void UJsonMessageDispatcher::UnregisterNotificationCallback(const FString& Method, const FJsonNotificationHandlerDelegate& Callback)
{
    // TODO: Filter out all matching notification handlers
}


/** Message handling */

void UJsonMessageDispatcher::HandleMessage(const FString& Message, TScriptInterface<IMessageSender> MessageSender)
{
    TSharedPtr<FJsonObject> JsonMessage;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Message);

    if (!FJsonSerializer::Deserialize(Reader, JsonMessage) || !JsonMessage.IsValid())
    {
        MessageSender->SendMessage(TEXT("invalid_json"));
        return;
    }

    HandleJsonMessage(JsonMessage, MessageSender);
}

void UJsonMessageDispatcher::HandleJsonMessage(const FJsonObjectWrapper& JsonMessage, TScriptInterface<IMessageSender> MessageSender)
{
    HandleJsonMessage(JsonMessage.JsonObject, MessageSender);
}

void SendJsonResponse(const TSharedPtr<FJsonObject>& JsonResponse, TScriptInterface<IMessageSender> MessageSender)
{
    FString StringResponse;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&StringResponse);

    if (!FJsonSerializer::Serialize(JsonResponse.ToSharedRef(), Writer))
    {
        MessageSender->SendMessage(TEXT("internal_serialization_error"));
        return;
    }

    MessageSender->SendMessage(StringResponse);
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
    bool bShouldReturn = JsonMessage->TryGetNumberField(TEXT(JSONRPC_ID), Id);

    FString Method;
    if (!JsonMessage->TryGetStringField(TEXT(JSONRPC_METHOD), Method))
    {
        if (bShouldReturn)
            SendJsonResponse(MakeErrorJson(Id, TEXT("no method in request")), MessageSender);
        return;
    }

    if (bShouldReturn)
    {
        TSharedPtr<FJsonRequestHandler>* Handler = RequestHandlers.Find(Method);
        if (Handler == nullptr || !Handler->IsValid())
            return SendJsonResponse(MakeErrorJson(Id, FString::Printf(TEXT("no handlers for method %s"), *Method)), MessageSender);

        (*Handler)->HandleRequest(JsonMessage->TryGetField(TEXT(JSONRPC_PARAMS)),
            [Id, MessageSender](const TSharedPtr<FJsonValue>& Response)
            {
            TSharedPtr<FJsonObject> JsonResponse = MakeShared<FJsonObject>();
            JsonResponse->SetNumberField(TEXT(JSONRPC_ID), Id);
                JsonResponse->SetObjectField(TEXT(JSONRPC_RESULT), Response->AsObject());
                SendJsonResponse(JsonResponse, MessageSender);
            }, [Id, MessageSender](const FString& Error)
            {
                SendJsonResponse(MakeErrorJson(Id, Error), MessageSender);
            });
    }
    else
    {
        auto* Handlers = NotificationHandlers.Find(Method);
        if (!Handlers)
            return;

        for (const auto& Handler : *Handlers)
        {
            Handler->HandleNotification(JsonMessage->TryGetField(TEXT(JSONRPC_PARAMS)));
        }
    }
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

void FJsonRequestHandlerWithStructuredArrayLambda::HandleRequest(
    const TSharedPtr<FJsonValue>& Param,
    const TJsonRequestCompletionCallback& Completion,
    const TJsonRequestErrorCallback& Error)
{
    if (Param->Type != EJson::Array)
    {
        Error(TEXT("Invalid parameters"));
        return;
    }

    TArray<TSharedPtr<FJsonValue>> Parameters = Param->AsArray();
    FString ErrorMessage;
    if (!ValidateParamsArray(Parameters, ExpectedTypes, &ErrorMessage))
    {
        Error(ErrorMessage);
        return;
    }

    try
    {
        TSharedPtr<FJsonValue> Result = Lambda(Parameters);
        Completion(Result);
    }
    catch (std::exception& e)
    {
        Error(e.what());
    }
}


void FJsonNotificationHandlerWithStructuredArrayLambda::HandleNotification(
    const TSharedPtr<FJsonValue>& Param)
{
    if (Param->Type != EJson::Array)
        return;

    TArray<TSharedPtr<FJsonValue>> Parameters = Param->AsArray();
    if (!ValidateParamsArray(Parameters, ExpectedTypes, nullptr))
        return;

    Lambda(Parameters);
}
