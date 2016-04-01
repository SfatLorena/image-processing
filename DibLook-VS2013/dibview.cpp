// dibview.cpp : implementation of the CDibView class
//
// This is a part of the Microsoft Foundation Classes C++ library.
// Copyright (C) 1992-1998 Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and related
// electronic documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.

#include "stdafx.h"
#include "diblook.h"

#include <iostream>
#include <queue>

#include "dibdoc.h"
#include "dibview.h"
#include "dibapi.h"
#include "mainfrm.h"

#include "HRTimer.h"
#include <math.h>

#include "DlgHistogram.h"

#include "BitmapInfoDlg.h"


using namespace std;

int maxIndex = 1;
float maxVal[256] = { 0 };

int labels[256][256] = { 0 };


#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

#define BEGIN_PROCESSING() INCEPUT_PRELUCRARI()

#define END_PROCESSING(Title) SFARSIT_PRELUCRARI(Title)

#define INCEPUT_PRELUCRARI() \
	CDibDoc* pDocSrc=GetDocument();										\
	CDocTemplate* pDocTemplate=pDocSrc->GetDocTemplate();				\
	CDibDoc* pDocDest=(CDibDoc*) pDocTemplate->CreateNewDocument();		\
	BeginWaitCursor();													\
	HDIB hBmpSrc=pDocSrc->GetHDIB();									\
	HDIB hBmpDest = (HDIB)::CopyHandle((HGLOBAL)hBmpSrc);				\
	if ( hBmpDest==0 ) {												\
		pDocTemplate->RemoveDocument(pDocDest);							\
		return;															\
	}																	\
	BYTE* lpD = (BYTE*)::GlobalLock((HGLOBAL)hBmpDest);					\
	BYTE* lpS = (BYTE*)::GlobalLock((HGLOBAL)hBmpSrc);					\
	int iColors = DIBNumColors((char *)&(((LPBITMAPINFO)lpD)->bmiHeader)); \
	RGBQUAD *bmiColorsDst = ((LPBITMAPINFO)lpD)->bmiColors;	\
	RGBQUAD *bmiColorsSrc = ((LPBITMAPINFO)lpS)->bmiColors;	\
	BYTE * lpDst = (BYTE*)::FindDIBBits((LPSTR)lpD);	\
	BYTE * lpSrc = (BYTE*)::FindDIBBits((LPSTR)lpS);	\
	int dwWidth  = ::DIBWidth((LPSTR)lpS);\
	int dwHeight = ::DIBHeight((LPSTR)lpS);\
	int w=WIDTHBYTES(dwWidth*((LPBITMAPINFOHEADER)lpS)->biBitCount);	\
	HRTimer my_timer;	\
	my_timer.StartTimer();	\

#define BEGIN_SOURCE_PROCESSING \
	CDibDoc* pDocSrc=GetDocument();										\
	BeginWaitCursor();													\
	HDIB hBmpSrc=pDocSrc->GetHDIB();									\
	BYTE* lpS = (BYTE*)::GlobalLock((HGLOBAL)hBmpSrc);					\
	int iColors = DIBNumColors((char *)&(((LPBITMAPINFO)lpS)->bmiHeader)); \
	RGBQUAD *bmiColorsSrc = ((LPBITMAPINFO)lpS)->bmiColors;	\
	BYTE * lpSrc = (BYTE*)::FindDIBBits((LPSTR)lpS);	\
	int dwWidth  = ::DIBWidth((LPSTR)lpS);\
	int dwHeight = ::DIBHeight((LPSTR)lpS);\
	int w=WIDTHBYTES(dwWidth*((LPBITMAPINFOHEADER)lpS)->biBitCount);	\
	


#define END_SOURCE_PROCESSING	\
	::GlobalUnlock((HGLOBAL)hBmpSrc);								\
    EndWaitCursor();												\
/////////////////////////////////////////////////////////////////////////////


//---------------------------------------------------------------
#define SFARSIT_PRELUCRARI(Titlu)	\
	double elapsed_time_ms = my_timer.StopTimer();	\
	CString Title;	\
	Title.Format(_TEXT("%s - Proc. time = %.2f ms"), _TEXT(Titlu), elapsed_time_ms);	\
	::GlobalUnlock((HGLOBAL)hBmpDest);								\
	::GlobalUnlock((HGLOBAL)hBmpSrc);								\
    EndWaitCursor();												\
	pDocDest->SetHDIB(hBmpDest);									\
	pDocDest->InitDIBData();										\
	pDocDest->SetTitle((LPCTSTR)Title);									\
	CFrameWnd* pFrame=pDocTemplate->CreateNewFrame(pDocDest,NULL);	\
	pDocTemplate->InitialUpdateFrame(pFrame, pDocDest);	\





/////////////////////////////////////////////////////////////////////////////
// CDibView
typedef struct _lSet
{
	int label;
	int rank;
	_lSet *parent;
}lSet;

typedef struct list
{
	lSet *set;
	list *next;
} list;

list *current_node;
list *root = current_node;
list array[500];

bool mmhm = true;

int current_label = 0;


IMPLEMENT_DYNCREATE(CDibView, CScrollView)

