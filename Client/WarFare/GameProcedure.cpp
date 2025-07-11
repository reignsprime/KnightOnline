﻿// GameProcedure.cpp: implementation of the CGameProcedure class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "GameProcedure.h"
#include "GameDef.h"
#include "GameEng.h"
#include "PacketDef.h"
#include "LocalInput.h"
#include "APISocket.h"
#include "N3FXMgr.h"
#include "PlayerMyself.h"

#include "GameProcLogIn.h"
#include "GameProcNationSelect.h"
#include "GameProcCharacterCreate.h"
#include "GameProcCharacterSelect.h"
#include "GameProcMain.h"
#include "GameProcOption.h"

#include "UILoading.h"
#include "UIMessageBox.h"
#include "UIMessageBoxManager.h"
#include "UIManager.h"
#include "UINotice.h"
#include "UIHelp.h"
#include "UIHotKeyDlg.h"
#include "UIChat.h"
#include "UIVarious.h"
#include "UIPartyOrForce.h"
#include "UIMessageWnd.h"
#include "UIEndingDisplay.h"
#include "MagicSkillMng.h"
#include "GameCursor.h"
#include "resource.h"

#include <N3Base/N3UIEdit.h>
#include <N3Base/N3SndObjStream.h>
#include <N3Base/N3FXBundle.h>

#include <N3Base/BitmapFile.h>

#include <JpegFile/JpegFile.h>

#include <shared/lzf.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CN3SndObjStream*	CGameProcedure::s_pSnd_BGM = nullptr;			// 메인 배경음악 포인터..
CLocalInput*		CGameProcedure::s_pLocalInput = nullptr;		// 마우스와 키보드 입력 객체 .. Direct Input 을 썼다.
CAPISocket*			CGameProcedure::s_pSocket = nullptr;			// 메인 소켓 객체
CAPISocket*			CGameProcedure::s_pSocketSub = nullptr;		// 서브 소켓 객체
CGameEng*			CGameProcedure::s_pEng = nullptr;				// 3D Wrapper Engine
CN3FXMgr*			CGameProcedure::s_pFX = nullptr;

CUIManager*			CGameProcedure::s_pUIMgr = nullptr;			// UI Manager
CUILoading*			CGameProcedure::s_pUILoading = nullptr;		// 로딩바..
CUIMessageBoxManager*	CGameProcedure::s_pMsgBoxMgr = nullptr;		// MessageBox Manager
//bool				CGameProcedure::s_bUseSpeedHack = false;

CGameProcedure*				CGameProcedure::s_pProcPrev = nullptr;
CGameProcedure*				CGameProcedure::s_pProcActive = nullptr;

CGameProcLogIn*				CGameProcedure::s_pProcLogIn = nullptr;
CGameProcNationSelect*		CGameProcedure::s_pProcNationSelect = nullptr;
CGameProcCharacterCreate*	CGameProcedure::s_pProcCharacterCreate = nullptr;
CGameProcCharacterSelect*	CGameProcedure::s_pProcCharacterSelect = nullptr;
CGameProcMain*				CGameProcedure::s_pProcMain = nullptr;
CGameProcOption*			CGameProcedure::s_pProcOption = nullptr;
CGameCursor*				CGameProcedure::s_pGameCursor = nullptr;

HCURSOR	CGameProcedure::s_hCursorNormal    = nullptr;
HCURSOR	CGameProcedure::s_hCursorNormal1   = nullptr;
HCURSOR CGameProcedure::s_hCursorClick     = nullptr;
HCURSOR CGameProcedure::s_hCursorClick1    = nullptr;
HCURSOR CGameProcedure::s_hCursorAttack    = nullptr;
HCURSOR CGameProcedure::s_hCursorPreRepair = nullptr;
HCURSOR CGameProcedure::s_hCursorNowRepair = nullptr;

e_LogInClassification CGameProcedure::s_eLogInClassification; // 접속한 서비스.. MGame, Daum, KnightOnLine ....
std::string	CGameProcedure::s_szAccount = ""; // 계정 문자열..
std::string	CGameProcedure::s_szPassWord = ""; // 계정 비번..
std::string	CGameProcedure::s_szServer = ""; // 서버 문자열..
bool CGameProcedure::m_bCursorLocked = false;
HCURSOR CGameProcedure::m_hPrevGameCursor = nullptr;
HWND CGameProcedure::s_hWndSubSocket = nullptr; // 서브 소켓용 윈도우 핸들..
int	CGameProcedure::s_iChrSelectIndex = 0;
bool CGameProcedure::s_bNeedReportConnectionClosed = false; // 서버접속이 끊어진걸 보고해야 하는지..
bool CGameProcedure::s_bWindowed = false; // 창모드 실행??
bool CGameProcedure::s_bKeyPress = false;	//키가 눌려졌을때 ui에서 해당하는 조작된적이 있다면
bool CGameProcedure::s_bKeyPressed = false;	//키가 올라갔을때 ui에서 해당하는 조작된적이 있다면

bool CGameProcedure::s_bIsRestarting = false;

// NOTE: adding boolean to check if window has focus or not
bool CGameProcedure::s_bIsWindowInFocus = true;

CGameProcedure::CGameProcedure()
{
	m_bCursorLocked = false;
}

CGameProcedure::~CGameProcedure()
{
	m_bCursorLocked = false;
}

void CGameProcedure::Release()
{
	s_pUIMgr->SetFocusedUI(NULL);
}

void CGameProcedure::Init()
{
	s_pUIMgr->SetFocusedUI(NULL);
}

