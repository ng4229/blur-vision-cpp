#include "COpenCVBlurProcessor.h"
#include "ImageObject.h"

bool COpenCVBlurProcessor::ImageBlur(const ImageObject* src, ImageObject* dst, const int kernelSize)
{
	// 예외처리
	if (src == nullptr || dst == nullptr || kernelSize < 21 || kernelSize % 2 == 0)	return false;

	const cv::Mat srcImage = src->convertToMat();
	// 비어있는지 확인
	if (srcImage.empty())	return false;

	// blur() 실행
	cv::Mat dstImage;
	cv::blur(srcImage, dstImage, cv::Size(kernelSize, kernelSize));

	// dst 객체에 값 저장
	*dst = ImageObject(dstImage);

	return true;
}

extern "C" 
{
	__declspec(dllexport) IImageProcessor* CreateBlurInstance()
	{
		return new COpenCVBlurProcessor();
	}
	__declspec(dllexport) void DestroyBlurInstance(IImageProcessor* instance)
	{
		delete instance;
	}
}