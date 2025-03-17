// Fill out your copyright notice in the Description page of Project Settings.


#include "Dispatcher/JsonHandlers.h"

#include "Async/JsonPromise.h"


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

void FJsonRequestHandlerWithJsonPromise::HandleRequest(const TSharedPtr<FJsonValue>& Param,
        const TJsonRequestCompletionCallback& Completion,
        const TJsonRequestErrorCallback& Error)
{
    if (!Owner.IsValid())
        return Error(TEXT("Owner is invalid"));

    UJsonPromise* Promise = NewObject<UJsonPromise>(Owner.Get());
    Promise->SetOnResolve([Completion](const TSharedPtr<FJsonValue>& Value) {
        Completion(Value);
    });
    Promise->SetOnReject([Error](const FString& ErrorString) {
        Error(ErrorString);
    });

    FJsonObjectWrapper ParamWrapper;
    if (Param.IsValid() && Param->Type == EJson::Object)
        ParamWrapper.JsonObject = Param->AsObject();

    Delegate.ExecuteIfBound(ParamWrapper, Promise);
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
