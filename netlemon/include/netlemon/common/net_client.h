#pragma once

#include "netlemon/net_common.h"

#include "net_connection.h"
#include "net_message.h"
#include "net_tsqueue.h"

namespace netlemon::common
{
    template<typename T>
    class ClientInterface
    {
    public:
        ClientInterface() : mSocket(mContext) {}
        ~ClientInterface() { Disconnect(); }

    public:
        // Connect to sv with hostname/ip-address and port
        bool Connect(const std::string& host, const uint16_t port)
        {
            try
            {
                // Resolve hostname/ip-address into tangible physical address
                asio::ip::tcp::resolver resolver(mContext);
                asio::ip::tcp::resolver::results_type endpoints = resolver.resolve(host, std::to_string(port));

                // Create connection
                mConnection = std::make_unique<Connection<T>>(
                    Connection<T>::owner::client, mContext, asio::ip::tcp::socket(mContext), mQMessagesIn);

                // Tell the connection object to connect to server
                mConnection->ConnectToServer(endpoints);

                // Start thread context
                mThrContext = std::thread([this](){ mContext.run(); });
            }
            catch(std::exception& e)
            {
                std::cerr << "Client Exception: " << e.what() << "\n";
                return false;
            }

            return true;
        }

        // Disconnect from server
        void Disconnect()
        {
            if(IsConnected())
                mConnection->Disconnect();
            mContext.stop();

            // TODO: create a thread guard
            if(mThrContext.joinable())
                mThrContext.join();
        }

        // Check if client is actually connected
        bool IsConnected() const
        {
            if(mConnection)
                return mConnection->IsConnected();
            else 
                return false;
        }

        // Retrieve queue of messages from server
        TSQueue<OwnedMessage<T>>& Incoming() { return mQMessagesIn; }

        void Send(const Message<T>& msg)
        {
            if(IsConnected())
                mConnection->Send(msg);
        }

    protected:
        // Asio context handles data transfer
        asio::io_context mContext;
        // Context needs a thread where to execute work
        std::thread mThrContext;
        // Hardware socket that is connected to the sv
        asio::ip::tcp::socket mSocket;
        // The client has a single instance of connection, which handles data transfer
        std::unique_ptr<Connection<T>> mConnection;

    private:
        // Incoming messages from server
        TSQueue<OwnedMessage<T>> mQMessagesIn;
    };
}