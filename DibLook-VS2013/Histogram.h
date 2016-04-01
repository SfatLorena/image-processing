#pragma once


// CHistogram

class CHistogram : public CStatic
{
	DECLARE_DYNAMIC(CHistogram)

public:
	CHistogram();
	virtual ~CHistogram();

protected:
	DECLARE_MESSAGE_MAP()
public:
	int values[256];	afx_msg void OnPaint();
};


