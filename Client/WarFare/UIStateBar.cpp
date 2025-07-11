﻿// UIStateBar.cpp: implementation of the CUIStateBar class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "UIStateBar.h"
#include "GameDef.h"
#include "LocalInput.h"
#include "GameProcMain.h"
#include "APISocket.h"
#include "PacketDef.h"
#include "PlayerMySelf.h"
#include "MagicSkillMng.h"
#include "UIManager.h"
#include "UILevelGuide.h"
#include "N3UIDBCLButton.h"

#include <N3Base/N3UIButton.h>
#include <N3Base/N3UIProgress.h>
#include <N3Base/N3UIString.h>
#include <N3Base/N3UIImage.h>
#include <N3Base/N3Texture.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CUIStateBar::CUIStateBar()
{
	m_pText_Position = NULL;
	m_pProgress_HP = NULL;
	m_pProgress_MSP = NULL;
	m_pProgress_ExpC = NULL;
	m_pProgress_ExpP = NULL;

	m_pText_FPS = NULL;

	// 미니맵...
	m_pGroup_MiniMap = NULL;
	m_pImage_Map = NULL;
	m_pBtn_ZoomIn = NULL;
	m_pBtn_ZoomOut = NULL;
	m_pBtn_Quest = NULL;
	m_pBtn_Power = NULL;

	memset(m_vArrows, 0, sizeof(m_vArrows));

	m_fZoom = 6.0f;
	m_fMapSizeX = 0.0f;
	m_fMapSizeZ = 0.0f;
	m_fYawPlayer = 0;
	m_vPosPlayer.Zero();
	m_vViewPos.Zero();

	m_pMagic.clear();
}

CUIStateBar::~CUIStateBar()
{
	it_MagicImg it, ite;
	ite = m_pMagic.end();	
	for(it = m_pMagic.begin(); it!=ite; it++)
	{
		__DurationMagicImg* pMagicImg = (*it);
		if(pMagicImg)
		{
			delete pMagicImg->pIcon;
			delete pMagicImg;
		}
	}

	m_pMagic.clear();
}

void CUIStateBar::Release()
{
	it_MagicImg it, ite;
	ite = m_pMagic.end();	
	for(it = m_pMagic.begin(); it!=ite; it++)
	{
		__DurationMagicImg* pMagicImg = (*it);
		if(pMagicImg)
		{
			delete pMagicImg->pIcon;
			delete pMagicImg;
		}
	}

	m_pMagic.clear();

	CN3UIBase::Release();

	m_pText_Position = NULL;
	m_pProgress_HP = NULL;
	m_pProgress_MSP = NULL;
	m_pProgress_ExpC = NULL;
	m_pProgress_ExpP = NULL;

	// 미니맵...
	m_pGroup_MiniMap = NULL;
	m_pImage_Map = NULL;
	m_pBtn_ZoomIn = NULL;
	m_pBtn_ZoomOut = NULL; 
	m_pBtn_Power = NULL;
	m_pBtn_Quest = NULL;

	memset(m_vArrows, 0, sizeof(m_vArrows));

	m_fZoom = 6.0f;
	m_fMapSizeX = 0.0f;
	m_fMapSizeZ = 0.0f;
	m_fYawPlayer = 0;
	m_vPosPlayer.Zero();
	m_vViewPos.Zero();
}

