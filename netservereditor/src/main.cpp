#include "netlemon/net.h"

#include <iostream>

enum class CustomMsgTypes : uint64_t
{
	ServerAccept,
	ServerDeny,
	ServerPing,
	MessageAll,
	ServerMessage,
};

class CustomServer : public netlemon::common::ServerInterface<CustomMsgTypes>
{
public:
	CustomServer(uint16_t port) : netlemon::common::ServerInterface<CustomMsgTypes>(port)
	{

	}
protected:
	virtual bool OnClientConnect(std::shared_ptr<netlemon::common::Connection<CustomMsgTypes>> client)
	{
		netlemon::common::Message<CustomMsgTypes> msg;
		msg.header.id = CustomMsgTypes::ServerAccept;
		client->Send(msg);
		return true;
	}

	virtual void OnClientDisconnect(std::shared_ptr<netlemon::common::Connection<CustomMsgTypes>> client)
	{
		std::cout << "Removing client [" << client->GetID() << "]\n";
	}

	// Called when a message arrives
	virtual void OnMessage(std::shared_ptr<netlemon::common::Connection<CustomMsgTypes>> client, netlemon::common::Message<CustomMsgTypes>& msg)
	{
		switch (msg.header.id)
		{
		case CustomMsgTypes::ServerPing:
			{
				std::cout << "[" << client->GetID() << "]: Server Ping\n";

				// Simply bounce message back to client
				client->Send(msg);
				
				break;
			}
		case CustomMsgTypes::MessageAll:
			{
				std::cout << "[" << client->GetID() << "]: Message All\n";

				// Construct a new message and send it to all clients
				netlemon::common::Message<CustomMsgTypes> msg;
				msg.header.id = CustomMsgTypes::ServerMessage;
				msg << client->GetID();
				MessageAllClients(msg, client);

				break;	
			}
		}
	}
};

int main()
{
	CustomServer server(60000);
	server.Start();

	while (1)
	{
		server.Update(-1);
	}

	return 0;
}