// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Messaging/MessageSender.h"
#include "WebSocketClientWrapper.generated.h"

class INetworkingWebSocket;
class UWebSocketServerWrapper;

UCLASS(ClassGroup = (Networking), BlueprintType)
class WEBAPISERVER_API UWebSocketClientWrapper : public UObject, public IMessageSender
{
    GENERATED_BODY()

public:
    UWebSocketClientWrapper();

    UFUNCTION(BlueprintCallable, Category = "WebSocketClient",
    meta = (DefaultToSelf = "Outer", DeprecatedFunction, DeprecationMessage = "Not yet fully implemented."))
    static UWebSocketClientWrapper* NewWebSocketConnection(UObject* Outer, const FString& Ip = "0.0.0.0", int32 Port = 8080);

    UFUNCTION(BlueprintCallable, Category = "WebSocket", meta = (DeprecatedFunction, DeprecationMessage = "Not yet fully implemented."))
    void Connect(const FString& Ip, int32 Port);

    UFUNCTION(BlueprintCallable, Category = "WebSocket", meta = (DeprecatedFunction, DeprecationMessage = "Not yet fully implemented."))
    void Disconnect();

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStatusChanged, UWebSocketClientWrapper*, client);

    UPROPERTY(BlueprintAssignable, Category = "WebSocket")
    FOnStatusChanged OnConnected;

    UPROPERTY(BlueprintAssignable, Category = "WebSocket")
    FOnStatusChanged OnDisconnected;

    UPROPERTY(BlueprintAssignable, Category = "WebSocket")
    FOnStatusChanged OnError;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMessageRecieved, UWebSocketClientWrapper*, Client,
                                             const FString&, Message);

    UPROPERTY(BlueprintAssignable, Category = "Message")
    FOnMessageRecieved OnMessageRecieved;

    virtual bool SendMessage_Implementation(const FString &Message);

    UFUNCTION(BlueprintCallable, Category = "Message")
    bool SendData(const TArray<uint8> &Data);

private:
    void Initialize(UWebSocketServerWrapper *InServer, INetworkingWebSocket *InNetworkingWebSocket);

    bool bInitialized = false;

    UPROPERTY(BlueprintReadOnly, Category = "WebSocket|Server", meta = (AllowPrivateAccess = true))
    TWeakObjectPtr<UWebSocketServerWrapper> Server = nullptr;

    INetworkingWebSocket *NetworkingWebSocket = nullptr;

    void OnClientConnected();
    void OnClientDisconnected();
    void OnClientError();
    void ReceivedRawPacket(void *Data, int32 Count);

    friend class UWebSocketServerWrapper;
};
