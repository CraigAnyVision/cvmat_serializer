#include <iostream>
#include <vector>

#include <boost/asio.hpp>
#include <boost/bind.hpp>

#include "Connection.hpp" // Must come before boost/serialization headers
#include <boost/serialization/vector.hpp>

#include "stock.hpp"

class Client
{
public:
	/// Constructor starts the asynchronous connect operation.
	Client(boost::asio::io_context &io_context, const std::string &host, const std::string &service)
			: m_connection(new Connection(io_context))
	{
		// Resolve the host name into an IP address.
		boost::asio::ip::tcp::resolver resolver(io_context);
		boost::asio::ip::tcp::resolver::query query(host, service);
		boost::asio::ip::tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);

		// Start an asynchronous connect operation.
		boost::asio::async_connect(m_connection->socket(), endpoint_iterator,
								   boost::bind(&Client::handle_connect, this, boost::asio::placeholders::error));

		// Create the data to be sent to the server
		stock s;
		s.code = "ABC";
		s.name = "A Big Company";
		s.open_price = 4.56;
		s.high_price = 5.12;
		s.low_price = 4.33;
		s.last_price = 4.98;
		s.buy_price = 4.96;
		s.buy_quantity = 1000;
		s.sell_price = 4.99;
		s.sell_quantity = 2000;
		m_stocks.push_back(s);
		s.code = "DEF";
		s.name = "Developer Entertainment Firm";
		s.open_price = 20.24;
		s.high_price = 22.88;
		s.low_price = 19.50;
		s.last_price = 19.76;
		s.buy_price = 19.72;
		s.buy_quantity = 34000;
		s.sell_price = 19.85;
		s.sell_quantity = 45000;
		m_stocks.push_back(s);

	}

	/// Handle completion of a connect operation.
	void handle_connect(const boost::system::error_code &e)
	{
		if (!e)
		{
			// Successfully established Connection. Start operation to write the list of stocks. The
			// Connection::async_write() function will automatically serialize the data that is written to the
			// underlying socket
			m_connection->async_write(m_stocks,
									  boost::bind(&Client::handle_write, this, boost::asio::placeholders::error,
												  m_connection));
		}
		else
		{
			// An error occurred. Log it and return. Since we are not starting a new
			// operation the io_context will run out of work to do and the client will
			// exit.
			std::cerr << e.message() << std::endl;
		}
	}

	/// Handle completion of a write operation.
	void handle_write(const boost::system::error_code &e, const connection_ptr &conn)
	{
		// Nothing to do. The socket will be closed automatically when the last
		// reference to the Connection object goes away
		std::cerr << "Client: Data written\n";
	}

private:
	/// The connection to the server
	connection_ptr m_connection;

	/// The data to be sent to the server
	std::vector<stock> m_stocks;
};

int main(int argc, char *argv[])
{
	try
	{
		if (argc != 3)
		{
			std::cerr << "Usage: client <host> <port>" << std::endl;
			return 1;
		}

		boost::asio::io_context io_context;
		Client client(io_context, argv[1], argv[2]);
		io_context.run();
	}
	catch (std::exception &e)
	{
		std::cerr << e.what() << '\n';
	}
}
