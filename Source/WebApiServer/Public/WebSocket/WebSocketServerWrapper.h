// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "IWebSocketServer.h"
#include "WebSocketServerWrapper.generated.h"

class UWebSocketClientConnectionWrapper;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnClientStatus, UWebSocketClientConnectionWrapper *, Client);

/**
 *
 */
UCLASS(BlueprintType)
class WEBAPISERVER_API UWebSocketServerWrapper : public UObject
{
    GENERATED_BODY()

public:
    virtual ~UWebSocketServerWrapper() override;

    UFUNCTION(BlueprintCallable, Category = "WebSocketServer")
    void StartServer(int32 Port);

    UFUNCTION(BlueprintCallable, Category = "WebSocketServer")
    void StopServer();

    UFUNCTION(BlueprintCallable, Category = "WebSocketServer")
    bool IsStarted() const;

    UFUNCTION(BlueprintCallable, Category = "WebSocketServer")
    void Broadcast(const FString &Payload);

    UPROPERTY(BlueprintAssignable, Category = "WebSocketServer")
    FOnClientStatus OnClientConnect;

    UPROPERTY(BlueprintAssignable, Category = "WebSocketServer")
    FOnClientStatus OnClientDisconnect;

protected:
    void OnWebSocketClientConnected(INetworkingWebSocket *ClientWebSocket);

    UPROPERTY(BlueprintReadOnly, Category = "WebSocketServer", meta = (AllowPrivateAccess = true))
    int32 WebSocketPort;

private:
    TUniquePtr<IWebSocketServer> ServerWebSocket;

    UPROPERTY(BlueprintReadOnly, Category = "WebSocketServer", meta = (AllowPrivateAccess = true))
    TSet<TObjectPtr<UWebSocketClientConnectionWrapper>> WebSocketClients;

    /** Delegate */
    FTSTicker::FDelegateHandle TickHandle;
};
