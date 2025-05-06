// Fill out your copyright notice in the Description page of Project Settings.


#include "WebSocket/WebSocketClientWrapper.h"

#include "SocketSubsystem.h"
#include "IWebSocketNetworkingModule.h"
#include "INetworkingWebSocket.h"

UWebSocketClientWrapper::UWebSocketClientWrapper()
{
}

UWebSocketClientWrapper* UWebSocketClientWrapper::NewWebSocketConnection(UObject* Outer, const FString& Ip, int32 Port)
{
	UWebSocketClientWrapper *NewClient = NewObject<UWebSocketClientWrapper>(Outer);
	NewClient->Connect(Ip, Port);

	return NewClient;
}

void UWebSocketClientWrapper::Connect(const FString& Ip, int32 Port)
{
	if (bInitialized)
		return;

	TSharedPtr<FInternetAddr> ServerAddr = ISocketSubsystem::Get()->CreateInternetAddr();

	TArray<uint8> RawIp;
	FTCHARToUTF8 Converter(*Ip);
	RawIp.Append((uint8*)Converter.Get(), Converter.Length());

	ServerAddr->SetRawIp(RawIp);
	ServerAddr->SetPort(Port);

	IWebSocketNetworkingModule& WebSocketModule = FModuleManager::Get().LoadModuleChecked<IWebSocketNetworkingModule>("WebSocketNetworkingModule");

	TSharedPtr<INetworkingWebSocket> Connection = WebSocketModule.CreateConnection(*ServerAddr);

	Initialize(nullptr, Connection.Get());
}

void UWebSocketClientWrapper::Disconnect()
{
	if (Server != nullptr)
		return;

	if (NetworkingWebSocket == nullptr)
		return;

	NetworkingWebSocket = nullptr;

	bInitialized = false;
}

void UWebSocketClientWrapper::Initialize(UWebSocketServerWrapper *InServer, INetworkingWebSocket *InNetworkingWebSocket)
{
	Server = InServer;
	NetworkingWebSocket = InNetworkingWebSocket;

	FWebSocketInfoCallBack ConnectedCallBack;
	ConnectedCallBack.BindUObject(this, &ThisClass::OnClientConnected);
	NetworkingWebSocket->SetConnectedCallBack(ConnectedCallBack);

	FWebSocketInfoCallBack DisconnectedCallBack;
	DisconnectedCallBack.BindUObject(this, &ThisClass::OnClientDisconnected);
	NetworkingWebSocket->SetSocketClosedCallBack(DisconnectedCallBack);

	FWebSocketInfoCallBack ErrorCallBack;
	ErrorCallBack.BindUObject(this, &ThisClass::OnClientError);
	NetworkingWebSocket->SetErrorCallBack(ErrorCallBack);

	FWebSocketPacketReceivedCallBack ReceiveCallBack;
	ReceiveCallBack.BindUObject(this, &ThisClass::ReceivedRawPacket);
	NetworkingWebSocket->SetReceiveCallBack(ReceiveCallBack);

	bInitialized = true;
}

bool UWebSocketClientWrapper::SendMessage_Implementation(const FString& Message)
{
	TArray<uint8> Data;
	FTCHARToUTF8 Converter(*Message);
	Data.Append((uint8*)Converter.Get(), Converter.Length());

	return SendData(Data);
}

bool UWebSocketClientWrapper::SendData(const TArray<uint8>& Data)
{
	return NetworkingWebSocket->Send(Data.GetData(), Data.Num(), false);
}

void UWebSocketClientWrapper::OnClientConnected()
{
	OnConnected.Broadcast(this);
}

void UWebSocketClientWrapper::OnClientDisconnected()
{
	OnDisconnected.Broadcast(this);
	Server = nullptr;
	NetworkingWebSocket = nullptr;
	bInitialized = false;
}

void UWebSocketClientWrapper::OnClientError()
{
	OnError.Broadcast(this);
}

void UWebSocketClientWrapper::ReceivedRawPacket(void *Data, int32 Count)
{
	if (Count == 0 || Data == nullptr)
	{
		return;
	}

	FString Message(Count, UTF8_TO_TCHAR(static_cast<const char*>(Data)));

	OnMessageRecieved.Broadcast(this, Message);
}