void CGameProcedure::StaticMemberInit(
	HINSTANCE hInstance,
	HWND hWndMain)
{
	//////////////////////////////////////////////////////////////////////////////////////////
	// 게임 기본 3D 엔진 만들기..
	s_bWindowed = true;
	// #if _DEBUG 

	if (s_Options.bWindowMode) 
	{
		DEVMODE dm = {};
		EnumDisplaySettings(nullptr, ENUM_REGISTRY_SETTINGS, &dm);
		if (dm.dmBitsPerPel != (DWORD) s_Options.iViewColorDepth)
		{
			dm.dmSize = sizeof(DEVMODE);
			dm.dmPelsWidth = s_Options.iViewWidth;
			dm.dmPelsHeight = s_Options.iViewHeight;
			dm.dmBitsPerPel = s_Options.iViewColorDepth;
			dm.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
			::ChangeDisplaySettings(&dm, CDS_FULLSCREEN);
		}
	}
	else
	{
		DEVMODE dm = {};
		dm.dmSize = sizeof(DEVMODE);
		dm.dmPelsWidth = s_Options.iViewWidth;
		dm.dmPelsHeight = s_Options.iViewHeight;
		dm.dmBitsPerPel = s_Options.iViewColorDepth;
		dm.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
		::ChangeDisplaySettings(&dm, CDS_FULLSCREEN);
	}

	s_pEng = new CGameEng();
	if (!s_pEng->Init(
		s_bWindowed,
		hWndMain,
		s_Options.iViewWidth,
		s_Options.iViewHeight,
		s_Options.iViewColorDepth,
		TRUE))
		exit(-1);

	// 게임 기본 3D 엔진 만들기..
	::SetFocus(hWndMain); // Set focus this window..

	RECT rc;
	::GetClientRect(s_hWndBase, &rc);
	RECT rcTmp = rc; rcTmp.left = (rc.right - rc.left) / 2; rcTmp.bottom = rcTmp.top + 30;
	CN3UIEdit::CreateEditWindow(s_hWndBase, rcTmp);
	//////////////////////////////////////////////////////////////////////////////////////////

	//s_hWndSubSocket = hWndSub; // 서브 소켓용 윈도우 핸들..

	CGameBase::StaticMemberInit(); // Table 및 지형, 오브젝트, 캐릭터 초기화...

	//////////////////////////////////////////////////////////////////////////////////////////
	// Game Procedure 소켓과 로컬 인풋, 3D엔진, Resource Table 로딩 및 초기화...
	s_pSocket = new CAPISocket();
	s_pSocketSub = new CAPISocket();

	// 커서 만들기..
	s_hCursorNormal		= LoadCursor(hInstance, MAKEINTRESOURCE(IDC_CURSOR_NORMAL));
	s_hCursorNormal1	= LoadCursor(hInstance, MAKEINTRESOURCE(IDC_CURSOR_NORMAL1));
	s_hCursorClick		= LoadCursor(hInstance, MAKEINTRESOURCE(IDC_CURSOR_CLICK));
	s_hCursorClick1		= LoadCursor(hInstance, MAKEINTRESOURCE(IDC_CURSOR_CLICK1));
	s_hCursorAttack		= LoadCursor(hInstance, MAKEINTRESOURCE(IDC_CURSOR_ATTACK));
	s_hCursorPreRepair	= LoadCursor(hInstance, MAKEINTRESOURCE(IDC_CURSOR_PRE_REPAIR));
	s_hCursorNowRepair	= LoadCursor(hInstance, MAKEINTRESOURCE(IDC_CURSOR_NOW_REPAIR));

	if (!CN3Base::s_Options.bWindowCursor)
	{
		s_pGameCursor = new CGameCursor();
		s_pGameCursor->LoadFromFile("ui\\cursor.uif");
	}

	SetGameCursor(s_hCursorNormal);

	s_pLocalInput = new CLocalInput();
	s_pLocalInput->Init(hInstance, hWndMain); // Input 만 초기화.

	//////////////////////////////////////////////////////////////////////////////////////////
	// Sound 초기화..
	if(CN3Base::s_Options.bSndEnable)
	{
		CN3Base::s_SndMgr.Init(s_hWndBase);//pWindow);
		CN3Base::s_SndMgr.SetDuplicated(CN3Base::s_Options.bSndDuplicated);
	}
	CN3FXBundle::SetEffectSndDistance(float(CN3Base::s_Options.iEffectSndDist));

	s_pFX = new CN3FXMgr();

	__TABLE_UI_RESRC* pTblUI = s_pTbl_UI.Find(NATION_ELMORAD); // 기본은 엘모라드 UI 로 한다..
	if(pTblUI == NULL) {
		printf("ERROR: UI table is NULL.\n");
		system("pause");
		//Sleep(1000 * 5);
		exit(-1);
	}

	s_pUIMgr = new CUIManager(); // 기본 UIManager
	s_pMsgBoxMgr = new CUIMessageBoxManager(); //MessageBox Manager

	// 툴팁..
	CN3UIBase::EnableTooltip(pTblUI->szToolTip);

	//////////////////////////////////////////////////////////////////////////////////////////
	// 각 프로시저들 생성
	s_pProcLogIn			= new CGameProcLogIn();				// 로그인 프로시져
	s_pProcNationSelect		= new CGameProcNationSelect();		// 나라 선택
	s_pProcCharacterSelect	= new CGameProcCharacterSelect();	// 캐릭터 선택
	s_pProcCharacterCreate	= new CGameProcCharacterCreate();	// 캐릭터 만들기
	s_pProcMain				= new CGameProcMain();				// 메인 게임 프로시져
	s_pProcOption			= new CGameProcOption();			// 게임 옵션 프로시져
}