bool CUIStateBar::Load(HANDLE hFile)
{
	if(CN3UIBase::Load(hFile)==false) return false;

	CN3UIString* pText = (CN3UIString*)(this->GetChildByID("Text_Version")); __ASSERT(pText, "NULL UI Component!!");
	if(pText) {
		char szVersion[128];
		sprintf(szVersion, "Ver. %.3f", CURRENT_VERSION / 1000.0f);
		pText->SetString(szVersion);
	}

	m_pText_Position =	(CN3UIString*)(this->GetChildByID("Text_Position"));	__ASSERT(m_pText_Position, "NULL UI Component!!");

	m_pProgress_HP =	(CN3UIProgress*)(this->GetChildByID("Progress_HP"));	__ASSERT(m_pProgress_HP, "NULL UI Component!!");
	m_pProgress_MSP =	(CN3UIProgress*)(this->GetChildByID("Progress_MSP"));	__ASSERT(m_pProgress_MSP, "NULL UI Component!!");
	m_pProgress_ExpC =	(CN3UIProgress*)(this->GetChildByID("Progress_ExpC"));	__ASSERT(m_pProgress_ExpC, "NULL UI Component!!");
	m_pProgress_ExpP =	(CN3UIProgress*)(this->GetChildByID("Progress_ExpP"));	__ASSERT(m_pProgress_ExpP, "NULL UI Component!!");
	
	if(m_pProgress_HP) m_pProgress_HP->SetRange(0, 100);
	if(m_pProgress_MSP) m_pProgress_MSP->SetRange(0, 100);
	if(m_pProgress_ExpC) m_pProgress_ExpC->SetRange(0, 100);
	if(m_pProgress_ExpP) m_pProgress_ExpP->SetRange(0, 100);

	// NOTE: new components not previously used

	//CN3UIProgress* m_pProgress_HP_poison = (CN3UIProgress*)(this->GetChildByID("Progress_HP_poison"));	__ASSERT(m_pProgress_HP_poison, "NULL UI Component!!");
	CN3UIProgress* m_pProgress_HP_poison = (CN3UIProgress*)(this->GetChildByID("Progress_HP_slow"));
	__ASSERT(m_pProgress_HP_poison, "NULL UI Component!!");
	if (m_pProgress_HP_poison) {
		m_pProgress_HP_poison->SetRange(0, 100);
		m_pProgress_HP_poison->SetVisible(false);
	}

	//CN3UIProgress* m_pProgress_HP_curse = (CN3UIProgress*)(this->GetChildByID("Progress_HP_curse"));	__ASSERT(m_pProgress_HP_curse, "NULL UI Component!!");
	CN3UIProgress* m_pProgress_HP_curse = (CN3UIProgress*)(this->GetChildByID("Progress_HP_drop"));
	__ASSERT(m_pProgress_HP_curse, "NULL UI Component!!");
	if (m_pProgress_HP_curse) {
		m_pProgress_HP_curse->SetRange(0, 100);
		m_pProgress_HP_curse->SetVisible(false);
	}

	CN3UIProgress* m_pProgress_HP_lasting = (CN3UIProgress*)(this->GetChildByID("Progress_HP_lasting"));
	__ASSERT(m_pProgress_HP_curse, "NULL UI Component!!");
	if (m_pProgress_HP_lasting) {
		m_pProgress_HP_lasting->SetRange(0, 100);
		m_pProgress_HP_lasting->SetVisible(false);
	}

	// NOTE: new components to display the text
	m_pText_HP = (CN3UIString*)GetChildByID("Text_HP");
	__ASSERT(m_pText_HP, "NULL UI Component!!");
	m_pText_MP = (CN3UIString*)GetChildByID("Text_MSP");
	__ASSERT(m_pText_MP, "NULL UI Component!!");
	m_pText_Exp = (CN3UIString*)GetChildByID("Text_ExpP");
	__ASSERT(m_pText_Exp, "NULL UI Component!!");

	CN3UIString* m_pText_SysTime = (CN3UIString*)GetChildByID("SystemTime");
	if (m_pText_SysTime) m_pText_SysTime->SetVisible(false);

	m_pText_FPS = (CN3UIString*)GetChildByID("string_fps");

	// MiniMap
	m_pGroup_MiniMap = GetChildByID("Group_MiniMap"); __ASSERT(m_pGroup_MiniMap, "NULL UI Component!!");
	if(m_pGroup_MiniMap)
	{
		m_pGroup_MiniMap->SetVisible(false);

		m_pImage_Map =		(CN3UIImage*)(m_pGroup_MiniMap->GetChildByID("Img_MiniMap"));	__ASSERT(m_pImage_Map, "NULL UI Component!!");
		m_pBtn_ZoomIn =		(CN3UIButton*)(m_pGroup_MiniMap->GetChildByID("Btn_ZoomIn"));	__ASSERT(m_pBtn_ZoomIn, "NULL UI Component!!");
		m_pBtn_ZoomOut =	(CN3UIButton*)(m_pGroup_MiniMap->GetChildByID("Btn_ZoomOut"));	__ASSERT(m_pBtn_ZoomOut, "NULL UI Component!!");
	}

	m_pBtn_Quest = (CN3UIButton*)(GetChildByID("btn_quest"));
	m_pBtn_Power = (CN3UIButton*)(GetChildByID("btn_power"));

	return true;
}

