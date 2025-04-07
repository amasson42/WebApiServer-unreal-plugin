// Fill out your copyright notice in the Description page of Project Settings.


#include "Async/JsonPromise.h"

#include "Json/JsonObjectWrapperType.h"

void UJsonPromise::ResolveWithValue(const TSharedPtr<FJsonValue>& JsonValue)
{
	if (!Terminate())
		return;

	OnResolveLambda.Broadcast(JsonValue);
	if (OnResolve.IsBound())
		OnResolve.Broadcast(ToJsonWrapper(JsonValue));
}

void UJsonPromise::ResolveWithNull()
{
	ResolveWithValue(nullptr);
}

void UJsonPromise::ResolveWithBoolean(bool Result)
{
	ResolveWithValue(MakeShared<FJsonValueBoolean>(Result));
}

void UJsonPromise::ResolveWithInteger(int32 Result)
{
	ResolveWithValue(MakeShared<FJsonValueNumber>(Result));
}

void UJsonPromise::ResolveWithNumber(float Result)
{
	ResolveWithValue(MakeShared<FJsonValueNumber>(Result));
}

void UJsonPromise::ResolveWithString(const FString& Result)
{
	ResolveWithValue(MakeShared<FJsonValueString>(Result));
}

void UJsonPromise::ResolveWithArray(const TArray<TSharedPtr<FJsonValue>>& Result)
{
	ResolveWithValue(MakeShared<FJsonValueArray>(Result));
}

void UJsonPromise::ResolveWithObject(const TSharedPtr<FJsonObject>& Result)
{
	ResolveWithValue(MakeShared<FJsonValueObject>(Result));
}

void UJsonPromise::ResolveWithWrappedArray(const FJsonObjectWrapper& Result)
{
	ResolveWithValue(FromJsonWrapper(Result, EJsonObjectWrapperType::JOWT_Array));
}

void UJsonPromise::ResolveWithWrappedObject(const FJsonObjectWrapper& Result)
{
	ResolveWithValue(MakeShared<FJsonValueObject>(Result.JsonObject));
}

void UJsonPromise::Reject(const FString& ErrorMessage)
{
	if (!Terminate())
		return;

	OnRejectLambda.Broadcast(ErrorMessage);
	if (OnReject.IsBound())
		OnReject.Broadcast(ErrorMessage);
}

bool UJsonPromise::Terminate()
{
	if (bTerminated)
	{
		UE_LOG(LogTemp, Error, TEXT("JsonPromise: Trying to terminate an already terminated promise"));
		return false;
	}
	bTerminated = true;
	return true;
}
