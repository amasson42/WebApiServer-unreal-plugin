// Fill out your copyright notice in the Description page of Project Settings.

#include "WebSocket/WebSocketServerWrapper.h"

#include "IWebSocketNetworkingModule.h"
#include "IWebSocketServer.h"
#include "INetworkingWebSocket.h"
#include "WebSocket/WebSocketClientWrapper.h"

UWebSocketServerWrapper::~UWebSocketServerWrapper()
{
    StopServer();
}

UWebSocketServerWrapper* UWebSocketServerWrapper::NewWebSocketServer(UObject* Outer, int32 Port)
{
    UWebSocketServerWrapper *NewServer = NewObject<UWebSocketServerWrapper>(Outer);
    NewServer->StartServer(Port);

    return NewServer;
}

void UWebSocketServerWrapper::StartServer(int32 Port)
{
    WebSocketPort = Port;
    ServerWebSocket = FModuleManager::Get().LoadModuleChecked<IWebSocketNetworkingModule>(TEXT("WebSocketNetworking")).CreateServer();

    FWebSocketClientConnectedCallBack CallBack;
    CallBack.BindUObject(this, &ThisClass::OnWebSocketClientConnected);

    if (!ServerWebSocket->Init(WebSocketPort, CallBack))
    {
        ServerWebSocket.Reset();
        CallBack.Unbind();
        return;
    }

    TWeakObjectPtr<UWebSocketServerWrapper> WeakThis(this);
    TickHandle = FTSTicker::GetCoreTicker().AddTicker(
        FTickerDelegate::CreateLambda([WeakThis](float time) {
		if (WeakThis.IsValid() && WeakThis->ServerWebSocket)
		{
			WeakThis->ServerWebSocket->Tick();
			return true;
		}
		return false;
    }));
}

void UWebSocketServerWrapper::StopServer()
{
    ServerWebSocket = nullptr;

    if (TickHandle.IsValid())
    {
        FTSTicker::GetCoreTicker().RemoveTicker(TickHandle);
        TickHandle.Reset();
    }
}

bool UWebSocketServerWrapper::IsRunning() const
{
    return ServerWebSocket != nullptr && ServerWebSocket.IsValid();
}

void UWebSocketServerWrapper::Broadcast(const FString &Payload)
{
    if (!IsRunning())
        return;

    TArray<uint8> Data;
    FTCHARToUTF8 Converter(*Payload);
    Data.Append((uint8 *)Converter.Get(), Converter.Length());

    for (auto &&Client : WebSocketClients)
    {
        if (!Client->SendData(Data))
        {
            // Handle failing sending data
        }
    }
}

void UWebSocketServerWrapper::OnWebSocketClientConnected(INetworkingWebSocket *ClientWebSocket)
{
    UWebSocketClientWrapper *NewClient = NewObject<UWebSocketClientWrapper>();
    NewClient->Initialize(this, ClientWebSocket);
    WebSocketClients.Add(NewClient);

    FWebSocketInfoCallBack ClosedCallBack;
    ClosedCallBack.BindLambda([this, NewClient]() {
		WebSocketClients.Remove(NewClient);
		OnClientDisconnect.Broadcast(this, NewClient);
    });
    ClientWebSocket->SetSocketClosedCallBack(ClosedCallBack);

    OnClientConnect.Broadcast(this, NewClient);
}