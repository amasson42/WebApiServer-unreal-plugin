// Fill out your copyright notice in the Description page of Project Settings.


#include "Dispatcher/JsonMessageDispatcher.h"

void UJsonMessageDispatcher::AddRequestHandler(const FString& Method, const FJsonRequestHandler& Handler)
{
    RequestHandlers.Add(Method, Handler);
}

void UJsonMessageDispatcher::RemoveRequestHandler(const FString& Method, const FJsonRequestHandler& Handler)
{
    // TODO:
}

void UJsonMessageDispatcher::AddNotificationCallback(const FString& Method, const FJsonNotificationCallback& Callback)
{
    NotificationCallbacks.Add(Method, Callback);
}

void UJsonMessageDispatcher::RemoveNotificationCallback(const FString& Method, const FJsonNotificationCallback& Callback)
{
    // TODO:
}


void UJsonMessageDispatcher::HandleMessage(const FString& Message)
{
    // TODO:
    // Add parameter IMessageCallback that can send a message as a response
    // 
}

void UJsonMessageDispatcher::HandleJsonMessage(const FJsonObjectWrapper& JsonMessage)
{

}