BEGIN_MESSAGE_MAP(CDibView, CScrollView)
	//{{AFX_MSG_MAP(CDibView)
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, OnUpdateEditCopy)
	ON_COMMAND(ID_EDIT_PASTE, OnEditPaste)
	ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE, OnUpdateEditPaste)
	ON_MESSAGE(WM_DOREALIZE, OnDoRealize)
	ON_COMMAND(ID_PROCESSING_PARCURGERESIMPLA, OnProcessingParcurgereSimpla)
	//}}AFX_MSG_MAP

	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, CScrollView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CScrollView::OnFilePrintPreview)
	ON_COMMAND(ID_PROCESSING_NEWPROCESSING, &CDibView::OnProcessingNewprocessing)
	ON_COMMAND(ID_PROCESSING_AFISAREBMPHEADER, &CDibView::OnProcessingAfisarebmpheader)
	ON_COMMAND(ID_PROCESSING_GRAY2, &CDibView::OnProcessingGray2)
	ON_COMMAND(ID_PROCESSING_GRAY24BP, &CDibView::OnProcessingGray24bp)
	ON_COMMAND(ID_PROCESSING_SORT, &CDibView::OnProcessingSort)
	ON_COMMAND(ID_PROCESSING_BLACKANDWHITE, &CDibView::OnProcessingBlackandwhite)
	ON_COMMAND(ID_PROCESSING_HISTOGRAM, &CDibView::OnProcessingHistogram)
	ON_COMMAND(ID_PROCESSING_THRESHOLD, &CDibView::OnProcessingThreshold)
	ON_COMMAND(ID_PROCESSING_DITHERING, &CDibView::OnProcessingDithering)
	ON_WM_LBUTTONDBLCLK()
	ON_COMMAND(ID_PROCESSING_HORIZONTALPROJECTION, &CDibView::OnProcessingHorizontalprojection)
	ON_COMMAND(ID_PROCESSING_VERTICALPROJECTION, &CDibView::OnProcessingVerticalProjection)
	ON_COMMAND(ID_PROCESSING_AXISOFELONGATION, &CDibView::OnProcessingAxisofelongation)
	ON_COMMAND(ID_PROCESSING_LABELING, &CDibView::OnProcessingLabeling)
	ON_COMMAND(ID_PROCESSING_LABELING2, &CDibView::OnProcessingLabeling2)
	ON_COMMAND(ID_PROCESSING_TRACING, &CDibView::OnProcessingTracing)
	ON_COMMAND(ID_PROCESSING_FILLTRACE, &CDibView::OnProcessingFilltrace)
END_MESSAGE_MAP()




/////////////////////////////////////////////////////////////////////////////
// CDibView construction/destruction

CDibView::CDibView()
{
}

CDibView::~CDibView()
{
}

/////////////////////////////////////////////////////////////////////////////
// CDibView drawing

void CDibView::OnDraw(CDC* pDC)
{
	CDibDoc* pDoc = GetDocument();

	HDIB hDIB = pDoc->GetHDIB();
	if (hDIB != NULL)
	{
		LPSTR lpDIB = (LPSTR) ::GlobalLock((HGLOBAL) hDIB);
		int cxDIB = (int) ::DIBWidth(lpDIB);         // Size of DIB - x
		int cyDIB = (int) ::DIBHeight(lpDIB);        // Size of DIB - y
		::GlobalUnlock((HGLOBAL) hDIB);
		CRect rcDIB;
		rcDIB.top = rcDIB.left = 0;
		rcDIB.right = cxDIB;
		rcDIB.bottom = cyDIB;
		CRect rcDest;
		if (pDC->IsPrinting())   // printer DC
		{
			// get size of printer page (in pixels)
			int cxPage = pDC->GetDeviceCaps(HORZRES);
			int cyPage = pDC->GetDeviceCaps(VERTRES);
			// get printer pixels per inch
			int cxInch = pDC->GetDeviceCaps(LOGPIXELSX);
			int cyInch = pDC->GetDeviceCaps(LOGPIXELSY);

			//
			// Best Fit case -- create a rectangle which preserves
			// the DIB's aspect ratio, and fills the page horizontally.
			//
			// The formula in the "->bottom" field below calculates the Y
			// position of the printed bitmap, based on the size of the
			// bitmap, the width of the page, and the relative size of
			// a printed pixel (cyInch / cxInch).
			//
			rcDest.top = rcDest.left = 0;
			rcDest.bottom = (int)(((double)cyDIB * cxPage * cyInch)
					/ ((double)cxDIB * cxInch));
			rcDest.right = cxPage;
		}
		else   // not printer DC
		{
			rcDest = rcDIB;
		}
		::PaintDIB(pDC->m_hDC, &rcDest, pDoc->GetHDIB(),
			&rcDIB, pDoc->GetDocPalette());
	}
}

/////////////////////////////////////////////////////////////////////////////
// CDibView printing

BOOL CDibView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

/////////////////////////////////////////////////////////////////////////////
// CDibView commands


LRESULT CDibView::OnDoRealize(WPARAM wParam, LPARAM)
{
	ASSERT(wParam != NULL);
	CDibDoc* pDoc = GetDocument();
	if (pDoc->GetHDIB() == NULL)
		return 0L;  // must be a new document

	CPalette* pPal = pDoc->GetDocPalette();
	if (pPal != NULL)
	{
		CMainFrame* pAppFrame = (CMainFrame*) AfxGetApp()->m_pMainWnd;
		ASSERT_KINDOF(CMainFrame, pAppFrame);

		CClientDC appDC(pAppFrame);
		// All views but one should be a background palette.
		// wParam contains a handle to the active view, so the SelectPalette
		// bForceBackground flag is FALSE only if wParam == m_hWnd (this view)
		CPalette* oldPalette = appDC.SelectPalette(pPal, ((HWND)wParam) != m_hWnd);

		if (oldPalette != NULL)
		{
			UINT nColorsChanged = appDC.RealizePalette();
			if (nColorsChanged > 0)
				pDoc->UpdateAllViews(NULL);
			appDC.SelectPalette(oldPalette, TRUE);
		}
		else
		{
			TRACE0("\tSelectPalette failed in CDibView::OnPaletteChanged\n");
		}
	}

	return 0L;
}

void CDibView::OnInitialUpdate()
{
	CScrollView::OnInitialUpdate();
	ASSERT(GetDocument() != NULL);

	SetScrollSizes(MM_TEXT, GetDocument()->GetDocSize());
}


void CDibView::OnActivateView(BOOL bActivate, CView* pActivateView,
					CView* pDeactiveView)
{
	CScrollView::OnActivateView(bActivate, pActivateView, pDeactiveView);

	if (bActivate)
	{
		ASSERT(pActivateView == this);
		OnDoRealize((WPARAM)m_hWnd, 0);   // same as SendMessage(WM_DOREALIZE);
	}
}

