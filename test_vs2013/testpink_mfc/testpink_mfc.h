
// testpink_mfc.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// Ctestpink_mfcApp: 
// �йش����ʵ�֣������ testpink_mfc.cpp
//

class Ctestpink_mfcApp : public CWinApp
{
public:
	Ctestpink_mfcApp();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern Ctestpink_mfcApp theApp;