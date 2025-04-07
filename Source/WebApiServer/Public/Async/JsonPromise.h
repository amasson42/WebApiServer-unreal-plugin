// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "JsonObjectWrapper.h"
#include "JsonPromise.generated.h"

/**
 * 
 */
UCLASS(BlueprintType)
class WEBAPISERVER_API UJsonPromise : public UObject
{
	GENERATED_BODY()

public:

	/* Delegate Declarations */

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnJsonPromiseResolveDelegate, FJsonObjectWrapper, Result);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnJsonPromiseRejectDelegate, const FString&, Error);
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnJsonPromiseResolve, const TSharedPtr<FJsonValue>&);
	DECLARE_MULTICAST_DELEGATE_OneParam(FOnJsonPromiseReject, const FString&);

	/* Termination */

	void ResolveWithValue(const TSharedPtr<FJsonValue>& JsonValue);
	
	UFUNCTION(BlueprintCallable, Category="Promise|Termination")
	void ResolveWithNull();

	UFUNCTION(BlueprintCallable, Category="Promise|Termination")
	void ResolveWithBoolean(bool Result);

	UFUNCTION(BlueprintCallable, Category="Promise|Termination")
	void ResolveWithInteger(int32 Result);

	UFUNCTION(BlueprintCallable, Category="Promise|Termination")
	void ResolveWithNumber(float Result);

	UFUNCTION(BlueprintCallable, Category="Promise|Termination")
	void ResolveWithString(const FString& Result);

	void ResolveWithArray(const TArray<TSharedPtr<FJsonValue>>& Result);

	void ResolveWithObject(const TSharedPtr<FJsonObject>& Result);
	
	UFUNCTION(BlueprintCallable, Category="Promise|Termination")
	void ResolveWithWrappedArray(const FJsonObjectWrapper& Result);

	UFUNCTION(BlueprintCallable, Category="Promise|Termination")
	void ResolveWithWrappedObject(const FJsonObjectWrapper& Result);

	UFUNCTION(BlueprintCallable, Category="Promise|Termination")
	void Reject(const FString& ErrorMessage);

	/* Delegate */

	UPROPERTY(BlueprintAssignable, Category="Promise|Completion")
	FOnJsonPromiseResolveDelegate OnResolve;

	UPROPERTY(BlueprintAssignable, Category="Promise|Completion")
	FOnJsonPromiseRejectDelegate OnReject;

	FOnJsonPromiseResolve& GetOnResolve()
	{
		return OnResolveLambda;
	}

	FOnJsonPromiseReject& GetOnReject()
	{
		return OnRejectLambda;
	}
	
private:

	FOnJsonPromiseResolve OnResolveLambda;

	FOnJsonPromiseReject OnRejectLambda;

	bool Terminate();

	bool bTerminated = false;

};
