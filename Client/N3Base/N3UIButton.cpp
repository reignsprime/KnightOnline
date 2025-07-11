﻿// N3UIButton.cpp: implementation of the CN3UIButton class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfxBase.h"
#include "N3UIButton.h"
#include "N3UIImage.h"

#include "N3SndMgr.h"
#include "N3SndObj.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CN3UIButton::CN3UIButton()
{
	m_eType = UI_TYPE_BUTTON;

	m_dwStyle = UISTYLE_BTN_NORMAL;
	m_eState = UI_STATE_BUTTON_NORMAL;
	ZeroMemory(m_ImageRef, sizeof(CN3UIImage*)*NUM_BTN_STATE);
	ZeroMemory(&m_rcClick, sizeof(m_rcClick));
	m_pSnd_On = NULL;
	m_pSnd_Click = NULL;
}

CN3UIButton::~CN3UIButton()
{
	CN3Base::s_SndMgr.ReleaseObj(&m_pSnd_On);
	CN3Base::s_SndMgr.ReleaseObj(&m_pSnd_Click);
}

void CN3UIButton::Release()
{
	CN3UIBase::Release();

	m_dwStyle = UISTYLE_BTN_NORMAL;
	m_eState = UI_STATE_BUTTON_NORMAL;
	ZeroMemory(m_ImageRef, sizeof(CN3UIImage*)*NUM_BTN_STATE);
	ZeroMemory(&m_rcClick, sizeof(m_rcClick));

	CN3Base::s_SndMgr.ReleaseObj(&m_pSnd_On);
	CN3Base::s_SndMgr.ReleaseObj(&m_pSnd_Click);
}

void CN3UIButton::SetRegion(const RECT& Rect)
{
	CN3UIBase::SetRegion(Rect);
	SetClickRect(Rect);
	for(UIListItor itor = m_Children.begin(); m_Children.end() != itor; ++itor)
	{
		(*itor)->SetRegion(Rect);
	}
}

BOOL CN3UIButton::MoveOffset(int iOffsetX, int iOffsetY)
{
	if (FALSE == CN3UIBase::MoveOffset(iOffsetX, iOffsetY)) return FALSE;
	// click 영역
	m_rcClick.left += iOffsetX;		m_rcClick.top += iOffsetY;
	m_rcClick.right += iOffsetX;	m_rcClick.bottom += iOffsetY;	
	return TRUE;
}

void CN3UIButton::Render()
{
 	if(!m_bVisible) return;

	switch(m_eState)
	{
	case UI_STATE_BUTTON_NORMAL:
		{
			if (m_ImageRef[BS_NORMAL]) m_ImageRef[BS_NORMAL]->Render();
		}
		break;
	case UI_STATE_BUTTON_DOWN:
	case UI_STATE_BUTTON_DOWN_2CHECKDOWN:
	case UI_STATE_BUTTON_DOWN_2CHECKUP:
		{
			if (m_ImageRef[BS_DOWN]) m_ImageRef[BS_DOWN]->Render();
		}
		break;
	case UI_STATE_BUTTON_ON:
		{
			if (m_ImageRef[BS_ON]) m_ImageRef[BS_ON]->Render();
		}
		break;
	case UI_STATE_BUTTON_DISABLE:
		{
			if (m_ImageRef[BS_DISABLE]) m_ImageRef[BS_DISABLE]->Render();
		}
	}

	int i = 0;
	for(UIListReverseItor itor = m_Children.rbegin(); m_Children.rend() != itor; ++itor)
	{
		CN3UIBase* pChild = (*itor);
		for(i = 0; i < NUM_BTN_STATE; i++) // 버튼의 구성 요소가 아닌지 보고..
			if(pChild == m_ImageRef[i]) break;
		if(i >= NUM_BTN_STATE) pChild->Render(); // 버튼 차일드가 아니면 렌더링..
	}
}

