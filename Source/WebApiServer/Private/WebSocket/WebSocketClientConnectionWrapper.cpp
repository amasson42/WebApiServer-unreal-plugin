// Fill out your copyright notice in the Description page of Project Settings.


#include "WebSocket/WebSocketClientConnectionWrapper.h"

#include "INetworkingWebSocket.h"
#include "WebSocket/WebSocketServerWrapper.h"

UWebSocketClientConnectionWrapper::UWebSocketClientConnectionWrapper()
{
}

void UWebSocketClientConnectionWrapper::Initialize(UWebSocketServerWrapper *InServer, INetworkingWebSocket *InNetworkingWebSocket)
{
	Server = InServer;
	NetworkingWebSocket = InNetworkingWebSocket;
	
	FWebSocketPacketReceivedCallBack ReceiveCallBack;
	ReceiveCallBack.BindUObject(this, &ThisClass::ReceivedRawPacket);
	NetworkingWebSocket->SetReceiveCallBack(ReceiveCallBack);
}

bool UWebSocketClientConnectionWrapper::SendMessage_Implementation(const FString& Message)
{
	TArray<uint8> Data;
	FTCHARToUTF8 Converter(*Message);
	Data.Append((uint8*)Converter.Get(), Converter.Length());

	return SendData(Data);
}

bool UWebSocketClientConnectionWrapper::SendData(const TArray<uint8>& Data)
{
	if (!Server.IsValid() || !Server->IsRunning())
		return false;

	return NetworkingWebSocket->Send(Data.GetData(), Data.Num(), false);
}

void UWebSocketClientConnectionWrapper::ReceivedRawPacket(void *Data, int32 Count)
{
	if (Count == 0 || Data == nullptr)
	{
		return;
	}

	FString Message(Count, UTF8_TO_TCHAR(static_cast<const char*>(Data)));

	OnMessageRecieved.Broadcast(this, Message);
}