
// GetEveryRightDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "GetEveryRight.h"
#include "GetEveryRightDlg.h"
#include "AccessRights.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// �Ի�������
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

// ʵ��
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


// CGetEveryRightDlg �Ի���




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


// CGetEveryRightDlg ��Ϣ�������

BOOL CGetEveryRightDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// ��������...���˵�����ӵ�ϵͳ�˵��С�

	// IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
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

	// ���ô˶Ի����ͼ�ꡣ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO: �ڴ���Ӷ���ĳ�ʼ������
	CString cdgInstallPath = _T("C:\\Program Files\\EsafeNet\\Cobra DocGuard Client\\");
	GetDlgItem(IDC_EDIT_AccessRightsDir)->SetWindowText(cdgInstallPath);
	UpdateData(TRUE);

	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
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

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CGetEveryRightDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CGetEveryRightDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CGetEveryRightDlg::OnBnClickedOk()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
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
			MessageBox(_T("ִ�гɹ�^-^\r\n\r\n���ʴ�����д: ") + country + _T("\r\n") 
			           + _T("ϵͳ������д: ") + lang + _T("\r\n")
					   + _T("�û�������д: ") + userLang + _T("\r\n")
					   + _T("EveryoneȨ����: ") + everyone,
					   _T("ϵͳ��ʾ"), MB_OK);
		} 
		else
		{
			MessageBox(_T("ִ��ʧ��!!!"), _T("ϵͳ��ʾ"), MB_OK);
		}
	} 
	else
	{
		AfxMessageBox(_T("��Ч��Ŀ¼����������"));
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
	bi.lpszTitle = _T("��ѡ��Ŀ¼:");
	bi.ulFlags =0;
	bi.lpfn = NULL;
	bi.lParam =0;
	bi.iImage =0;
	//����ѡ��Ŀ¼�Ի���
	LPITEMIDLIST lp = SHBrowseForFolder(&bi);
	if (lp && SHGetPathFromIDList(lp, szDir))
	{
		m_everyoneRightName = szDir;
		GetDlgItem(IDC_EDIT_AccessRightsDir)->SetWindowText(szDir);
		UpdateData(TRUE);
	}
	else
	{
		AfxMessageBox(_T("��Ч��Ŀ¼��������ѡ��"));
	}
}

CString CGetEveryRightDlg::SelectDirectory()
{
    BROWSEINFO bi;
    TCHAR szDir[MAX_PATH] = {0};
    ZeroMemory(&bi, sizeof(BROWSEINFO));
    bi.hwndOwner = AfxGetMainWnd()->GetSafeHwnd();
    bi.pszDisplayName = szDir;
    bi.lpszTitle = _T("ѡ���ļ���Ŀ¼");
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
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
	//��ȡ�ļ�·��
	TCHAR szPath[MAX_PATH] = {0};
	DragQueryFile(hDropInfo, 0, szPath, MAX_PATH);
	//��ʾ���ؼ���
	GetDlgItem(IDC_EDIT_AccessRightsDir)->SetWindowText(szPath);
	UpdateData(TRUE);

	CDialog::OnDropFiles(hDropInfo);
}
