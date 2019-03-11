#pragma once

#include <vector>

#include <boost/serialization/access.hpp>
#include <boost/serialization/split_member.hpp>
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

	std::string to_string() const
	{
		std::string ret = m_name + ", " + std::to_string(m_frame_id);

		for (size_t i = 0; i < num_features; ++i)
		{
			ret += " " + std::to_string(m_features[i]);
		}

		return ret;
	}

	void jpeg_encode() const
	{
		std::vector<int> param{cv::IMWRITE_JPEG_QUALITY, 100};
		cv::imencode(".jpg", m_frame, m_jpeg, param);
	}

	void jpeg_decode()
	{
		cv::imdecode(m_jpeg, CV_LOAD_IMAGE_COLOR, &m_frame);
	}

	static constexpr size_t num_features = 256;
	size_t m_frame_id;
	cv::Mat m_frame;
	mutable std::vector<uchar> m_jpeg;
	std::vector<float> m_features;
	std::string m_name;

private:
	friend class boost::serialization::access;

	template<class Archive>
	void save(Archive &ar, const unsigned int version) const
	{
		// compress the cv::Mat
		this->jpeg_encode();

		// List all the fields to be serialized
		ar & m_frame_id;
		ar & m_jpeg;
		ar & m_features;
		ar & m_name;
	}

	template<class Archive>
	void load(Archive &ar, const unsigned int version)
	{
		// List all the fields to be deserialized
		ar & m_frame_id;
		ar & m_jpeg;
		ar & m_features;
		ar & m_name;

		// decompress the jpeg
		this->jpeg_decode();
	}
	BOOST_SERIALIZATION_SPLIT_MEMBER()
};

BOOST_CLASS_VERSION(Task, 1)
