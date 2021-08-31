#pragma once

#include "netlemon/net_common.h"

#include "net_connection.h"
#include "net_message.h"
#include "net_tsqueue.h"

namespace netlemon::common
{
    // Enable share pointer from (this) keyword rather than raw pointer
    template<typename T>
    class Connection : std::enable_shared_from_this<Connection<T>>
    {
    public:
        enum class owner
        {
            server,
            client
        };
    public:
        Connection(owner parent, asio::io_context& asioContext, 
            asio::ip::tcp::socket socket, TSQueue<OwnedMessage<T>>& qIn)
                : mAsioContext (asioContext), mSocket(std::move(socket)), mQMessagesIn(qIn)
        {
            mOwnerType = parent;    
        }
        virtual ~Connection() {}
    public:
        // Clients only utility function
        void ConnectToClient(uint64_t uid = 0)
        {
            if(mOwnerType == owner::server && mSocket.is_open())
            {
                id = uid;
                readHeader();
            }
        }

        void ConnectToServer(const asio::ip::tcp::resolver::results_type& endpoints)
        {
            if(mOwnerType == owner::client)
            {
                // Request asio to connect to an endpoint
                asio::async_connect(mSocket, endpoints, 
                    [this](std::error_code ec, asio::ip::tcp::endpoint endpoint)
                    {
                        if(!ec) readHeader();
                    });
            }
        }
        
        void Disconnect()
        {
            if(IsConnected())
                asio::post(mAsioContext, [this](){ mSocket.close(); });
        }

        bool IsConnected() const { return mSocket.is_open(); }
    public:
        // Connections are responsible for sending messages
        void Send(const Message<T>& msg)
        {
            asio::post(mAsioContext, [this, msg](){
                bool bWritingMessage = !mQMessagesOut.empty();
                mQMessagesOut.push_back(msg);
                
                // write only if the current context doesn't have a write header
                // in the queue already
                if(!bWritingMessage)
                    writeHeader(); 
            });
        }

        inline uint64_t GetID() const { return id; }
    private:
        // ASYNC - prime context ready to read a message header
        void readHeader()
        {
            asio::async_read(mSocket, asio::buffer(&mMsgTemporaryIn.header, sizeof(MessageHeader<T>)),
                [this](std::error_code ec, std::size_t length){
                    if(!ec) 
                    {
                        // Interogate the header to see if the size of message is > 0
                        if(mMsgTemporaryIn.header.size > 0)
                        {
                            mMsgTemporaryIn.body.resize(mMsgTemporaryIn.header.size);
                            readBody();
                        }
                        else
                        {
                            // There are messages without body, which should be added to the queue as is
                            addToIncomingMessageQueue();
                        }
                    }
                    else
                    {
                        std::cout << "[" << id << "] Read Header Fail.\n";
                        mSocket.close();
                    }
                });
        }



        // ASYNC - prime context ready to read a message body
        void readBody()
        {
            asio::async_read(mSocket, asio::buffer(mMsgTemporaryIn.body.data(), mMsgTemporaryIn.body.size()),
                [this](std::error_code ec, std::size_t length){
                    if(!ec) 
                    {
                        addToIncomingMessageQueue();
                    }
                    else
                    {
                        std::cout << "[" << id << "] Read Body Fail.\n";
                        mSocket.close();
                    }
                });
        }



        // ASYNC - prime context ready to write a message header
        void writeHeader()
        {
            // Allocate transmission buffer to hold the message, issue the work - asio, send these bytes
            asio::async_write(mSocket, asio::buffer(&mQMessagesOut.front().header, sizeof(MessageHeader<T>)),
            [this](std::error_code ec, std::size_t length)
            {
                if(!ec) 
                {
                    // Check if the header also sent a message body with it
                    if(mQMessagesOut.front().body.size() > 0)
                    {
                        //mMsgTemporaryIn.body.resize(mMsgTemporaryIn.header.size);
                        writeBody();
                    }
                    else
                    {
                        mQMessagesOut.pop_front();

                        // Check if there are more messages to send
                        if(!mQMessagesOut.empty())
                            writeHeader();
                    }
                }
                else
                    {
                        std::cout << "[" << id << "] Write Header Fail.\n";
                        mSocket.close();
                    }
            });
        }


        // ASYNC - prime context ready to write a message body
        void writeBody()
        {
            asio::async_write(mSocket, asio::buffer(mQMessagesOut.front().body.data(), mQMessagesOut.front().body.size()),
                [this](std::error_code ec, std::size_t length){
                    if(!ec) 
                    {
                        // Sending was successful, so we are done with the message
                        mQMessagesOut.pop_front();

                        // If the queue still has messages in it, then issue the task to 
                        // send the next messages' header.
                        if (!mQMessagesOut.empty())
                            writeHeader();
                    }
                    else
                    {
                        // Sending failed, see WriteHeader() equivalent for description :P
                        std::cout << "[" << id << "] Write Body Fail.\n";
                        mSocket.close();
                    }
                });
        }


        // Add processed message to queue
        void addToIncomingMessageQueue()
        {
            if(mOwnerType == owner::server)
                mQMessagesIn.push_back({this->shared_from_this(), mMsgTemporaryIn});
            else
                mQMessagesIn.push_back({ nullptr, mMsgTemporaryIn});

            // Submit more work to the asio context
            readHeader();
        }
    protected:
        // Each connection has a unique socket to a remote
        asio::ip::tcp::socket mSocket;

        // This context is shared with the whole asio instance
        asio::io_context& mAsioContext;

        // This queue holds all messages to be sent to remote side of the connection
        TSQueue<Message<T>> mQMessagesOut;

        // This queue holds all messages that are received from the remote
        // side of the connection. Note it is a reference as the "owner"
        // of this connection is expected to provide a queue
        TSQueue<OwnedMessage<T>>& mQMessagesIn;

        // The owner decides how some of the connection behaves
        owner mOwnerType = owner::server;
        uint64_t id = 0;

        // Temporary variables
        Message<T> mMsgTemporaryIn;
    };
}