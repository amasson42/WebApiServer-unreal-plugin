// Fill out your copyright notice in the Description page of Project Settings.


#include "Dispatcher/JsonMessageDispatcher.h"


bool UJsonMessageDispatcher::RegisterRequestHandler(const FString& Method, const FJsonRequestHandler& Handler, bool bOverride)
{
    return false;
}

bool UJsonMessageDispatcher::IsRequestHandlerRegistered(const FString& Method, const FJsonRequestHandler& Handler) const
{
    return false;
}

bool UJsonMessageDispatcher::UnregisterRequestHandler(const FString& Method, const FJsonRequestHandler& Handler)
{
    return false;
}

void UJsonMessageDispatcher::RegisterNotificationCallback(const FString& Method, const FJsonNotificationCallback& Callback)
{

}

bool UJsonMessageDispatcher::IsNotificationCallbackRegistered(const FString& Method, const FJsonNotificationCallback& Callback) const
{
    return false;
}

void UJsonMessageDispatcher::UnregisterNotificationCallback(const FString& Method, const FJsonNotificationCallback& Callback)
{

}


/** Message handling */

void UJsonMessageDispatcher::HandleMessage(const FString& Message, TScriptInterface<IMessageSender> MessageSender)
{
    // TODO:
    // Add parameter IMessageCallback that can send a message as a response
    // 
}

void UJsonMessageDispatcher::HandleJsonMessage(const FJsonObjectWrapper& JsonMessage, TScriptInterface<IMessageSender> MessageSender)
{

}
