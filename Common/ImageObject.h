#pragma once
#include <vector>
#include <opencv2/opencv.hpp>

class ImageObject;
typedef std::shared_ptr<ImageObject> ImageObjectPtr;

class ImageObject
{
private:
	uint32_t m_kWidth;
	uint32_t m_kHeight;
	std::vector<uint8_t> m_vecData;

public:
	// 기본 생성자 (이미지 크기 0 x 0 으로 기본 셋팅)
	ImageObject(int height, int width) : m_kHeight(height), m_kWidth(width) 
	{
		m_vecData.resize(height * width, 0);
	}
	
	// 매개변수 : cv::Mat
	// 초기화할 image 받는 생성자
	ImageObject(const cv::Mat& image) 
	{
		if (image.type() != CV_8UC1)	// Grayscale 외의 이미지는 Exception 발생
		{
			throw std::invalid_argument("Only grayscale images are supported.");
		}
		m_kWidth = image.cols;
		m_kHeight = image.rows;

        m_vecData.assign(image.data, image.data + (m_kHeight * m_kWidth));
	}
	
	// 기본 소멸자
	~ImageObject() {};

	// return type : cv::Mat
	// return value : Image
	// 기능 : Buffer data를 cv::Mat으로 변환 메서드
	cv::Mat convertToMat() const
	{
		return cv::Mat(m_kHeight, m_kWidth, CV_8UC1, (void*)m_vecData.data()).clone();
	}

	// return type : std::pair<uint32_t, uint32_t>
	// return value : (Height, Width)
	// 기능 : 이미지 크기 반환 메서드
	const std::vector<uint8_t>& getImageData() const
	{
		return m_vecData;
	}
	std::vector<uint8_t>& getImageData()
	{
		return m_vecData;
	}

	// return type : std::pair<uint32_t, uint32_t>
	// return value : (Height, Width)
	// 기능 : 이미지 크기 반환 메서드
	std::pair<uint32_t, uint32_t> getImageSize() const
	{
		return { m_kHeight, m_kWidth };
	}
};

