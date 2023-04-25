#include "stdafx.h"
#include "AccessRights.h"

#include <sddl.h>
#include <AccCtrl.h>
#include <Aclapi.h>
//#include"EstLog.h"
//////////////////////////////////////////////////////////////////////////

//ʹ��Windows��HeapAlloc�������ж�̬�ڴ����
#define memheapalloc(x) (HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, x))
#define memheapfree(x)  (HeapFree(GetProcessHeap(), 0, x))

typedef BOOL (WINAPI *SetSecurityDescriptorControlFnPtr)
	(
	IN PSECURITY_DESCRIPTOR pSecurityDescriptor,
	IN SECURITY_DESCRIPTOR_CONTROL ControlBitsOfInterest,
	IN SECURITY_DESCRIPTOR_CONTROL ControlBitsToSet);

typedef BOOL (WINAPI *AddAccessAllowedAceExFnPtr)
	(
	PACL pAcl,
	DWORD dwAceRevision,
	DWORD AceFlags,
	DWORD AccessMask,
	PSID pSid
	);


//////////////////////////////////////////////////////////////////////////

BOOL AddFileAccessRights(
	const TCHAR *lpszFileName, 
	const TCHAR *lpszAccountName, 
	DWORD dwAccessMask
	) 
{
	// ����SID����
	SID_NAME_USE   snuType;

	// ������LookupAccountName��صı�����ע�⣬ȫΪ0��Ҫ�ڳ����ж�̬���䣩
	TCHAR *        szDomain       = NULL;
	DWORD          cbDomain       = 0;
	LPVOID         pUserSID       = NULL;
	DWORD          cbUserSID      = 0;

	// ���ļ���صİ�ȫ������ SD �ı���
	PSECURITY_DESCRIPTOR pFileSD  = NULL;     // �ṹ����
	DWORD          cbFileSD       = 0;        // SD��size

	// һ���µ�SD�ı��������ڹ����µ�ACL�������е�ACL����Ҫ�¼ӵ�ACL����������
	SECURITY_DESCRIPTOR  newSD;

	// ��ACL ��صı���
	PACL           pACL           = NULL;
	BOOL           fDaclPresent;
	BOOL           fDaclDefaulted;
	ACL_SIZE_INFORMATION AclInfo;

	// һ���µ� ACL ����
	PACL           pNewACL        = NULL;  //�ṹָ�����
	DWORD          cbNewACL       = 0;     //ACL��size

	// һ����ʱʹ�õ� ACE ����
	LPVOID         pTempAce       = NULL;
	UINT           CurrentAceIndex = 0;  //ACE��ACL�е�λ��

	UINT           newAceIndex = 0;  //�����ACE��ACL�е�λ��

	//API�����ķ���ֵ���������еĺ���������ʧ�ܡ�
	BOOL           fResult = FALSE;
	BOOL           fAPISuccess;

	SECURITY_INFORMATION secInfo = DACL_SECURITY_INFORMATION;

	// ����������������µ�API����������Windows 2000���ϰ汾�Ĳ���ϵͳ֧�֡� 
	// �ڴ˽���Advapi32.dll�ļ��ж�̬���롣�����ʹ��VC++ 6.0������򣬶�������
	// ʹ�������������ľ�̬���ӡ�����Ϊ��ı�����ϣ�/D_WIN32_WINNT=0x0500
	// �ı������������ȷ�����SDK��ͷ�ļ���lib�ļ������µġ�
	SetSecurityDescriptorControlFnPtr _SetSecurityDescriptorControl = NULL;
	AddAccessAllowedAceExFnPtr _AddAccessAllowedAceEx = NULL; 

	__try 
	{

	 // 
	 // STEP 1: ͨ���û���ȡ��SID
	 //     ����һ����LookupAccountName���������������Σ���һ����ȡ������Ҫ
	 // ���ڴ�Ĵ�С��Ȼ�󣬽����ڴ���䡣�ڶ��ε��ò���ȡ�����û����ʻ���Ϣ��
	 // LookupAccountNameͬ������ȡ�����û������û������Ϣ������ο�MSDN��
	 //

	 fAPISuccess = LookupAccountName(NULL, lpszAccountName,
		 pUserSID, &cbUserSID, szDomain, &cbDomain, &snuType);

	 // ���ϵ���API��ʧ�ܣ�ʧ��ԭ�����ڴ治�㡣��������Ҫ���ڴ��С������
	 // �����Ǵ�����ڴ治��Ĵ���

	 if (fAPISuccess)
		 __leave;
	 else if (GetLastError() != ERROR_INSUFFICIENT_BUFFER) 
	 {
		 _tprintf(TEXT("LookupAccountName() failed. Error %d/n"), 
			 GetLastError());
		 __leave;
	 }

	 pUserSID = memheapalloc(cbUserSID);
	 if (!pUserSID) 
	 {
		 _tprintf(TEXT("HeapAlloc() failed. Error %d/n"), GetLastError());
		 __leave;
	 }

	 szDomain = (TCHAR *) memheapalloc(cbDomain * sizeof(TCHAR));
	 if (!szDomain) 
	 {
		 _tprintf(TEXT("HeapAlloc() failed. Error %d/n"), GetLastError());
		 __leave;
	 }

	 fAPISuccess = LookupAccountName(NULL, lpszAccountName,
		 pUserSID, &cbUserSID, szDomain, &cbDomain, &snuType);
	 if (!fAPISuccess) 
	 {
		 _tprintf(TEXT("LookupAccountName() failed. Error %d/n"), 
			 GetLastError());
		 __leave;
	 }

	 // 
	 // STEP 2: ȡ���ļ���Ŀ¼����صİ�ȫ������SD
	 //     ʹ��GetFileSecurity����ȡ��һ���ļ�SD�Ŀ�����ͬ�����������Ҳ
	 // �Ǳ��������Σ���һ��ͬ����ȡSD���ڴ泤�ȡ�ע�⣬SD�����ָ�ʽ������ص�
	 // ��self-relative���� ��ȫ�ģ�absolute����GetFileSecurityֻ��ȡ������
	 // ��صġ�����SetFileSecurity����Ҫ��ȫ�ġ������Ϊʲô��Ҫһ���µ�SD��
	 // ������ֱ����GetFileSecurity���ص�SD�Ͻ����޸ġ���Ϊ������صġ���Ϣ
	 // �ǲ������ġ�

	 fAPISuccess = GetFileSecurity(lpszFileName, 
		 secInfo, pFileSD, 0, &cbFileSD);

	 // ���ϵ���API��ʧ�ܣ�ʧ��ԭ�����ڴ治�㡣��������Ҫ���ڴ��С������
	 // �����Ǵ�����ڴ治��Ĵ���
	 if (fAPISuccess)
		 __leave;
	 else if (GetLastError() != ERROR_INSUFFICIENT_BUFFER) 
	 {
		 _tprintf(TEXT("GetFileSecurity() failed. Error %d/n"), 
			 GetLastError());
		 __leave;
	 }

	 pFileSD = memheapalloc(cbFileSD);
	 if (!pFileSD) 
	 {
		 _tprintf(TEXT("HeapAlloc() failed. Error %d/n"), GetLastError());
		 __leave;
	 }

	 fAPISuccess = GetFileSecurity(lpszFileName, 
		 secInfo, pFileSD, cbFileSD, &cbFileSD);
	 if (!fAPISuccess) 
	 {
		 _tprintf(TEXT("GetFileSecurity() failed. Error %d/n"), 
			 GetLastError());
		 __leave;
	 }

	 // 
	 // STEP 3: ��ʼ��һ���µ�SD
	 // 
	 if (!InitializeSecurityDescriptor(&newSD, 
		 SECURITY_DESCRIPTOR_REVISION)) 
	 {
			 _tprintf(TEXT("InitializeSecurityDescriptor() failed.")
				 TEXT("Error %d/n"), GetLastError());
			 __leave;
	 }

	 // 
	 // STEP 4: ��GetFileSecurity ���ص�SD��ȡDACL
	 // 
	 if (!GetSecurityDescriptorDacl(pFileSD, &fDaclPresent, &pACL,
		 &fDaclDefaulted)) 
	 {
			 _tprintf(TEXT("GetSecurityDescriptorDacl() failed. Error %d/n"),
				 GetLastError());
			 __leave;
	 }

	 // 
	 // STEP 5: ȡ DACL���ڴ�size
	 //     GetAclInformation�����ṩDACL���ڴ��С��ֻ����һ������Ϊ
	 // ACL_SIZE_INFORMATION��structure�Ĳ�������DACL����Ϣ����Ϊ��
	 // �������Ǳ������е�ACE��
	 AclInfo.AceCount = 0; // Assume NULL DACL.
	 AclInfo.AclBytesFree = 0;
	 AclInfo.AclBytesInUse = sizeof(ACL);

	 if (pACL == NULL)
		 fDaclPresent = FALSE;

	 // ���DACL��Ϊ�գ���ȡ����Ϣ�������������¡��Թ�������DACLΪ�գ�
	 if (fDaclPresent) 
	 {            
		 if (!GetAclInformation(pACL, &AclInfo, 
			 sizeof(ACL_SIZE_INFORMATION), AclSizeInformation)) 
		 {
				 _tprintf(TEXT("GetAclInformation() failed. Error %d/n"),
					 GetLastError());
				 __leave;
		 }
	 }

	 // 
	 // STEP 6: �����µ�ACL��size
	 //    ����Ĺ�ʽ�ǣ�ԭ�е�DACL��size������Ҫ��ӵ�һ��ACE��size����
	 // ������һ����ACE��ص�SID��size������ȥ�����ֽ��Ի�þ�ȷ�Ĵ�С��
	 cbNewACL = AclInfo.AclBytesInUse + sizeof(ACCESS_ALLOWED_ACE) 
		 + GetLengthSid(pUserSID) - sizeof(DWORD);


	 // 
	 // STEP 7: Ϊ�µ�ACL�����ڴ�
	 // 
	 pNewACL = (PACL) memheapalloc(cbNewACL);
	 if (!pNewACL)
	 {
		 _tprintf(TEXT("HeapAlloc() failed. Error %d/n"), GetLastError());
		 __leave;
	 }

	 // 
	 // STEP 8: ��ʼ���µ�ACL�ṹ
	 // 
	 if (!InitializeAcl(pNewACL, cbNewACL, ACL_REVISION2))
	 {
		 _tprintf(TEXT("InitializeAcl() failed. Error %d/n"), 
			 GetLastError());
		 __leave;
	 }

	 // 
	 // STEP 9  ����ļ���Ŀ¼�� DACL �����ݣ��������е�ACE���µ�DACL��
	 // 
	 //     ����Ĵ���������ȼ��ָ���ļ���Ŀ¼���Ƿ���ڵ�DACL������еĻ���
	 // ��ô�Ϳ������е�ACE���µ�DACL�ṹ�У����ǿ��Կ���������ķ����ǲ���
	 // ACL_SIZE_INFORMATION�ṹ�е�AceCount��Ա����ɵġ������ѭ���У�
	 // �ᰴ��Ĭ�ϵ�ACE��˳�������п�����ACE��ACL�е�˳���Ǻܹؼ��ģ����ڿ�
	 // �������У��ȿ����Ǽ̳е�ACE������֪��ACE����ϲ�Ŀ¼�м̳�������
	 // 

	 newAceIndex = 0;

	 if (fDaclPresent && AclInfo.AceCount) 
	 {

		 for (CurrentAceIndex = 0; 
			 CurrentAceIndex < AclInfo.AceCount;
			 CurrentAceIndex++)
		 {

				 // 
				 // STEP 10: ��DACL��ȡACE
				 // 
				 if (!GetAce(pACL, CurrentAceIndex, &pTempAce)) 
				 {
					 _tprintf(TEXT("GetAce() failed. Error %d/n"), 
						 GetLastError());
					 __leave;
				 }

				 // 
				 // STEP 11: ����Ƿ��ǷǼ̳е�ACE
				 //     �����ǰ��ACE��һ���Ӹ�Ŀ¼�̳�����ACE����ô���˳�ѭ����
				 // ��Ϊ���̳е�ACE�����ڷǼ̳е�ACE֮�󣬶�������Ҫ��ӵ�ACE
				 // Ӧ�������еķǼ̳е�ACE֮�����еļ̳е�ACE֮ǰ���˳�ѭ��
				 // ����Ϊ��Ҫ���һ���µ�ACE���µ�DACL�У���������ٰѼ̳е�
				 // ACE�������µ�DACL�С�
				 //
				 if (((ACCESS_ALLOWED_ACE *)pTempAce)->Header.AceFlags
					 & INHERITED_ACE)
					 break;

				 // 
				 // STEP 12: ���Ҫ������ACE��SID�Ƿ����Ҫ�����ACE��SIDһ����
				 // ���һ������ô��Ӧ�÷ϵ��Ѵ��ڵ�ACE��Ҳ����˵��ͬһ���û��Ĵ�ȡ
				 // Ȩ�޵����õ�ACE����DACL��Ӧ��Ψһ�������������ͬһ�û�������
				 // �˵�ACE�����ǿ��������û���ACE��
				 // 
				 if (EqualSid(pUserSID,
					 &(((ACCESS_ALLOWED_ACE *)pTempAce)->SidStart)))
					 continue;

				 // 
				 // STEP 13: ��ACE���뵽�µ�DACL��
				 //    ����Ĵ����У�ע�� AddAce �����ĵ����������������������˼�� 
				 // ACL�е�����ֵ����ΪҪ��ACE�ӵ�ĳ����λ��֮�󣬲���MAXDWORD��
				 // ��˼��ȷ����ǰ��ACE�Ǳ����뵽����λ�á�
				 //
				 if (!AddAce(pNewACL, ACL_REVISION, MAXDWORD, pTempAce,
					 ((PACE_HEADER) pTempAce)->AceSize)) 
				 {
						 _tprintf(TEXT("AddAce() failed. Error %d/n"), 
							 GetLastError());
						 __leave;
				 }

				 newAceIndex++;
		 }
	 }

	 // STEP 14: ��һ�� access-allowed ��ACE ���뵽�µ�DACL��
	 //     ǰ���ѭ�����������еķǼ̳���SIDΪ�����û���ACE���˳�ѭ���ĵ�һ����
	 // ���Ǽ�������ָ����ACE����ע�������ȶ�̬װ����һ��AddAccessAllowedAceEx
	 // ��API���������װ�ز��ɹ����͵���AddAccessAllowedAce������ǰһ��������
	 // ��Windows 2000�Ժ�İ汾֧�֣�NT��û�У�����Ϊ��ʹ���°汾�ĺ�����������
	 // ���ȼ��һ�µ�ǰϵͳ�пɲ�����װ���������������������ʹ�á�ʹ�ö�̬����
	 // ��ʹ�þ�̬���ӵĺô��ǣ���������ʱ������Ϊû�����API����������
	 // 
	 // Ex��ĺ��������һ������AceFlag�������˲�������������������ǿ���������һ
	 // ����ACE_HEADER�Ľṹ���Ա������������õ�ACE���Ա�����Ŀ¼���̳���ȥ���� 
	 // AddAccessAllowedAce�������ܶ��������������AddAccessAllowedAce����
	 // �У�����ACE_HEADER����ṹ���óɷǼ̳еġ�
	 // 
	 _AddAccessAllowedAceEx = (AddAccessAllowedAceExFnPtr)
		 GetProcAddress(GetModuleHandle(TEXT("advapi32.dll")),
		 "AddAccessAllowedAceEx");

	 if (_AddAccessAllowedAceEx) 
	 {
		 if (!_AddAccessAllowedAceEx(pNewACL, ACL_REVISION2,
			 CONTAINER_INHERIT_ACE | OBJECT_INHERIT_ACE ,
			 dwAccessMask, pUserSID)) 
		 {
				 _tprintf(TEXT("AddAccessAllowedAceEx() failed. Error %d/n"),
					 GetLastError());
				 __leave;
		 }
	 }
	 else
	 {
		 if (!AddAccessAllowedAce(pNewACL, ACL_REVISION2, 
			 dwAccessMask, pUserSID)) 
		 {
				 _tprintf(TEXT("AddAccessAllowedAce() failed. Error %d/n"),
					 GetLastError());
				 __leave;
		 }
	 }

	 // 
	 // STEP 15: �����Ѵ��ڵ�ACE��˳�򿽱��Ӹ�Ŀ¼�̳ж�����ACE
	 // 
	 if (fDaclPresent && AclInfo.AceCount)
	 {

		 for (; 
			 CurrentAceIndex < AclInfo.AceCount;
			 CurrentAceIndex++) 
		 {

				 // 
				 // STEP 16: ���ļ���Ŀ¼����DACL�м���ȡACE
				 // 
				 if (!GetAce(pACL, CurrentAceIndex, &pTempAce)) 
				 {
					 _tprintf(TEXT("GetAce() failed. Error %d/n"), 
						 GetLastError());
					 __leave;
				 }

				 // 
				 // STEP 17: ��ACE���뵽�µ�DACL��
				 // 
				 if (!AddAce(pNewACL, ACL_REVISION, MAXDWORD, pTempAce,
					 ((PACE_HEADER) pTempAce)->AceSize)) 
				 {
					 _tprintf(TEXT("AddAce() failed. Error %d/n"), 
						 GetLastError());
					 __leave;
				 }
		 }
	 }

	 // 
	 // STEP 18: ���µ�ACL���õ��µ�SD��
	 // 
	 if (!SetSecurityDescriptorDacl(&newSD, TRUE, pNewACL, 
		 FALSE)) 
	 {
			 _tprintf(TEXT("SetSecurityDescriptorDacl() failed. Error %d/n"),
				 GetLastError());
			 __leave;
	 }

	 // 
	 // STEP 19: ���ϵ�SD�еĿ��Ʊ���ٿ������µ�SD�У�����ʹ�õ���һ���� 
	 // SetSecurityDescriptorControl() ��API�������������ͬ��ֻ������
	 // Windows 2000�Ժ�İ汾�У��������ǻ���Ҫ��̬�ذ����advapi32.dll 
	 // �����룬���ϵͳ��֧������������ǾͲ������ϵ�SD�Ŀ��Ʊ���ˡ�
	 // 
	 _SetSecurityDescriptorControl =(SetSecurityDescriptorControlFnPtr)
		 GetProcAddress(GetModuleHandle(TEXT("advapi32.dll")),
		 "SetSecurityDescriptorControl");
	 if (_SetSecurityDescriptorControl) 
	 {

		 SECURITY_DESCRIPTOR_CONTROL controlBitsOfInterest = 0;
		 SECURITY_DESCRIPTOR_CONTROL controlBitsToSet = 0;
		 SECURITY_DESCRIPTOR_CONTROL oldControlBits = 0;
		 DWORD dwRevision = 0;

		 if (!GetSecurityDescriptorControl(pFileSD, &oldControlBits,
			 &dwRevision)) 
		 {
				 _tprintf(TEXT("GetSecurityDescriptorControl() failed.")
					 TEXT("Error %d/n"), GetLastError());
				 __leave;
		 }

		 if (oldControlBits & SE_DACL_AUTO_INHERITED)
		 {
			 controlBitsOfInterest =
				 SE_DACL_AUTO_INHERIT_REQ |
				 SE_DACL_AUTO_INHERITED ;
			 controlBitsToSet = controlBitsOfInterest;
		 }
		 else if (oldControlBits & SE_DACL_PROTECTED) 
		 {
			 controlBitsOfInterest = SE_DACL_PROTECTED;
			 controlBitsToSet = controlBitsOfInterest;
		 }        

		 if (controlBitsOfInterest) 
		 {
			 if (!_SetSecurityDescriptorControl(&newSD,
				 controlBitsOfInterest,
				 controlBitsToSet)) 
			 {
					 _tprintf(TEXT("SetSecurityDescriptorControl() failed.")
						 TEXT("Error %d/n"), GetLastError());
					 __leave;
			 }
		 }
	 }

	 // 
	 // STEP 20: ���µ�SD�������õ��ļ��İ�ȫ�����У�ǧɽ��ˮ�������ڵ��ˣ�
	 // 
	 if (!SetFileSecurity(lpszFileName, secInfo,
		 &newSD))
	 {
			 _tprintf(TEXT("SetFileSecurity() failed. Error %d/n"), 
				 GetLastError());
			 __leave;
	 }

	 fResult = TRUE;

	}
	__finally 
	{

		 // 
		 // STEP 21: �ͷ��ѷ�����ڴ棬����Memory Leak
		 // 
		 if (pUserSID)  
			 memheapfree(pUserSID);
		 if (szDomain)  
			 memheapfree(szDomain);
		 if (pFileSD) 
			 memheapfree(pFileSD);
		 if (pNewACL) 
			 memheapfree(pNewACL);
	}

	return fResult;
}