bool CUIStateBar::LoadMap(const std::string& szMiniMapFN, float fMapSizeX, float fMapSizeZ)
{
	m_fMapSizeX = fMapSizeX;
	m_fMapSizeZ = fMapSizeZ;
	if(NULL == m_pImage_Map) return false;

	m_pImage_Map->SetTex(szMiniMapFN);
	return true;
}


void CUIStateBar::UpdateExp(uint64_t iExp, uint64_t iExpNext, bool bUpdateImmediately)
{
	__ASSERT(iExpNext, "Next Exp is 0");
	if(iExpNext <= 0) return;
	if(NULL == m_pProgress_ExpC || NULL == m_pProgress_ExpP) return;

	int iPercentage = (int)(100.0 * ((double) iExp / (double) iExpNext));

	if(iExpNext > 10)
	{
		uint64_t iExpNext2 = iExpNext/10;
		uint64_t iExp2 = iExp%iExpNext2;
		int iPercentage2 = (int)(100 * iExp2 / iExpNext2);

		if(bUpdateImmediately) m_pProgress_ExpC->SetCurValue(iPercentage2);	 //SetCurValue --> set경우 
		else m_pProgress_ExpC->SetCurValue(iPercentage2, 0.7f, 50.0f);
	}
	else
	{
		m_pProgress_ExpC->SetCurValue(0);	 //SetCurValue --> set경우 
	}

	if(bUpdateImmediately) m_pProgress_ExpP->SetCurValue(iPercentage);	 //SetCurValue --> set경우 
	else m_pProgress_ExpP->SetCurValue(iPercentage, 0.3f, 100.0f);

	// NOTE: adding the EXP text
	__ASSERT(iExp >= 0 && iExpNext > 0, "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
	if (NULL == m_pText_Exp) return;

	double iPercentage2 = 100.0 * ((double)iExp / (double)iExpNext);

	char szVal[64] = "0 %%";
	sprintf(szVal, "%.2f %%", iPercentage2);
	m_pText_Exp->SetString(szVal);
}

void CUIStateBar::UpdateMSP(int iMSP, int iMSPMax, bool bUpdateImmediately)
{
	__ASSERT(iMSPMax, "Max MP is 0");
	if(iMSPMax <= 0) return;
	if(NULL == m_pProgress_MSP) return;

	int iPercentage = 100 * iMSP / iMSPMax;

	if(bUpdateImmediately) m_pProgress_MSP->SetCurValue(iPercentage);	 //SetCurValue --> set경우 
	else m_pProgress_MSP->SetCurValue(iPercentage, 0.3f, 100.0f);

	// NOTE: adding the MP text
	__ASSERT(iMSP >= 0 && iMSPMax > 0, "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
	if (NULL == m_pText_MP) return;

	char szVal[64] = "0 / 0";
	sprintf(szVal, "%d / %d", iMSP, iMSPMax);
	m_pText_MP->SetString(szVal);
}

void CUIStateBar::UpdateHP(int iHP, int iHPMax, bool bUpdateImmediately)
{
	__ASSERT(iHPMax, "Max HP is 0");
	if(iHPMax <= 0) return;

	int iPercentage = 100 * iHP / iHPMax;

	if(bUpdateImmediately) m_pProgress_HP->SetCurValue(iPercentage);	 //SetCurValue --> set경우 
	else m_pProgress_HP->SetCurValue(iPercentage, 0.3f, 100.0f);

	// NOTE: adding the HP text
	__ASSERT(iHP >= 0 && iHP < 10000 && iHPMax >= 0 && iHPMax < 10000, "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
	if (NULL == m_pText_HP) return;

	char szVal[64] = "0 / 0";
	sprintf(szVal, "%d / %d", iHP, iHPMax);
	m_pText_HP->SetString(szVal);
}

void CUIStateBar::UpdatePosition(const __Vector3 &vPos, float fYaw)
{
	if(NULL == m_pText_Position) return;

	char szPos[64];
	sprintf(szPos, "%.1f, %.1f", vPos.x, vPos.z);
	m_pText_Position->SetString(szPos);

	// 미니맵.
	m_vPosPlayer = vPos;
	m_fYawPlayer = fYaw;
}

void CUIStateBar::Render()
{
	if(false == m_bVisible) return;

	CN3UIBase::Render();

	if(NULL == m_pGroup_MiniMap || false == m_pGroup_MiniMap->IsVisible()) return; // 미니맵이 안켜져 있음 돌아간다..
	if(NULL == m_pImage_Map) return;
	if(m_fMapSizeX <= 0 || m_fMapSizeZ <= 0) return;

	__VertexTransformedColor vPositions[4], vOutLines[4];
 
	vPositions[0].Set(0, 0, UI_DEFAULT_Z, UI_DEFAULT_RHW, 0xffffffff);
	vPositions[1] = vPositions[0];
	vPositions[2] = vPositions[0];
	vPositions[3] = vPositions[0];
	vOutLines[0].Set(0, 0, UI_DEFAULT_Z, UI_DEFAULT_RHW, 0xff000000);
	vOutLines[1] = vOutLines[0];
	vOutLines[2] = vOutLines[0];
	vOutLines[3] = vOutLines[0];
	
	__PositionInfo info;

	RECT rc = m_pImage_Map->GetRegion();
	float fWidth = (float)(rc.right - rc.left);
	float fHeight = (float)(rc.bottom - rc.top);
	float fCenterX = (float)(rc.left) + fWidth / 2.0f;
	float fCenterY = (float)(rc.top) + fHeight / 2.0f;

	DWORD dwZ, dwFog, dwAlpha, dwCOP, dwCA1, dwSrcBlend, dwDestBlend, dwVertexShader, dwAOP, dwAA1;
	CN3Base::s_lpD3DDev->GetRenderState(D3DRS_ZENABLE, &dwZ);
	CN3Base::s_lpD3DDev->GetRenderState(D3DRS_FOGENABLE, &dwFog);
	CN3Base::s_lpD3DDev->GetRenderState(D3DRS_ALPHABLENDENABLE, &dwAlpha);
	CN3Base::s_lpD3DDev->GetRenderState(D3DRS_SRCBLEND, &dwSrcBlend);
	CN3Base::s_lpD3DDev->GetRenderState(D3DRS_DESTBLEND, &dwDestBlend);
	CN3Base::s_lpD3DDev->GetTextureStageState(0, D3DTSS_COLOROP, &dwCOP);
	CN3Base::s_lpD3DDev->GetTextureStageState(0, D3DTSS_COLORARG1, &dwCA1);
	CN3Base::s_lpD3DDev->GetTextureStageState(0, D3DTSS_ALPHAOP, &dwAOP);
	CN3Base::s_lpD3DDev->GetTextureStageState(0, D3DTSS_ALPHAARG1, &dwAA1);
	CN3Base::s_lpD3DDev->GetFVF(&dwVertexShader);

	CN3Base::s_lpD3DDev->SetRenderState(D3DRS_ZENABLE, FALSE);
	CN3Base::s_lpD3DDev->SetRenderState(D3DRS_FOGENABLE, FALSE);
	CN3Base::s_lpD3DDev->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	CN3Base::s_lpD3DDev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	CN3Base::s_lpD3DDev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	CN3Base::s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	CN3Base::s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
	CN3Base::s_lpD3DDev->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
	CN3Base::s_lpD3DDev->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);

	CN3Base::s_lpD3DDev->SetFVF(FVF_TRANSFORMEDCOLOR); // 플레이어의 위치및 방향 표시..

	__Vector3 vPos;
	it_PositionInfo it = m_Positions.begin(), itEnd = m_Positions.end();
	for(; it != itEnd; it++)
	{
		info = *it;
		
		vPos = m_vViewPos - info.vPos;
		vPos.x = (float)(int)(fCenterX - m_fZoom * fWidth * (vPos.x / m_fMapSizeX));
		vPos.y = (float)(int)(fCenterY + m_fZoom * fHeight * (vPos.z / m_fMapSizeZ));

		if (vPos.x < rc.left || vPos.x > rc.right || vPos.y < rc.top || vPos.y > rc.bottom)
		{
			continue;
		}

		vPositions[0].color = info.crType;
		vPositions[1].color = info.crType;
		vPositions[2].color = info.crType;
		vPositions[3].color = info.crType;

		vPositions[0].x = vPos.x - 1.0f;
		vPositions[0].y = vPos.y - 1.0f;
		vPositions[1].x = vPos.x + 1.0f;
		vPositions[1].y = vPos.y - 1.0f;
		vPositions[2].x = vPos.x + 1.0f;
		vPositions[2].y = vPos.y + 1.0f;
		vPositions[3].x = vPos.x - 1.0f;
		vPositions[3].y = vPos.y + 1.0f;

		vOutLines[0].x = vPos.x - 2.0f;
		vOutLines[0].y = vPos.y - 2.0f;
		vOutLines[1].x = vPos.x + 2.0f;
		vOutLines[1].y = vPos.y - 2.0f;
		vOutLines[2].x = vPos.x + 2.0f;
		vOutLines[2].y = vPos.y + 2.0f;
		vOutLines[3].x = vPos.x - 2.0f;
		vOutLines[3].y = vPos.y + 2.0f;

		CN3Base::s_lpD3DDev->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, vOutLines, sizeof(__VertexTransformedColor));
		CN3Base::s_lpD3DDev->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, vPositions, sizeof(__VertexTransformedColor));
	}

	CN3Base::s_lpD3DDev->DrawPrimitiveUP(D3DPT_TRIANGLELIST, 2, m_vArrows, sizeof(__VertexTransformedColor));

	it = m_PositionsTop.begin(), itEnd = m_PositionsTop.end(); // 나중에 그릴 점덜...
	for(; it != itEnd; it++)
	{
		info = *it;
		
		vPos = m_vViewPos - info.vPos;
		vPos.x = (float)(int)(fCenterX - m_fZoom * fWidth * (vPos.x / m_fMapSizeX));
		vPos.y = (float)(int)(fCenterY + m_fZoom * fHeight * (vPos.z / m_fMapSizeZ));

		if (vPos.x < rc.left || vPos.x > rc.right || vPos.y < rc.top || vPos.y > rc.bottom)
		{
			continue;
		}

		vPositions[0].color = info.crType;
		vPositions[1].color = info.crType;
		vPositions[2].color = info.crType;
		vPositions[3].color = info.crType;

		vPositions[0].x = vPos.x - 1.0f;
		vPositions[0].y = vPos.y - 1.0f;
		vPositions[1].x = vPos.x + 1.0f;
		vPositions[1].y = vPos.y - 1.0f;
		vPositions[2].x = vPos.x + 1.0f;
		vPositions[2].y = vPos.y + 1.0f;
		vPositions[3].x = vPos.x - 1.0f;
		vPositions[3].y = vPos.y + 1.0f;

		vOutLines[0].x = vPos.x - 2.0f;
		vOutLines[0].y = vPos.y - 2.0f;
		vOutLines[1].x = vPos.x + 2.0f;
		vOutLines[1].y = vPos.y - 2.0f;
		vOutLines[2].x = vPos.x + 2.0f;
		vOutLines[2].y = vPos.y + 2.0f;
		vOutLines[3].x = vPos.x - 2.0f;
		vOutLines[3].y = vPos.y + 2.0f;

		CN3Base::s_lpD3DDev->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, vOutLines, sizeof(__VertexTransformedColor));
		CN3Base::s_lpD3DDev->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, vPositions, sizeof(__VertexTransformedColor));
	}

	//Render..
	//m_pMagic.clear();
	it_MagicImg itMagic, iteMagic;
	iteMagic = m_pMagic.end();
	for(itMagic = m_pMagic.begin(); itMagic!=iteMagic; itMagic++)
	{
		__DurationMagicImg* pMagicImg = (*itMagic);
		pMagicImg->pIcon->Render();
	}
	
	CN3Base::s_lpD3DDev->SetRenderState(D3DRS_ZENABLE, dwZ);
	CN3Base::s_lpD3DDev->SetRenderState(D3DRS_FOGENABLE, dwFog);
	CN3Base::s_lpD3DDev->SetRenderState(D3DRS_ALPHABLENDENABLE, dwAlpha);
	CN3Base::s_lpD3DDev->SetRenderState(D3DRS_SRCBLEND, dwSrcBlend);
	CN3Base::s_lpD3DDev->SetRenderState(D3DRS_DESTBLEND, dwDestBlend);
	CN3Base::s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLOROP, dwCOP);
	CN3Base::s_lpD3DDev->SetTextureStageState(0, D3DTSS_COLORARG1, dwCA1);
	CN3Base::s_lpD3DDev->SetTextureStageState(0, D3DTSS_ALPHAOP, dwAOP);
	CN3Base::s_lpD3DDev->SetTextureStageState(0, D3DTSS_ALPHAARG1, dwAA1);
	CN3Base::s_lpD3DDev->SetFVF(dwVertexShader);
}

