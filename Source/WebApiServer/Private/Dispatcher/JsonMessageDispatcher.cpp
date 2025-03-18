// Fill out your copyright notice in the Description page of Project Settings.


#include "Dispatcher/JsonMessageDispatcher.h"
#include "JsonHandlers.h"

FJsonObjectWrapper ToJsonWrapper(const TSharedPtr<FJsonValue>& Value)
{
    TSharedPtr<FJsonObject> JsonObject = nullptr;
    switch (Value->Type)
    {
        case EJson::Object:
            JsonObject = Value->AsObject();
            break;
        case EJson::Array:
            JsonObject = MakeShared<FJsonObject>();
            {
                TArray<TSharedPtr<FJsonValue>> JsonArray = Value->AsArray();
                for (int32 Index = 0; Index < JsonArray.Num(); ++Index)
                {
                    FString Key = FString::FromInt(Index);
                    JsonObject->SetField(Key, JsonArray[Index]);
                }
            }
            break;
        default:
            JsonObject = MakeShared<FJsonObject>();
            JsonObject->SetField(TEXT("value"), Value);
            break;
    }
    FJsonObjectWrapper Wrapper;
    Wrapper.JsonObject = JsonObject;
    return Wrapper;
}

TSharedPtr<FJsonValue> FromJsonWrapper(const FJsonObjectWrapper& Wrapper, EJsonObjectWrapperType Type)
{
    switch (Type)
    {
        case EJsonObjectWrapperType::JOWT_Object:
            return MakeShared<FJsonValueObject>(Wrapper.JsonObject);
        case EJsonObjectWrapperType::JOWT_Array:
            {
                TArray<TSharedPtr<FJsonValue>> JsonArray;
                for (const auto& Pair : Wrapper.JsonObject->Values)
                {
                    if (Pair.Key.IsEmpty() || !Pair.Key.IsNumeric())
                        continue;
                    int32 Index = FCString::Atoi(*Pair.Key);
                    if (Index < 0)
                        continue;
                    if (Index >= JsonArray.Num())
                        JsonArray.SetNum(Index + 1);
                    JsonArray[Index] = Pair.Value;
                }
                return MakeShared<FJsonValueArray>(JsonArray);
            }
        case EJsonObjectWrapperType::JOWT_Value:
            return Wrapper.JsonObject->TryGetField(TEXT("value"));
    }
    return nullptr;
}

bool UJsonMessageDispatcher::HaveValidRequestHandler(const FString& Method) const
{
    auto* CurrentHandlerPtr = RequestHandlers.Find(Method);
    if (CurrentHandlerPtr == nullptr)
        return false;

    return CurrentHandlerPtr->IsValid();
}

bool UJsonMessageDispatcher::RegisterRequestHandler(const FString& Method, const FJsonRequestHandlerDelegate& Handler, bool bOverride)
{
    if (!bOverride && HaveValidRequestHandler(Method))
        return false;

    auto NewHandler = MakeShared<FJsonRequestHandlerWithDelegate>();
    NewHandler->Delegate = Handler;

    RequestHandlers.Add(Method, NewHandler);
    return true;
}

bool UJsonMessageDispatcher::RegisterRequestHandler(const FString& Method, const TJsonRequestHandlerLambda& Handler, bool bOverride)
{
    if (!bOverride && HaveValidRequestHandler(Method))
        return false;

    auto NewHandler = MakeShared<FJsonRequestHandlerWithLambda>();
    NewHandler->Lambda = Handler;

    RequestHandlers.Add(Method, NewHandler);
    return true;
}

bool UJsonMessageDispatcher::RegisterRequestHandler(const FString& Method, const TArray<EJson>& ExpectedTypes, const TJsonRequestHandlerStructuredArrayLambda& Handler, bool bOverride)
{
    if (!bOverride && HaveValidRequestHandler(Method))
        return false;

    auto NewHandler = MakeShared<FJsonRequestHandlerWithStructuredArrayLambda>();
    NewHandler->ExpectedTypes = ExpectedTypes;
    NewHandler->Lambda = Handler;

    RequestHandlers.Add(Method, NewHandler);
    return true;
}

bool UJsonMessageDispatcher::RegisterRequestAsyncHandler(const FString& Method, const FJsonRequestHandlerAsyncDelegate& Handler, bool bOverride)
{
    if (!bOverride && HaveValidRequestHandler(Method))
        return false;

    auto NewHandler = MakeShared<FJsonRequestHandlerWithJsonPromise>();
    NewHandler->Owner = this;
    NewHandler->Delegate = Handler;

    RequestHandlers.Add(Method, NewHandler);
    return false;
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

bool SendMessageIfBound(const TScriptInterface<IMessageSender>& MessageSender, const FString& Message)
{
    UObject* Object = MessageSender.GetObject();
    if (!IsValid(Object))
        return false;

    return IMessageSender::Execute_SendMessage(Object, Message);
}

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

void SendJsonResponse(const TSharedPtr<FJsonObject>& JsonResponse, TScriptInterface<IMessageSender> MessageSender)
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
        // TODO: Handle request results (or errors)
        if (bHasId)
            SendJsonResponse(MakeErrorJson(Id, TEXT("no method in request")), MessageSender);
    }
}

void UJsonMessageDispatcher::HandleRequest(int32 Id,const FString& Method, const TSharedPtr<FJsonValue>& Params, TScriptInterface<IMessageSender> MessageSender)
{
    TSharedPtr<FJsonRequestHandler>* Handler = RequestHandlers.Find(Method);
    if (Handler == nullptr || !Handler->IsValid())
        return SendJsonResponse(MakeErrorJson(Id, FString::Printf(TEXT("no handlers for method %s"), *Method)), MessageSender);

    (*Handler)->HandleRequest(Params,
        [Id, MessageSender](const TSharedPtr<FJsonValue>& Result)
        {
            TSharedPtr<FJsonObject> JsonResponse = MakeShared<FJsonObject>();
            JsonResponse->SetNumberField(TEXT(JSONRPC_ID), Id);
            JsonResponse->SetField(TEXT(JSONRPC_RESULT), Result != nullptr ? Result : MakeShared<FJsonValueNull>());
            SendJsonResponse(JsonResponse, MessageSender);
        }, [Id, MessageSender](const FString& Error)
        {
            SendJsonResponse(MakeErrorJson(Id, Error), MessageSender);
        });
}

void UJsonMessageDispatcher::HandleNotification(const FString& Method, const TSharedPtr<FJsonValue>& Params)
{
    auto* Handlers = NotificationHandlers.Find(Method);
    if (!Handlers)
        return;

    for (const auto& Handler : *Handlers)
    {
        Handler->HandleNotification(Params);
    }
}
