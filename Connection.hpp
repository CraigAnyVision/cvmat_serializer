#pragma once

#include <iomanip>
#include <string>
#include <sstream>
#include <vector>

#include <boost/asio.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/smart_ptr/enable_shared_from_this.hpp>
#include <boost/tuple/tuple.hpp>

/**
 * Provides serialization primitives on top of a socket
 *
 * Each message sent using this class consists of:
 *  - An 8-byte header containing the length of the serialized data in hexadecimal
 *  - The serialized data
 */
class Connection : public boost::enable_shared_from_this<Connection>
{
public:
	explicit Connection(boost::asio::io_context &io_context)
			: m_socket(io_context)
	{
	}

	// Get the underlying socket. Used for making a connection or for accepting an incoming connection
	boost::asio::ip::tcp::socket &socket()
	{
		return m_socket;
	}

	// Asynchronously write a data structure to the socket
	template<typename T, typename Handler>
	void async_write(const T &data, Handler handler)
	{
		// Serialize the data first so we know how large it is
		std::ostringstream archive_stream;
		boost::archive::binary_oarchive archive(archive_stream);
		archive << data;
		m_outbound_data = archive_stream.str();

		// Format the header
		std::ostringstream header_stream;
		header_stream << std::setw(header_length) << std::hex << m_outbound_data.size();

		if (!header_stream || header_stream.str().size() != header_length)
		{
			// Something went wrong, inform the caller
			boost::system::error_code error(boost::asio::error::invalid_argument);
			boost::asio::post(m_socket.get_executor(), boost::bind(handler, error));
			return;
		}
		m_outbound_header = header_stream.str();

		// Write the serialized data to the socket. We use "gather-write" to send
		// both the header and the data in a single write operation
		std::vector<boost::asio::const_buffer> buffers;
		buffers.emplace_back(boost::asio::buffer(m_outbound_header));
		buffers.emplace_back(boost::asio::buffer(m_outbound_data));
		boost::asio::async_write(m_socket, buffers, handler);
	}

	// Asynchronously read a data structure from the socket
	template<typename T, typename Handler>
	void async_read(T &t, Handler handler)
	{
		// Issue a read operation to read exactly the number of bytes in a header
		void (Connection::*f)(const boost::system::error_code &, T &,
							  boost::tuple<Handler>) = &Connection::handle_read_header<T, Handler>;

		boost::asio::async_read(m_socket, boost::asio::buffer(m_inbound_header),
								boost::bind(f, shared_from_this(), boost::asio::placeholders::error, boost::ref(t),
											boost::make_tuple(handler)));
	}

	// Handle a completed read of a message header. The handler is passed using a tuple since boost::bind seems
	// to have trouble binding a function object created using boost::bind as a parameter
	template<typename T, typename Handler>
	void handle_read_header(const boost::system::error_code &e, T &t, boost::tuple<Handler> handler)
	{
		if (e)
		{
			boost::get<0>(handler)(e);
		}
		else
		{
			// Determine the length of the serialized data
			std::istringstream is(std::string(m_inbound_header, header_length));
			size_t inbound_data_size = 0;
			if (!(is >> std::hex >> inbound_data_size))
			{
				// Header doesn't seem to be valid. Inform the caller
				boost::system::error_code error(boost::asio::error::invalid_argument);
				boost::get<0>(handler)(error);
				return;
			}

			// Start an asynchronous call to receive the data
			m_inbound_data.resize(inbound_data_size);
			void (Connection::*f)(const boost::system::error_code &, T &, boost::tuple<Handler>)
			= &Connection::handle_read_data<T, Handler>;

			boost::asio::async_read(m_socket, boost::asio::buffer(m_inbound_data),
									boost::bind(f, shared_from_this(), boost::asio::placeholders::error,
												boost::ref(t), handler));
		}
	}

	// Handle a completed read of message data
	template<typename T, typename Handler>
	void handle_read_data(const boost::system::error_code &e, T &t, boost::tuple<Handler> handler)
	{
		if (e)
		{
			boost::get<0>(handler)(e);
		}
		else
		{
			// Extract the data structure from the data just received
			try
			{
				std::string archive_data(&m_inbound_data[0], m_inbound_data.size());
				std::istringstream archive_stream(archive_data);
				boost::archive::binary_iarchive archive(archive_stream);
				archive >> t;
			}
			catch (std::exception &e)
			{
				// Unable to decode data
				boost::system::error_code error(boost::asio::error::invalid_argument);
				boost::get<0>(handler)(error);
				return;
			}

			// Inform caller that data has been received ok
			boost::get<0>(handler)(e);
		}
	}

private:
	static constexpr size_t header_length = 8;

	boost::asio::ip::tcp::socket m_socket;
	std::string m_outbound_header;
	std::string m_outbound_data;
	char m_inbound_header[header_length];
	std::vector<char> m_inbound_data;
};

typedef boost::shared_ptr<Connection> connection_ptr;