// �����ļ������Լ���
// icacls file /setintegritylevel M

BOOL SetLowLabelToFile( LPCTSTR pwszFileName )
{
	// The LABEL_SECURITY_INFORMATION SDDL SACL to be set for low integrity 
#define LOW_INTEGRITY_SDDL_SACL_W L"S:(ML;;NW;;;LW)"
	DWORD dwErr = -1;
	PSECURITY_DESCRIPTOR pSD = NULL;  

	PACL pSacl = NULL; // not allocated
	BOOL fSaclPresent = FALSE;
	BOOL fSaclDefaulted = FALSE;
	//LPCWSTR pwszFileName = L"c:\\ole.txt";

	if (ConvertStringSecurityDescriptorToSecurityDescriptorW(
		LOW_INTEGRITY_SDDL_SACL_W, SDDL_REVISION_1, &pSD, NULL)) 
	{
		if ( GetSecurityDescriptorSacl(pSD, &fSaclPresent, &pSacl, &fSaclDefaulted) )
		{
			// Note that psidOwner, psidGroup, and pDacl are 
			// all NULL and set the new LABEL_SECURITY_INFORMATION
			dwErr = SetNamedSecurityInfo((LPTSTR) pwszFileName, 
				SE_FILE_OBJECT, LABEL_SECURITY_INFORMATION, 
				NULL, NULL, NULL, pSacl);
		}
		LocalFree(pSD);
	}

	return ( ERROR_SUCCESS == dwErr );
}

