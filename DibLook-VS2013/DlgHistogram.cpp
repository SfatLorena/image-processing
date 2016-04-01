// DlgHistogram.cpp : implementation file
//

#include "stdafx.h"
#include "diblook.h"
#include "DlgHistogram.h"
#include "afxdialogex.h"


// CDlgHistogram dialog

IMPLEMENT_DYNAMIC(CDlgHistogram, CDialogEx)

CDlgHistogram::CDlgHistogram(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDlgHistogram::IDD, pParent)
{

}

CDlgHistogram::~CDlgHistogram()
{
}

void CDlgHistogram::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_HISTOGRAM, m_Histogram);
}


BEGIN_MESSAGE_MAP(CDlgHistogram, CDialogEx)
END_MESSAGE_MAP()


// CDlgHistogram message handlers
