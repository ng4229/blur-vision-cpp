
// BlurManagerDlg.cpp: 구현 파일
//

#include "pch.h"
#include "framework.h"
#include "BlurManager.h"
#include "BlurManagerDlg.h"
#include "afxdialogex.h"
#include "IImageProcessor.h"
#include <filesystem>
#include <type_traits>
#include <format>
#include <chrono>
#include <windows.h>
#include <psapi.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace fs = std::filesystem;
using namespace CONFIG;

typedef IImageProcessor* (*CreateBlurInstanceFunc)();
typedef void (*DestroyBlurInstanceFunc)(IImageProcessor*);

ImageObjectPtr ProcessBlurWithDLL(const std::string& strDLLName, const ImageObjectPtr& srcImage, int nKernelSize)
{
	HMODULE hDll = LoadLibraryA(strDLLName.c_str());
	if (!hDll)
	{
		// #Log : DLL 로드 실패
		CLogger::Log(std::format("[ERROR] {} 로드 실패.", strDLLName));
		return nullptr;
	}

	CreateBlurInstanceFunc CreateInstance = (CreateBlurInstanceFunc)GetProcAddress(hDll, "CreateBlurInstance");
	DestroyBlurInstanceFunc DestroyInstance = (DestroyBlurInstanceFunc)GetProcAddress(hDll, "DestroyBlurInstance");

	if (!CreateInstance || !DestroyInstance) 
	{
		// #Log : DLL 함수 찾기 실패
		CLogger::Log(std::format("[DEBUG] {} blur 함수 찾기 실패.", strDLLName));
		FreeLibrary(hDll);
		return nullptr;
	}

	IImageProcessor* pBlurProcessor = CreateInstance();
	if (!pBlurProcessor) 
	{
		// #Log : DLL 인스턴스 생성 실패
		CLogger::Log(std::format("[DEBUG] {} 인스턴스 생성 실패.", strDLLName));
		FreeLibrary(hDll);
		return nullptr;
	}

	auto [height, width] = srcImage->getImageSize();
	ImageObjectPtr dstImage = std::make_shared<ImageObject>(height, width);
	CLogger::Log(std::format("[INFO] {} Blur 실행. [height:{}, width:{}, kernel:{}]", strDLLName, height, width, nKernelSize));
	
	if (!pBlurProcessor->ImageBlur(srcImage.get(), dstImage.get(), nKernelSize))
	{
		// #Log : 블러 적용 실패
		CLogger::Log(std::format("[DEBUG] {} Blur 적용 실패. 이미지 크기, 커널 사이즈 등 확인해주세요.[height:{}, width:{}, kernel:{}]", strDLLName, height, width, nKernelSize));
	}
	// 메모리 해제
	DestroyInstance(pBlurProcessor);
	FreeLibrary(hDll);

	return dstImage;
}

HBITMAP MatToHBITMAP(const cv::Mat& scrImage, int targetWidth, int targetHeight) 
{
	if (scrImage.empty()) return nullptr;

	// 크기 조정
	cv::Mat resizedImage;
	cv::resize(scrImage, resizedImage, cv::Size(targetWidth, targetHeight));

	cv::Mat dstImage;
	if (resizedImage.channels() == 1) 
	{
		cv::cvtColor(resizedImage, dstImage, cv::COLOR_GRAY2BGR); // 컬러로 변환
	}

	int width = dstImage.cols;
	int height = dstImage.rows;
	int bpp = dstImage.channels() * 8; // 24-bit 또는 32-bit

	BITMAPINFO bmi;
	ZeroMemory(&bmi, sizeof(BITMAPINFO));
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = width;
	bmi.bmiHeader.biHeight = -height;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = bpp;
	bmi.bmiHeader.biCompression = BI_RGB;

	void* bits = nullptr;
	HBITMAP hBitmap = CreateDIBSection(NULL, &bmi, DIB_RGB_COLORS, &bits, NULL, 0);
	if (!hBitmap) return nullptr;

	memcpy(bits, dstImage.data, width * height * dstImage.channels());
	return hBitmap;
}

double getCurrentTime() {
	using namespace std::chrono;
	return duration<double, std::milli>(steady_clock::now().time_since_epoch()).count();
}

