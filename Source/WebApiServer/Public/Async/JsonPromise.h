// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "JsonPromise.generated.h"

/**
 * 
 */
UCLASS(BlueprintType)
class WEBAPISERVER_API UJsonPromise : public UObject
{
	GENERATED_BODY()

public:

	/* Initilization */
	
	void SetOnResolve(const TFunction<void(const TSharedPtr<FJsonValue>&)>& OnResolveLambda);
	void SetOnReject(const TFunction<void(const FString&)>& OnRejectLambda);

	/* Termination */

	void ResolveWithValue(const TSharedPtr<FJsonValue>& JsonValue);
	
	UFUNCTION(BlueprintCallable, Category="Promise|Termination")
	void Resolve();

	UFUNCTION(BlueprintCallable, Category="Promise|Termination")
	void ResolveWithString(const FString& Result);

	UFUNCTION(BlueprintCallable, Category="Promise|Termination")
	void ResolveWithInteger(int32 Result);

	UFUNCTION(BlueprintCallable, Category="Promise|Termination")
	void ResolveWithNumber(float Result);

	UFUNCTION(BlueprintCallable, Category="Promise|Termination")
	void ResolveWithBoolean(bool Result);

	UFUNCTION(BlueprintCallable, Category="Promise|Termination")
	void ResolveWithObject(const FJsonObjectWrapper& Result);

	UFUNCTION(BlueprintCallable, Category="Promise|Termination")
	void Reject(const FString& ErrorMessage);

private:

	TFunction<void(const TSharedPtr<FJsonValue>&)> OnResolve;
	TFunction<void(const FString&)> OnReject;
	bool bTerminated = false;

};