uint32_t CN3UIButton::MouseProc(uint32_t dwFlags, const POINT& ptCur, const POINT& ptOld)
{
	uint32_t dwRet = UI_MOUSEPROC_NONE;
	if (!m_bVisible) return dwRet;

#ifndef _REPENT
#ifdef _N3GAME
	if (s_bWaitFromServer)
		return dwRet;
#endif
#endif

	if(false == IsIn(ptCur.x, ptCur.y))	// 영역 밖이면
	{
		if (false == IsIn(ptOld.x, ptOld.y))	return dwRet; // 이전 pointer도 영역 밖이었으면 그냥 리턴
		dwRet |= UI_MOUSEPROC_PREVINREGION;	// 이전 마우스 좌표는 영역 안이었다.

		if (UI_STATE_BUTTON_DISABLE == m_eState) return dwRet;	// disable이면 그냥 리턴

		if(UISTYLE_BTN_NORMAL & m_dwStyle) // normal 버튼 이면
		{
			SetState(UI_STATE_BUTTON_NORMAL);	// normal 상태로
		}
		else if (UISTYLE_BTN_CHECK & m_dwStyle) // check 버튼 이면
		{
			if (UI_STATE_BUTTON_DOWN_2CHECKUP == m_eState)	// up시키려다 만 경우 
				SetState(UI_STATE_BUTTON_DOWN);	// down 상태로
			else if (UI_STATE_BUTTON_DOWN_2CHECKDOWN == m_eState ||	// down시키려다 만 경우 또는
				UI_STATE_BUTTON_ON == m_eState)	// on 상태일 경우
				SetState(UI_STATE_BUTTON_NORMAL);	// normal 상태로
		}
		return dwRet; // 영역 밖이므로 더이상 처리 하지 않는다.
	}
	dwRet |= UI_MOUSEPROC_INREGION;	// 이번 마우스 좌표는 영역 안이다

	if (UI_STATE_BUTTON_DISABLE == m_eState) return dwRet;	// disable이면 그냥 리턴

	// 클릭 영역 밖이면
	if (FALSE == PtInRect(&m_rcClick, ptCur))
	{
		if(UISTYLE_BTN_NORMAL & m_dwStyle) // normal 버튼 이면
		{
			SetState(UI_STATE_BUTTON_NORMAL);	// normal 상태로
		}
		else if (UISTYLE_BTN_CHECK & m_dwStyle) // check 버튼 이면
		{
			if (UI_STATE_BUTTON_DOWN_2CHECKUP == m_eState)	// up시키려다 만 경우 
				SetState(UI_STATE_BUTTON_DOWN);	// down 상태로
			else if (UI_STATE_BUTTON_DOWN_2CHECKDOWN == m_eState ||	// down시키려다 만 경우 또는
				UI_STATE_BUTTON_ON == m_eState)	// on 상태일 경우
				SetState(UI_STATE_BUTTON_NORMAL);	// normal 상태로
		}
		return dwRet;
	}

	// 아래는 클릭 영역 안일때..
	// normal 버튼 이면
	if(UISTYLE_BTN_NORMAL & m_dwStyle)
	{
		if(dwFlags & UI_MOUSE_LBCLICK)  // 왼쪽버튼 눌르는 순간
		{
			SetState(UI_STATE_BUTTON_DOWN); // 누른 상태로 만들고..
			if (m_pSnd_Click) m_pSnd_Click->Play();	// 사운드가 있으면 play 하기
			dwRet |= UI_MOUSEPROC_DONESOMETHING;
			return dwRet;
		}
		else if(dwFlags & UI_MOUSE_LBCLICKED) // 왼쪽버튼을 떼는 순간
		{
			if(m_pParent && UI_STATE_BUTTON_DOWN == m_eState) // 이전 상태가 버튼을 Down 상태이면
			{
				SetState(UI_STATE_BUTTON_ON); // 버튼을 On 상태로 만든다..
				m_pParent->ReceiveMessage(this, UIMSG_BUTTON_CLICK); // 부모에게 버튼 클릭 통지..
			}
			dwRet |= UI_MOUSEPROC_DONESOMETHING;
			return dwRet;
		}
		else if (UI_STATE_BUTTON_NORMAL == m_eState)	// normal상태이면 on상태로..
		{
			SetState(UI_STATE_BUTTON_ON); // On 상태로 만들고..
			if (m_pSnd_On) m_pSnd_On->Play();	// 사운드가 있으면 play 하기
			dwRet |= CN3UIBase::MouseProc(dwFlags, ptCur, ptOld);
			return dwRet;
			// UI_MOUSEPROC_DONESOMETHING를 넣으면 안된다.(마우스 포인터가 버튼에서 다른 버튼으로 빠르게 옮겨갈때 
			// 이전 버튼의 상태가 이상해지는 것을 방지하기 위해)
		}
	}

	// 체크 버튼이면
	else if(UISTYLE_BTN_CHECK & m_dwStyle) 
	{
		if(dwFlags & UI_MOUSE_LBCLICK)  // 왼쪽버튼 눌르는 순간
		{
			if (UI_STATE_BUTTON_NORMAL == m_eState || UI_STATE_BUTTON_ON == m_eState)
			{
				SetState(UI_STATE_BUTTON_DOWN_2CHECKDOWN); // 임시로 누른 상태(DOWN_2CHECKDOWN)로 만들고..
				if (m_pSnd_Click) m_pSnd_Click->Play();	// 사운드가 있으면 play 하기
				dwRet |= UI_MOUSEPROC_DONESOMETHING;
				return dwRet;
			}
			else if(UI_STATE_BUTTON_DOWN == m_eState)
			{
				SetState(UI_STATE_BUTTON_DOWN_2CHECKUP); // 임시로 누른 상태(DOWN_2CHECKUP)로 만들고..
				if (m_pSnd_Click) m_pSnd_Click->Play();	// 사운드가 있으면 play 하기
				dwRet |= UI_MOUSEPROC_DONESOMETHING;
				return dwRet;
			}
		}
		else if(dwFlags & UI_MOUSE_LBCLICKED)  // 왼쪽버튼 떼는 순간
		{
			if(UI_STATE_BUTTON_DOWN_2CHECKDOWN == m_eState) // 이전 상태가 2CHECKDOWN 상태이면
			{
				SetState(UI_STATE_BUTTON_DOWN); // down 상태로 만들기
				if (m_pParent) m_pParent->ReceiveMessage(this, UIMSG_BUTTON_CLICK); // 부모에게 버튼 클릭 통지..
				dwRet |= UI_MOUSEPROC_DONESOMETHING;
				return dwRet;
			}
			else if (UI_STATE_BUTTON_DOWN_2CHECKUP == m_eState) // 전의 상태가 2CHECKUP 상태이면
			{
				SetState(UI_STATE_BUTTON_ON); // On 상태로 만들기
				if (m_pParent) m_pParent->ReceiveMessage(this, UIMSG_BUTTON_CLICK); // 부모에게 버튼 클릭 통지..
				dwRet |= UI_MOUSEPROC_DONESOMETHING;
				return dwRet;
			}
		}
		else if (UI_STATE_BUTTON_NORMAL == m_eState)	// normal상태이면 on상태로..
		{
			SetState(UI_STATE_BUTTON_ON); // On 상태로 만들고..
			if (m_pSnd_On) m_pSnd_On->Play();	// 사운드가 있으면 play 하기
			dwRet |= CN3UIBase::MouseProc(dwFlags, ptCur, ptOld);
			return dwRet;
			// UI_MOUSEPROC_DONESOMETHING를 넣으면 안된다.(마우스 포인터가 버튼에서 다른 버튼으로 빠르게 옮겨갈때 
			// 이전 버튼의 상태가 이상해지는 것을 방지하기 위해)
		}

	}
	dwRet |= CN3UIBase::MouseProc(dwFlags, ptCur, ptOld);
	return dwRet;
}