SIZE_T GetMemoryUsage() {
	PROCESS_MEMORY_COUNTERS pmc;
	if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
		return pmc.WorkingSetSize; // 현재 프로세스 메모리 사용량 (bytes)
	}
	return 0;
}
// 응용 프로그램 정보에 사용되는 CAboutDlg 대화 상자입니다.

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

// 구현입니다.
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CBlurManagerDlg 대화 상자



CBlurManagerDlg::CBlurManagerDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_BLURMANAGER_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	// ini 파일 Load
	m_pConfigManager = std::make_shared<CConfigManager>();

}

void CBlurManagerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CBlurManagerDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_QUERYDRAGICON()
	ON_MESSAGE(WM_UPDATE_IMAGE, &CBlurManagerDlg::OnUpdateImage)
	ON_BN_CLICKED(IDC_BUTTON_LOAD_IMAGE, &CBlurManagerDlg::OnBnClickedButtonLoadImage)
END_MESSAGE_MAP()


// CBlurManagerDlg 메시지 처리기

BOOL CBlurManagerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 시스템 메뉴에 "정보..." 메뉴 항목을 추가합니다.

	// IDM_ABOUTBOX는 시스템 명령 범위에 있어야 합니다.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 이 대화 상자의 아이콘을 설정합니다.  응용 프로그램의 주 창이 대화 상자가 아닐 경우에는
	//  프레임워크가 이 작업을 자동으로 수행합니다.
	SetIcon(m_hIcon, TRUE);			// 큰 아이콘을 설정합니다.
	SetIcon(m_hIcon, FALSE);		// 작은 아이콘을 설정합니다.

	// 1. Logger 초기화
	CLogger::Init();
	CLogger::Log("Program on.");

	// 2. UI 초기화 (이미지 표현 창 위치 조정)
	initUI();

	return TRUE;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
}

void CBlurManagerDlg::initUI()
{
	// 다이얼로그 창 사이즈 가져오기
	CRect dlgRect;
	GetClientRect(&dlgRect);
	int dlgWidth = dlgRect.Width();
	int dlgHeight = dlgRect.Height();

	// 창 마진 설정
	int nMargin = 10;
	int nTopMargin = 40;
	int nBottomMargin = 100;

	// 가로 : 다이얼로그 가로의 50% 차지 (양 옆 마진 제외)
	int imgWidth = (dlgWidth - (nMargin * 2)) / 2;

	// 세로 : 다이얼로그 남은 공간(상,하 마진 + 바닥 마진)을 3등분하여 각 이미지가 1/3씩 차지
	int availableHeight = dlgHeight - nBottomMargin - nTopMargin - (nMargin * (IMAGE_COUNT + 1));
	int imgHeight = availableHeight / IMAGE_COUNT;

	// 이미지 디스플레이 위치 설정 (세로 3등분 배치)
	for (int type = TYPE_CUSTOM; type < TYPE_MAX; type++)
	{
		int xPos = nMargin + type * imgWidth;

		for (int i = 0; i < IMAGE_COUNT; i++)
		{
			int yPos = nTopMargin + nMargin + i * (imgHeight + nMargin);

			CRect rectControl(xPos, yPos, xPos + imgWidth, yPos + imgHeight);
			auto pictureControl = std::make_unique<CStatic>();

			if (!pictureControl->Create(_T(""), WS_CHILD | WS_VISIBLE | SS_BITMAP | SS_CENTERIMAGE, rectControl, this))
			{
				CLogger::Log("Picture Control 생성 실패!");
				return;
			}

			// 벡터에 추가
			m_mapPictureImage[type].push_back(std::move(pictureControl));
		}
	}
}

void CBlurManagerDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 대화 상자에 최소화 단추를 추가할 경우 아이콘을 그리려면
//  아래 코드가 필요합니다.  문서/뷰 모델을 사용하는 MFC 애플리케이션의 경우에는
//  프레임워크에서 이 작업을 자동으로 수행합니다.

