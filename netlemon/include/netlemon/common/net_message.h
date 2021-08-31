#pragma once

#include "netlemon/net_common.h"

namespace netlemon::common
{
	// Templated Message Header is sent at start of all messages.
	// Template allows us to use "enum class" to ensure that all messages
	// are valid at compile time
	template<typename T>
	struct MessageHeader
	{
		T id{};
		uint64_t size = 0;
	};

	template<typename T>
	struct Message
	{
		MessageHeader<T> header{};
		std::vector<uint8_t> body; // uint8_t - to work with bytes
		
		// Returns size of entire message packet in bytes
		inline uint64_t Size() const { return body.size(); }

		// Friendly description of message
		friend std::ostream& operator << (std::ostream& os, const Message<T>& msg)
		{
			os << "\tID:" << (int)msg.header.id << "\tSize:" << msg.header.size;
			return os;
		}

		// Pushes POD-like data into the message buffer
		template<typename DataType>
		friend Message<T>& operator << (Message<T>& msg, const DataType& data)
		{
			static_assert(std::is_standard_layout<DataType>::value, "Data is too complex to be pushed into vector!");

			// Cache current size of vector, this will be the point we insert the data at
			uint64_t i = msg.body.size();

			// Resize the vector by the size of the data being pushed
			msg.body.resize(msg.body.size() + sizeof(DataType));

			// Physically copy the data to the newly allocated vector space
			std::memcpy(msg.body.data() + i, &data, sizeof(DataType));

			// Recalculate the message size
			msg.header.size = msg.Size();

			// Return target message so that multiple operations can be chained
			return msg;
		}

		// Pulling POD-like data into user variables
		template<typename DataType>
		friend Message<T>& operator >> (Message<T>& msg, DataType& data)
		{
			static_assert(std::is_standard_layout<DataType>::value, "Data is too complex to be pushed into vector!");

			// Cache current size of vector, this will be the point we insert the data at
			uint64_t i = msg.body.size() - sizeof(DataType);

			// Physically copy the data to the newly allocated vector space
			std::memcpy(&data, msg.body.data() + i, sizeof(DataType));

			// Shrink vector to remove read bytes, and reset end position
			msg.body.resize(i);

			// Recalculate message size
			msg.header.size = msg.Size();

			// Return target message so it can be "chained" with multiple operations
			return msg;
		}
	};

	// Forward declare the connection
	template<typename T>
	class Connection;

	template<typename T>
	struct OwnedMessage
	{
		// This will allow the server to respond back to the client 
		std::shared_ptr<Connection<T>> remote = nullptr;
		Message<T> msg;

		friend std::ostream& operator<<(std::ostream& os, const OwnedMessage<T>& msg)
		{
			os << msg.msg;
			return os;
		}
	};
}