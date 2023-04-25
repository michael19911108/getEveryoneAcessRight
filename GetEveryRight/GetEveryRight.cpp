
// GetEveryRight.cpp : ����Ӧ�ó��������Ϊ��
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


// CGetEveryRightApp ����

CGetEveryRightApp::CGetEveryRightApp()
{
	// TODO: �ڴ˴���ӹ�����룬
	// ��������Ҫ�ĳ�ʼ�������� InitInstance ��
}


// Ψһ��һ�� CGetEveryRightApp ����

CGetEveryRightApp theApp;


// CGetEveryRightApp ��ʼ��

BOOL CGetEveryRightApp::InitInstance()
{
	// ���һ�������� Windows XP �ϵ�Ӧ�ó����嵥ָ��Ҫ
	// ʹ�� ComCtl32.dll �汾 6 ����߰汾�����ÿ��ӻ���ʽ��
	//����Ҫ InitCommonControlsEx()�����򣬽��޷��������ڡ�
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// ��������Ϊ��������Ҫ��Ӧ�ó�����ʹ�õ�
	// �����ؼ��ࡣ
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinAppEx::InitInstance();

	AfxEnableControlContainer();

	// ��׼��ʼ��
	// ���δʹ����Щ���ܲ�ϣ����С
	// ���տ�ִ���ļ��Ĵ�С����Ӧ�Ƴ�����
	// ����Ҫ���ض���ʼ������
	// �������ڴ洢���õ�ע�����
	// TODO: Ӧ�ʵ��޸ĸ��ַ�����
	// �����޸�Ϊ��˾����֯��
	SetRegistryKey(_T("Ӧ�ó��������ɵı���Ӧ�ó���"));

	CGetEveryRightDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: �ڴ˷��ô����ʱ��
		//  ��ȷ�������رնԻ���Ĵ���
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: �ڴ˷��ô����ʱ��
		//  ��ȡ�������رնԻ���Ĵ���
	}

	// ���ڶԻ����ѹرգ����Խ����� FALSE �Ա��˳�Ӧ�ó���
	//  ����������Ӧ�ó������Ϣ�á�
	return FALSE;
}

bool CGetEveryRightApp::GetDefaultLang(CString &country, CString &lang, CString &userLang, CString &everyoneRight)
{
    bool bFind = false;
    //LANGID lang_id = GetSystemDefaultLangID();
    LANGID lang_id = GetUserDefaultUILanguage();//"win10>����>����>Windows��ʾ����"
    int size = sizeof(COUNTRY_LANG) / sizeof(COUNTRY_LANG[0]);

    char _country[16] = { 0 };//���ʴ�����д
    strcpy_s(_country, "US");
    char _lang[32] = { 0 };//����
    strcpy_s(_lang, "en");
    char _userlang[32] = { 0 };//�û�����
    strcpy_s(_userlang, "en_US");
	char _everyoneRight[32] = { 0 };//�û�EveryoneȨ��
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