//�жϲ���ϵͳ��Win8����ϵͳ��֧��AppContainer��IEΪ11��Ҳ���Բ�ѯע���HKEY_CURRENT_USER\Software\Classes\Local Settings\Software\Microsoft\Windows\CurrentVersion\AppContainer��
// ��ʱֻ�ж�Win8,��ΪWin8��ʼ��Windows�������µĽ��̸������AppContainer��
//////////////////////////////////////////////////////////////////////////

BOOL OSIsSupportAppContainer()
{
	BOOL bResult = FALSE;

	OSVERSIONINFOEX os; 
	os.dwOSVersionInfoSize=sizeof(OSVERSIONINFOEX); 
	if(GetVersionEx((OSVERSIONINFO *)&os))
	{ 
		if(os.dwMajorVersion ==6 || os.dwMajorVersion > 6)
		{
			if(os.dwMinorVersion ==2 || os.dwMinorVersion > 2)
			{
				bResult = TRUE;
			}
		}
	} 

	return bResult;
}

// ��ȡEveryOne�û���SID����ת�����ַ���
//////////////////////////////////////////////////////////////////////////
BOOL GetEveryOneUserSIDString(
	TCHAR *szStringSid,
	DWORD dwStringSidLen
	) 
{
	// ����SID����
	SID_NAME_USE   snuType;

	// ������LookupAccountName��صı�����ע�⣬ȫΪ0��Ҫ�ڳ����ж�̬���䣩
	TCHAR *        szDomain       = NULL;
	DWORD          cbDomain       = 0;
	LPTSTR pszSID = NULL;
	PSID		   pUserSID       = NULL;
	DWORD          cbUserSID      = 0;

	//API�����ķ���ֵ���������еĺ���������ʧ�ܡ�
	BOOL           fResult = FALSE;
	BOOL           fAPISuccess;

	SECURITY_INFORMATION secInfo = DACL_SECURITY_INFORMATION;

	__try 
	{
		// 
		// STEP 1: ͨ���û���ȡ��SID
		//     ����һ����LookupAccountName���������������Σ���һ����ȡ������Ҫ
		// ���ڴ�Ĵ�С��Ȼ�󣬽����ڴ���䡣�ڶ��ε��ò���ȡ�����û����ʻ���Ϣ��
		// LookupAccountNameͬ������ȡ�����û������û������Ϣ������ο�MSDN��
		//

		fAPISuccess = LookupAccountName(NULL, L"EveryOne", pUserSID, &cbUserSID, szDomain, &cbDomain, &snuType);

		// ���ϵ���API��ʧ�ܣ�ʧ��ԭ�����ڴ治�㡣��������Ҫ���ڴ��С������
		// �����Ǵ�����ڴ治��Ĵ���

		if (fAPISuccess)
			__leave;
		else if (GetLastError() != ERROR_INSUFFICIENT_BUFFER) 
		{
			_tprintf(TEXT("LookupAccountName() failed. Error %d/n"), GetLastError());
			__leave;
		}

		pUserSID = memheapalloc(cbUserSID);
		if (!pUserSID) 
		{
			_tprintf(TEXT("HeapAlloc() failed. Error %d/n"), GetLastError());
			__leave;
		}

		szDomain = (TCHAR *) memheapalloc(cbDomain * sizeof(TCHAR));
		if (!szDomain) 
		{
			_tprintf(TEXT("HeapAlloc() failed. Error %d/n"), GetLastError());
			__leave;
		}

		fAPISuccess = LookupAccountName(NULL, L"EveryOne", pUserSID, &cbUserSID, szDomain, &cbDomain, &snuType);
		if (!fAPISuccess) 
		{
			_tprintf(TEXT("LookupAccountName() failed. Error %d/n"), GetLastError());
			__leave;
		}

		fAPISuccess = IsValidSid(pUserSID);
		if(!fAPISuccess)
		{
			_tprintf(TEXT("IsValidSid() failed. Error %d/n"), GetLastError());
			__leave;
		}


		fAPISuccess = ConvertSidToStringSid(pUserSID, &pszSID);
		if(!fAPISuccess)
		{
			_tprintf(TEXT("ConvertSidToStringSid() failed. Error %d/n"), GetLastError());
			__leave;
		}

		wcscpy_s(szStringSid, dwStringSidLen, pszSID);

		fResult = TRUE;
	}
	__finally 
	{

		// 
		// STEP 21: �ͷ��ѷ�����ڴ棬����Memory Leak
		// 
		if (pUserSID)  
			memheapfree(pUserSID);

		if (szDomain)  
			memheapfree(szDomain);

		if (pszSID)
		{
			LocalFree( pszSID );
			pszSID = NULL;
		}   
	}

	return fResult;
}

