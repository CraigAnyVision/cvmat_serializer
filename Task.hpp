#pragma once

#include <vector>

#include <boost/serialization/access.hpp>
#include <boost/serialization/vector.hpp>

#include <opencv2/opencv.hpp>

#include "cvmat_serialization.hpp"

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
