
// GetEveryRight.cpp : 定义应用程序的类行为。
//

#include "stdafx.h"
#include "GetEveryRight.h"
#include "GetEveryRightDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CGetEveryRightApp

BEGIN_MESSAGE_MAP(CGetEveryRightApp, CWinAppEx)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CGetEveryRightApp 构造

CGetEveryRightApp::CGetEveryRightApp()
{
	// TODO: 在此处添加构造代码，
	// 将所有重要的初始化放置在 InitInstance 中
}


// 唯一的一个 CGetEveryRightApp 对象

CGetEveryRightApp theApp;


// CGetEveryRightApp 初始化

BOOL CGetEveryRightApp::InitInstance()
{
	// 如果一个运行在 Windows XP 上的应用程序清单指定要
	// 使用 ComCtl32.dll 版本 6 或更高版本来启用可视化方式，
	//则需要 InitCommonControlsEx()。否则，将无法创建窗口。
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// 将它设置为包括所有要在应用程序中使用的
	// 公共控件类。
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinAppEx::InitInstance();

	AfxEnableControlContainer();

	// 标准初始化
	// 如果未使用这些功能并希望减小
	// 最终可执行文件的大小，则应移除下列
	// 不需要的特定初始化例程
	// 更改用于存储设置的注册表项
	// TODO: 应适当修改该字符串，
	// 例如修改为公司或组织名
	SetRegistryKey(_T("应用程序向导生成的本地应用程序"));

	CGetEveryRightDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: 在此放置处理何时用
		//  “确定”来关闭对话框的代码
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: 在此放置处理何时用
		//  “取消”来关闭对话框的代码
	}

	// 由于对话框已关闭，所以将返回 FALSE 以便退出应用程序，
	//  而不是启动应用程序的消息泵。
	return FALSE;
}

bool CGetEveryRightApp::GetDefaultLang(CString &country, CString &lang, CString &userLang, CString &everyoneRight)
{
    bool bFind = false;
    //LANGID lang_id = GetSystemDefaultLangID();
    LANGID lang_id = GetUserDefaultUILanguage();//"win10>设置>语言>Windows显示语言"
    int size = sizeof(COUNTRY_LANG) / sizeof(COUNTRY_LANG[0]);

    char _country[16] = { 0 };//国际代码缩写
    strcpy_s(_country, "US");
    char _lang[32] = { 0 };//语言
    strcpy_s(_lang, "en");
    char _userlang[32] = { 0 };//用户语言
    strcpy_s(_userlang, "en_US");
	char _everyoneRight[32] = { 0 };//用户Everyone权限
    strcpy_s(_everyoneRight, "Everyone");

    for (int i = 0; i < size; i++)
    {
        if (lang_id == COUNTRY_LANG[i].langId)
        {
            memset(_country, 0, sizeof(_country));
            memset(_lang, 0, sizeof(_lang));
            memset(_userlang, 0, sizeof(_userlang));
			memset(_everyoneRight, 0, sizeof(_everyoneRight));
            strcpy(_country, COUNTRY_LANG[i].country);
            strcpy(_lang, COUNTRY_LANG[i].lang);
            strcpy(_userlang, COUNTRY_LANG[i].userLang);
			strcpy(_everyoneRight, COUNTRY_LANG[i].everyoneRight);
            bFind = true;
            break;
        }
    }
    country = CString(_country);
    lang = CString(_lang);
    userLang = CString(_userlang);
	everyoneRight = CString(_everyoneRight);
    return bFind;
}
