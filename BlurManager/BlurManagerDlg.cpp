
// BlurManagerDlg.cpp: 구현 파일
//

#include "pch.h"
#include "framework.h"
#include "BlurManager.h"
#include "BlurManagerDlg.h"
#include "afxdialogex.h"
#include "IImageProcessor.h"
#include <filesystem>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace fs = std::filesystem;

typedef IImageProcessor* (*CreateBlurInstanceFunc)();
typedef void (*DestroyBlurInstanceFunc)(IImageProcessor*);

ImageObjectPtr ProcessBlurWithDLL(const std::string& strDLLName, const ImageObjectPtr& srcImage, int nKernelSize)
{
	HMODULE hDll = LoadLibraryA(strDLLName.c_str());
	if (!hDll)
	{
		// #Log : DLL 로드 실패
		return nullptr;
	}

	CreateBlurInstanceFunc CreateInstance = (CreateBlurInstanceFunc)GetProcAddress(hDll, "CreateBlurInstance");
	DestroyBlurInstanceFunc DestroyInstance = (DestroyBlurInstanceFunc)GetProcAddress(hDll, "DestroyBlurInstance");

	if (!CreateInstance || !DestroyInstance) 
	{
		// #Log : DLL 함수 찾기 실패
		FreeLibrary(hDll);
		return nullptr;
	}

	IImageProcessor* pBlurProcessor = CreateInstance();
	if (!pBlurProcessor) 
	{
		// #Log : DLL 인스턴스 생성 실패
		FreeLibrary(hDll);
		return nullptr;
	}

	ImageObjectPtr dstImage = std::make_shared<ImageObject>();
	if (pBlurProcessor->ImageBlur(srcImage.get(), dstImage.get(), nKernelSize))
	{
		// #Log : 블러 적용 완료
	}
	else {
		// #Log : 블러 적용 실패
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
}

void CBlurManagerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_IMAGE_1, m_pictureImage[0]);
	DDX_Control(pDX, IDC_STATIC_IMAGE_2, m_pictureImage[1]);
	DDX_Control(pDX, IDC_STATIC_IMAGE_3, m_pictureImage[2]);
}

BEGIN_MESSAGE_MAP(CBlurManagerDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_QUERYDRAGICON()
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

	// 1. UI 초기화 (이미지 표현 창 위치 조정)
	initUI();

	// 2. ini 파일 Load


	// 3. 

	return TRUE;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
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
	if (m_vecImagePtr.empty() == false)
	{
		m_vecImagePtr.clear();
	}

	const std::string& strFolderPath = "D:\\1.Program\\98.Study\\blur-vision-cpp\\LoadImage\\";

	// #Log : 로드 시작
	// 파일 로드
	bool bLoadedImage = openFileAndLoadImage(strFolderPath);

	// #Log : 로드 완료
	static int nSize = 3;
	if (bLoadedImage == false)	return;
	
	for (auto& image : m_vecImagePtr)
		image = ProcessBlurWithDLL("OpenCVBlurDLL.dll", image, nSize);
	nSize += 10;
	// 이미지 그리기 메세지
	displayImage();
}

bool CBlurManagerDlg::openFileAndLoadImage(const std::string& strFolderPath)
{
	if (strFolderPath.empty() == true)	return false;

	for (const auto& entry : fs::directory_iterator(strFolderPath))
	{
		if (entry.is_regular_file())
		{
			cv::Mat img = cv::imread(entry.path().string(), cv::IMREAD_GRAYSCALE);
			if (!img.empty())
			{
				// #Log : 파일명 로드 성공 (idx)
				// 이미지 vector에 적재
				m_vecImagePtr.push_back(std::make_shared<ImageObject>(img));
			}
			else
			{
				// #Log : warning : 파일명 로드 실패 (idx)
			}
		}
	}

	// 이미지 로드 성공시, true 반환
	return !m_vecImagePtr.empty();
}

void CBlurManagerDlg::displayImage() {
	if (m_vecImagePtr.empty()) return;

	// 기존 HBITMAP 제거
	for (auto& hBitmap : m_vecHBitmap) 
	{
		if (hBitmap) 
		{
			DeleteObject(hBitmap);
		}
	}
	m_vecHBitmap.clear();
	
	int numImages = m_vecImagePtr.size();

	int nMargin = 10;
	int nBottomMargin = 100;

	// Dialog 크기 get
	CRect rect;
	GetClientRect(&rect);
	std::pair<int, int> targetSize((rect.Width() - nMargin * 2) * 0.5, (rect.Height() - nBottomMargin - nMargin * (numImages + 1)) / numImages);
	targetSize.first -= targetSize.first % 4;
	targetSize.second -= targetSize.second % 4;

	// 각 이미지 출력 
	for (int i = 0; i < numImages; i++) 
	{
		HBITMAP hBitmap = MatToHBITMAP(m_vecImagePtr[i]->convertToMat(), targetSize.first, targetSize.second);
		m_vecHBitmap.push_back(hBitmap);
		m_pictureImage[i].SetBitmap(hBitmap);
	}
}

void CBlurManagerDlg::initUI()
{
	// 다이얼로그 창 사이즈 가져오기
	CRect dlgRect;
	GetClientRect(&dlgRect);
	int dlgWidth = dlgRect.Width();
	int dlgHeight = dlgRect.Height();

	// 창 마진 설정
	int margin = 10;
	int nBottomMargin = 100;

	// 가로 : 다이얼로그 가로의 50% 차지 (양 옆 마진 제외)
	int imgWidth = (dlgWidth - (margin * 2)) / 2;

	// 세로 : 다이얼로그 남은 공간(상,하 마진 + 바닥 마진)을 3등분하여 각 이미지가 1/3씩 차지
	int availableHeight = dlgHeight - nBottomMargin - (margin * (IMAGE_COUNT + 1));
	int imgHeight = availableHeight / IMAGE_COUNT;

	// 이미지 디스플레이 위치 설정 (세로 3등분 배치)
	for (int i = 0; i < IMAGE_COUNT; i++) 
	{
		int xPos = margin;
		int yPos = margin + i * (imgHeight + margin);

		m_pictureImage[i].SetWindowPos(NULL, xPos, yPos, imgWidth, imgHeight, SWP_NOZORDER);
	}
}
