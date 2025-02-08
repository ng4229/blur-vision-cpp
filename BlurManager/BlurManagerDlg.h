
// BlurManagerDlg.h: 헤더 파일
//

#pragma once
#include "ImageObject.h"

// CBlurManagerDlg 대화 상자
class CBlurManagerDlg : public CDialogEx
{
// 생성입니다.
public:
	CBlurManagerDlg(CWnd* pParent = nullptr);	// 표준 생성자입니다.

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_BLURMANAGER_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 지원입니다.


// 구현입니다.
private:
	// 불러온 이미지 적재용
	std::vector<ImageObjectPtr> m_vecImagePtr;

	CStatic m_pictureImage[IMAGE_COUNT];

	// 이미지 UI 표현
	std::vector<HBITMAP> m_vecHBitmap;

protected:
	HICON m_hIcon;

	// 생성된 메시지 맵 함수
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButtonLoadImage();

	bool openFileAndLoadImage(const std::string& strFolderPath);

	void displayImage();

	void initUI();

};
