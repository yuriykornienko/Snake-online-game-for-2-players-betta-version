#include <iostream>
#include <old_net.h>
//#include <SimpleClient.cpp>

enum class CustomMsgTypes : uint32_t
{
	ServerAccept,
	ServerDeny,
	ServerPing,
	MessageAll,
	ServerMessage,
	Cursor,
};



class CustomServer : public olc::net::server_interface<CustomMsgTypes>
{
public:
	CustomServer(uint16_t nPort) : olc::net::server_interface<CustomMsgTypes>(nPort)
	{

	}

protected:
	virtual bool OnClientConnect(std::shared_ptr<olc::net::connection<CustomMsgTypes>> client)
	{
		olc::net::message<CustomMsgTypes> msg;
		msg.header.id = CustomMsgTypes::ServerAccept;
		client->Send(msg);
		return true;
	}

	//Вызывается, когда кажется, что клиент отключился
	virtual void OnClientDisconnect(std::shared_ptr<olc::net::connection<CustomMsgTypes>> client)
	{
		std::cout << "Removing client [" << client->GetID() << "]\n";
	}

	//Вызывается при поступлении сообщения
	virtual void OnMessage(std::shared_ptr<olc::net::connection<CustomMsgTypes>> client, olc::net::message<CustomMsgTypes>& msg)
	{
		
		switch (msg.header.id)
		{

		case CustomMsgTypes::Cursor:
		{
			int vvodcursor;
			msg >> vvodcursor;
			olc::net::message<CustomMsgTypes> msg;
			msg.header.id = CustomMsgTypes::Cursor;
			//auto z = m_qMessagesIn.pop_front().msg;
			msg << vvodcursor; //vvod_cursor;
			//	m_qMessagesIn.pop_front();
			//auto z = server.m_qMessagesIn.pop_front();
			//client->Send(msg);
			MessageAllClients(msg, client);
		}
		break;





		case CustomMsgTypes::ServerPing:
		{
			std::cout << "[" << client->GetID() << "]: Server Ping\n";

			// Просто отправьте сообщение обратно клиенту
			client->Send(msg);
		}
		break;
		//vvod_cursor;
		case CustomMsgTypes::MessageAll:
		{
			std::cout << "[" << client->GetID() << "]: Message All\n";

			//Создайте новое сообщение и отправьте его всем клиентам
			olc::net::message<CustomMsgTypes> msg;
			msg.header.id = CustomMsgTypes::ServerMessage;
			//auto z = m_qMessagesIn.pop_front().msg;
			msg <<  client->GetID(); //vvod_cursor;
		//	m_qMessagesIn.pop_front();
			//auto z = server.m_qMessagesIn.pop_front();
			MessageAllClients(msg, client);
			
		}
		break;
		}
	}
};

int main()
{
	CustomServer server(60000);
	server.Start();

	while (1)
	{
		server.Update(-1, true);
	}



	return 0;
}