void CGameProcedure::StaticMemberRelease()
{
	delete s_pSocket; s_pSocket = NULL; // 통신 끊기..
	delete s_pSocketSub; s_pSocketSub = NULL; // 서브 소켓 없애기..
	delete s_pFX; s_pFX = NULL;

	////////////////////////////////////////////////////////////
	// 기본값 쓰기..
	if(s_pPlayer)
	{
		int iRun = s_pPlayer->IsRunning(); // 이동 모드가 뛰는 상태였으면 
		CGameProcedure::RegPutSetting("UserRun", &iRun, 4); // 걷기, 뛰기 상태 기록..
	}
	
	if(s_pEng)
	{
		e_ViewPoint eVP = s_pEng->ViewPoint();
		CGameProcedure::RegPutSetting("CameraMode", &eVP, 4); // 카메라 상태 기록
	}
	// 기본값 쓰기..
	////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////////
	// 엔딩화면 보이기..
	if(s_pPlayer)
	{
		e_Nation eNation = s_pPlayer->m_InfoBase.eNation;
		__TABLE_UI_RESRC* pTbl = s_pTbl_UI.Find(eNation);
		if(pTbl)
		{
			CUIEndingDisplay Credit; // 엔딩 표시하기..
			Credit.LoadFromFile(pTbl->szEndingDisplay);
			Credit.Render();
		}
	}
	// 엔딩화면 보이기..
	////////////////////////////////////////////////////////////////////////

	Sleep(1000);

	if (s_Options.bWindowMode)
	{
		DEVMODE dm = {};
		::EnumDisplaySettings(nullptr, ENUM_REGISTRY_SETTINGS, &dm);

		if (dm.dmBitsPerPel != (DWORD) s_Options.iViewColorDepth)
		{
			dm.dmSize = sizeof(DEVMODE);
			dm.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
			::ChangeDisplaySettings(&dm, CDS_FULLSCREEN);
		}
	}
	else
	{
		DEVMODE dm = {};
		::EnumDisplaySettings(nullptr, ENUM_REGISTRY_SETTINGS, &dm);

		if (dm.dmPelsWidth != (DWORD) s_Options.iViewWidth
			|| dm.dmPelsHeight != (DWORD) s_Options.iViewHeight
			|| dm.dmBitsPerPel != (DWORD) s_Options.iViewColorDepth)
		{
			dm.dmSize = sizeof(DEVMODE);
			dm.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
			::ChangeDisplaySettings(&dm, CDS_FULLSCREEN);
		}
	}

//	if ( (s_pProcMain) && (s_pProcMain->m_pUIHotKeyDlg) )
//			s_pProcMain->m_pUIHotKeyDlg->CloseIconRegistry();

	// UI 위치및 보이기 등의 정보 저장..
	if(s_pProcMain)
	{
		UIPostData_Write(UI_POST_WND_CHAT, s_pProcMain->m_pUIChatDlg);
		UIPostData_Write(UI_POST_WND_HOTKEY, s_pProcMain->m_pUIHotKeyDlg);
		UIPostData_Write(UI_POST_WND_HELP, s_pProcMain->m_pUIHelp);
		UIPostData_Write(UI_POST_WND_PARTY, s_pProcMain->m_pUIPartyOrForce);
		UIPostData_Write(UI_POST_WND_INFO, s_pProcMain->m_pUIMsgDlg);
	}

	// 각 프로시저들
	delete s_pProcLogIn; s_pProcLogIn = NULL; 						// 로그인 프로시져
	delete s_pProcNationSelect; s_pProcNationSelect = NULL; 		// 나라 선택
	delete s_pProcCharacterSelect; s_pProcCharacterSelect = NULL; 	// 캐릭터 선택
	delete s_pProcCharacterCreate; s_pProcCharacterCreate = NULL; 	// 캐릭터 만들기
	delete s_pProcMain; s_pProcMain = NULL; 						// 메인 게임 프로시져
	delete s_pProcOption; s_pProcOption = NULL; 					// 게임 옵션 프로시져

	// UI 들 날리기..
	if(s_pUILoading) delete s_pUILoading; s_pUILoading = NULL;		// Loading Bar
	delete s_pMsgBoxMgr;
	delete s_pUIMgr; s_pUIMgr = NULL;				// UI Manager

//	delete s_pIME; s_pIME = NULL;
	delete s_pLocalInput; s_pLocalInput = NULL;
	delete s_pEng; s_pEng = NULL; // 젤 마지막에 엔진 날리기.!!!!!

	if(s_pGameCursor) delete s_pGameCursor; s_pGameCursor = NULL;

	CGameBase::StaticMemberRelease();
}

