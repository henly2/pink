
// testpink_mfc2.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// Ctestpink_mfc2App: 
// �йش����ʵ�֣������ testpink_mfc2.cpp
//

class Ctestpink_mfc2App : public CWinApp
{
public:
	Ctestpink_mfc2App();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern Ctestpink_mfc2App theApp;