void CUIStateBar::Tick()
{
	if(!m_bVisible) return;

	CN3UIBase::Tick();

	// NOTE(srmeier): set the FPS string to be displayed
	static int iCount = 0;
	static char strFPS[0x10] = "";

	if (iCount++ == 60) {
		iCount = 0;
		sprintf(strFPS, "%.1f", CN3Base::s_fFrmPerSec);
		if(m_pText_FPS) m_pText_FPS->SetString(strFPS);
	}


	TickMiniMap(); // 맵 이미지...
	TickMagicIcon(); // 아이콘 처리..
}

void CUIStateBar::TickMiniMap()
{
	if (NULL == m_pImage_Map) return;
	if (m_fMapSizeX <= 0 || m_fMapSizeZ <= 0) return;

	m_vViewPos = m_vPosPlayer;

	RECT rc = m_pImage_Map->GetRegion();

	int minimapWidth = rc.right - rc.left;
	int minimapWidth2 = minimapWidth / 2;
	int minimapHeight = rc.bottom - rc.top;
	int minimapHeight2 = minimapHeight / 2;

	float factorX = m_fZoom * minimapWidth / m_fMapSizeX;
	float factorY = m_fZoom * minimapHeight / m_fMapSizeZ;

	// limit map from scrolling out of bounds

	if (minimapWidth2 > factorX * m_vViewPos.x)
	{
		m_vViewPos.x = minimapWidth2 / factorX;
	}

	if (m_fZoom * minimapWidth - minimapWidth2 < factorX * m_vViewPos.x)
	{
		m_vViewPos.x = (m_fZoom * minimapWidth - minimapWidth2) / factorX;
	}

	if (minimapHeight2 > factorY * m_vViewPos.z)
	{
		m_vViewPos.z = minimapHeight2 / factorY;
	}

	if (m_fZoom * minimapHeight - minimapHeight2 < factorY * m_vViewPos.z)
	{
		m_vViewPos.z = (m_fZoom * minimapHeight - minimapHeight2) / factorY;
	}

	float fOffset = (0.5f / m_fZoom);
	float fX = (m_vViewPos.x / m_fMapSizeX);
	float fY = (m_vViewPos.z / m_fMapSizeZ);

	float x1, y1, x2, y2 = 0;
	x1 = fX - fOffset;
	y1 = fY + fOffset;
	x2 = fX + fOffset;
	y2 = fY - fOffset;

	m_pImage_Map->SetUVRect(x1, 1.0f - y1, x2, 1.0f - y2);

	float fH = (rc.bottom - rc.top) / 30.0f;

	__Matrix44 mtxRot; mtxRot.RotationZ(m_fYawPlayer);
	mtxRot.PosSet(rc.left + minimapWidth2 + factorX * (m_vPosPlayer.x - m_vViewPos.x), rc.top + minimapHeight2 - factorY * (m_vPosPlayer.z - m_vViewPos.z), 0);

	// 화살표 세팅...
	m_vArrows[0].Set(0, -fH, UI_DEFAULT_Z, UI_DEFAULT_RHW, 0xff00ff00);
	m_vArrows[1].Set(0, fH / 2.0f, UI_DEFAULT_Z, UI_DEFAULT_RHW, 0xff00ff00);
	m_vArrows[2].Set(-fH, fH, UI_DEFAULT_Z, UI_DEFAULT_RHW, 0xff00ff00);

	m_vArrows[3] = m_vArrows[0];
	m_vArrows[4] = m_vArrows[2]; m_vArrows[4].x = -m_vArrows[4].x;
	m_vArrows[5] = m_vArrows[1];

	for (int i = 0; i < 6; i++) m_vArrows[i] *= mtxRot; // 위치 및 회전 변환..
}