// ��ȡ��ǰ�û���SID����ת�����ַ���
//////////////////////////////////////////////////////////////////////////

BOOL GetCurrentUserSIDString(
	TCHAR *szStringSid,
	DWORD dwStringSidLen
	)
{
	BOOL bResult = FALSE;
	HANDLE hToken = NULL; 
	LPTSTR pszSID = NULL;
	DWORD dwBufferSize = 0;
	LPBYTE pbyBuffer = NULL;
	PTOKEN_USER pTokenUser = NULL;

	// Check if we can get information for this process  
// 	HANDLE hCurProcess = OpenProcess(READ_CONTROL | PROCESS_QUERY_INFORMATION, FALSE, GetCurrentProcessId());
// 	if (hCurProcess == NULL)
// 		goto  Cleanup;

	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken))
		goto  Cleanup;

	// Get required buffer size and allocate the TOKEN_USERS buffer. 
	if(!GetTokenInformation(
		hToken,				//  handle to the access token 
		TokenUser,			//  get information about the token's groups  
		NULL,				//  pointer to TOKEN_GROUPS buffer 
		0 ,					//  size of buffer 
		&dwBufferSize			//  receives required buffer size 
		)) 
	{
		if(GetLastError() != ERROR_INSUFFICIENT_BUFFER)
			goto  Cleanup;
	} 

	pbyBuffer = new byte[dwBufferSize];
	if(NULL == pbyBuffer)
		goto  Cleanup;

	::SecureZeroMemory(pbyBuffer, sizeof(pbyBuffer));
	pTokenUser = reinterpret_cast<PTOKEN_USER>(pbyBuffer);
				
	if(!GetTokenInformation(hToken, TokenUser, pTokenUser, dwBufferSize, &dwBufferSize))
		goto  Cleanup;

	if(!IsValidSid(pTokenUser->User.Sid)) 
		goto  Cleanup;

	if(!ConvertSidToStringSid(pTokenUser->User.Sid, &pszSID))
		goto  Cleanup;

	wcscpy_s(szStringSid, dwStringSidLen, pszSID);
	
	bResult = TRUE;				

