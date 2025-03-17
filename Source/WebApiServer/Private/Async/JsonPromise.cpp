// Fill out your copyright notice in the Description page of Project Settings.


#include "Async/JsonPromise.h"

#include "JsonObjectWrapper.h"

void UJsonPromise::SetOnResolve(const TFunction<void(const TSharedPtr<FJsonValue>&)>& OnResolveLambda)
{
	OnResolve = OnResolveLambda;
}

void UJsonPromise::SetOnReject(const TFunction<void(const FString&)>& OnRejectLambda)
{
	OnReject = OnRejectLambda;
}

void UJsonPromise::ResolveWithValue(const TSharedPtr<FJsonValue>& JsonValue)
{
	if (bTerminated)
	{
		UE_LOG(LogTemp, Error, TEXT("JsonPromise: Trying to resolve a finished Promise"));
		return;
	}
	bTerminated = true;
	if (OnResolve)
		OnResolve(JsonValue);
}

void UJsonPromise::Resolve()
{
	ResolveWithValue(nullptr);
}

void UJsonPromise::ResolveWithString(const FString& Result)
{
	ResolveWithValue(MakeShared<FJsonValueString>(Result));
}

void UJsonPromise::ResolveWithInteger(int32 Result)
{
	ResolveWithValue(MakeShared<FJsonValueNumber>(Result));
}

void UJsonPromise::ResolveWithNumber(float Result)
{
	ResolveWithValue(MakeShared<FJsonValueNumber>(Result));
}

void UJsonPromise::ResolveWithBoolean(bool Result)
{
	ResolveWithValue(MakeShared<FJsonValueBoolean>(Result));
}

void UJsonPromise::ResolveWithObject(const FJsonObjectWrapper& Result)
{
	ResolveWithValue(MakeShared<FJsonValueObject>(Result.JsonObject));
}

void UJsonPromise::Reject(const FString& ErrorMessage)
{
	if (bTerminated)
	{
		UE_LOG(LogTemp, Error, TEXT("JsonPromise: Trying to reject a terminated promise"));
		return;
	}
	bTerminated = true;
	if (OnReject)
		OnReject(ErrorMessage);
}
