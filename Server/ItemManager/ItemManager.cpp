﻿// ItemManager.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "ItemManager.h"
#include "ItemManagerDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CItemManagerApp

BEGIN_MESSAGE_MAP(CItemManagerApp, CWinApp)
	//{{AFX_MSG_MAP(CItemManagerApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CItemManagerApp construction

CItemManagerApp::CItemManagerApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CItemManagerApp object

CItemManagerApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CItemManagerApp initialization

BOOL CItemManagerApp::InitInstance()
{
	AfxEnableControlContainer();

	CItemManagerDlg dlg;
	m_pMainWnd = &dlg;
	int nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with Cancel
	}

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}
