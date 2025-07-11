﻿// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__BAE49B19_5BD7_4BEB_9612_9FF91395CB3D__INCLUDED_)
#define AFX_STDAFX_H__BAE49B19_5BD7_4BEB_9612_9FF91395CB3D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define _WIN32_WINNT _WIN32_WINNT_MAXVER
#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers
#define NOMINMAX 1
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxdisp.h>        // MFC Automation classes
#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <winsock2.h>
#include <afxsock.h>		// MFC socket extensions
#include <afxtempl.h>
#include <afxdb.h>

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__BAE49B19_5BD7_4BEB_9612_9FF91395CB3D__INCLUDED_)