Cleanup:

	if (pszSID)
	{
		LocalFree( pszSID );
		pszSID = NULL;
	}                        

	if(pbyBuffer)
	{
		delete []pbyBuffer;
		pbyBuffer = NULL;
	}

	// Don't forget to close the token handle.	  
	if (hToken)
	{
		CloseHandle( hToken );
		hToken = NULL;
	}

	// Don't forget to close the Process handle.
// 	if (hCurProcess)
// 	{
// 		CloseHandle( hCurProcess );
// 		hCurProcess = NULL;
// 	}

	return bResult;
}

// ��ȡLogon�����SID����ת�����ַ���
//////////////////////////////////////////////////////////////////////////

BOOL GetLogonSIDString(
	TCHAR *szStringSid,
	DWORD dwStringSidLen
	)
{
	BOOL bSuccess = FALSE;

	DWORD dwIndex;
	DWORD dwLength = 0 ;
	HANDLE hToken = NULL;
	LPTSTR pStringSid = NULL;
	PTOKEN_GROUPS ptg  =  NULL;

	// Check if we can get information for this process  
// 	HANDLE hProcess = OpenProcess(READ_CONTROL | PROCESS_QUERY_INFORMATION, FALSE, GetCurrentProcessId());
// 	if (hProcess == NULL)
// 		goto  Cleanup;

	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_READ, &hToken))
		goto  Cleanup;

	// Get required buffer size and allocate the TOKEN_GROUPS buffer. 
	if(!GetTokenInformation(
		hToken,          //  handle to the access token 
		TokenGroups,     //  get information about the token's groups  
		(LPVOID)ptg,     //  pointer to TOKEN_GROUPS buffer 
		0 ,              //  size of buffer 
		&dwLength        //  receives required buffer size 
		)) 
	{
		if  (GetLastError()  !=  ERROR_INSUFFICIENT_BUFFER) 
			goto  Cleanup;

		ptg  =  (PTOKEN_GROUPS)HeapAlloc(GetProcessHeap(),
			HEAP_ZERO_MEMORY, dwLength);

		if  (ptg  ==  NULL)
			goto  Cleanup;
	} 

	// Get the token group information from the access token. 
	if(!GetTokenInformation(
		hToken,          //  handle to the access token 
		TokenGroups,     //  get information about the token's groups  
		(LPVOID)ptg,     //  pointer to TOKEN_GROUPS buffer 
		dwLength,        //  size of buffer 
		&dwLength        //  receives required buffer size 
		)) 
	{
		goto  Cleanup;
	} 

	// Loop through the groups to find the logon SID. 
	for(dwIndex = 0; dwIndex < ptg->GroupCount; dwIndex++) 
		if((ptg -> Groups[dwIndex].Attributes  &  SE_GROUP_LOGON_ID) == SE_GROUP_LOGON_ID) 
		{
			if(!ConvertSidToStringSid(ptg -> Groups[dwIndex].Sid,&pStringSid)) 
			{
				goto  Cleanup;
			}

			wcscpy_s(szStringSid, dwStringSidLen, pStringSid);

			break;
		} 

	bSuccess = TRUE;

