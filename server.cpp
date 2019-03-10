#include <iostream>
#include <vector>

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/smart_ptr/enable_shared_from_this.hpp>

#include "Connection.hpp" // Must come before boost/serialization headers
#include <boost/serialization/vector.hpp>

#include "stock.hpp"

/// Serves stock quote information to any client that connects to it
class Server : public boost::enable_shared_from_this<Server>
{
public:
	/// Constructor opens the acceptor and starts waiting for the first incoming connection
	Server(boost::asio::io_context &io_context, unsigned short port)
			: m_acceptor(io_context, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port))
	{
		// Start an accept operation for a new Connection.
		connection_ptr new_conn(new Connection(m_acceptor.get_io_context()));
		m_acceptor.async_accept(new_conn->socket(), boost::bind(&Server::handle_accept, this,
																boost::asio::placeholders::error, new_conn));
	}

	/// Handle completion of a accept operation.
	void handle_accept(const boost::system::error_code &e, connection_ptr conn)
	{
		if (!e)
		{
			// Successfully accepted a new Connection. Receive the list of stocks from the client. The
			// Connection::async_write() function will automatically deserialize the data structure for us
			conn->async_read(m_stocks,
							 boost::bind(&Server::handle_read, shared_from_this(), boost::asio::placeholders::error));
		}

		// Start an accept operation for a new Connection.
		connection_ptr new_conn(new Connection(m_acceptor.get_io_context()));
		m_acceptor.async_accept(new_conn->socket(),
								boost::bind(&Server::handle_accept, shared_from_this(),
											boost::asio::placeholders::error, new_conn));
	}

	/// Handle completion of a read operation.
	void handle_read(const boost::system::error_code &e)
	{
		std::cerr << "Server: Reading data\n";
		if (!e)
		{
			// Print out the data that was received.
			for (std::size_t i = 0; i < m_stocks.size(); ++i)
			{
				std::cout << "Stock number " << i << "\n";
				std::cout << "  code: " << m_stocks[i].code << "\n";
				std::cout << "  name: " << m_stocks[i].name << "\n";
				std::cout << "  open_price: " << m_stocks[i].open_price << "\n";
				std::cout << "  high_price: " << m_stocks[i].high_price << "\n";
				std::cout << "  low_price: " << m_stocks[i].low_price << "\n";
				std::cout << "  last_price: " << m_stocks[i].last_price << "\n";
				std::cout << "  buy_price: " << m_stocks[i].buy_price << "\n";
				std::cout << "  buy_quantity: " << m_stocks[i].buy_quantity << "\n";
				std::cout << "  sell_price: " << m_stocks[i].sell_price << "\n";
				std::cout << "  sell_quantity: " << m_stocks[i].sell_quantity << "\n";
			}
		}
		else
		{
			// An error occurred.
			std::cerr << "Server error: " << e.message() << std::endl;
		}
	}

private:
	/// The acceptor object used to accept incoming socket connections
	boost::asio::ip::tcp::acceptor m_acceptor;

	/// The data to be received from clients
	std::vector<stock> m_stocks;
};

int main(int argc, char *argv[])
{
	try
	{
		if (argc != 2)
		{
			std::cerr << "Usage: server <port>\n";
			return 1;
		}

		auto port = boost::lexical_cast<unsigned short>(argv[1]);

		boost::asio::io_context io_context;
		boost::shared_ptr<Server> server(new Server(io_context, port));
		io_context.run();
	}
	catch (std::exception &e)
	{
		std::cerr << "Server error: " << e.what() << '\n';
	}
}