void CDibView::OnEditCopy()
{
	CDibDoc* pDoc = GetDocument();
	// Clean clipboard of contents, and copy the DIB.

	if (OpenClipboard())
	{
		BeginWaitCursor();
		EmptyClipboard();
		SetClipboardData (CF_DIB, CopyHandle((HANDLE) pDoc->GetHDIB()) );
		CloseClipboard();
		EndWaitCursor();
	}
}



void CDibView::OnUpdateEditCopy(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(GetDocument()->GetHDIB() != NULL);
}


void CDibView::OnEditPaste()
{
	HDIB hNewDIB = NULL;

	if (OpenClipboard())
	{
		BeginWaitCursor();

		hNewDIB = (HDIB) CopyHandle(::GetClipboardData(CF_DIB));

		CloseClipboard();

		if (hNewDIB != NULL)
		{
			CDibDoc* pDoc = GetDocument();
			pDoc->ReplaceHDIB(hNewDIB); // and free the old DIB
			pDoc->InitDIBData();    // set up new size & palette
			pDoc->SetModifiedFlag(TRUE);

			SetScrollSizes(MM_TEXT, pDoc->GetDocSize());
			OnDoRealize((WPARAM)m_hWnd,0);  // realize the new palette
			pDoc->UpdateAllViews(NULL);
		}
		EndWaitCursor();
	}
}


void CDibView::OnUpdateEditPaste(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(::IsClipboardFormatAvailable(CF_DIB));
}

void CDibView::OnProcessingParcurgereSimpla() 
{
	// TODO: Add your command handler code here
	BEGIN_PROCESSING();

	// Makes a grayscale image by equalizing the R, G, B components from the LUT
	for (int k=0;  k < iColors ; k++)
		bmiColorsDst[k].rgbRed=bmiColorsDst[k].rgbGreen=bmiColorsDst[k].rgbBlue=k;

	//  Goes through the bitmap pixels and performs their negative	
	for (int i=0;i<dwHeight;i++)
		for (int j=0;j<dwWidth;j++)
		  {	
			lpDst[i*w+j]= 255 - lpSrc[i*w+j]; //makes image negative
	  }

	END_PROCESSING("Negativ imagine");
}



void CDibView::OnProcessingNewprocessing()
{
	// My code ;)
	BEGIN_PROCESSING();

	//Populate the LUT
	for (int k = 0; k < iColors; k++){

			bmiColorsDst[k].rgbRed = min((bmiColorsSrc[k].rgbRed * 0.393) + (bmiColorsSrc[k].rgbGreen *.769) + (bmiColorsSrc[k].rgbBlue * .189), 255);
			bmiColorsDst[k].rgbGreen = min((bmiColorsSrc[k].rgbRed * 0.349) + (bmiColorsSrc[k].rgbGreen *.686) + (bmiColorsSrc[k].rgbBlue * .168), 255);
			bmiColorsDst[k].rgbBlue = min((bmiColorsSrc[k].rgbRed * 0.272) + (bmiColorsSrc[k].rgbGreen *.534) + (bmiColorsSrc[k].rgbBlue * .131), 255);
			mmhm = !mmhm;
	}
	int r = rand();
	for (int i = 0; i<dwHeight; i++)
		for (int j = 0; j<dwWidth; j++)
		{
			lpDst[i*w + j] =  (lpSrc[i*w + j] + r)%255; 
		}

	END_PROCESSING("Something Something");
}



void CDibView::OnProcessingAfisarebmpheader()
{

		BEGIN_SOURCE_PROCESSING;
		CBitmapInfoDlg dlgBmpHeader;

		LPBITMAPINFO pBitmapInfoSrc = (LPBITMAPINFO)lpS;
		dlgBmpHeader.m_Width.Format(_TEXT("Image width [pixels]: %d"), pBitmapInfoSrc->bmiHeader.biWidth);
		dlgBmpHeader.m_Height.Format(_TEXT("Image height [pixels]: %d"), pBitmapInfoSrc->bmiHeader.biHeight);
		dlgBmpHeader.m_BitsPixel.Format(_TEXT("Bits / Pixel: %d / %d"), pBitmapInfoSrc->bmiHeader.biXPelsPerMeter);
		//+ others
		CString buffer;
		/*for (int i = 0; i < iColors; i++)
		{
			buffer.Format(_TEXT("%3d.\t%3d\t%3d\r\n"), i, bmiColorsSrc[i].rgbRed, bmiColorsSrc[i].rgbGreen, bmiColorsSrc[i].rgbBlue);
			dlgBmpHeader.m_LUT += buffer;
		}*/

		//TODO add code for reading bla bla

		dlgBmpHeader.DoModal();
		END_SOURCE_PROCESSING;
	
}





void CDibView::OnProcessingGray2()
{

		BEGIN_PROCESSING();
		// TODO: Add your command handler code here

		// Makes a grayscale image 
		for (int k = 0; k < iColors; k++)
			bmiColorsDst[k].rgbRed = bmiColorsDst[k].rgbGreen = bmiColorsDst[k].rgbBlue = (bmiColorsDst[k].rgbRed + bmiColorsDst[k].rgbGreen + bmiColorsDst[k].rgbBlue) / 3;

		//  Goes through the bitmap pixels and performs their negative	
		for (int i = 0; i<dwHeight; i++)
			for (int j = 0; j<dwWidth; j++)
			{
				lpDst[i*w + j] = lpSrc[i*w + j]; //makes image negative
			}

		END_PROCESSING("Grayscale8p");
	
}


void CDibView::OnProcessingGray24bp()
{
	BEGIN_PROCESSING();
	BYTE gray;

	for (int k = 0; k < iColors; k++)
		bmiColorsDst[k].rgbRed = bmiColorsDst[k].rgbGreen = bmiColorsDst[k].rgbBlue = (bmiColorsDst[k].rgbRed + bmiColorsDst[k].rgbGreen + bmiColorsDst[k].rgbBlue) / 3;
	
	for (int i = 0; i < dwHeight; i++)
	{
		for (int j = 0; j < dwWidth; j++)
		{
			gray = (lpSrc[i*w + 3 * j] + lpSrc[i*w + 3 * j + 1] + lpSrc[i*w + 3 * j] + 2) / 3;
			lpDst[i*w + 3 * j] = gray;
			lpDst[i*w + 3 * j + 1] = gray;
			lpDst[i*w + 3 * j + 2] = gray;
		}
	}

	END_PROCESSING("ColorToGray24b");
}


