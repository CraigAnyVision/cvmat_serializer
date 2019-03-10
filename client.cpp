#include <iostream>
#include <vector>

#include <boost/asio.hpp>
#include <boost/bind.hpp>

#include <opencv2/highgui/highgui.hpp>

#include "Connection.hpp" // Must come before boost/serialization headers

#include "Task.hpp"

class Client
{
public:
	// Constructor starts the asynchronous connect operation
	Client(boost::asio::io_context &io_context, const std::string &host, const std::string &service)
			: m_connection(new Connection(io_context))
	{
		// Resolve the host name into an IP address
		boost::asio::ip::tcp::resolver resolver(io_context);
		boost::asio::ip::tcp::resolver::query query(host, service);
		boost::asio::ip::tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);

		// Start an asynchronous connect operation
		boost::asio::async_connect(m_connection->socket(), endpoint_iterator,
								   boost::bind(&Client::handle_connect, this, boost::asio::placeholders::error));

		// Create the data to be sent to the server
		std::string test_file{"../sunny.png"};
		cv::Mat image = cv::imread(test_file);

		std::vector<float> features;

		for (size_t i = 0; i < Task::num_features; ++i)
		{
			features.push_back(i);
		}

		m_tasks.emplace_back(123, 456, image, features, "Hula");
		m_tasks.emplace_back(321, 654, image, features, "Nurse");

	}

	// Handle completion of a connect operation
	void handle_connect(const boost::system::error_code &e)
	{
		if (!e)
		{
			// Successfully established Connection. Start operation to write the list of Tasks. The
			// Connection::async_write() function will automatically serialize the data that is written to the
			// underlying socket
			m_connection->async_write(m_tasks,
									  boost::bind(&Client::handle_write, this, boost::asio::placeholders::error,
												  m_connection));
		}
		else
		{
			// An error occurred. Log it and return. Since we are not starting a new operation the io_context will run
			// out of work to do and the client will exit
			std::cerr << e.message() << '\n';
		}
	}

	// Handle completion of a write operation
	void handle_write(const boost::system::error_code &e, const connection_ptr &conn)
	{
		// Nothing to do. The socket will be closed automatically when the last
		// reference to the Connection object goes away
		std::cerr << "Client: Data written\n";
	}

private:
	connection_ptr m_connection;
	std::vector<Task> m_tasks;
};

int main(int argc, char *argv[])
{
	try
	{
		if (argc != 3)
		{
			std::cerr << "Usage: client <host> <port>\n";
			exit(EXIT_FAILURE);
		}

		boost::asio::io_context io_context;
		Client client(io_context, argv[1], argv[2]);
		io_context.run();
	}
	catch (std::exception &e)
	{
		std::cerr << e.what() << '\n';
		exit(EXIT_FAILURE);
	}
}
