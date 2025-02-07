// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "INetworkingWebSocket.h"
#include "WebSocketClientConnectionWrapper.generated.h"

class UWebSocketServerWrapper;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMessageRecieved, UWebSocketClientConnectionWrapper *, Client,
                                             const FString &, Payload);

UCLASS(BlueprintType)
class WEBSOCKETAPI_API UWebSocketClientConnectionWrapper : public UObject
{
    GENERATED_BODY()

public:
    UWebSocketClientConnectionWrapper();

    UPROPERTY(BlueprintAssignable)
    FOnMessageRecieved OnMessageRecieved;

    UFUNCTION(BlueprintCallable)
    bool SendMessage(const FString &message);

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
