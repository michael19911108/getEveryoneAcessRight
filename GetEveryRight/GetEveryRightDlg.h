
// GetEveryRightDlg.h : ͷ�ļ�
//

#pragma once


// CGetEveryRightDlg �Ի���
class CGetEveryRightDlg : public CDialog
{
// ����
public:
	CGetEveryRightDlg(CWnd* pParent = NULL);	// ��׼���캯��
	CString SelectDirectory();
// �Ի�������
	enum { IDD = IDD_GETEVERYRIGHT_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedBtnSelect();

private:
	CString m_everyoneRightName;
public:
	afx_msg void OnDropFiles(HDROP hDropInfo);
};