void CUIStateBar::TickMagicIcon()
{
	int cnt = m_pMagic.size();
	it_MagicImg it = m_pMagic.begin();
	__TABLE_UPC_SKILL* pRemoveSkill = NULL;

	for (int i = 0; i < cnt; i++, it++)
	{
		__DurationMagicImg* pMagicImg = (*it);

		pMagicImg->fDuration -= CN3Base::s_fSecPerFrm;

		if (pMagicImg->fDuration <= 0.0f)
		{
			pMagicImg->pIcon->SetVisible(false);
			pRemoveSkill = CGameBase::s_pTbl_Skill.Find(pMagicImg->dwSkillID);
			break;
		}
		else if (pMagicImg->fDuration <= 10.0f)
		{
			pMagicImg->pIcon->SetVisible(pMagicImg->fDuration - (int)pMagicImg->fDuration<0.5f);
		}
	}

	if (pRemoveSkill) DelMagic(pRemoveSkill);
}

bool CUIStateBar::ReceiveMessage(CN3UIBase* pSender, uint32_t dwMsg)
{
	if (dwMsg == UIMSG_BUTTON_CLICK)
	{
		if (pSender == m_pBtn_ZoomIn)
		{
			ZoomSet(m_fZoom*1.1f);
			return true;
		}
		else if (pSender == m_pBtn_ZoomOut)
		{
			ZoomSet(m_fZoom*0.9f);
			return true;
		}
		else if (pSender == m_pBtn_Quest)
		{
			if (CGameProcedure::s_pProcMain->m_pUILevelGuide != nullptr)
				CGameProcedure::s_pProcMain->CommandToggleLevelGuide();

			return true;
		}
		else if (pSender == m_pBtn_Power)
		{
			//TODO: Launch msgbox with warning message, on confirm -> open store (Web_Browser UI or maybe external?)
			return false;
		}
	}
	else if(dwMsg==UIMSG_ICON_DBLCLK)
	{
		it_MagicImg it, ite;
		ite = m_pMagic.end();	
		for(it = m_pMagic.begin(); it!=ite; it++)
		{
			__DurationMagicImg* pMagicImg = (*it);

			if(pSender==pMagicImg->pIcon)
			{
				uint32_t MagicID = pMagicImg->dwSkillID;
				__TABLE_UPC_SKILL* pSkill = CGameBase::s_pTbl_Skill.Find(MagicID);
				if(!pSkill) return false;

				if(CGameProcedure::s_pProcMain->m_pMagicSkillMng->IsPositiveMagic(MagicID))
				{
					if(pSkill->dw1stTableType==3 || pSkill->dw2ndTableType==3) pMagicImg->fDuration = 0.0f;

					//없애라..
					uint8_t byBuff[32];
					int iOffset=0;
					CAPISocket::MP_AddByte(byBuff, iOffset, (uint8_t)WIZ_MAGIC_PROCESS);
					CAPISocket::MP_AddByte(byBuff, iOffset, (uint8_t)N3_SP_MAGIC_CANCEL);
					CAPISocket::MP_AddDword(byBuff, iOffset, (int)MagicID);
					CAPISocket::MP_AddShort(byBuff, iOffset, (int16_t)CGameBase::s_pPlayer->IDNumber());
					CAPISocket::MP_AddShort(byBuff, iOffset, (int16_t)CGameBase::s_pPlayer->IDNumber());

					CAPISocket::MP_AddShort(byBuff, iOffset, 0);
					CAPISocket::MP_AddShort(byBuff, iOffset, 0);
					CAPISocket::MP_AddShort(byBuff, iOffset, 0);

					CAPISocket::MP_AddShort(byBuff, iOffset, 0);
					CAPISocket::MP_AddShort(byBuff, iOffset, 0);
					CAPISocket::MP_AddShort(byBuff, iOffset, 0);

					CGameProcedure::s_pSocket->Send(byBuff, iOffset); // 보낸다..					
					return true;
				}
			}
		}	
	}	
	return false;
}

