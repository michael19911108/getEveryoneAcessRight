
// GetEveryRightDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "GetEveryRight.h"
#include "GetEveryRightDlg.h"
#include "AccessRights.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CGetEveryRightDlg 对话框




CGetEveryRightDlg::CGetEveryRightDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CGetEveryRightDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CGetEveryRightDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_AccessRightsDir, m_everyoneRightName);	
}

BEGIN_MESSAGE_MAP(CGetEveryRightDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, &CGetEveryRightDlg::OnBnClickedOk)
    ON_BN_CLICKED(IDC_BTN_Select, &CGetEveryRightDlg::OnBnClickedBtnSelect)
	ON_WM_DROPFILES()
END_MESSAGE_MAP()


// CGetEveryRightDlg 消息处理程序

BOOL CGetEveryRightDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
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

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	CString cdgInstallPath = _T("C:\\Program Files\\EsafeNet\\Cobra DocGuard Client\\");
	GetDlgItem(IDC_EDIT_AccessRightsDir)->SetWindowText(cdgInstallPath);
	UpdateData(TRUE);

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CGetEveryRightDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CGetEveryRightDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CGetEveryRightDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CGetEveryRightDlg::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
    UpdateData(TRUE);
	if (!m_everyoneRightName.IsEmpty() && PathIsDirectory(m_everyoneRightName))
	{
		CString country; 
		CString lang;
		CString userLang;
		CString everyone;
		CGetEveryRightApp app;
		app.GetDefaultLang(country, lang, userLang, everyone);
		
		if (AddFileAccessRights(m_everyoneRightName, everyone, GENERIC_ALL))
		{
			MessageBox(_T("执行成功^-^\r\n\r\n国际代码缩写: ") + country + _T("\r\n") 
			           + _T("系统语言缩写: ") + lang + _T("\r\n")
					   + _T("用户语言缩写: ") + userLang + _T("\r\n")
					   + _T("Everyone权限名: ") + everyone,
					   _T("系统提示"), MB_OK);
		} 
		else
		{
			MessageBox(_T("执行失败!!!"), _T("系统提示"), MB_OK);
		}
	} 
	else
	{
		AfxMessageBox(_T("无效的目录，请重输入"));
	}
}

void CGetEveryRightDlg::OnBnClickedBtnSelect()
{
	TCHAR szDir[MAX_PATH] = {0};
	CString str;
	BROWSEINFO bi;
	bi.hwndOwner = m_hWnd;
	bi.pidlRoot = NULL;
	bi.pszDisplayName = (LPWSTR)szDir;
	bi.lpszTitle = _T("请选择目录:");
	bi.ulFlags =0;
	bi.lpfn = NULL;
	bi.lParam =0;
	bi.iImage =0;
	//弹出选择目录对话框
	LPITEMIDLIST lp = SHBrowseForFolder(&bi);
	if (lp && SHGetPathFromIDList(lp, szDir))
	{
		m_everyoneRightName = szDir;
		GetDlgItem(IDC_EDIT_AccessRightsDir)->SetWindowText(szDir);
		UpdateData(TRUE);
	}
	else
	{
		AfxMessageBox(_T("无效的目录，请重新选择"));
	}
}

CString CGetEveryRightDlg::SelectDirectory()
{
    BROWSEINFO bi;
    TCHAR szDir[MAX_PATH] = {0};
    ZeroMemory(&bi, sizeof(BROWSEINFO));
    bi.hwndOwner = AfxGetMainWnd()->GetSafeHwnd();
    bi.pszDisplayName = szDir;
    bi.lpszTitle = _T("选择文件夹目录");
    bi.ulFlags = BIF_RETURNFSANCESTORS;
    LPITEMIDLIST idl = SHBrowseForFolder(&bi);
    if (idl == NULL)
        return _T("");
    CString strDirectoryPath;
    SHGetPathFromIDList(idl, strDirectoryPath.GetBuffer(MAX_PATH));
    strDirectoryPath.ReleaseBuffer();
    if (strDirectoryPath.IsEmpty())
        return _T("");
    if (strDirectoryPath.Right(1) != _T("\\"))
        strDirectoryPath += _T("\\");
 
    return strDirectoryPath;
}

void CGetEveryRightDlg::OnDropFiles(HDROP hDropInfo)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	//获取文件路径
	TCHAR szPath[MAX_PATH] = {0};
	DragQueryFile(hDropInfo, 0, szPath, MAX_PATH);
	//显示到控件上
	GetDlgItem(IDC_EDIT_AccessRightsDir)->SetWindowText(szPath);
	UpdateData(TRUE);

	CDialog::OnDropFiles(hDropInfo);
}
