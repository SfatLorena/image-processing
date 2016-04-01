// BitmapInfoDlg.cpp : implementation file
//

#include "stdafx.h"
#include "diblook.h"
#include "BitmapInfoDlg.h"
#include "afxdialogex.h"


// CBitmapInfoDlg dialog

IMPLEMENT_DYNAMIC(CBitmapInfoDlg, CDialog)

CBitmapInfoDlg::CBitmapInfoDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CBitmapInfoDlg::IDD, pParent)
	, m_Width(_T(""))
	, m_Height(_T(""))
	, m_BitsPixel(_T(""))
{

}

CBitmapInfoDlg::~CBitmapInfoDlg()
{
}

void CBitmapInfoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_name1, m_Width);
	DDX_Text(pDX, IDC_name2, m_Height);
	DDX_Text(pDX, IDC_name3, m_BitsPixel);
}


BEGIN_MESSAGE_MAP(CBitmapInfoDlg, CDialog)
END_MESSAGE_MAP()


// CBitmapInfoDlg message handlers