// NOTE: appears to add NPC/Players as dots on map?
void CUIStateBar::PositionInfoAdd(int iID, const __Vector3 &vPos, D3DCOLOR crID, bool bDrawTop)
{
	__PositionInfo info;
	info.iID = iID;
	info.vPos = vPos;
	info.crType = crID;

	if(bDrawTop) m_PositionsTop.push_back(info);
	else m_Positions.push_back(info);
}

void CUIStateBar::PositionInfoClear()
{
	m_Positions.clear();
	m_PositionsTop.clear();
}

void CUIStateBar::ZoomSet(float fZoom)
{
	if (fZoom < 1.0f)
	{
		m_fZoom = 1.0f;
	}
	else if (fZoom > 6.0f)
	{
		m_fZoom = 6.0f;
	}
	else
	{
		m_fZoom = fZoom;
	}
}

bool CUIStateBar::ToggleMiniMap()
{
	if(NULL == m_pGroup_MiniMap) return false;

	bool bVisible = m_pGroup_MiniMap->IsVisible();
	m_pGroup_MiniMap->SetVisible(!bVisible);

	return !bVisible;
}

void CUIStateBar::AddMagic(__TABLE_UPC_SKILL* pSkill, float fDuration)
{
	std::vector<char> buffer(256, NULL);
	sprintf(&buffer[0],	"UI\\skillicon_%.2d_%d.dxt", pSkill->dwID%100, pSkill->dwID/100);

	__DurationMagicImg* pMagicImg = new __DurationMagicImg;
	pMagicImg->fDuration = fDuration;
	pMagicImg->pIcon = new CN3UIDBCLButton;
	pMagicImg->dwSkillID = pSkill->dwID;

	CN3UIDBCLButton* pIcon = pMagicImg->pIcon;
	pIcon->Init(this);
	pIcon->SetTex(&buffer[0]);
	pIcon->SetTooltipText(pSkill->szName.c_str());
	pIcon->SetUVRect(0,0,1,1);

	CN3Texture* pTex = pIcon->GetTex();
	if(!pTex)
	{
		delete pIcon;
		delete pMagicImg;
		return;
	}
	
	RECT rt;
	rt.left = rt.top = 0;
	rt.right = pTex->Width();
	rt.bottom = pTex->Height();		
	int PosX = CN3Base::s_CameraData.vp.Width - (rt.right)*(m_pMagic.size()+1);
	pIcon->SetRegion(rt);
	pIcon->SetPos(PosX,0);

	m_pMagic.push_back(pMagicImg);
}