Cleanup:

	// Free the buffer for the token groups.
	if(ptg != NULL)
		HeapFree(GetProcessHeap(), 0, (LPVOID)ptg);

	// Free the buffer for the StringSid.
	if (pStringSid != NULL)
	{
		LocalFree(pStringSid); 
		pStringSid = NULL;
	} 

	// Don't forget to close the token handle.	  
	if (hToken)
	{
		CloseHandle( hToken );
		hToken = NULL;
	}

	// Don't forget to close the Process handle.
// 	if (hProcess)
// 	{
// 		CloseHandle( hProcess );
// 		hProcess = NULL;
// 	}

	return bSuccess;
}

// ��ȡĿ¼�����ļ��İ�ȫ�������е�DACL����ת�����ַ���
//////////////////////////////////////////////////////////////////////////

BOOL GetFileStringSecurityDescriptor(
	const TCHAR *lpszFileName,
	TCHAR *szStringSecurityDescriptor,
	DWORD dwStringSecurityDescriptorLen
	)
{
	// ���ļ���صİ�ȫ������ SD �ı���
	PSECURITY_DESCRIPTOR pFileSD = NULL;      // �ṹ����
	DWORD                cbFileSD = 0;        // SD��size

	//API�����ķ���ֵ���������еĺ���������ʧ�ܡ�
	BOOL                 fResult = FALSE;
	BOOL                 fAPISuccess = FALSE;

	LPTSTR               pStringSecurityDescriptor = NULL;
	SECURITY_INFORMATION secInfo = DACL_SECURITY_INFORMATION;

	__try 
	{
		// 
		// STEP 1: ȡ���ļ���Ŀ¼����صİ�ȫ������SD
		//     ʹ��GetFileSecurity����ȡ��һ���ļ�SD�Ŀ�����ͬ�����������Ҳ
		// �Ǳ��������Σ���һ��ͬ����ȡSD���ڴ泤�ȡ�ע�⣬SD�����ָ�ʽ������ص�
		// ��self-relative���� ��ȫ�ģ�absolute����GetFileSecurityֻ��ȡ������
		// ��صġ�����SetFileSecurity����Ҫ��ȫ�ġ������Ϊʲô��Ҫһ���µ�SD��
		// ������ֱ����GetFileSecurity���ص�SD�Ͻ����޸ġ���Ϊ������صġ���Ϣ
		// �ǲ������ġ�

		fAPISuccess = GetFileSecurity(lpszFileName, secInfo, pFileSD, 0, &cbFileSD);

		// ���ϵ���API��ʧ�ܣ�ʧ��ԭ�����ڴ治�㡣��������Ҫ���ڴ��С�����������Ǵ�����ڴ治��Ĵ���
		if (fAPISuccess)
			__leave;
		else if (GetLastError() != ERROR_INSUFFICIENT_BUFFER) 
		{
			_tprintf(TEXT("GetFileSecurity() failed. Error %d/n"), GetLastError());
			__leave;
		}

		pFileSD = memheapalloc(cbFileSD);
		if (!pFileSD) 
		{
			_tprintf(TEXT("HeapAlloc() failed. Error %d/n"), GetLastError());
			__leave;
		}

		fAPISuccess = GetFileSecurity(lpszFileName, secInfo, pFileSD, cbFileSD, &cbFileSD);
		if (!fAPISuccess) 
		{
			_tprintf(TEXT("GetFileSecurity() failed. Error %d/n"), GetLastError());
			__leave;
		}

		ULONG ulStringSecurityDescriptorLen = 0;
		fAPISuccess = ConvertSecurityDescriptorToStringSecurityDescriptor(pFileSD,SDDL_REVISION_1,
																	    DACL_SECURITY_INFORMATION,
																		&pStringSecurityDescriptor,
																		&ulStringSecurityDescriptorLen);
		if(!fAPISuccess)
		{
			_tprintf(TEXT("ConvertSecurityDescriptorToStringSecurityDescriptor() failed. Error %d/n"), GetLastError());
			__leave;
		}

		wcscpy_s(szStringSecurityDescriptor, dwStringSecurityDescriptorLen, pStringSecurityDescriptor);

		fResult = TRUE;
	}
	__finally 
	{

		// 
		// STEP 2: �ͷ��ѷ�����ڴ棬����Memory Leak
		// 
		if (pFileSD) 
			memheapfree(pFileSD);

		if (pStringSecurityDescriptor != NULL)
			LocalFree(pStringSecurityDescriptor); 
	}

	return fResult;
}

