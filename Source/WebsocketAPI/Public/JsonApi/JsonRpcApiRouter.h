// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "JsonRpcApiRouter.generated.h"

typedef TFunction<TSharedPtr<FJsonValue>(const TArray<TSharedPtr<FJsonValue>> &)> JsonRpcMethod;

USTRUCT()
struct WEBSOCKETAPI_API FJsonRpcMethodHandler
{
    GENERATED_BODY()

public:
    FJsonRpcMethodHandler() {}

    FJsonRpcMethodHandler(const TArray<EJson> &ExpectedTypes, const JsonRpcMethod &Function);

    TArray<EJson> ExpectedTypes;

    JsonRpcMethod Function;
};

/**
 *
 */
UCLASS()
class WEBSOCKETAPI_API UJsonRpcApiRouter : public UObject
{
    GENERATED_BODY()

public:
    void AddRouteHandler(const FString &Route, const TArray<EJson> &ExpectedTypes, const JsonRpcMethod &JsonMethod);

    FString HandleMessage(const FString &Message);

    TSharedPtr<FJsonObject> HandleJsonMessage(const TSharedPtr<FJsonObject> &JsonMessage);

private:
    UPROPERTY()
    TMap<FString, FJsonRpcMethodHandler> MethodHandlers;
};
