
// BlurManagerDlg.h: 헤더 파일
//

#pragma once
#include "ImageObject.h"
#include "ConfigManager.h"
#include "CLogger.h"

enum TYPE_PROCESSOR {
	TYPE_CUSTOM = 0,
	TYPE_OPENCV,
	TYPE_MAX
};

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
	std::map<int, std::vector<ImageObjectPtr>> m_mapImagePtr;	 // Image 저장 객체
	std::map<int, std::vector<std::unique_ptr<CStatic>>> m_mapPictureImage;	// Picture control
	std::map<int, std::vector<HBITMAP>> m_mapHBitmap;		// 이미지 UI 표현

	std::shared_ptr<CConfigManager> m_pConfigManager;


protected:
	HICON m_hIcon;

	// 생성된 메시지 맵 함수
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButtonLoadImage();

	LRESULT OnUpdateImage(WPARAM wParam, LPARAM lParam);

	void initUI();

	bool openFileAndLoadImage(const std::string& strFolderPath, std::shared_ptr<std::vector<cv::Mat>>& vecLoadedImage);

	void displayImage(const std::vector<ImageObjectPtr>& vecImagePtr, std::vector<std::unique_ptr<CStatic>>& pictureImage, std::vector<HBITMAP>& vecHBitmap);

	bool checkImageEqual(const ImageObjectPtr image1, const ImageObjectPtr image2);


};
