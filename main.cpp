#include <iostream>

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/vector.hpp>

#include <opencv2/highgui/highgui.hpp>

#include "mat_serialization.hpp"

class Task
{
public:
	Task() = default;

	Task(size_t camera_id, size_t frame_id, cv::Mat frame, std::vector<float> features, std::string name)
			: m_frame_id(frame_id), m_frame(std::move(frame)), m_features(std::move(features)),
			  m_name(std::move(name))
	{}

	std::string print()
	{
		std::string ret = m_name + ", " + std::to_string(m_frame_id);

		for (size_t i = 0; i < num_features; ++i)
		{
			ret += " " + std::to_string(m_features[i]);
		}

		return ret;
	}

	static const size_t num_features = 256;
	size_t m_frame_id;
	cv::Mat m_frame;
	std::vector<float> m_features;
	std::string m_name;

private:
	friend class boost::serialization::access;

	template<class Archive>
	void serialize(Archive &ar, const unsigned int version)
	{
		// List all the fields to be serialized/deserialized
		ar & m_frame_id;
		ar & m_frame;
		ar & m_features;
		ar & m_name;
	}
};

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

	// 4150 as text archive, 1105 as binary
	std::cout << oss.str().size() << '\n';

	// Deserialize
	Task task_out;
	{
		std::istringstream iss(oss.str());
		boost::archive::binary_iarchive ia(iss);
		ia >> task_out;
	}

	std::cout << task_out.print() << '\n';

	cv::imshow("task2", task_out.m_frame);
	cv::waitKey();
}
