#ifndef _ACCESS_RIGHTS_H_
#define _ACCESS_RIGHTS_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <tchar.h>
#include <Windows.h>
#include <accctrl.h>

// lpszFileName    �C �ļ���Ŀ¼����
// lpszAccountName �C �û����飩��
// dwAccessMask    -- GENERIC_ALL��ʾ���е�Ȩ�ޣ�����һϵ�е�NTFSȨ�޵Ļ� NTFS���ļ�Ȩ�޺�ϸ������ο�MSDN��

BOOL AddFileAccessRights( 
	const TCHAR *lpszFileName, 
	const TCHAR *lpszAccountName, 
	DWORD dwAccessMask 
	);

// �����ļ������Լ���
// icacls file /setintegritylevel M

BOOL SetLowLabelToFile( 
	LPCTSTR pwszFileName 
	);

// ��ǰ����ϵͳ�Ƿ�֧��AppContainer
BOOL OSIsSupportAppContainer();

// ��ȡEveryOne�û���SID����ת�����ַ���
BOOL GetEveryOneUserSIDString(
	TCHAR *szStringSid,
	DWORD dwStringSidLen
	);

// ��ȡ��ǰ�û���SID����ת�����ַ���
BOOL GetCurrentUserSIDString(
    TCHAR *szStringSid,
	DWORD dwStringSidLen
	);

// ��ȡLogon�����SID����ת�����ַ���
BOOL GetLogonSIDString(
    TCHAR *szStringSid,
    DWORD dwStringSidLen
    );

// ��ȡĿ¼�����ļ��İ�ȫ�������е�DACL����ת�����ַ���

BOOL GetFileStringSecurityDescriptor(
	const TCHAR *lpszFileName,
	TCHAR *szStringSecurityDescriptor,
	DWORD dwStringSecurityDescriptorLen
	); 

//LowIntegrity
BOOL SetObjectToLowIntegrity(
	HANDLE hObject, 
	SE_OBJECT_TYPE secObjType = SE_KERNEL_OBJECT
	);

//AppContainer
BOOL SetObjectToLowBoxToken(
	HANDLE hObject,
	TCHAR *pszStringSecurityDescriptor
	);

BOOL ChangeHookMapDacl(
	TCHAR *mappingname, 
	TCHAR *szDACLStringSecurityDescriptor
	);
#endif //_ACCESS_RIGHTS_H_