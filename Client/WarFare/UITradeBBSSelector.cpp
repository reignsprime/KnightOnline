﻿// UITradeBBSSelector.cpp: implementation of the CUITradeBBSSelector class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "UITradeBBSSelector.h"
#include "PacketDef.h"
#include "APISocket.h"
#include "GameProcMain.h"
#include "UITradeSellBBS.h"
#include "LocalInput.h"
#include "UIManager.h"

#include <N3Base/N3UIButton.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CUITradeBBSSelector::CUITradeBBSSelector()
{
	m_pBtn_BBSSell		= NULL;
	m_pBtn_BBSBuy		= NULL;
	m_pBtn_BBSCancel	= NULL;
}

CUITradeBBSSelector::~CUITradeBBSSelector()
{

}

bool CUITradeBBSSelector::ReceiveMessage(CN3UIBase *pSender, uint32_t dwMsg)
{
	if (dwMsg == UIMSG_BUTTON_CLICK)
	{
		if(pSender == m_pBtn_BBSSell)
		{
			MsgSend_OpenTradeSellBBS();
			SetVisible(false);
			return true;
		}
		else if(pSender == m_pBtn_BBSBuy)
		{
			MsgSend_OpenTradeBuyBBS();
			SetVisible(false);
			return true;
		}
		else if(pSender == m_pBtn_BBSCancel)
		{
			SetVisible(false);
			return true;
		}
	}
	return true;
}

bool CUITradeBBSSelector::Load(HANDLE hFile)
{
	if(CN3UIBase::Load(hFile)==false) return false;

	m_pBtn_BBSSell	= (CN3UIButton*)GetChildByID("btn_sell");		__ASSERT(m_pBtn_BBSSell,	"NULL UI Component!!");;
	m_pBtn_BBSBuy	= (CN3UIButton*)GetChildByID("btn_buy");		__ASSERT(m_pBtn_BBSBuy,	"NULL UI Component!!");;
	m_pBtn_BBSCancel = (CN3UIButton*)GetChildByID("btn_cancel");	__ASSERT(m_pBtn_BBSCancel,	"NULL UI Component!!");;

	return true;
}

void CUITradeBBSSelector::MsgSend_OpenTradeSellBBS()
{
	uint8_t byBuff[5];		// 패킷 버퍼..
	int iOffset=0;		// 패킷 오프셋..

	CAPISocket::MP_AddByte(byBuff, iOffset, WIZ_MARKET_BBS);		
	CAPISocket::MP_AddByte(byBuff, iOffset, N3_SP_TYPE_BBS_OPEN);
	CAPISocket::MP_AddByte(byBuff, iOffset, N3_SP_TRADE_BBS_SELL);
	CGameProcedure::s_pSocket->Send(byBuff, iOffset);
}

void CUITradeBBSSelector::MsgSend_OpenTradeBuyBBS()
{
	uint8_t byBuff[5];		// 패킷 버퍼..
	int iOffset=0;		// 패킷 오프셋..

	CAPISocket::MP_AddByte(byBuff, iOffset, WIZ_MARKET_BBS);		
	CAPISocket::MP_AddByte(byBuff, iOffset, N3_SP_TYPE_BBS_OPEN);
	CAPISocket::MP_AddByte(byBuff, iOffset, N3_SP_TRADE_BBS_BUY);
	CGameProcedure::s_pSocket->Send(byBuff, iOffset);
}

bool CUITradeBBSSelector::OnKeyPress(int iChar)
{
	switch(iChar)
	{
	case DIK_ESCAPE:
		SetVisible(false);
		return true;
	}

	return CN3UIBase::OnKeyPress(iChar);
}

void CUITradeBBSSelector::SetVisible(bool bVisible)
{
	CN3UIBase::SetVisible(bVisible);
	if(bVisible)
		CGameProcedure::s_pUIMgr->SetVisibleFocusedUI(this);
	else
		CGameProcedure::s_pUIMgr->ReFocusUI();//this_ui
}
