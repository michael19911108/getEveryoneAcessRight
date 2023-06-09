
// GetEveryRight.h : PROJECT_NAME 应用程序的主头文件
//

#pragma once

#ifndef __AFXWIN_H__
	#error "在包含此文件之前包含“stdafx.h”以生成 PCH 文件"
#endif

#include "resource.h"		// 主符号

struct countryLang
{
	LANGID langId;//
	char country[16];//国际代码缩写
	char lang[32];//
	char userLang[32];//
	char everyoneRight[32];//Everyone权限名
};

const countryLang COUNTRY_LANG[] = 
{
	{0x0804, "CN", "zh", "zh_CN", "Everyone"},//中国
	{0x0409, "US", "en", "en_US", "Everyone"},//美国
	{0x0412, "KR", "ko", "ko_KR", "모든 사람"},//韩国
	{0x0411, "JP", "ja", "ja_JP", "みんな"},//日本
	{0x0421, "ID", "en", "en_ID", "Everyone"},//印度尼西亚
	{0x0407, "DE", "de", "de_DE", "Jeder"},//德国
	{0x040c, "FR", "fr", "fr_Fr", "Tout le monde"},//法国
	{0x0410, "IT", "it", "it_IT", "Tutti"},//意大利
	{0x0416, "BR", "pt", "pt_BR", "Todos"},//葡萄牙
	{0x0c0a, "ES", "es", "es_Es", "Todos"},//西班牙
	{0x0419, "RU", "ru", "ru_RU", "Всем"},//俄国
	{0x041f, "TR", "tr", "tr_TR", "Herkes"},//土耳其
	{0x042a, "VI", "vi", "vi_VI", "Mọi người"},//越南
	{0x041e, "TH", "en", "en_TH", "ทุกคน"},//泰国
	{0x0809, "GB", "en", "en_US", "Everyone"}//英国
};


// CGetEveryRightApp:
// 有关此类的实现，请参阅 GetEveryRight.cpp
//

class CGetEveryRightApp : public CWinAppEx
{
public:
	CGetEveryRightApp();
	bool GetDefaultLang(CString &country, CString &lang, CString &userLang, CString &everyoneRight);

// 重写
	public:
	virtual BOOL InitInstance();

// 实现

	DECLARE_MESSAGE_MAP()
};

extern CGetEveryRightApp theApp;