// 사용자가 최소화된 창을 끄는 동안에 커서가 표시되도록 시스템에서
//  이 함수를 호출합니다.
HCURSOR CBlurManagerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CBlurManagerDlg::OnBnClickedButtonLoadImage()
{
	if (m_mapImagePtr.empty() == false)
	{
		m_mapImagePtr.clear();
	}

	std::thread([this]() {
		const std::string& strFolderPath = m_pConfigManager->getStrValue(CONFIG::SECTION_SYSTEM, CONFIG::SYSTEM_LOAD_IMAGE_PATH);
		const int nKernelSize = m_pConfigManager->getIntValue(CONFIG::SECTION_SYSTEM, CONFIG::SYSTEM_KERNEL_SIZE);

		// #Log : 로드 시작
		CLogger::Log(std::format("[INFO] 이미지 Load 시작."));

		// 파일 로드
		std::shared_ptr<std::vector<cv::Mat>> vecLoadedImage = std::make_shared<std::vector<cv::Mat>>();
		bool bLoadedImage = openFileAndLoadImage(strFolderPath, vecLoadedImage);

		if (bLoadedImage == false)	return;

		// #Log : 로드 완료
		CLogger::Log(std::format("[INFO] 이미지 Load 완료."));

		std::vector<std::thread> threads;
		for (int i = TYPE_CUSTOM; i < TYPE_MAX; i++) {
			const std::string strDLLName = (i == TYPE_CUSTOM) ? "Custom.dll" : "Opencv.dll";
			for (auto& image : *vecLoadedImage) {
				m_mapImagePtr[i].push_back(std::make_shared<ImageObject>(image.clone()));
			}

			if (i == TYPE_CUSTOM) {
				for (auto& imagePtr : m_mapImagePtr[i]) {
					threads.emplace_back([this, strDLLName, nKernelSize, &imagePtr]() mutable {
						DWORD threadID = GetCurrentThreadId();
						thread_local SIZE_T memBefore = GetMemoryUsage();

						CLogger::Log(std::format("[THREAD] Custom Blur 시작 - ThreadID: {}", threadID));
						auto startTime = getCurrentTime();

						imagePtr = ProcessBlurWithDLL(strDLLName, imagePtr, nKernelSize);

						auto endTime = getCurrentTime();
						CLogger::Log(std::format("[THREAD] Custom Blur 완료 ({}ms)- ThreadID: {}", static_cast<int>(endTime-startTime), threadID));

						thread_local SIZE_T memAfter = GetMemoryUsage();
						SIZE_T memoryUsed = memAfter - memBefore;
						CLogger::Log(std::format("[MEMORY] Custom Blur 메모리 사용량 ({:.3f}MB)- ThreadID: {}", memoryUsed / (1024.0 * 1024.0), threadID));
						});
				}
			} else {
				threads.emplace_back([&, i]() {
					DWORD threadID = GetCurrentThreadId();
					int nIdx = 1;
					for (auto& imagePtr : m_mapImagePtr[i]) {
						thread_local SIZE_T memBefore = GetMemoryUsage();

						CLogger::Log(std::format("[THREAD] OpenCV Blur 시작 - {}", nIdx));
						auto startTime = getCurrentTime();

						imagePtr = ProcessBlurWithDLL("Opencv.dll", imagePtr, nKernelSize);

						auto endTime = getCurrentTime();
						CLogger::Log(std::format("[THREAD] OpenCV Blur 완료 ({}ms) - {}", static_cast<int>(endTime - startTime), nIdx));

						thread_local SIZE_T memAfter = GetMemoryUsage();
						SIZE_T memoryUsed = memAfter - memBefore;
						CLogger::Log(std::format("[MEMORY] OpenCV Blur 메모리 사용량 ({:.3f}MB)- {}", memoryUsed / (1024.0 * 1024.0), nIdx++));
					}
				});
			}
		}

		for (auto& t : threads) {
			t.join();
		}

		// UI 업데이트
		PostMessage(WM_UPDATE_IMAGE, 0, 0);

		bool isEqual = true;
		for (int i = 0; i < m_mapImagePtr[TYPE_CUSTOM].size(); i++)
		{
			if(!checkImageEqual(m_mapImagePtr[TYPE_CUSTOM][i], m_mapImagePtr[TYPE_OPENCV][i]))
			{
				isEqual = false;
				CLogger::Log(std::format("[INFO] {}번째 이미지 픽셀값 불일치.", i + 1));
			}
		}

		if(isEqual)
		{
			CLogger::Log(std::format("[INFO] 전체 이미지 픽셀값 일치."));
		}

		const std::string& strSaveFolderPath = m_pConfigManager->getStrValue(CONFIG::SECTION_SYSTEM, CONFIG::SYSTEM_SAVE_IMAGE_PATH);
		
		// 폴더가 없으면 생성
		std::filesystem::path savePath(strSaveFolderPath);
		if (!std::filesystem::exists(savePath))
		{
			std::filesystem::create_directories(savePath);
		}
		for (int i = 0; i < m_mapImagePtr[TYPE_CUSTOM].size(); i++)
		{
			const std::string strDLLName = (i == TYPE_CUSTOM) ? "Custom" : "Opencv";
			int cnt = 1;
			for (auto& imagePtr : m_mapImagePtr[i]) {
				std::string strName = strSaveFolderPath + std::format("/{}_Image{}.jpg", strDLLName, cnt++);
				cv::imwrite(strName, imagePtr->convertToMat());
			}
		}
	}).detach();
}

