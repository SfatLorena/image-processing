// dibview.h : interface of the CDibView class
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

class CDibView : public CScrollView
{
protected: // create from serialization only
	CDibView();
	DECLARE_DYNCREATE(CDibView)

// Attributes
public:
	CDibDoc* GetDocument()
		{
			ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CDibDoc)));
			return (CDibDoc*) m_pDocument;
		}

// Operations
public:

// Implementation
public:
	virtual ~CDibView();
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view

	virtual void OnInitialUpdate();
	virtual void OnActivateView(BOOL bActivate, CView* pActivateView,
					CView* pDeactiveView);

	// Printing support
protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);

// Generated message map functions
protected:
	//{{AFX_MSG(CDibView)
	afx_msg void OnEditCopy();
	afx_msg void OnUpdateEditCopy(CCmdUI* pCmdUI);
	afx_msg void OnEditPaste();
	afx_msg void OnUpdateEditPaste(CCmdUI* pCmdUI);
	afx_msg LRESULT OnDoRealize(WPARAM wParam, LPARAM lParam);  // user message
	afx_msg void OnProcessingParcurgereSimpla();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnProcessingNewprocessing();
	afx_msg void OnProcessingAfisarebmpheader();
	afx_msg void OnProcessingGrayscale8bp();
	afx_msg void OnProcessingGray2();
	afx_msg void OnProcessingGray24bp();
	afx_msg void OnProcessingSort();
	afx_msg void OnProcessingBlackandwhite();
	afx_msg void OnProcessingHistogram();
	afx_msg void OnProcessingThreshold();
	afx_msg void OnProcessingDithering();
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnProcessingHorizontalprojection();
	afx_msg void OnProcessingVerticalProjection();
	afx_msg void OnProcessingAxisofelongation();
	afx_msg void OnProcessingLabeling();
	afx_msg void OnProcessingLabeling2();
	afx_msg void OnProcessingTracing();
	afx_msg void OnProcessingFilltrace();
};

/////////////////////////////////////////////////////////////////////////////
