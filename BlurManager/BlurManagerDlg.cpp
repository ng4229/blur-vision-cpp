﻿
// BlurManagerDlg.cpp: 구현 파일
//

#include "pch.h"
#include "framework.h"
#include "BlurManager.h"
#include "BlurManagerDlg.h"
#include "afxdialogex.h"
#include <filesystem>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

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
}

BEGIN_MESSAGE_MAP(CBlurManagerDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
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

	// TODO: 여기에 추가 초기화 작업을 추가합니다.

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
	
	if (bLoadedImage == false)	return;

	// 이미지 그리기 메세지
	UpdateImage();
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

				std::cout << "Loaded: " << entry.path().filename() << std::endl;
			}
			else
			{
				// #Log : warning : 파일명 로드 실패 (idx)
				std::cerr << "Failed to load: " << entry.path().filename() << std::endl;
			}
		}
	}

	// 이미지 로드 성공시, true 반환
	return !m_vecImagePtr.empty();
}