void CDibView::OnProcessingSort()
{
	// TODO: Add your command handler code here
	BEGIN_PROCESSING();
	BYTE g[256];
	for (int k = 0; k < iColors; k++)
	{
		g[k] = bmiColorsSrc[k].rgbRed;
		bmiColorsDst[k].rgbRed = bmiColorsDst[k].rgbGreen = bmiColorsDst[k].rgbBlue = k;
	}
	for (int i = 0; i<dwHeight; i++)
		for (int j = 0; j < dwWidth; j++)
		{
			int k = lpDst[i*w + j];
			lpDst[i*w + j] = g[k];
		}

	END_PROCESSING("Sorting LUT");
}


void CDibView::OnProcessingBlackandwhite()
{
	BEGIN_PROCESSING();
	int step = 150;
	// Makes a grayscale image 

	for (int k = 0; k < iColors; k++)
	{
		int x = bmiColorsDst[k].rgbRed + bmiColorsDst[k].rgbGreen + bmiColorsDst[k].rgbBlue / 3;
		if (x < step)
		{
			bmiColorsDst[k].rgbRed = bmiColorsDst[k].rgbGreen = bmiColorsDst[k].rgbBlue = 0;
		}
		else
		{
			bmiColorsDst[k].rgbRed = bmiColorsDst[k].rgbGreen = bmiColorsDst[k].rgbBlue = 255;
		}
	}

	
	for (int i = 0; i<dwHeight; i++)
		for (int j = 0; j<dwWidth; j++)
		{
				lpDst[i*w + j] = lpSrc[i*w + j];

		}
	END_PROCESSING("Black & White");
}


void CDibView::OnProcessingHistogram()
{
	// TODO: Add your command handler code here

	BEGIN_SOURCE_PROCESSING;

	int histValues[256] = { 0 };
	float FDPValues[256];
	// write the code for computing the histogram and store it in the array of int, histValues[256]
		// write the code for computing the PDF and store it in the array of double FDPValues[256]
		// instantiate a dialog box for display and associate the histogram

	for (int i = 0; i < dwHeight; i++)
		for (int j = 0; j < dwWidth; j++)
		{
			int k = lpSrc[i*w + j];
			histValues[k]++;

			//FDPValues[i] = histValues[i] / (dwHeight*dwWidth);
		}

	CDlgHistogram dlg;
	memcpy(dlg.m_Histogram.values, histValues, sizeof(histValues));
	// display the dialog box
	dlg.DoModal();
	END_SOURCE_PROCESSING;
}

int findMaxHisto(float x, float maxVal[], int maxIndex)
{

	int dist = 5555;
	int val;
	for (int i = 0; i < maxIndex; i++)
	{
		if (abs((int)maxVal[i] - x) < dist)
		{
			dist = abs(x - (int)maxVal[i]);
			val = maxVal[i];
		}
	}
	return val;
}

void CDibView::OnProcessingThreshold()
{
	BEGIN_PROCESSING();
	// TODO: Add your command handler code here
	int histValues[256] = { 0 };
	float FDPValues[256];


	// write the code for computing the histogram and store it in the arrayof int, histValues[256]
	// write the code for computing the PDF and store it in the array of double FDPValues[256]
	// instantiate a dialog box for display and associate the histogram 

	CDlgHistogram dlg;

	for (int i = 0; i < dwHeight; i++)
		for (int j = 0; j < dwWidth; j++)
		{
			int k = lpSrc[i*w + j];
			histValues[k]++;
		}

	for (int i = 0; i <255; i++)
	{
		FDPValues[i] = (float)histValues[i] / (dwHeight*dwWidth);
	}

	int wh = 5;
	float th = 0.0003, sum = 0, max;

	int contor;

	for (int k = wh; k < 255 - wh; k++)
	{
		contor = 0;
		sum = 0;
		max = 0;
		for (int i = k - wh; i <= k + wh; i++)
		{
			sum += FDPValues[i];
			contor++;
			if (FDPValues[i] > max)
				max = FDPValues[i];
		}
		float avg = sum / (2 * wh + 1); //to see
		if (FDPValues[k]>(avg + th) && FDPValues[k] >= max)
		{
			
			maxVal[maxIndex] = k;
			maxIndex++;
			
		}
	}

	maxVal[0] = 0;
	maxVal[maxIndex] = 255;


	for (int i = 0; i<dwHeight; i++)
		for (int j = 0; j<dwWidth; j++)
		{
			lpDst[i*w + j] = findMaxHisto(lpSrc[i*w + j], maxVal, maxIndex);
		}

	END_PROCESSING("Threshold");
}


/*
	for each y from bottom to top
		for each x from left to right
			oldpixel := pixel(x,y)
			newpixel := find_closest_histogram_maximum(oldpixel)
			pixel(x,y) := newpixel
			error := oldpixel - newpixel
			pixel(x+1,y) := pixel(x+1,y) + 7*error /16
			pixel(x-1,y+1) := pixel(x-1,y+1) + 3*error/16
			pixel(x,y+1) := pixel(x,y+1) + 5*error/16
			pixel(x+1,y+1) := pixel(x+1,y+1) + error/16
*/

byte valuesWrap(int error)
{
	if (error > 255)
		return 255;

	if (error < 0)
		return 0;

	return error;
}