void CUIStateBar::DelMagic(__TABLE_UPC_SKILL* pSkill)
{
	std::vector<char> buffer(256, NULL);
	sprintf(&buffer[0],	"UI\\skillicon_%.2d_%d.dxt", pSkill->dwID%100, pSkill->dwID/100);

	it_MagicImg it, ite, itRemove;
	itRemove = ite = m_pMagic.end();	
	for(it = m_pMagic.begin(); it!=ite; it++)
	{
		__DurationMagicImg* pMagicImg = (*it);
		CN3UIDBCLButton* pIcon = pMagicImg->pIcon;
		CN3Texture* pTex = pIcon->GetTex();
		if(pTex && lstrcmpi(pTex->FileName().c_str(), &buffer[0])==0)
		{
			itRemove = it;
		}
		if(itRemove!=ite)
		{
			POINT pos = pIcon->GetPos();
			RECT rt = pIcon->GetRegion();			
			pIcon->SetPos(pos.x + (rt.right-rt.left),0);
		}
	}
	if(itRemove!=ite)
	{
		__DurationMagicImg* pMagicImg = (*itRemove);
		CN3UIDBCLButton* pIcon = pMagicImg->pIcon;
		delete pIcon;
		delete pMagicImg;
		m_pMagic.erase(itRemove);
	}	
}

