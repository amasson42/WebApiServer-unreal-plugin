// Fill out your copyright notice in the Description page of Project Settings.


#include "Json/JsonObjectWrapperType.h"

FJsonObjectWrapper ToJsonWrapper(const TSharedPtr<FJsonValue>& Value)
{
    if (!Value.IsValid())
        return FJsonObjectWrapper();

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
                for (int32 i = 0; i < JsonArray.Num(); i++)
                {
                    if (JsonArray[i] == nullptr)
                        JsonArray[i] = MakeShared<FJsonValueNull>();
                }
                return MakeShared<FJsonValueArray>(JsonArray);
            }
        case EJsonObjectWrapperType::JOWT_Value:
            return Wrapper.JsonObject->TryGetField(TEXT("value"));
    }
    return nullptr;
}