void CDibView::OnProcessingDithering()
{
	// TODO: Add your command handler code here
	BEGIN_PROCESSING();

	// TODO: Add your command handler code here
	int histValues[256] = { 0 };
	float FDPValues[256];


	// write the code for computing the histogram and store it in the arrayof int, histValues[256]
	// write the code for computing the PDF and store it in the array of double FDPValues[256]
	// instantiate a dialog box for display and associate the histogram 

	CDlgHistogram dlg;

	for (int i = 0; i < dwHeight; i++)
	for (int j = 0; j < dwWidth; j++)
	{
		int k = lpSrc[i*w + j];
		histValues[k]++;
	}

	for (int i = 0; i <255; i++)
	{
		FDPValues[i] = (float)histValues[i] / (dwHeight*dwWidth);
	}

	int wh = 5;
	float th = 0.0003, sum = 0, max;

	int contor;

	for (int k = wh; k < 255 - wh; k++)
	{
		contor = 0;
		sum = 0;
		max = 0;
		for (int i = k - wh; i <= k + wh; i++)
		{
			sum += FDPValues[i];
			contor++;
			if (FDPValues[i] > max)
				max = FDPValues[i];
		}
		float avg = sum / (2 * wh + 1); //to see
		if (FDPValues[k]>(avg + th) && FDPValues[k] >= max)
		{

			maxVal[maxIndex] = k;
			maxIndex++;

		}
	}

	maxVal[0] = 0;
	maxVal[maxIndex] = 255;


	for (int i = 0; i<dwHeight; i++)
	for (int j = 0; j<dwWidth; j++)
	{
		lpDst[i*w + j] = findMaxHisto(lpSrc[i*w + j], maxVal, maxIndex);
	}

	for (int i = 0; i<dwHeight-1; i++)
		for (int j = 1; j < dwWidth-1; j++)
		{
			int oldpix = lpDst[i*w + j],dist;
			int newpix = findMaxHisto(lpDst[i*w + j], maxVal, maxIndex);

			lpDst[i*w + j] = newpix;
			float error = oldpix - newpix;

			lpDst[(i + 1)*w + j] = valuesWrap(lpDst[(i + 1)*w + j] + 5 * error / 16);
			lpDst[(i + 1)*w + j - 1] = valuesWrap(lpDst[(i + 1)*w + j - 1] + 3 * error / 16);
			lpDst[i*w + j + 1] = valuesWrap(lpDst[i*w + j + 1] + 7 * error / 16);
			lpDst[(i + 1)*w + j + 1] = valuesWrap(lpDst[(i + 1)*w + j + 1] + error / 16);

			/*
			int aux = lpDst[(i + 1)*w + j] + 7 * error / 16;
			if (aux < 0)
			{
				lpDst[(i + 1)*w + j] = 0;
			}
			if (aux > 255){
				lpDst[(i + 1)*w + j] = 255;
			}
			else{
				lpDst[(i + 1)*w + j] = aux;
			}

			aux = lpDst[(i - 1)*w + (j + 1)] + 3 * error / 16;
			if (aux < 0)
			{
				lpDst[(i - 1)*w + (j + 1)] = 0;
			}
			if (aux > 255){
				lpDst[(i - 1)*w + (j + 1)] = 255;
			}
			else{
				lpDst[(i - 1)*w + (j + 1)] = aux;
			}

			aux = lpDst[i*w + (j + 1)] + 5 * error / 16;
			if (aux < 0)
			{
				lpDst[(i - 1)*w + (j + 1)] = 0;
			}
			if (aux > 255){
				lpDst[(i - 1)*w + (j + 1)] = 255;
			}
			else{
				lpDst[i*w + (j + 1)] = aux;
			}

			aux =  lpSrc[(i + 1)*w + (j + 1)] + error / 16;
			if (aux < 0)
			{
				lpDst[(i - 1)*w + (j + 1)] = 0;
			}
			if (aux > 255){
				lpDst[(i - 1)*w + (j + 1)] = 255;
			}
			else{
				lpDst[(i + 1)*w + (j + 1)] = aux;
			}*/
		}

	END_PROCESSING("Floyd - Steinberg Alg");
}