void CUIStateBar::ClearMagic()
{
	it_MagicImg it, ite;
	ite = m_pMagic.end();
	for(it = m_pMagic.begin(); it!=ite; it++)
	{
		__DurationMagicImg* pMagicImg = (*it);
		CN3UIDBCLButton* pIcon = pMagicImg->pIcon;
		delete pIcon;
		delete pMagicImg;		
	}
	m_pMagic.clear();
}

uint32_t CUIStateBar::MouseProc(uint32_t dwFlags, const POINT &ptCur, const POINT &ptOld)
{
	uint32_t dwRet = UI_MOUSEPROC_NONE;

	int cnt = m_pMagic.size();
	it_MagicImg it = m_pMagic.begin();
	for(int i=0;i<cnt;i++,it++)
	{
		__DurationMagicImg* pMagicImg = (*it);

		dwRet |= pMagicImg->pIcon->MouseProc(CGameProcedure::s_pLocalInput->MouseGetFlag(),
									CGameProcedure::s_pLocalInput->MouseGetPos(),
									CGameProcedure::s_pLocalInput->MouseGetPosOld());
	}

	dwRet |= CN3UIBase::MouseProc(dwFlags, ptCur, ptOld);

	return dwRet;
}

bool CUIStateBar::OnKeyPress(int iKey)
{
	switch(iKey)
	{
	case DIK_ESCAPE:
		{	//hotkey가 포커스 잡혀있을때는 다른 ui를 닫을수 없으므로 DIK_ESCAPE가 들어오면 포커스를 다시잡고
			//열려있는 다른 유아이를 닫아준다.
			CGameProcedure::s_pUIMgr->ReFocusUI();//this_ui
			CN3UIBase* pFocus = CGameProcedure::s_pUIMgr->GetFocusedUI();
			if(pFocus && pFocus != this) pFocus->OnKeyPress(iKey);
		}
		return true;
	}

	return CN3UIBase::OnKeyPress(iKey);
}
