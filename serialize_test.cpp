#include <iostream>

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>

#include <opencv2/highgui/highgui.hpp>

#include "Task.hpp"

int main()
{
	std::string test_file{"../sunny.png"};
	cv::Mat image = cv::imread(test_file);

	std::vector<float> features;

	for (size_t i = 0; i < Task::num_features; ++i)
	{
		features.push_back(i);
	}

	Task task_in(123, 456, image, features, "Hula");

	// Serialize
	std::ostringstream oss;
	{
		boost::archive::binary_oarchive oa(oss);
		oa << task_in;
	}

	// Without jpeg compression: 733966
	// With jpeg compression:    207318
	std::cout << oss.str().size() << '\n';

	// Clear the Mat (to prove deserialization works as expected)
	task_in.m_frame = cv::Scalar(0,0,0);

	// Deserialize
	Task task_out;
	{
		std::istringstream iss(oss.str());
		boost::archive::binary_iarchive ia(iss);
		ia >> task_out;
	}

	std::cout << task_out.to_string() << '\n';

	cv::imshow("task_out", task_out.m_frame);
	cv::waitKey();
}