void CDibView::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default

	BEGIN_SOURCE_PROCESSING;

	
	//obtain the scroll position (because of scroll bars' positions
	//the coordinates may be shifted) and adjust the position
	CPoint pos = GetScrollPosition() + point;
	//point contains the window's client area coordinates
	//the y axis is inverted because of the way bitmaps
	//are represented in memory
	double tangent, alpha, elongation;
	int x = pos.x, rCentre, cCentre, sum1 = 0, sum2 = 0;
	double dif1=0, dif2=0, sqdif1=0, sqdif2=0, thinnes = 0, ar=0;
	int y = dwHeight - pos.y - 1;
	int color = 0, objCount[256] = { 0 };
	int area = 0, perimeter = -1, cmax=-1, cmin=55555, rmax=-1, rmin=55555;

	//test if the position is inside the image
	if (x>0 && x<dwWidth && y>0 && y<dwHeight)
	{
		//prepare a CString for formating the output message
		CString info, info2;
		bool inside = true;
		
		info2.Format(_TEXT("x=%d, y=%d  \n Outside an obj!"), x, y);

		color = lpSrc[y*w + x];


		if (lpSrc[(y + 1) *w + (x - 1)] == color && lpSrc[(y + 1)*w + x] == color && lpSrc[(y + 1)*w + (x + 1)] == color &&
			lpSrc[y*w + (x - 1)] == color && lpSrc[y*w + (x + 1)] == color
			&& lpSrc[(y - 1)*w + (x - 1)] == color && lpSrc[(y - 1)*w + x] == color && lpSrc[(y - 1)*w + (x + 1)] == color && color != 255)
		{
			area = -1;
			sum1 = 0; 
			sum2 = 0;
			for (int i = 0; i < dwHeight; i++)
			{
				for (int j = 0; j < dwWidth; j++)
				{
					if (lpSrc[y*w + x] == lpSrc[i*w + j])
					{
						//objCount[i*w + j] = 1;
						area++;
						sum1 += i;
						sum2 += j;
						if (cmax < j)
							cmax = j;
						if (cmin > j)
							cmin = j;
						if (rmax < i)
							rmax = i;
						if (rmin > i)
							rmin = i;
					}
				}
			}

			cCentre = sum1 / area;
			rCentre = sum2 / area;

			color = 255;

			for (int i = 0; i < dwHeight; i++)
			{
				for (int j = 0; j < dwWidth; j++)
				{
					if (lpSrc[y*w + x] == lpSrc[i*w + j])
					{
						dif1 = dif1 + (i - rCentre)*(j - cCentre);
						sqdif1 = sqdif1 + (i - rCentre)*(i - rCentre);
						sqdif2 = sqdif2 + (j - cCentre)*(j - cCentre);


						if ((lpSrc[(i + 1) *w + (j - 1)]) == color || (lpSrc[(i + 1)*w + j] == color) || (lpSrc[(i + 1)*w + (j + 1)] == color) ||
							(lpSrc[i*w + (j - 1)] == color) || (lpSrc[i*w + (j + 1)] == color)
							|| (lpSrc[(i - 1)*w + (j - 1)] == color) || (lpSrc[(i - 1)*w + j] == color) || (lpSrc[(i - 1)*w + (j + 1)] == color))
							perimeter++;
					}
				}
			}


			perimeter =(int) perimeter * (3.14 /4);

			tangent = 2 * dif1 / (sqdif2 - sqdif1);
			alpha = atan2(2 * dif1, sqdif2 - sqdif1) / 2;
			elongation = alpha * 180 / 3.14;

			thinnes = 4 * 3.14 * area / (perimeter * perimeter);
			ar = (cmax - cmin + 1) / (rmax - rmin +1);


			info.Format(_TEXT("x=%d, y=%d, area = %d\n MC=(%d,%d), El=%lf Perr=%d Th=%lf Ar=%lf L=%lf\n Inside an obj!"), x, y, area, rCentre, cCentre, elongation, perimeter, thinnes, ar, alpha);
			//info.Format(_TEXT("cmax=%lf  cmin=%lf rmax=%lf rmin=%lf"),cmax, cmin, rmax, rmin );
			AfxMessageBox(info);
		}
		else
		{
			inside = false;
			AfxMessageBox(info2);
		}

		
	}
	END_SOURCE_PROCESSING;
	//call the superclass' method 
	
	/*
	BEGIN_PROCESSING();
	CDC dc; //memory DC
	dc.CreateCompatibleDC(0); //create it compatible with the screen

	CBitmap ddBitmap;//to hold a device dependent bitmap compatible with
	//the screen

	//create a DDB, compatible with the screen
	//and initialize it with the data from the source DIB
	HBITMAP hDDBitmap =
		CreateDIBitmap(::GetDC(0), &((LPBITMAPINFO)lpS)->bmiHeader, CBM_INIT,
		lpSrc, (LPBITMAPINFO)lpS, DIB_RGB_COLORS);

	//attach the handle to the CBitmap object
	ddBitmap.Attach(hDDBitmap);
	//select the DDB into the memory DC
	//so that all drawing will be performed on the DDB
	CBitmap* pTempBmp = dc.SelectObject(&ddBitmap);

	//will be made on the DDB
	//obtain the scroll position (because of scroll bars' positions
	//the coordinates may be shifted) and adjust the position
	CPoint pos = GetScrollPosition() + point;

	//create a green pen for drawing
	CPen pen(PS_SOLID, 1, RGB(0, 255, 0));
	//select the pen on the device context
	CPen *pTempPen = dc.SelectObject(&pen);
	//draw a text
	dc.TextOut(pos.x, pos.y, "test");
	//and a line
	dc.MoveTo(pos.x, pos.y);
	dc.LineTo(pos.x, pos.y - 20);

	//select back the old pen
	dc.SelectObject(pTempPen);
	//and the old bitmap
	dc.SelectObject(pTempBmp);
	//copy the pixel data from the device dependent bitmap
	//to the destination DIB
	GetDIBits(dc.m_hDC, ddBitmap, 0, dwHeight, lpDst, (LPBITMAPINFO)lpD,
		DIB_RGB_COLORS);

	END_PROCESSING("line");
	*/

	//from this point onward, all drawing done using the DC object 
	CScrollView::OnLButtonDblClk(nFlags, point);
}


void CDibView::OnProcessingHorizontalprojection()
{
	// TODO: Add your command handler code here

	BEGIN_PROCESSING();

	int line[3000] = { 0 };

	for (int i = 0; i < dwHeight; i++)
	{
		for (int j = 0; j < dwWidth; j++)
		{
			if (lpSrc[i*w+j] == 0)
				line[i]++;
		}
	}

	for (int i = 0; i < dwHeight; i++)
	{
		for (int j = 0; j < dwWidth; j++)
		{
			lpDst[i*w + j] = 255;
		}
	}

	for (int i = 0; i < dwHeight; i++)
	{
		
			for (int k = 0; k < line[i]; k++)
				lpDst[i*w + k] = 0;
		
	}

	END_PROCESSING("Horizontal Projection");
}


void CDibView::OnProcessingVerticalProjection()
{
	// TODO: Add your command handler code here
	BEGIN_PROCESSING();

	int line[3000] = { 0 };

	for (int i = 0; i < dwHeight; i++)
	{
		for (int j = 0; j < dwWidth; j++)
		{
			if (lpSrc[i*w + j] == 0)
				line[j]++;
		}
	}

	for (int i = 0; i < dwHeight; i++)
	{
		for (int j = 0; j < dwWidth; j++)
		{
			lpDst[i*w + j] = 255;
		}
	}

	for (int i = 0; i < dwHeight; i++)
	{

		for (int k = 0; k < line[i]; k++)
			lpDst[k*w + i] = 0;

	}

	END_PROCESSING("Vertical Projection");
}


void CDibView::OnProcessingAxisofelongation()
{
	// TODO: Add your command handler code here
	BEGIN_PROCESSING();

	for (int i = 0; i < dwHeight; i++)
	{
		for (int j = 0; j < dwWidth; j++)
		{
			lpDst[i*w + j] = 255;
		}
	}

	int oneX = 0, oneY, twoX = 0, twoY;

	for (int i = 0; i < dwHeight; i++)
	{
		for (int j = 0; j < dwWidth; j++)
		{
			if (i - 181 == -0.83 * (j - 67))
			{
				lpDst[i*w + j] = 0;
				oneX = i;
				oneY = j;
			}
		}
	}
	END_PROCESSING("aXIS");
}

