// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "JsonObjectWrapper.h"

/** Enumeration used to define the method of holding data in FJsonObjectWrapper.
 *
 * The type FJsonObjectWrapper can only hold json values of type object as root.
 * When designing apis in blueprint, we may need different types as the root type.
 * This enumeration comes coupled with the wrapped object to explicit its internal dynamic typing. 
 */
UENUM(BlueprintType)
enum class EJsonObjectWrapperType : uint8 {
	/** Object will use the json object in the wrapper */
	JOWT_Object = 0 UMETA(DisplayName = "Object"),
	/** Array will structure the wrapper with field keys being the string representations of the array indices. (example: [null, true, "third", 4] <=> {"0": null, "1": true, "2": "third", "3": 4}) */
	JOWT_Array = 1 UMETA(DisplayName = "Array"),
	/** Value will put a field "value" in the wrapper holding the json value. (examples: "Hello" <=> {"value": "Hello"}; 12 <=> {"value": 12}; false <=> {"value": false}) */
	JOWT_Value = 2 UMETA(DisplayName = "Value"),
};

FJsonObjectWrapper ToJsonWrapper(const TSharedPtr<FJsonValue>& Value);
TSharedPtr<FJsonValue> FromJsonWrapper(const FJsonObjectWrapper& Wrapper, EJsonObjectWrapperType Type);