//=====================================================================================================
//LowIntegrity
BOOL SetObjectToLowIntegrity(
	HANDLE hObject,			
	SE_OBJECT_TYPE secObjType
	)
{
	bool bRet = false;
	DWORD dwErr = ERROR_SUCCESS;

	PACL pSacl = NULL;
	BOOL fSaclPresent = FALSE;
	BOOL fSaclDefaulted = FALSE;
	PSECURITY_DESCRIPTOR pSD = NULL;
//	LPCWSTR LOW_INTEGRITY_SDDL_SACL_W = L"S:(ML;;NW;;;LW)";
	TCHAR szLowIntegritySDDL_SACL_W[40] = L"S:(ML;;NW;;;LW)";
	SECURITY_INFORMATION secInfo = LABEL_SECURITY_INFORMATION;

	if(ConvertStringSecurityDescriptorToSecurityDescriptor(szLowIntegritySDDL_SACL_W,SDDL_REVISION_1,&pSD,NULL))
	{
		if(GetSecurityDescriptorSacl(pSD,&fSaclPresent,&pSacl,&fSaclDefaulted))
		{
			dwErr = SetSecurityInfo(hObject,secObjType,secInfo,NULL,NULL,NULL,pSacl);

			bRet = (ERROR_SUCCESS == dwErr);
		}
	}

	if (pSD != NULL)
	{
		LocalFree(pSD); 
		pSD = NULL;
	}

	return bRet;
}