int A_label(int i, int j){
	return labels[i][j - 1];
}

int B_label(int i, int j){
	return labels[i - 1][j - 1];
}

int C_label(int i, int j){
	return labels[i - 1][j];
}

int D_label(int i, int j){
	return labels[i][j + 1];
}

lSet* makeSet(int l)
{
	lSet *set = (lSet*)malloc(sizeof(lSet));
	set->label = l;
	set->rank = 0;
	set->parent = set;

	return set;
}

lSet *findSet(lSet *x)
{
	if (x->parent != x)
	{
		x->parent = (findSet(x->parent));
	}

	return x->parent;
}

int findRoot(lSet *set)
{
	if (set->parent != set)
		findRoot(set->parent);
	else
		return set->label;
}

lSet* unionSet(lSet *x, lSet *y)
{
	lSet *rootX, *rootY;

	rootX = findSet(x);
	rootY = findSet(y);

	if (rootX == rootY)
		return NULL;

	if (rootX->rank < rootY->rank)
	{
		rootY->parent = rootX;
	}
	else
	{
		rootY->parent = rootX;
		rootX->rank++;
	}
	return rootX;
}

void newPair(int l1, int l2)
{
	lSet *set1 = makeSet(l1);
	lSet *set2 = makeSet(l2);

	current_node = (list *)malloc(sizeof(list));

	current_node->set = unionSet(set1, set2);
	current_node = current_node->next;
	current_node = NULL;
}

lSet* setNewLabel(lSet *set, int label)
{
	set->label=label;
	return set;
}


void equiv()
{
	list *curr = root;
	int label = curr->set->label;
	while (curr != NULL)
	{
		curr->set->label = label;
		curr = curr->next;
	}
}


void CDibView::OnProcessingLabeling()
{
	// TODO: Add your command handler code here
	BEGIN_PROCESSING();

	current_label = 0;

	for (int i = dwHeight - 2; i > 0; i--)
	{
		for (int j = 0; j < dwWidth - 1; j++)
		{
			if (lpSrc[i*w + j] != 0)
			{
				//NW and NE but not N
				if (B_label(i,j) != 0 && D_label(i,j)!=0)
				{
					labels[i][j] = B_label(i, j);
					//add pair NW and NE
					newPair(B_label(i, j), D_label(i, j));
					
				}
				else if (B_label(i, j) != 0 && (C_label(i, j) != 0 || D_label(i, j) == 0))
				{
					labels[i][j] = B_label(i, j);
				}
				else if (B_label(i, j) == 0 && A_label(i, j) != 0 && C_label(i, j) != 0)
				{
					labels[i][j] = A_label(i, j);
					//add pair W and N
					newPair(A_label(i, j), C_label(i, j));
				}
				else if (B_label(i, j) == 0 && C_label(i, j) == 0 && A_label(i, j) != 0 && D_label(i, j) != 0)
				{
					labels[i][j] = A_label(i, j);
					//add pair for W and NE
					newPair(A_label(i, j), D_label(i, j));
				}
				else if (A_label(i, j) != 0 && B_label(i, j) == 0 && C_label(i, j) == 0 && D_label(i, j) == 0)
				{
					labels[i][j] = A_label(i, j);
				}
				else if (A_label(i, j) == 0 && B_label(i, j) == 0 && C_label(i, j) != 0)
				{
					labels[i][j] = C_label(i, j);
				}
				else if (A_label(i, j) == 0 && B_label(i, j) == 0 && C_label(i, j) == 0 && D_label(i, j) != 0)
				{
					labels[i][j] = D_label(i, j);
				}
				else if (A_label(i, j) == 0 && B_label(i, j) == 0 && C_label(i, j) == 0 && D_label(i, j) == 0)
				{
					//generate new label
					current_label++;
					labels[i][j] = current_label;
				}
			}
		}
	}

	for (int i = dwHeight - 2; i > 0; i--)
	{
		for (int j = 0; j < dwWidth - 1; j++)
		{
			//labels[i][j]=
		}
	}

	for (int k = 1; k <= 254; k++)
	{
		bmiColorsDst[k].rgbRed = rand() % 255;
		bmiColorsSrc[k].rgbGreen = rand() % 255;
		bmiColorsSrc[k].rgbBlue = rand() % 255;
	}

	for (int i = 0; i < dwHeight; i++)
	{
		for (int j = 0; j < dwWidth; j++)
		{
			lpDst[i*w + j] = labels[i][j];
		}
	}

	END_PROCESSING("lABELING");
}


