﻿// UITradeSellBBS.h: interface for the CUITradeSellBBS class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_UITRADESELLBBS_H__2550F618_FFC2_425A_B66A_2275D1E1FCAB__INCLUDED_)
#define AFX_UITRADESELLBBS_H__2550F618_FFC2_425A_B66A_2275D1E1FCAB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <N3Base/N3UIBase.h>

#include "UIMessageBox.h"
#include "UITradeExplanation.h"

#include <list>

struct __InfoTradeSellBBS // 파티 지원 게시판 구조체..
{
	std::string szID;			// 파티 이름 문자열
	std::string szTitle;		// 제목
	std::string szExplanation;	// 설명
	int			iPrice;			// 가격
	int16_t		sIndex;			// 등록 인덱스
	int16_t		sID;			// 캐릭터 아이디

	void Init()
	{
		szID			= "";
		szTitle			= "";
		szExplanation	= "";
		iPrice			= 0;
		sIndex			= -1;
	};

	__InfoTradeSellBBS()
	{
		this->Init();
	}
};

typedef std::list<__InfoTradeSellBBS>::iterator it_TradeSellBBS;

class Packet;
class CUITradeSellBBS : public CN3UIBase  
{
protected:
//	class CN3UIList*		m_pList_Infos;

	class CN3UIButton*		m_pBtn_PageUp;
	class CN3UIButton*		m_pBtn_PageDown;
	class CN3UIButton*		m_pBtn_Refresh;
	class CN3UIButton*		m_pBtn_Close;
	class CN3UIButton*		m_pBtn_Register;
	class CN3UIButton*		m_pBtn_RegisterCancel;
	class CN3UIButton*		m_pBtn_Whisper;
	class CN3UIButton*		m_pBtn_Trade;

	class CN3UIImage*		m_pImage_Sell;
	class CN3UIImage*		m_pImage_Buy;
	class CN3UIImage*		m_pImage_Sell_Title;
	class CN3UIImage*		m_pImage_Buy_Title;

	class CN3UIString*		m_pString_Page;
	class CN3UIString*		m_pText[69];

	CUIMessageBox			m_MsgBox;
	CUITradeExplanation		m_UIExplanation;

	std::list<__InfoTradeSellBBS>	m_Datas;
	__InfoTradeSellBBS				m_ITSB;

	int			m_iCurPage; // 현재 페이지..
	int			m_iMaxPage;	// 총 페이지..
	bool		m_bProcessing;
	uint8_t		m_byBBSKind;
	int			m_iCurIndex;
	float		m_fTime;

public:
	void SetContentString();
	void ResetContent();
	void Render();
	void RenderSelectContent();
	bool OnKeyPress(int iKey);
	void MsgSend_PerTrade();
	void OnListExplanation();
	void RefreshExplanation(bool bPageUp = true);
	void OnButtonTrade();
	void OnButtonWhisper();
	void OnButtonRegisterCancel();
	void OnButtonRegister();
	void CallBackProc(int iID, uint32_t dwFlag);
	void MsgSend_RegisterCancel(int16_t sIndex);
	void MsgSend_Register();
	void MsgSend_RefreshData(int iCurPage);
	void RefreshPage();
	bool SelectedString(CN3UIBase* pSender, int& iID);
	void MsgRecv_RefreshData(Packet& pkt);
	void MsgRecv_TradeBBS(Packet& pkt);
	void SetContentString(int iIndex, std::string szID, int iPrice, std::string szTitle);

	void SetBBSKind(uint8_t byKind){ m_byBBSKind = byKind; }
	bool Load(HANDLE hFile);
	bool ReceiveMessage(class CN3UIBase* pSender, uint32_t dwMsg);
	void SetVisible(bool bVisible);

	CUITradeSellBBS();
	virtual ~CUITradeSellBBS();

};

#endif // !defined(AFX_UITRADESELLBBS_H__2550F618_FFC2_425A_B66A_2275D1E1FCAB__INCLUDED_)
