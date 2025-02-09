#include "CCustomBlurProcessor.h"

void ThreadImageBlur(const ImageObject* src, ImageObject* dst, const CRect rectArea, const int kernelSize)
{
	// 예외처리
	if (src == nullptr || dst == nullptr || kernelSize < 21 || kernelSize % 2 == 0)	return;

	auto [height, width] = dst->getImageSize();
	
	const std::vector<uint8_t>& srcImageData = src->getImageData();
	std::vector<uint8_t>& dstImageData = dst->getImageData();

	double dWeight = 1.0 / (kernelSize * kernelSize);
	int nKernelPos = kernelSize / 2;

	int start_x = rectArea.left , end_x = rectArea.right
		, start_y = rectArea.top, end_y = rectArea.bottom;
	
	for (int y = start_y; y < end_y; y++) {
		for (int x = start_x; x < end_x; x++) {
			int sum = 0;

			for (int kernel_y = -nKernelPos; kernel_y <= nKernelPos; kernel_y++)
			{
				int target_y = y + kernel_y;

				// 대칭 형식 적용
				if (target_y < 0)
				{
					target_y = -target_y;	// 0 보다 낮으면 반전
				}
				else if (height <= target_y)
				{
					target_y = (height - 1) + ((height - 1) - target_y);	// height 보다 크면 height 보다 큰 값을 반전
				}

				for (int kernel_x = -nKernelPos; kernel_x <= nKernelPos; kernel_x++)
				{
					int target_x = x + kernel_x;

					// 대칭 형식 적용
					if (target_x < 0)
					{
						target_x = -target_x;	// 0 보다 낮으면 반전
					}
					else if (width <= target_x)
					{
						target_x = (width - 1) + ((width - 1) - target_x);	// width 보다 크면 width 보다 큰 값을 반전
					}

					sum += srcImageData[target_y * width + target_x];
				}
			}

			// blur 의 방식과 동일하게 반올림 방식으로 소수점 처리하여 filter 적용
			dstImageData[y * width + x] = static_cast<uint8_t>(std::round(sum * dWeight));
		}
	}

	return;
}

bool CCustomBlurProcessor::ImageBlur(const ImageObject* src, ImageObject* dst, const int kernelSize)
{
	// 예외처리
	if (src == nullptr || dst == nullptr || kernelSize < 21 || kernelSize % 2 == 0)	return false;

	auto [height, width] = dst->getImageSize();

	std::vector<std::thread> vecThread;
	const int nThreadCnt = 4;
	const int nThreadTotalCnt = nThreadCnt * nThreadCnt;
	for(int threadIdx = 0; threadIdx < nThreadTotalCnt; threadIdx++)
	{
		int splitWidth = width / nThreadCnt;
		int splitHeight = height / nThreadCnt;

		CRect rectThreadArea((threadIdx % nThreadCnt) * splitWidth, (threadIdx / nThreadCnt) * splitHeight
							, (threadIdx + 1) % nThreadCnt * splitWidth, (threadIdx / nThreadCnt + 1) * splitHeight);
		if ((threadIdx + 1) % nThreadCnt == 0)	rectThreadArea.right = width;
		if (nThreadCnt * (nThreadCnt - 1) <= threadIdx)				rectThreadArea.bottom = height;

		vecThread.emplace_back(ThreadImageBlur, src, dst, rectThreadArea, kernelSize);
	}

	for (auto& thread : vecThread)
	{
		thread.join();
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