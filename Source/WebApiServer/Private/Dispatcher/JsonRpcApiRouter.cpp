// Fill out your copyright notice in the Description page of Project Settings.


#include "Dispatcher/JsonRpcApiRouter.h"

FJsonRpcMethodHandler::FJsonRpcMethodHandler(const TArray<EJson> &ExpectedTypes, const TFunction<TSharedPtr<FJsonValue>(const TArray<TSharedPtr<FJsonValue>> &)> &Function)
    : ExpectedTypes(ExpectedTypes), Function(Function)
{
}

void UJsonRpcApiRouter::AddRouteHandler(const FString &Route, const TArray<EJson> &ExpectedTypes, const JsonRpcMethod &JsonMethod)
{
    FJsonRpcMethodHandler NewHandler(ExpectedTypes, JsonMethod);
    MethodHandlers.Add(Route, NewHandler);
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

bool ValidateParamsArray(const TArray<TSharedPtr<FJsonValue>> &Parameters, const TArray<EJson> &ExpectedTypes, FString *ErrorMessage)
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

FString UJsonRpcApiRouter::HandleMessage(const FString &Message)
{
    TSharedPtr<FJsonObject> JsonMessage;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Message);

    if (!FJsonSerializer::Deserialize(Reader, JsonMessage) || !JsonMessage.IsValid())
    {
        return TEXT("invalid_json");
    }

    TSharedPtr<FJsonObject> JsonResponse = HandleJsonMessage(JsonMessage);
    if (!JsonResponse.IsValid())
    {
        return TEXT("");
    }

    FString StringResponse;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&StringResponse);

    if (!FJsonSerializer::Serialize(JsonResponse.ToSharedRef(), Writer))
    {
        return TEXT("internal_serialization_error");
    }

    return StringResponse;
}

TSharedPtr<FJsonObject> MakeErrorJson(int32 Id, const FString &Error)
{
    TSharedPtr<FJsonObject> ErrorJson = MakeShared<FJsonObject>();

    ErrorJson->SetNumberField(TEXT("id"), Id);
    ErrorJson->SetStringField(TEXT("error"), Error);
    return ErrorJson;
}

TSharedPtr<FJsonObject> UJsonRpcApiRouter::HandleJsonMessage(const TSharedPtr<FJsonObject> &JsonMessage)
{
    int32 Id;
    bool bShouldReturn = JsonMessage->TryGetNumberField(TEXT("id"), Id);

    FString Method;
    if (!JsonMessage->TryGetStringField(TEXT("method"), Method))
    {
        return bShouldReturn ? MakeErrorJson(Id, TEXT("No method")) : nullptr;
    }

    TArray<TSharedPtr<FJsonValue>> Parameters;
    if (JsonMessage->HasField(TEXT("params")))
    {
        Parameters = JsonMessage->GetArrayField(TEXT("params"));
    }

    FJsonRpcMethodHandler *Handler = MethodHandlers.Find(Method);
    if (!Handler)
    {
        return bShouldReturn ? MakeErrorJson(Id, FString::Printf(TEXT("Unknown method %s"), *Method)) : nullptr;
    }

    FString ParametersError;
    if (!ValidateParamsArray(Parameters, Handler->ExpectedTypes, &ParametersError))
    {
        return bShouldReturn ? MakeErrorJson(Id, FString::Printf(TEXT("Invalid parameters %s"), *ParametersError)) : nullptr;
    }

    TSharedPtr<FJsonValue> ReturnValue;
    try
    {
        ReturnValue = (Handler->Function)(Parameters);
    }
    catch (std::exception &Error)
    {
        return bShouldReturn ? MakeErrorJson(Id, FString::Printf(TEXT("Method error: %hs"), Error.what())) : nullptr;
    }

    if (!bShouldReturn)
        return nullptr;

    if (!ReturnValue.IsValid())
    {
        return MakeErrorJson(Id, TEXT("No response"));
    }

    TSharedPtr<FJsonObject> ReturnMessage = MakeShared<FJsonObject>();
    ReturnMessage->SetNumberField(TEXT("id"), Id);
    ReturnMessage->SetField(TEXT("result"), ReturnValue);
    return ReturnMessage;
}