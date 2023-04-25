#ifndef _ACCESS_RIGHTS_H_
#define _ACCESS_RIGHTS_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <tchar.h>
#include <Windows.h>
#include <accctrl.h>

// lpszFileName    – 文件（目录）名
// lpszAccountName – 用户（组）名
// dwAccessMask    -- GENERIC_ALL表示所有的权限，其是一系列的NTFS权限的或 NTFS的文件权限很细，还请参看MSDN。

BOOL AddFileAccessRights( 
	const TCHAR *lpszFileName, 
	const TCHAR *lpszAccountName, 
	DWORD dwAccessMask 
	);

// 降低文件完整性级别
// icacls file /setintegritylevel M

BOOL SetLowLabelToFile( 
	LPCTSTR pwszFileName 
	);

// 当前操作系统是否支持AppContainer
BOOL OSIsSupportAppContainer();

// 获取EveryOne用户的SID，并转换成字符串
BOOL GetEveryOneUserSIDString(
	TCHAR *szStringSid,
	DWORD dwStringSidLen
	);

// 获取当前用户的SID，并转换成字符串
BOOL GetCurrentUserSIDString(
    TCHAR *szStringSid,
	DWORD dwStringSidLen
	);

// 获取Logon界面的SID，并转换成字符串
BOOL GetLogonSIDString(
    TCHAR *szStringSid,
    DWORD dwStringSidLen
    );

// 获取目录或者文件的安全描述符中的DACL，并转换成字符串

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