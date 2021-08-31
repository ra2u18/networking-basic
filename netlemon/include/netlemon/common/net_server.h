
#pragma once

#include "netlemon/net_common.h"

#include "net_connection.h"
#include "net_message.h"
#include "net_tsqueue.h"

namespace netlemon::common
{
    template<typename T>
    class ServerInterface
    {
    public:
        ServerInterface(uint16_t port)
            : mAsioAcceptor(mAsioContext, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port)) {}
        ~ServerInterface() { Stop(); } 
    public:
        bool Start()
        {
            try
            {
                // Issue some work to the asio context
                WaitForClientConnection();

                mThrContext = std::thread([this](){ mAsioContext.run(); });
            }
            catch(std::exception& ec)
            {
                // Prohibited the server from listening
                std::cerr << "[SERVER] Exception: " << ec.what() << "\n";
                return false;
            }

            std::cout << "[SERVER] Started!\n";
            return true;
        }

        void Stop()
        {
            // Request the context to close
            mAsioContext.stop();

            if(mThrContext.joinable())
                mThrContext.join();
            
            std::cout << "[SERVER] Stopped!\n";
        }

        // Intruct ASIO to wait for connection
        void WaitForClientConnection()
        {
            // Sit and wait for incoming connections
            mAsioAcceptor.async_accept(
                [this](std::error_code ec, asio::ip::tcp::socket socket)
                {
                    if(!ec)
                    {
                        std::cout << "[SERVER] New Connection: " << socket.remote_endpoint() << "\n";
                        
                        std::shared_ptr<Connection<T>> newConnection = 
                            std::make_shared<Connection<T>>(Connection<T>::owner::server,
                                mAsioContext, std::move(socket), mQMessagesIn);
                        
                        // Give the user a chance to deny connection
                        if(OnClientConnect(newConnection))
                        {
                            // Connection allowed, add to the container of new connections
                            mDeqConnections.push_back(std::move(newConnection));

                            mDeqConnections.back()->ConnectToClient(nIDCounter++);
                            
                            std::cout << "[" << mDeqConnections.back()->GetID() << "] Connection Approved!\n";
                        }
                        else
                        {
                            // Connection will go out of scope and thus deleted
                            std::cout << "[------] Connection denied!\n"; 
                        }
                    }
                    else
                    {
                        // Error has occured during acceptance
                        std::cout << "[SERVER] New Connection Error: " << ec.message() << "\n";
                    }

                    // Prime asio context with more work
                    WaitForClientConnection();
                });
        }

        void MessageClient(std::shared_ptr<Connection<T>> client, const Message<T>& msg)
        {
            if(client && client->IsConnected()) 
            {
                client->Send(msg);
            }
            else
            {
                OnClientDisconnect(client);
                client.reset();
                mDeqConnections.erase()
                mDeqConnections.erase(std::remove(mDeqConnections.begin(), mDeqConnections.end(), client), mDeqConnections.end());
            }
        }

        void MessageAllClients(const Message<T>& msg, std::shared_ptr<Connection<T>> pIgnoreClient = nullptr)
        {
            bool bInvalidClientExists = false;
            
            for(auto& client : mDeqConnections)
            {
                // Check client is connected
                if(client && client->IsConnected() && client != pIgnoreClient)
                {
                    client->Send(msg);
                }
                else
                {
                    OnClientDisconnect(client);
                    client.reset();
                    bInvalidClientExists = true;
                }
            }

            if(bInvalidClientExists)
                mDeqConnections.erase(std::remove(mDeqConnections.begin(), mDeqConnections.end(), nullptr), mDeqConnections.end());
        }

        void Update(uint64_t nMaxMessages = -1, bool bWait = false)
        {
            //if (bWait) mQMessagesIn.wait();

            // Process as many messages as you can up to the nMaxMessages
            uint64_t nMessageCount = 0;
            while(nMessageCount < nMaxMessages && !mQMessagesIn.empty())
            {
                // Grab the front message
                auto msg = mQMessagesIn.pop_front();

                // Pass to message handler
                OnMessage(msg.remote, msg.msg);

                nMessageCount ++;
            }
        }

    protected:
        // Called when client connects, you can veto the connection by returning false
        virtual bool OnClientConnect(std::shared_ptr<Connection<T>> client)
        {
            return false;
        }
        virtual void OnClientDisconnect(std::shared_ptr<Connection<T>> client)
        {

        }
        // Called when message arrives
        virtual void OnMessage(std::shared_ptr<Connection<T>> client, Message<T>& msg)
        {
            
        }
    protected:
        // Thread Safe queue for incoming message packets
        TSQueue<OwnedMessage<T>> mQMessagesIn;

        // Container of active validated connections
        std::deque<std::shared_ptr<Connection<T>>> mDeqConnections;

        // Order of declaration is important - it is also the order of initialization
        asio::io_context mAsioContext;
        std::thread mThrContext;

        // These things need an asio context
        asio::ip::tcp::acceptor mAsioAcceptor;

        // Clients will be identified in the wider system via an ID
        uint64_t nIDCounter = 10000;
    };
}