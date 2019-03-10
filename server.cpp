#include <iostream>
#include <vector>

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/smart_ptr/enable_shared_from_this.hpp>

#include "Connection.hpp" // Must come before boost/serialization headers

#include "Task.hpp"

class Server : public boost::enable_shared_from_this<Server>
{
public:
	Server(boost::asio::io_context &io_context, unsigned short port)
			: m_acceptor(io_context, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port))
	{
		// Start an accept operation for a new Connection
		connection_ptr new_conn(new Connection(m_acceptor.get_io_context()));

		// Open the acceptor and start waiting for the first incoming connection
		m_acceptor.async_accept(new_conn->socket(), boost::bind(&Server::handle_accept, this,
																boost::asio::placeholders::error, new_conn));
	}

	// Handle completion of a accept operation
	void handle_accept(const boost::system::error_code &e, const connection_ptr &conn)
	{
		if (!e)
		{
			// Successfully accepted a new Connection. Receive the list of Tasks from the client. The
			// Connection::async_read() function will automatically deserialize the data structure for us
			conn->async_read(m_tasks,
							 boost::bind(&Server::handle_read, shared_from_this(), boost::asio::placeholders::error));
		}

		// Start an accept operation for a new Connection
		connection_ptr new_conn(new Connection(m_acceptor.get_io_context()));
		m_acceptor.async_accept(new_conn->socket(),
								boost::bind(&Server::handle_accept, shared_from_this(),
											boost::asio::placeholders::error, new_conn));
	}

	// Handle completion of a read operation
	void handle_read(const boost::system::error_code &e)
	{
		std::cerr << "Server: Reading data\n";
		if (!e)
		{
			// Display the data that was received
			for (const auto &task : m_tasks)
			{
				std::cout << task.to_string() << '\n';
				cv::imshow("rx", task.m_frame);
				cv::waitKey(1000);
				cv::destroyAllWindows();
			}
		}
		else
		{
			std::cerr << "Server error: " << e.message() << '\n';
		}
	}

private:
	// The acceptor object used to accept incoming socket connections
	boost::asio::ip::tcp::acceptor m_acceptor;

	// The data to be received from clients
	std::vector<Task> m_tasks;
};

int main(int argc, char *argv[])
{
	try
	{
		if (argc != 2)
		{
			std::cerr << "Usage: server <port>\n";
			exit(EXIT_FAILURE);
		}

		auto port = boost::lexical_cast<unsigned short>(argv[1]);

		boost::asio::io_context io_context;
		boost::shared_ptr<Server> server(new Server(io_context, port));
		io_context.run();
	}
	catch (std::exception &e)
	{
		std::cerr << "Server error: " << e.what() << '\n';
		exit(EXIT_FAILURE);
	}
}
