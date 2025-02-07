// Fill out your copyright notice in the Description page of Project Settings.


#include "WebSocket/WebSocketClientConnectionWrapper.h"

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

bool UWebSocketClientConnectionWrapper::SendMessage(const FString& Payload)
{
	TArray<uint8> Data;
	FTCHARToUTF8 Converter(*Payload);
	Data.Append((uint8*)Converter.Get(), Converter.Length());

	return SendData(Data);
}

bool UWebSocketClientConnectionWrapper::SendData(const TArray<uint8>& Data)
{
	if (!Server.IsValid() || !Server->IsStarted())
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
