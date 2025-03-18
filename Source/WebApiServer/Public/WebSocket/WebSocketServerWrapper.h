// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WebSocketServerWrapper.generated.h"

class IWebSocketServer;
class UWebSocketClientConnectionWrapper;

/**
 *
 */
UCLASS(ClassGroup = (Networking), BlueprintType)
class WEBAPISERVER_API UWebSocketServerWrapper : public UObject
{
    GENERATED_BODY()

public:
    virtual ~UWebSocketServerWrapper() override;

    UFUNCTION(BlueprintCallable, Category = "WebSocketServer",
        meta = (DefaultToSelf = "Outer"))
    static UWebSocketServerWrapper* NewWebSocketServer(UObject* Outer, int32 Port = 8080);
    
    UFUNCTION(BlueprintCallable, Category = "WebSocketServer")
    void StartServer(int32 Port);

    UFUNCTION(BlueprintCallable, Category = "WebSocketServer")
    void StopServer();

    UFUNCTION(BlueprintCallable, Category = "WebSocketServer")
    bool IsRunning() const;

    UFUNCTION(BlueprintCallable, Category = "WebSocketServer")
    void Broadcast(const FString &Payload);

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnClientStatus, UWebSocketClientConnectionWrapper *, Client);

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