void CDibView::OnProcessingLabeling2()
{
	// TODO: Add your command handler code here
	BEGIN_PROCESSING();
	int *label = new int[w*dwHeight];
	for (int i = 0; i < w*dwHeight; i++)
	{
		label[i] = 255;
	}
	int pair[5000][2];
	for (int i = 0; i < 5000; i++)
	{
		pair[i][0] = 0;
		pair[i][1] = 0;
	}
	int pairnr = 0;
	int count = 1;
	int W, NW, N, NE;
	for (int i = dwHeight - 2; i >= 0; i--)
	{
		for (int j = 1; j < dwWidth - 1; j++)
		{
			if (lpSrc[i*w + j] == 0)
			{
				W = i*w + j - 1;		//A
				NW = (i + 1)*w + j - 1;	//B
				N = (i + 1)*w + j;		//C
				NE = (i + 1)*w + j + 1;	//D
				if (lpSrc[NW] == 0)
				{
					if ((lpSrc[N] != 0) && (lpSrc[NE] == 0))
					{
						pair[pairnr][0] = label[NW];
						pair[pairnr][1] = label[NE];
						pairnr++;
						label[i*w + j] = label[NW];
					}
					else
						label[i*w + j] = label[NW];
				}
				else
				if (lpSrc[W] == 0)
				{
					if (lpSrc[N] != 0)
					{
						if (lpSrc[NE] != 0)
						{
							label[i*w + j] = label[W];
						}
						else
						{
							label[i*w + j] = label[W];
							pair[pairnr][0] = label[W];
							pair[pairnr][1] = label[NE];
							pairnr++;
						}
					}
					else
					{
						label[i*w + j] = label[W];
						pair[pairnr][0] = label[W];
						pair[pairnr][1] = label[N];
						pairnr++;
					}
				}
				else
				{
					if (lpSrc[N] != 0)
					{
						if (lpSrc[NE] != 0)
						{
							label[i*w + j] = count;
							count++;
						}
						else
						{
							label[i*w + j] = label[NE];
						}
					}
					else
					{
						label[i*w + j] = label[N];
					}
				}
			}
		}
	}
	srand(time(0));
	for (int k = 1; k <= 254; k++)
	{
		bmiColorsDst[k].rgbRed = rand() % 256;
		bmiColorsDst[k].rgbGreen = rand() % 256;
		bmiColorsDst[k].rgbBlue = rand() % 256;
	}
	int *cls = new int[count];
	for (int i = 1; i < count; i++)
		cls[i] = 0;
	int cla = 0;
	for (int k = 1; k < count; k++)
	{
		std::queue<int> q;
		q.push(k);
		if (cls[k] == 0)
		{
			cla++;
			cls[k] = cla;
			while (!q.empty())
			{
				int f = q.front();
				for (int j = 0; j < pairnr; j++)
				{
					if ((pair[j][0] == f) && (cls[pair[j][1]] == 0))
					{
						cls[pair[j][1]] = cla;
						q.push(pair[j][1]);
					}
					if ((pair[j][1] == f) && (cls[pair[j][0]] == 0))
					{
						cls[pair[j][0]] = cla;
						q.push(pair[j][0]);
					}
				}
				q.pop();
			}
		}
	}
	for (int i = dwHeight - 2; i >= 0; i--)
	{
		for (int j = 1; j < dwWidth - 1; j++)
		{
			if (lpSrc[i*w + j] == 0)
				lpDst[i*w + j] = cls[label[i*w + j]];
		}
	}
	delete[] label;
	END_PROCESSING("Labeled");
}


void CDibView::OnProcessingTracing()
{
	 BEGIN_PROCESSING();

	int dim = 8;
	int dx[] = { 1, 1, 0, -1, -1, -1, 0, 1 };
	int dy[] = { 0, 1, 1, 1, 0, -1, -1, -1 };



	int P0x;
	int P0y;

	int P1x;
	int P1y;

	int Pnx;
	int Pny;

	int anterior_x;
	int anterior_y;

	int dir = 7;
	int n = 0;
	int directions[1000];

	FILE *f = fopen("problema1.txt", "w+");

	for (int i = dwHeight - 1; i >= 0; i--)
	{
		for (int j = 0; j < dwWidth - 1; j++)
		{
			if (lpSrc[i*w + j] == 0)
			{
				P0x = j;
				P0y = i;
				
				anterior_x = P0x;
				anterior_y = P0y;

				//even
				if (dir % 2 == 0)
				{
					dir = (dir + 7) % 8;
				}
				else
				{
					//odd
					dir = (dir + 6) % 8;
				}

				//search next pixel
				//				i		*		w	+	j
				while (lpSrc[(P0y + dy[dir]) * w + (P0x + dx[dir])] != 0)
				{
					//goes counter clockwise 
					dir = (dir + 1) % 8;
				}

				//nr de pixeli
				n++;

				P1x = j + dx[dir];
				P1y = i + dy[dir];
				//Ultimul
				Pnx = P1x;
				Pny = P1y;

				break;
			}
		}
	}

	int x_new;
	int y_new;

	//white screen
	for (int i = 0; i < dwHeight; i++)
	{
		for (int j = 0; j < dwWidth; j++)
		{
			lpDst[i*w + j] = 255;
		}
	}

	do {
		if (dir % 2 == 0)
		{
			dir = (dir + 7) % 8;
		}
		else
		{
			dir = (dir + 6) % 8;
		}

		x_new = Pnx;
		y_new = Pny;

		while (lpSrc[(y_new + dy[dir]) * w + (x_new + dx[dir])] != 0)
		{
			dir = (dir + 1) % 8;
		}

		anterior_x = Pnx;
		anterior_y = Pny;

		Pnx = x_new + dx[dir];
		Pny = y_new + dy[dir];

		lpDst[Pny*w + Pnx] = 0;

		directions[n] = dir;
		n++;

	} while (!(((Pnx == P1x && Pny == P1y && anterior_x == P0x && anterior_y == P0y)) && n >= 2));

	fprintf(f, "n = %d \n", n);

	/*//directii
	for (int i = 0; i < n; i++)
	{
		fprintf(f, "directions: ");
		fprintf(f, "%d ", directions[i]);
	}*/

	//derivata
	fprintf(f, "\n \n \n ");
	fprintf(f, "derrivate directions: ");
	for (int i = 0; i < n - 1; i++)
	{

		fprintf(f, "%d ", (directions[i + 1] - directions[i] + 8) % 8);
	}

	fclose(f);

	END_PROCESSING("Tracing Algorithm");
}


void CDibView::OnProcessingFilltrace()
{
	BEGIN_PROCESSING();

	FILE *f = fopen("reconstruct.txt", "r+");
	int P0x;
	int P0y;
	int n;
	int CA;
	int i;
	int dx[] = { 1, 1, 0, -1, -1, -1, 0, 1 };
	int dy[] = { 0, 1, 1, 1, 0, -1, -1, -1 };

	// Pixeli Initialiali + numar total pixeli  
	fscanf(f, "%d %d\n%d", &P0x, &P0y, &n);

	while (fscanf(f, "%d ", &CA) > 0)
	{
		lpDst[P0y * w + P0x] = 0;
		P0x = P0x + dx[CA];
		P0y = P0y + dy[CA];
	}
	fclose(f);


	END_PROCESSING("Redraw");
}