LRESULT CBlurManagerDlg::OnUpdateImage(WPARAM wParam, LPARAM lParam)
{
	for (int i = TYPE_CUSTOM; i < TYPE_MAX; i++) {
		displayImage(m_mapImagePtr[i], m_mapPictureImage[i], m_mapHBitmap[i]);
	}

	return 0;
}

bool CBlurManagerDlg::openFileAndLoadImage(const std::string& strFolderPath, std::shared_ptr<std::vector<cv::Mat>>& vecLoadedImage)
{
	if (strFolderPath.empty() == true)	return false;

	for (const auto& entry : fs::directory_iterator(strFolderPath))
	{
		if (vecLoadedImage->size() >= 3)	break;

		if (entry.is_regular_file())
		{
			const std::string& strFile = entry.path().string();
			cv::Mat img = cv::imread(strFile, cv::IMREAD_GRAYSCALE);

			if (!img.empty())
			{
				// #Log : 파일명 로드 성공 (idx)
				CLogger::Log(std::format("[INFO] {} Load 성공. height:{}, width:{}, channels:{}", strFile, img.rows, img.cols, img.channels()));
				// 이미지 vector에 적재
				vecLoadedImage->push_back(std::move(img));
			}
			else
			{
				// #Log : warning : 파일명 로드 실패 (idx)
				CLogger::Log(std::format("[INFO] {} Load 실패.", strFile));
			}

			if (img.channels() != 1)
			{
				CLogger::Log(std::format("[WARNING] Load 종료. 1채널 이미지 아님, 파일명:{}", strFile));

				return false;
			}
		}
	}

	// 이미지 로드 성공시, true 반환
	return !vecLoadedImage->empty();
}

void CBlurManagerDlg::displayImage(const std::vector<ImageObjectPtr>& vecImagePtr, std::vector<std::unique_ptr<CStatic>>& pictureImage, std::vector<HBITMAP>& vecHBitmap) {
	if (vecImagePtr.empty()) return;

	// 기존 HBITMAP 제거
	for (auto& hBitmap : vecHBitmap)
	{
		if (hBitmap) 
		{
			DeleteObject(hBitmap);
		}
	}
	vecHBitmap.clear();
	
	int numImages = vecImagePtr.size();

	int nMargin = 10;
	int nTopMargin = 40;
	int nBottomMargin = 100;

	// Dialog 크기 get
	CRect rect;
	GetClientRect(&rect);
	std::pair<int, int> targetSize((rect.Width() - nMargin * 2) * 0.5, (rect.Height() - nTopMargin - nBottomMargin - nMargin * (numImages + 1)) / numImages);
	targetSize.first -= targetSize.first % 4;
	targetSize.second -= targetSize.second % 4;

	// 각 이미지 출력 
	for (int i = 0; i < numImages; i++) 
	{
		HBITMAP hBitmap = MatToHBITMAP(vecImagePtr[i]->convertToMat(), targetSize.first, targetSize.second);
		vecHBitmap.push_back(hBitmap);
		pictureImage[i]->SetBitmap(hBitmap);
	}
}

bool CBlurManagerDlg::checkImageEqual(const ImageObjectPtr image1, const ImageObjectPtr image2)
{
	cv::Mat img1 = image1->convertToMat();
	cv::Mat img2 = image2->convertToMat();

	if (img1.size() != img2.size() || img1.type() != img2.type()) {
		return false;  // 크기나 타입이 다르면 다른 이미지
	}

	cv::Mat compareImage;
	cv::absdiff(img1, img2, compareImage);  // 절대 차이 계산
	int nDiffCount = cv::countNonZero(compareImage);
	return nDiffCount == 0;  // 모든 픽셀이 0이면 동일한 이미지
}