bool CN3UIButton::Load(HANDLE hFile)
{
	if (false == CN3UIBase::Load(hFile)) return false;

	DWORD dwNum;
	ReadFile(hFile, &m_rcClick, sizeof(m_rcClick), &dwNum, NULL);	// click 영역

	// m_ImageRef 설정하기
	for(UIListItor itor = m_Children.begin(); m_Children.end() != itor; ++itor)
	{
		CN3UIBase* pChild = (*itor);
		if (UI_TYPE_IMAGE != pChild->UIType()) continue;	// image만 골라내기
		int iBtnState = (int)(pChild->GetReserved());
		if (iBtnState<NUM_BTN_STATE)
		{
			m_ImageRef[iBtnState] = (CN3UIImage*)pChild;
		}
	}

	// 이전 uif파일을 컨버팅 하려면 사운드 로드 하는 부분 막기
	int iSndFNLen = 0;
	ReadFile(hFile, &iSndFNLen, sizeof(iSndFNLen), &dwNum, NULL);		//	사운드 파일 문자열 길이
	if (iSndFNLen>0)
	{
		std::vector<char> buffer(iSndFNLen+1, NULL);
		ReadFile(hFile, &buffer[0], iSndFNLen, &dwNum, NULL);

		__ASSERT(NULL == m_pSnd_On, "memory leak");
		m_pSnd_On = s_SndMgr.CreateObj(&buffer[0], SNDTYPE_2D);
	}

	ReadFile(hFile, &iSndFNLen, sizeof(iSndFNLen), &dwNum, NULL);		//	사운드 파일 문자열 길이
	if (iSndFNLen>0)
	{
		std::vector<char> buffer(iSndFNLen+1, NULL);
		ReadFile(hFile, &buffer[0], iSndFNLen, &dwNum, NULL);

		__ASSERT(NULL == m_pSnd_Click, "memory leak");
		m_pSnd_Click = s_SndMgr.CreateObj(&buffer[0], SNDTYPE_2D);
	}

	return true;
}

