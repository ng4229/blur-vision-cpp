#include "CCustomBlurProcessor.h"

bool CCustomBlurProcessor::ImageBlur(const ImageObject* src, ImageObject* dst, const int kernelSize)
{
	// 예외처리
	if (src == nullptr || dst == nullptr || kernelSize < 21 || kernelSize % 2 == 0)	return false;

	auto [height, width] = dst->getImageSize();

	const std::vector<uint8_t>& srcImageData = src->getImageData();
	std::vector<uint8_t>& dstImageData = dst->getImageData();

	int nKernelPos = kernelSize / 2;

	for(int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			int sum = 0, cnt = 0;
			
			for (int kernel_y = -nKernelPos; kernel_y <= nKernelPos; kernel_y++)
			{
				int target_y = y + kernel_y;
				if (target_y < 0 || height <= target_y) target_y = y - kernel_y;	// 대칭 형식 적용

				for (int kernel_x = -nKernelPos; kernel_x <= nKernelPos; kernel_x++)
				{
					int target_x = x + kernel_x;
					if (target_x < 0 || width <= target_x) target_x = x - kernel_x;	// 대칭 형식 적용

					sum += srcImageData[target_y * width + target_x];
					cnt++;
				}
			}

			dstImageData[y * width + x] = static_cast<uint8_t>(sum / cnt);
		}
	}

	return true;
}

extern "C" 
{
	__declspec(dllexport) IImageProcessor* CreateBlurInstance() 
	{
		return new CCustomBlurProcessor();
	}
	__declspec(dllexport) void DestroyBlurInstance(IImageProcessor* instance) 
	{
		delete instance;
	}
}