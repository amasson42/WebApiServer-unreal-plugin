// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "INetworkingWebSocket.h"
#include "Messaging/MessageSender.h"
#include "WebSocketClientConnectionWrapper.generated.h"

class UWebSocketServerWrapper;

UCLASS(ClassGroup = (Networking), BlueprintType)
class WEBAPISERVER_API UWebSocketClientConnectionWrapper : public UObject, public IMessageSender
{
    GENERATED_BODY()

public:
    UWebSocketClientConnectionWrapper();

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMessageRecieved, UWebSocketClientConnectionWrapper*, Client,
                                             const FString&, Message);

    UPROPERTY(BlueprintAssignable)
    FOnMessageRecieved OnMessageRecieved;

    virtual bool SendMessage_Implementation(const FString &Message);

    UFUNCTION(BlueprintCallable)
    bool SendData(const TArray<uint8> &Data);

private:
    void Initialize(UWebSocketServerWrapper *InServer, INetworkingWebSocket *InNetworkingWebSocket);

    UPROPERTY(BlueprintReadOnly, Category = "Server", meta = (AllowPrivateAccess = true))
    TWeakObjectPtr<UWebSocketServerWrapper> Server = nullptr;

    INetworkingWebSocket *NetworkingWebSocket = nullptr;

    void ReceivedRawPacket(void *Data, int32 Count);

    friend class UWebSocketServerWrapper;
};