void CN3UIButton::operator = (const CN3UIButton& other)
{
	CN3UIBase::operator = (other);

	m_rcClick = other.m_rcClick;			// 클릭 영역
	SetSndOn(other.GetSndFName_On());		// 사운드
	SetSndClick(other.GetSndFName_Click());	// 사운드

	// m_ImageRef 설정하기
	for(UIListItor itor = m_Children.begin(); m_Children.end() != itor; ++itor)
	{
		CN3UIBase* pChild = (*itor);
		if (UI_TYPE_IMAGE != pChild->UIType()) continue;	// image만 골라내기
		int iBtnState = (int)(pChild->GetReserved());
		if (iBtnState<NUM_BTN_STATE)
		{
			m_ImageRef[iBtnState] = (CN3UIImage*)pChild;
		}
	}
}

#ifdef _N3TOOL
bool CN3UIButton::Save(HANDLE hFile)
{
	if (false == CN3UIBase::Save(hFile)) return false;
	DWORD dwNum;
	WriteFile(hFile, &m_rcClick, sizeof(m_rcClick), &dwNum, NULL);	// click 영역

	int iSndFNLen = 0;
	if (m_pSnd_On) iSndFNLen = m_pSnd_On->m_szFileName.size();
	WriteFile(hFile, &iSndFNLen, sizeof(iSndFNLen), &dwNum, NULL);		//	사운드 파일 문자열 길이
	if (iSndFNLen>0) WriteFile(hFile, m_pSnd_On->m_szFileName.c_str(), iSndFNLen, &dwNum, NULL);

	iSndFNLen = 0;
	if (m_pSnd_Click) iSndFNLen = m_pSnd_Click->m_szFileName.size();
	WriteFile(hFile, &iSndFNLen, sizeof(iSndFNLen), &dwNum, NULL);		//	사운드 파일 문자열 길이
	if (iSndFNLen>0) WriteFile(hFile, m_pSnd_Click->m_szFileName.c_str(), iSndFNLen, &dwNum, NULL);

	return true;
}

// 툴에서 사용하기 위한 함수 : n3uiImage를 생성한다.
void CN3UIButton::CreateImages()
{
	int i;
	for (i=0; i<NUM_BTN_STATE; ++i)
	{
		__ASSERT(NULL == m_ImageRef[i],"이미지가 이미 할당되어 있어여");
		m_ImageRef[i] = new CN3UIImage();
		m_ImageRef[i]->Init(this);
		m_ImageRef[i]->SetRegion(m_rcRegion);

		m_ImageRef[i]->SetReserved(i);		// 상태 번호(eBTN_STATE) 할당.
	}
}
#endif

void CN3UIButton::SetSndOn(const std::string& strFileName)
{
	CN3Base::s_SndMgr.ReleaseObj(&m_pSnd_On);
	if (0 == strFileName.size()) return;

	CN3BaseFileAccess tmpBase;
	tmpBase.FileNameSet(strFileName);	// Base경로에 대해서 상대적 경로를 넘겨준다.

	SetCurrentDirectory(tmpBase.PathGet().c_str());
	m_pSnd_On = s_SndMgr.CreateObj(tmpBase.FileName(), SNDTYPE_2D);
}

void CN3UIButton::SetSndClick(const std::string& strFileName)
{
	CN3Base::s_SndMgr.ReleaseObj(&m_pSnd_Click);
	if (0 == strFileName.size()) return;

	CN3BaseFileAccess tmpBase;
	tmpBase.FileNameSet(strFileName);	// Base경로에 대해서 상대적 경로를 넘겨준다.

	SetCurrentDirectory(tmpBase.PathGet().c_str());
	m_pSnd_Click = s_SndMgr.CreateObj(tmpBase.FileName(), SNDTYPE_2D);
}

std::string CN3UIButton::GetSndFName_On() const
{
	if (m_pSnd_On) return m_pSnd_On->m_szFileName;
	else return std::string("");
}

std::string CN3UIButton::GetSndFName_Click() const
{
	if (m_pSnd_Click) return m_pSnd_Click->m_szFileName;
	else return std::string("");
}

