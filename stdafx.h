// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#ifndef __STDAFX_H__
#define __STDAFX_H__

// Change these values to use different versions
#define WINVER		0x0400
#define _WIN32_IE	0x0400
#define _RICHEDIT_VER	0x0100

#ifdef WIN32_PLATFORM_PSPC
#include <aygshell.h>
#endif // WIN32_PLATFORM_PSPC

#include <atlbase.h>
#include <atlapp.h>

extern CAppModule _Module;

#include <atlwin.h>

#include <atlframe.h>
#include <atlctrls.h>
#ifndef _WIN32_WCE
#include <atlctrlw.h>
#endif // _WIN32_WCE
#ifndef _WIN32_WCE
#include <atlprint.h>
#endif // _WIN32_WCE
#include <atlcrack.h>
#include <atldlgs.h>
#include <atlscrl.h>
#include <atlmisc.h>

#endif // __STDAFX_H__
