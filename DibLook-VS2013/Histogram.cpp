// Histogram.cpp : implementation file
//

#include "stdafx.h"
#include "diblook.h"
#include "Histogram.h"


// CHistogram

IMPLEMENT_DYNAMIC(CHistogram, CStatic)

CHistogram::CHistogram()
{

}

CHistogram::~CHistogram()
{
}


BEGIN_MESSAGE_MAP(CHistogram, CStatic)
	ON_WM_PAINT()
END_MESSAGE_MAP()



// CHistogram message handlers




void CHistogram::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	// TODO: Add your message handler code here
	// Do not call CStatic::OnPaint() for painting messages

	CPen pen(PS_SOLID, 1, RGB(255, 0, 0)); // define the display pen-for red color
		CPen *pTempPen = dc.SelectObject(&pen); // select the display pen
	CRect rect;
	GetClientRect(rect); // get the available display rectangular area
	int height = rect.Height(); // height of the display area
	int width = rect.Width(); // width of the display area
	// find the maximum in the array values[256]
	int i;
	int maxValue = 0;
	for (i = 0; i<256; i++)
	if (values[i]>maxValue)
		maxValue = values[i];
	// check if scaling is necessary
	double scaleFactor = 1.0;
	if (maxValue >= height)
	{
		// scaling is necessary
		scaleFactor = (double)height / maxValue;
	}

	// display the histogram in the form of vertical bars
	for (i = 0; i<256; i++)
	{
		// find the length of the line
		int lengthLine = (int)(scaleFactor*values[i]);
		//display the line
		dc.MoveTo(i, height);
		dc.LineTo(i, height - lengthLine);
	}
	dc.SelectObject(pTempPen); // restore the display pen
}