void CGameProcedure::Tick()
{
	s_pLocalInput->Tick(); // 키보드와 마우스로부터 입력을 받는다.
	if(s_pGameCursor) s_pGameCursor->Tick();

	ProcessUIKeyInput();

	uint32_t dwMouseFlags = s_pLocalInput->MouseGetFlag();
	POINT ptPrev = s_pLocalInput->MouseGetPosOld();
	POINT ptCur = s_pLocalInput->MouseGetPos();

	e_Nation eNation = s_pPlayer->m_InfoBase.eNation;
	if(dwMouseFlags & MOUSE_LBCLICK) SetGameCursor(((NATION_ELMORAD == eNation) ? s_hCursorClick1 : s_hCursorClick));
	else if(dwMouseFlags & MOUSE_LBCLICKED) SetGameCursor(((NATION_ELMORAD == eNation) ? s_hCursorNormal1 : s_hCursorNormal));
	if(dwMouseFlags & MOUSE_RBCLICKED)
	{
		if(s_pPlayer->m_bAttackContinous && s_pProcActive == s_pProcMain) // 메인 프로시져 이면..
			SetGameCursor(s_hCursorAttack);
		else
			SetGameCursor(((NATION_ELMORAD == eNation) ? s_hCursorNormal1 : s_hCursorNormal));
	}

	uint32_t dwRet = 0;
	dwRet = s_pMsgBoxMgr->MouseProcAndTick(dwMouseFlags, s_pLocalInput->MouseGetPos(), s_pLocalInput->MouseGetPosOld());

	if(0 == dwRet)
	{
		dwRet = s_pUIMgr->MouseProc(dwMouseFlags, ptCur, ptPrev);
		s_pUIMgr->Tick();
	}

	// 몬가 하면... 
//	if((dwRet & UI_MOUSEPROC_CHILDDONESOMETHING) || (dwRet & UI_MOUSEPROC_DONESOMETHING))
//		s_pLocalInput->MouseRemoveFlag(0xffMOUSE_LBCLICK | MOUSE_LBCLICKED | MOUSE_LBDBLCLK);
	s_pUIMgr->m_bDoneSomething = false;		// UI 에서 조작을 했다...
	if(dwRet != UI_MOUSEPROC_NONE)
		s_pUIMgr->m_bDoneSomething = true;		// UI 에서 조작을 했다...

	CN3Base::s_SndMgr.Tick(); // Sound Engine...

	// 스크린 캡쳐 키..
	if(s_pLocalInput->IsKeyPress(DIK_NUMPADMINUS)) // 키패드의 마이너스 키를 누르면..
	{
		SYSTEMTIME st;
		::GetLocalTime(&st);
		char szFN[128] = "";
//		sprintf(szFN, "%d_%d_%d_%d.%d.%d.jpg", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
		sprintf(szFN, "%d_%d_%d_%d.%d.%d.ksc", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
		this->CaptureScreenAndSaveToFile(szFN);
	}

	//////////////////////////////////
	// Network Msg 처리하기
	while (!s_pSocket->m_qRecvPkt.empty())
	{
		auto pkt = s_pSocket->m_qRecvPkt.front();
		if (!ProcessPacket(*pkt))
			CLogWriter::Write("Invalid Packet... (%d)", pkt->GetOpcode());

		delete pkt;
		s_pSocket->m_qRecvPkt.pop();
	}

	while (!s_pSocketSub->m_qRecvPkt.empty())
	{
		auto pkt = s_pSocketSub->m_qRecvPkt.front();
		if (!ProcessPacket(*pkt))
			break;

		delete pkt;
		s_pSocketSub->m_qRecvPkt.pop();
	}
	// Network Msg 처리하기
	//////////////////////////////////
}

void CGameProcedure::Render()
{
	if(s_pUIMgr) s_pUIMgr->Render(); // UI 들 렌더링..

	s_pMsgBoxMgr->Render();
	if(s_pGameCursor) s_pGameCursor->Render();
}

void CGameProcedure::TickActive()
{
	if(s_pProcActive != s_pProcPrev) // 프로시저가 바뀌면..
	{
		if(s_pProcPrev) s_pProcPrev->Release();
		if(s_pProcActive) s_pProcActive->Init();

		s_pProcPrev = s_pProcActive;
	}

	if(s_pProcActive)	s_pProcActive->Tick();		// 현재 프로시저 Tick ................................
}

void CGameProcedure::RenderActive()
{
//	if(s_pProcActive != s_pProcPrev) // 프로시저가 바뀌면..
//	{
//		if(s_pProcPrev) s_pProcPrev->Release();
//		if(s_pProcActive) s_pProcActive->Init();
//
//		s_pProcPrev = s_pProcActive;
//	}
//	else 
//	{
		if(s_pProcActive == s_pProcPrev) s_pProcActive->Render();
//	}
}

bool CGameProcedure::CaptureScreenAndSaveToFile(const std::string& szFN)
{
	/*
	if(szFN.empty()) return false;
	CJpegFile file;

	RECT wndRect;
	GetWindowRect(CN3Base::s_hWndBase, &wndRect);
	
	HANDLE hDIB = file.CopyScreenToDIB(&wndRect);
	if(hDIB)
	{
		int nQuality = 90;
		char szBuf[256] = "";

		//운영자는 양질의 스크린 캡쳐를 할수 있게...
		if(s_pPlayer->m_InfoBase.iAuthority == AUTHORITY_MANAGER)
			nQuality = 100;

		if(file.EncryptJPEG(hDIB, nQuality, szFN, szBuf) == TRUE)
		{
			TRACE("Screen Captue %s\n", szFN.c_str());
		}
		GlobalFree(hDIB);
	}
	*/
	return true;
/*
	int iW = CN3Base::s_CameraData.vp.Width;
	int iH = CN3Base::s_CameraData.vp.Height;

	bool bResult = false;
	LPDIRECT3DSURFACE8 lpDDSTmp = NULL;
	LPDIRECT3DSURFACE8 lpDDSBack = NULL;
	CN3Base::s_lpD3DDev->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &lpDDSBack);
	if(lpDDSBack)
	{
		CN3Base::s_lpD3DDev->CreateImageSurface(iW, iH, D3DFMT_X8R8G8B8, &lpDDSTmp);
		if(lpDDSTmp)
		{
			HRESULT rval = D3DXLoadSurfaceFromSurface(lpDDSTmp, NULL, NULL, lpDDSBack, NULL, NULL, D3DX_FILTER_NONE, 0);
//			HRESULT rval = s_lpD3DDev->CopyRects(lpDDSBack, NULL, 0, lpDDSTmp, NULL);
//			char szErr[256];
//			::D3DXGetErrorString(rval, szErr, 256);

			if(D3D_OK == rval)
			{
				D3DLOCKED_RECT LR;
				if(D3D_OK == lpDDSTmp->LockRect(&LR, NULL, 0))
				{
//					std::vector<uint8_t> buff(iW * iH * 3, 0);
					CBitMapFile bmf;
					bmf.Create(iW, iH);

					for(int y = 0; y < iH; y++)
					{
						uint8_t* pPS = ((uint8_t*)LR.pBits) + LR.Pitch * y;
//						uint8_t* pPD = (uint8_t*)(&(buff[y * (iW * 3)]));
						uint8_t* pPD = (uint8_t*)(bmf.Pixels(0, y));

						for(int x = 0; x < iW; x++, pPS += 4, pPD +=3 )
						{
							pPD[0] = pPS[0];
							pPD[1] = pPS[1];
							pPD[2] = pPS[2];
						}
					}
					lpDDSTmp->UnlockRect();

//					CJpeg jpg;
//					jpg.SaveJPG(szFN.c_str(), iW, iH, &(buff[0]));
					bmf.SaveToFile(szFN.c_str());
				}
			}
			
			lpDDSTmp->Release();
			lpDDSTmp = NULL;
		}


		lpDDSBack->Release();
		lpDDSBack = NULL;
	}

	return bResult;
*/
}

void CGameProcedure::ProcActiveSet(CGameProcedure* pProc)
{
	if (pProc == nullptr
		|| s_pProcActive == pProc)
		return;

	if (s_pUIMgr != nullptr)
		s_pUIMgr->EnableOperationSet(true); // UI를 조작할수 있게 한다..

	CGameProcedure::MessageBoxClose(-1); // MessageBox 가 떠 있으면 감춘다.

	s_pProcPrev = s_pProcActive; // 전의 것 포인터 기억..
	s_pProcActive = pProc;
}

void CGameProcedure::ReConnect()
{
	s_bNeedReportConnectionClosed = false; // 서버접속이 끊어진걸 보고해야 하는지..
	CGameProcedure::s_pSocket->ReConnect();
	s_bNeedReportConnectionClosed = true; // 서버접속이 끊어진걸 보고해야 하는지..
}

std::string CGameProcedure::MessageBoxPost(const std::string& szMsg, const std::string& szTitle, int iStyle, e_Behavior eBehavior)
{
	return s_pMsgBoxMgr->MessageBoxPost(szMsg, szTitle, iStyle, eBehavior);
}

void CGameProcedure::MessageBoxClose(const std::string& szMsg)
{
	s_pMsgBoxMgr->MessageBoxClose(szMsg);
}

void CGameProcedure::MessageBoxClose(int iMsgBoxIndex)
{
	if( iMsgBoxIndex == -1 )
		s_pMsgBoxMgr->MessageBoxCloseAll();
}


bool CGameProcedure::RegPutSetting( const char *ValueName, void *pValueData, long length )
{
	HKEY	hKey;

	if ( RegOpenKey( HKEY_CURRENT_USER, GetStrRegKeySetting().c_str(), &hKey ) != ERROR_SUCCESS ) 
	{
		if ( RegCreateKey( HKEY_CURRENT_USER, GetStrRegKeySetting().c_str(), &hKey ) != ERROR_SUCCESS )
		{
			__ASSERT(0, "Registry Create Failed!!!");
			return false;
		}
		if(RegOpenKey( HKEY_CURRENT_USER, GetStrRegKeySetting().c_str(), &hKey ) != ERROR_SUCCESS ) 
		{
			__ASSERT(0, "Registry Open Failed!!!");
			return false;
		}
	}

	// set the value
	if ( RegSetValueEx( hKey, ValueName, 0, REG_BINARY, (const uint8_t *)pValueData, length ) != ERROR_SUCCESS )
	{
		__ASSERT(0, "Registry Write Failed!!!");
		RegCloseKey( hKey );
		return false;
	}

	if ( RegCloseKey( hKey ) != ERROR_SUCCESS ) 
	{
		__ASSERT(0, "Registry Close Failed!!!");
		return false;
	}

	return true;
}

bool CGameProcedure::RegGetSetting( const char *ValueName, void *pValueData, long length)
{
	HKEY	hKey;
	DWORD	Type;
	DWORD	len;

	len = length;

	if ( RegOpenKey( HKEY_CURRENT_USER, GetStrRegKeySetting().c_str(), &hKey ) != ERROR_SUCCESS ) 
	{
//		__ASSERT(0, "Registry Open Failed!!!");
		return false;
	}

	// get the value
	if ( RegQueryValueEx( hKey, ValueName, NULL, &Type, (uint8_t *)pValueData, &len ) != ERROR_SUCCESS )
	{
//		__ASSERT(0, "Registry Query Failed!!!");
		RegCloseKey( hKey );
		return false;
	}

	if ( RegCloseKey( hKey ) != ERROR_SUCCESS ) 
	{
//		__ASSERT(0, "Registry Close Failed!!!");
		return false;
	}

	return true;
}

void CGameProcedure::UIPostData_Write(const std::string& szKey, CN3UIBase* pUI)
{
	if(szKey.empty() || NULL == pUI) return;

	__WndInfo WI;
	lstrcpyn(WI.szName, szKey.c_str(), 16);
	WI.bVisible = pUI->IsVisible();
	WI.ptPosition = pUI->GetPos();

	RegPutSetting(WI.szName, &WI, sizeof(__WndInfo) );
}

void CGameProcedure::UIPostData_Read(const std::string& szKey, CN3UIBase* pUI, int iDefaultX, int	 iDefaultY)
{
	if(szKey.empty() || NULL == pUI) return;

	// 1. 디폴트 데이터를 만든다..
	// 2. 데이터를 읽어온다..
	// 3. 영역이 유효한지를 판단한다..

	__WndInfo WI;
	WI.ptPosition.x = iDefaultX;
	WI.ptPosition.y = iDefaultY;
	if(false == RegGetSetting(szKey.c_str(), &WI, sizeof(__WndInfo))) WI.bVisible = true; // 기본 데이터가 없으면 무조건 보이게 한다..

	RECT rc = pUI->GetRegion();

	if (WI.ptPosition.x < 0) WI.ptPosition.x = 0;
	if (WI.ptPosition.x + (rc.right - rc.left) > (int)s_CameraData.vp.Width)
		WI.ptPosition.x = s_CameraData.vp.Width - (rc.right - rc.left);
	if (WI.ptPosition.y < 0) WI.ptPosition.y = 0;
	if (WI.ptPosition.y + (rc.bottom - rc.top) >(int)s_CameraData.vp.Height)
		WI.ptPosition.y = s_CameraData.vp.Height - (rc.bottom - rc.top);

	pUI->SetVisible(WI.bVisible);
	if(0 == WI.ptPosition.x && 0 == WI.ptPosition.y)
		pUI->SetPos(iDefaultX, iDefaultY);
	else pUI->SetPos(WI.ptPosition.x, WI.ptPosition.y);
}

void CGameProcedure::SetGameCursor(HCURSOR hCursor, bool bLocked)
{
	if(s_pGameCursor)
	{
		e_Cursor eCursor = CURSOR_KA_NORMAL;

		if(hCursor == s_hCursorNormal)
			eCursor = CURSOR_KA_NORMAL;
		else if(hCursor == s_hCursorNormal1)
			eCursor = CURSOR_EL_NORMAL;
		else if(hCursor == s_hCursorClick)
			eCursor = CURSOR_KA_CLICK;
		else if(hCursor == s_hCursorClick1)
			eCursor = CURSOR_EL_CLICK;
		else if(hCursor == s_hCursorAttack)
			eCursor = CURSOR_ATTACK;
		else if(hCursor == s_hCursorPreRepair)
			eCursor = CURSOR_PRE_REPAIR;
		else if(hCursor == s_hCursorNowRepair)
			eCursor = CURSOR_NOW_REPAIR;
		else if(hCursor == NULL)
			eCursor = CURSOR_UNKNOWN;

		SetGameCursor(eCursor, bLocked);

		if ((!m_bCursorLocked) && bLocked)
		{
			m_bCursorLocked = true;
		}

	}
	else
	{
		if ((m_bCursorLocked) && (!bLocked) ) return;
		else if ( ((m_bCursorLocked) && bLocked) || ((!m_bCursorLocked) && !bLocked) )
		{
			SetCursor(hCursor);
			return;
		}
		else if ((!m_bCursorLocked) && bLocked)
		{
			m_hPrevGameCursor = GetCursor();
			m_bCursorLocked = true;
			SetCursor(hCursor);
		}
	}
}

void CGameProcedure::SetGameCursor(e_Cursor eCursor, bool bLocked)
{
	if(s_pGameCursor == NULL) return;
	s_pGameCursor->SetGameCursor(eCursor, bLocked);
}

void CGameProcedure::RestoreGameCursor()
{
	if(s_pGameCursor)
	{
		if (m_bCursorLocked) 
			m_bCursorLocked = false;

		if(s_pGameCursor) s_pGameCursor->RestoreGameCursor();
	}
	else
	{
		if (m_bCursorLocked) 
			m_bCursorLocked = false;

		SetCursor(m_hPrevGameCursor);	
	}
}

std::string CGameProcedure::GetStrRegKeySetting()
{
	char szBuff[256];
	sprintf(szBuff, "Software\\KnightOnline\\%s_%s_%d", s_szAccount.c_str(), s_szServer.c_str(), s_iChrSelectIndex);
	return szBuff;
}

bool CGameProcedure::ProcessPacket(Packet& pkt)
{
	int iCmd = pkt.read<uint8_t>();	// 커멘드 파싱..
	switch ( iCmd )										// 커멘드에 다라서 분기..
	{
		case WIZ_COMPRESS_PACKET:
			this->MsgRecv_CompressedPacket(pkt);
			return true;

		case WIZ_VERSION_CHECK: // 암호화도 같이 받는다..
			this->MsgRecv_VersionCheck(pkt); // virtual
			return true;

		case WIZ_LOGIN:
			this->MsgRecv_GameServerLogIn(pkt);
			return true;

		case WIZ_SERVER_CHANGE:				// 서버 바꾸기 메시지..
		{
			// 다른 존 서버로 다시 접속한다.
			int iLen = 0;
			std::string szName, szIP;
//			iLen = pkt.read<int16_t>(); // 서버 이름
//			pkt.readString(szName, iLen);
			iLen = pkt.read<int16_t>(); // 서버 IP
			pkt.readString(szIP, iLen);
			uint32_t dwPort = pkt.read<int16_t>();
			s_pPlayer->m_InfoExt.iZoneInit = pkt.read<uint8_t>();
			s_pPlayer->m_InfoExt.iZoneCur = pkt.read<uint8_t>();
			int iVictoryNation = pkt.read<uint8_t>();
			CGameProcedure::LoadingUIChange(iVictoryNation);

			s_bNeedReportConnectionClosed = false; // 서버접속이 끊어진걸 보고해야 하는지..
			s_pSocket->Disconnect(); // 끊고...
			Sleep(2000); // 2초 딜레이.. 서버가 처리할 시간을 준다.
			int iErr = s_pSocket->Connect(s_hWndBase, szIP.c_str(), dwPort);
			s_bNeedReportConnectionClosed = true; // 서버접속이 끊어진걸 보고해야 하는지..

			if(iErr) this->ReportServerConnectionFailed("Current Zone", iErr, true); // 서버 접속 오류.. Exit.
			else
			{
				// 버전체크를 보내면.. 응답으로 버전과 암호화 키가 온다.
				// 메인 프로시저의 경우 Character_Select 를 보내고 로그인일경우 GameServer_LogIn 을 보낸다.
				this->MsgSend_VersionCheck(); 
			}
		}
		return true;

		case WIZ_SEL_CHAR:
		{
			this->MsgRecv_CharacterSelect(pkt); // virtual
		}
		return true;
	}

	return false;
}

void CGameProcedure::ReportServerConnectionFailed(const std::string& szServerName, int iErrCode, bool bNeedQuitGame)
{
	std::string szMsg;
	GetTextF(IDS_FMT_CONNECT_ERROR, &szMsg, szServerName.c_str(), iErrCode);
	
	e_Behavior eBehavior = ((bNeedQuitGame) ? BEHAVIOR_EXIT : BEHAVIOR_NOTHING);
	MessageBoxPost(szMsg, "", MB_OK, eBehavior);
	return;
}

void CGameProcedure::ReportServerConnectionClosed(bool bNeedQuitGame)
{
	if (!s_bNeedReportConnectionClosed)
		return;

	std::string szMsg;
	GetText(IDS_CONNECTION_CLOSED, &szMsg);
	e_Behavior eBehavior = ((bNeedQuitGame) ? BEHAVIOR_EXIT : BEHAVIOR_NOTHING);
	MessageBoxPost(szMsg, "", MB_OK, eBehavior);

	if(s_pPlayer)
	{
		__Vector3 vPos = s_pPlayer->Position();
		CLogWriter::Write("Socket Closed... Zone(%d) Pos(%.1f, %.1f, %.1f) Exp(%I64u)",
			s_pPlayer->m_InfoExt.iZoneCur, vPos.x, vPos.y, vPos.z, s_pPlayer->m_InfoExt.iExp);
	}
	else
	{
		CLogWriter::Write("Socket Closed...");
	}

	if(s_pSocket) s_pSocket->Release();
}

void CGameProcedure::ReportDebugStringAndSendToServer(const std::string& szDebug)
{
	if(szDebug.empty()) return;

	CLogWriter::Write(szDebug.c_str());

	if(s_pSocket && s_pSocket->IsConnected())
	{
		int iLen = szDebug.size();
		std::vector<uint8_t> buffer;	// 버퍼.. 
		buffer.assign(iLen + 4, 0x00);
		int iOffset=0;												// 옵셋..
		s_pSocket->MP_AddByte(&(buffer[0]), iOffset, WIZ_DEBUG_STRING_PACKET);
		s_pSocket->MP_AddShort(&(buffer[0]), iOffset, iLen);
		s_pSocket->MP_AddString(&(buffer[0]), iOffset, szDebug);
		s_pSocket->Send(&(buffer[0]), iOffset);				// 보냄..
	}
}

void CGameProcedure::MsgSend_GameServerLogIn()
{
	uint8_t byBuff[128];										// 패킷 버퍼..
	int iOffset = 0;										// 버퍼의 오프셋..

	CAPISocket::MP_AddByte(byBuff, iOffset, WIZ_LOGIN);	// 커멘드.
	CAPISocket::MP_AddShort(byBuff, iOffset, (int16_t)s_szAccount.size());	// 아이디 길이..
	CAPISocket::MP_AddString(byBuff, iOffset, s_szAccount);			// 실제 아이디..
	CAPISocket::MP_AddShort(byBuff, iOffset, (int16_t)s_szPassWord.size());	// 패스워드 길이
	CAPISocket::MP_AddString(byBuff, iOffset, s_szPassWord);		// 실제 패스워드
		
	s_pSocket->Send(byBuff, iOffset);								// 보낸다
}

void CGameProcedure::MsgSend_VersionCheck() // virtual
{
	// Version Check
	int iOffset = 0;
	uint8_t byBuffs[4];
	CAPISocket::MP_AddByte(byBuffs, iOffset, WIZ_VERSION_CHECK);				// 커멘드.
	s_pSocket->Send(byBuffs, iOffset);	// 보낸다

#ifdef _CRYPTION
	s_pSocket->m_bEnableSend = FALSE; // 보내기 가능..?
#endif // #ifdef _CRYPTION
}

void CGameProcedure::MsgSend_CharacterSelect() // virtual
{
	uint8_t byBuff[64];
	int iOffset = 0;
	CAPISocket::MP_AddByte(byBuff, iOffset, WIZ_SEL_CHAR);				// 커멘드.
	CAPISocket::MP_AddShort(byBuff, iOffset, (int16_t)s_szAccount.size());				// 계정 길이..
	CAPISocket::MP_AddString(byBuff, iOffset, s_szAccount);						// 계정 문자열..
	CAPISocket::MP_AddShort(byBuff, iOffset, (int16_t)s_pPlayer->IDString().size());		// 캐릭 아이디 길이..
	CAPISocket::MP_AddString(byBuff, iOffset, s_pPlayer->IDString());			// 캐릭 아이디 문자열..
	CAPISocket::MP_AddByte(byBuff, iOffset, s_pPlayer->m_InfoExt.iZoneInit);	// 처음 접속인지 아닌지 0x01:처음 접속
	CAPISocket::MP_AddByte(byBuff, iOffset, s_pPlayer->m_InfoExt.iZoneCur);		// 캐릭터 선택창에서의 캐릭터 존 번호
	s_pSocket->Send(byBuff, iOffset);	// 보낸다

	CLogWriter::Write("MsgSend_CharacterSelect - name(%s) zone(%d)",
		s_pPlayer->IDString().c_str(), s_pPlayer->m_InfoExt.iZoneCur); // 디버깅 로그..
}

void CGameProcedure::MsgRecv_CompressedPacket(Packet& pkt) // 압축된 데이터 이다... 한번 더 파싱해야 한다!!!
{
	uint16_t compressedLength	= pkt.read<uint16_t>();
	uint16_t originalLength		= pkt.read<uint16_t>();
	uint32_t originalChecksum	= pkt.read<uint32_t>();

	std::vector<uint8_t> decompressedBuffer(originalLength);

	uint32_t decompressedLength = lzf_decompress(
		pkt.contents() + pkt.rpos(),
		compressedLength,
		&decompressedBuffer[0],
		originalLength);

	_ASSERT(decompressedLength == originalLength);

	if (decompressedLength != originalLength)
		return;

	// Don't bother to verify checksums in release.
	// It's just unnecessarily slow.
#if defined(_DEBUG)
	if (originalChecksum != 0)
	{
		uint32_t actualChecksum = crc32(&decompressedBuffer[0], decompressedLength);
		_ASSERT(actualChecksum == originalChecksum);

		if (actualChecksum != originalChecksum)
			return;
	}
#endif

	Packet decompressedPkt;
	decompressedPkt.append(&decompressedBuffer[0], originalLength);

	ProcessPacket(decompressedPkt);
}

int CGameProcedure::MsgRecv_VersionCheck(Packet& pkt) // virtual
{
	int iVersion = pkt.read<int16_t>();	// 버전
#ifdef _CRYPTION
	uint64_t iPublicKey = pkt.read<uint64_t>(); // 암호화 공개키
	CAPISocket::InitCrypt(iPublicKey);
	s_pSocket->m_bEnableSend = TRUE; // 보내기 가능..?
#endif // #ifdef _CRYPTION

	if (iVersion != CURRENT_VERSION)
	{
		std::string szMsg;

		int iLangID = ::GetUserDefaultLangID();

		// Taiwan Language
		if (0x0404 == iLangID)
		{
			GetText(IDS_VERSION_CONFIRM_TW, &szMsg);
		}
		else
		{
			GetTextF(
				IDS_VERSION_CONFIRM,
				&szMsg,
				CURRENT_VERSION / 1000.0f,
				iVersion / 1000.0f);
		}

		MessageBoxPost(szMsg, "", MB_OK, BEHAVIOR_EXIT);
	}

	return iVersion;
}

int CGameProcedure::MsgRecv_GameServerLogIn(Packet& pkt) // virtual
{
	int iNation = pkt.read<uint8_t>(); // 국가 - 0 없음 0xff - 실패..
	return iNation;
}

bool CGameProcedure::MsgRecv_CharacterSelect(Packet& pkt) // virtual
{
	int iResult = pkt.read<uint8_t>(); // 0x00 실패
	if(1 == iResult) // 성공..
	{
		int iZoneCur = pkt.read<uint8_t>();
		float fX = (pkt.read<uint16_t>())/10.0f;
		float fZ = (pkt.read<uint16_t>())/10.0f;
		float fY = (pkt.read<int16_t>())/10.0f;

		int iVictoryNation = pkt.read<uint8_t>();
		CGameProcedure::LoadingUIChange(iVictoryNation);

		int iZonePrev;
		if(N3FORMAT_VER_DEFAULT & N3FORMAT_VER_1264) {
			iZonePrev = s_pPlayer->m_InfoExt.iZoneCur = 10*iZoneCur;
		} else {
			iZonePrev = s_pPlayer->m_InfoExt.iZoneCur = iZoneCur;
		}
		s_pPlayer->PositionSet(__Vector3(fX, fY, fZ), true);

		CLogWriter::Write("MsgRecv_CharacterSelect - name(%s) zone(%d -> %d)", s_pPlayer->m_InfoBase.szID.c_str(), iZonePrev, iZoneCur);
		return true;
	}
	else // 실패
	{
		CLogWriter::Write("MsgRecv_CharacterSelect - failed(%d)", iResult);
		return false;
	}

	if(iResult) return true;
	else return false;
}

void CGameProcedure::ProcessUIKeyInput(bool bEnable)
{
	s_bKeyPressed	= false;	//키가 올라갔을때 ui에서 해당하는 조작된적이 있다면

	if(!bEnable)
	{
		if(s_bKeyPress)
		{
			for(int i = 0; i < NUMDIKEYS; i++)
			{
				if(s_pLocalInput->IsKeyPressed(i))
				{
					if(!s_bKeyPressed) s_bKeyPress = false;
					break;
				}
			}
		}
		return;
	}


	CN3UIBase* pMsgBox = s_pMsgBoxMgr->GetFocusMsgBox();
	CN3UIBase* pUIFocus = s_pUIMgr->GetFocusedUI();

	if(pMsgBox && pMsgBox->IsVisible())//this_ui
	{
		for(int i = 0; i < NUMDIKEYS; i++)
		{
			if(s_pLocalInput->IsKeyPress(i))
				s_bKeyPress |= pMsgBox->OnKeyPress(i);
			if(s_pLocalInput->IsKeyPressed(i))
				s_bKeyPressed |= pMsgBox->OnKeyPressed(i);
		}
	}
	else if(pUIFocus && pUIFocus->IsVisible()) // 포커싱 된 UI 가 있으면...
	{
		for(int i = 0; i < NUMDIKEYS; i++)
		{
			if(s_pLocalInput->IsKeyPress(i))
			{
				if(pUIFocus->m_pChildUI && pUIFocus->m_pChildUI->IsVisible())
					s_bKeyPress |= pUIFocus->m_pChildUI->OnKeyPress(i);
				else
					s_bKeyPress |= pUIFocus->OnKeyPress(i);
			}
			if(s_pLocalInput->IsKeyPressed(i))
			{
				if(pUIFocus->m_pChildUI && pUIFocus->m_pChildUI->IsVisible())
					s_bKeyPressed |= pUIFocus->m_pChildUI->OnKeyPressed(i);
				else
					s_bKeyPressed |= pUIFocus->OnKeyPressed(i);
			}
		}
	}

	if(s_bKeyPress)
	{
		for(int i = 0; i < NUMDIKEYS; i++)
		{
			if(s_pLocalInput->IsKeyPressed(i))
			{
				if(!s_bKeyPressed) s_bKeyPress = false;
				break;
			}
		}
	}
}

bool CGameProcedure::IsUIKeyOperated()
{
	if (!s_bKeyPress && !s_bKeyPressed)
		return false;

	return true;
}

void CGameProcedure::LoadingUIChange(int iVictoryNation)
{
	if(s_pPlayer->m_InfoExt.iVictoryNation == iVictoryNation)
		return;

	s_pPlayer->m_InfoExt.iVictoryNation = iVictoryNation;

	std::string szLoading;
	if(s_pUILoading) delete s_pUILoading; s_pUILoading = NULL;		// Loading Bar

	s_pUILoading = new CUILoading();
	__ASSERT(s_pUILoading, "로딩화면 생성 실패");
	if(s_pUILoading == NULL) return;

	__TABLE_UI_RESRC* pTblUI = s_pTbl_UI.Find(NATION_ELMORAD); // 기본은 엘모라드 UI 로 한다..
	__ASSERT(pTblUI, "기본 UI 가 없습니다.");
	if(pTblUI == NULL) return;

	switch(iVictoryNation)
	{
	case VICTORY_ABSENCE:
		szLoading = pTblUI->szLoading;
		break;
	case VICTORY_ELMORAD:
		szLoading = pTblUI->szElLoading;
		break;
	case VICTORY_KARUS:
		szLoading = pTblUI->szKaLoading;
		break;
	default:
		szLoading = pTblUI->szLoading;
		break;
	}

	//TRACE("Loading UIF : %s\n", szLoading.c_str());
	s_pUILoading->LoadFromFile(szLoading); // 기본적인 로딩 바 만들기..
}