//AppContainer
BOOL SetObjectToLowBoxToken(
	HANDLE hObject,
	TCHAR *pszStringSecurityDescriptor
	)
{
	CString strAppContainerLog;

	bool bRet = false;
	DWORD dwErr = ERROR_SUCCESS;

	PACL pDacl = NULL;
	BOOL fDaclPresent = FALSE;
	BOOL fDaclDefaulted = FALSE;
	PSECURITY_DESCRIPTOR pSD = NULL;
	SE_OBJECT_TYPE secObjType = SE_KERNEL_OBJECT;
	SECURITY_INFORMATION secInfo = DACL_SECURITY_INFORMATION;

	if(ConvertStringSecurityDescriptorToSecurityDescriptor(pszStringSecurityDescriptor,SDDL_REVISION_1,&pSD,NULL))
	{
		if(GetSecurityDescriptorDacl(pSD,&fDaclPresent,&pDacl,&fDaclDefaulted))
		{
			dwErr = SetSecurityInfo(hObject,secObjType,secInfo,NULL,NULL,pDacl,NULL);

			bRet = (ERROR_SUCCESS == dwErr);
		}
	}

	if (pSD != NULL)
	{
		LocalFree(pSD); 
		pSD = NULL;
	}

	return bRet;
}

BOOL ChangeHookMapDacl(
	TCHAR *mappingname,
	TCHAR *szDACLStringSecurityDescriptor
	)
{
	BOOL bRet = FALSE;
	HANDLE hMapFile = NULL;
	CString strAppContainerLog;

	OutputDebugStringW(L"\n==============================================================================\n");

	hMapFile = OpenFileMapping(FILE_MAP_ALL_ACCESS,FALSE,mappingname);
	if(hMapFile == NULL)
	{
		strAppContainerLog.Format(_T("\n*****longj::ChangeHookMapDacl m_hMapFile == NULL return FALSE! Last Error %ld*****"), GetLastError());
		OutputDebugStringW(strAppContainerLog);
		return bRet;
	}

	PACL pDacl = NULL;
	DWORD dwErr = ERROR_SUCCESS;
	PSECURITY_DESCRIPTOR pSD = NULL;

	dwErr = GetSecurityInfo(hMapFile,SE_KERNEL_OBJECT,
		DACL_SECURITY_INFORMATION,
		NULL,
		NULL,
		&pDacl,
		NULL,
		&pSD);

	strAppContainerLog.Format(_T("\n*****longj::ChangeHookMapDacl GetSecurityInfo pDacl->AceCount %ld return %ld  Enter*****"), pDacl->AceCount, dwErr);
	OutputDebugStringW(strAppContainerLog);

	bRet = (ERROR_SUCCESS == dwErr);

	if(bRet)
	{
		if(pDacl->AceCount)
		{
			bRet = SetObjectToLowBoxToken(hMapFile, L"");
			strAppContainerLog.Format(_T("*****longj::ChangeHookMapDacl SetObjectToLowBoxToken 1 return %s*****"), bRet ? L"TRUE" : L"FALSE");
			OutputDebugStringW(strAppContainerLog);
		}
		else
		{
			bRet = SetObjectToLowBoxToken(hMapFile, szDACLStringSecurityDescriptor);
			strAppContainerLog.Format(_T("*****longj::ChangeHookMapDacl SetObjectToLowBoxToken 2 return %s*****"), bRet ? L"TRUE" : L"FALSE");
			OutputDebugStringW(strAppContainerLog);
		}
	}

	strAppContainerLog.Format(_T("\n*****longj::ChangeHookMapDacl SetObjectToLowBoxToken d return %s*****"), bRet ? L"TRUE" : L"FALSE");
	OutputDebugStringW(strAppContainerLog);

	if(hMapFile != NULL)
	{
		CloseHandle(hMapFile);
		hMapFile = NULL;
	}

	OutputDebugStringW(L"\n==============================================================================\n");

	return bRet;
}