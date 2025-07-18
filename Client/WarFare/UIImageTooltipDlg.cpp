﻿// UIImageTooltipDlg.cpp: implementation of the CUIImageTooltipDlg class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "UIImageTooltipDlg.h"
#include "PlayerMySelf.h"
#include "resource.h"

#include <N3Base/N3UIString.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CUIImageTooltipDlg::CUIImageTooltipDlg() : m_CYellow(D3DCOLOR_RGBA(255, 255, 0, 255)), 
						m_CBlue(D3DCOLOR_RGBA(128, 128, 255, 255)),
						m_CGold(D3DCOLOR_RGBA(220, 199, 124, 255)),
						m_CIvory(D3DCOLOR_RGBA(200, 124, 199, 255)),
						m_CGreen(D3DCOLOR_RGBA(128, 255, 0, 255)),
						m_CWhite(D3DCOLOR_RGBA(255, 255, 255, 255)),
						m_CRed(D3DCOLOR_RGBA(255, 60, 60, 255))
{
	m_iPosXBack = 0;
	m_iPosYBack = 0;
	m_spItemBack = NULL;
	m_pImg = NULL;
}

CUIImageTooltipDlg::~CUIImageTooltipDlg()
{
	Release();
}

void CUIImageTooltipDlg::Release()
{
	CN3UIBase::Release();
}

void CUIImageTooltipDlg::InitPos()
{
	std::string str; 
	char	cstr[4];

	for (int i = 0; i < MAX_TOOLTIP_COUNT; i++)
	{
		str = "string_";
		sprintf(cstr, "%d", i);	str += cstr;
		m_pStr[i] = (CN3UIString* )GetChildByID(str);	 __ASSERT(m_pStr[i], "NULL UI Component!!");
	}

	m_pImg = (CN3UIImage* )GetChildByID("mins");	 __ASSERT(m_pImg, "NULL UI Component!!");
}

void CUIImageTooltipDlg::DisplayTooltipsDisable()
{
	m_spItemBack = NULL;
	if ( IsVisible() ) SetVisible(false);
}

bool CUIImageTooltipDlg::SetTooltipTextColor(int iMyValue, int iTooltipValue)
{
	if ( iMyValue >= iTooltipValue )
		return true;
	return false;
}

bool CUIImageTooltipDlg::SetTooltipTextColor(e_Race eMyValue, e_Race eTooltipValue)
{
	if ( eMyValue == eTooltipValue )
		return true;
	return false;
}

bool CUIImageTooltipDlg::SetTooltipTextColor(e_Class eMyValue, e_Class eTooltipValue)
{
	if ( eMyValue == eTooltipValue )
		return true;
	return false;
}

void CUIImageTooltipDlg::SetPosSomething(int xpos, int ypos, int iNum)
{
	int iWidth = 0;

	int iPadding = 8;

	for (int i = 0; i < iNum; i++)
	{
		if (m_pstdstr[i].empty())	continue;
		int currentWidth = m_pStr[0]->GetStringRealWidth(m_pstdstr[i]);
		if (currentWidth > iWidth)
			iWidth = currentWidth;
	}

	int iHeight = m_pStr[iNum - 1]->GetRegion().bottom - m_pStr[0]->GetRegion().top;

	iWidth += iPadding * 2;
	iHeight += iPadding * 1.5;

	RECT rect, rect2;

	int iRight, iTop, iBottom, iX, iY;

	iRight = CN3Base::s_CameraData.vp.Width;
	iTop = 0;
	iBottom = CN3Base::s_CameraData.vp.Height;

	if ((xpos + 26 + iWidth)<iRight)
	{
		rect.left = xpos + 26;
		rect.right = rect.left + iWidth;
		iX = xpos + 26;
	}
	else
	{
		rect.left = xpos - iWidth;
		rect.right = xpos;
		iX = xpos - iWidth;
	}

	if ((ypos - iHeight)>iTop)
	{
		rect.top = ypos - iHeight; rect.bottom = ypos;
		iY = ypos - iHeight;
	}
	else
	{
		if ((ypos + iHeight)<iBottom)
		{
			rect.top = ypos; rect.bottom = ypos + iHeight;
			iY = ypos;
		}
		else
		{
			rect.top = iBottom - iHeight; rect.bottom = iBottom;
			iY = rect.top;
		}
	}

	SetPos(iX, iY);
	SetSize(iWidth, iHeight);

	for (int i = 0; i < iNum; i++)
	{
		if (!m_pStr[i])	continue;

		// add padding to rects
		rect2 = m_pStr[i]->GetRegion();
		rect2.left = rect.left + iPadding;
		rect2.right = rect.right - iPadding;
		m_pStr[i]->SetRegion(rect2);

		if(m_pStr[i]->GetStyle() & UISTYLE_STRING_ALIGNCENTER)
			m_pStr[i]->SetString(m_pstdstr[i]);
		else
			m_pStr[i]->SetString("  " + m_pstdstr[i]);
	}

	for (int i = iNum; i < MAX_TOOLTIP_COUNT; i++)
		m_pStr[i]->SetString("");

	m_pImg->SetRegion(rect);

	m_iPosXBack = xpos;
	m_iPosYBack = ypos;
}

int	CUIImageTooltipDlg::CalcTooltipStringNumAndWrite(__IconItemSkill* spItem, bool bPrice, bool bBuy)
{
	int iIndex = 0;

	__InfoPlayerMySelf*	pInfoExt = &(CGameBase::s_pPlayer->m_InfoExt);

	if ( (!m_spItemBack) || (m_spItemBack->pItemBasic->dwID != spItem->pItemBasic->dwID) || 
		(m_spItemBack->pItemExt->dwID != spItem->pItemExt->dwID) ||
		(m_spItemBack->iDurability != spItem->iDurability) )
	{

#define ERROR_EXCEPTION									\
{		\
	if (bPrice && (iIndex > MAX_TOOLTIP_COUNT-2))	 {__ASSERT(0, "Too Many Tooltip Item Info");	goto exceptions;}	\
	if (!bPrice && (iIndex > MAX_TOOLTIP_COUNT-1))	 {__ASSERT(0, "Too Many Tooltip Item Info");	goto exceptions;}	\
}
		m_spItemBack = spItem;

		if (m_pStr[iIndex] != nullptr)
		{
			std::string szStr;
			m_pStr[iIndex]->SetStyle(UI_STR_TYPE_HALIGN, UISTYLE_STRING_ALIGNCENTER);
			CGameBase::GetText(IDS_TOOLTIP_GOLD, &szStr);
			if ( spItem->pItemBasic->szName == szStr )
			{
				char szBuff[64] = {};
				sprintf(szBuff, "%d  %s", spItem->iCount, spItem->pItemBasic->szName.c_str());
				// 돈이면 흰색..
				m_pStr[iIndex]->SetColor(m_CWhite);
				m_pstdstr[iIndex] = szBuff;
				iIndex++;			

				for( int i = iIndex; i < MAX_TOOLTIP_COUNT; i++ )
					m_pstdstr[iIndex] = "";

				return iIndex;	
			}
			else if ( spItem->pItemBasic->szName != m_pStr[iIndex]->GetString() )
			{
				e_ItemAttrib eTA = (e_ItemAttrib)(spItem->pItemExt->byMagicOrRare);
				switch (eTA)
				{
					case ITEM_ATTRIB_GENERAL:
						m_pStr[iIndex]->SetColor(m_CWhite);
						break;
					case ITEM_ATTRIB_MAGIC:
						m_pStr[iIndex]->SetColor(m_CBlue);
						break;
					case ITEM_ATTRIB_LAIR:
						m_pStr[iIndex]->SetColor(m_CYellow);
						break;
					case ITEM_ATTRIB_CRAFT:
						m_pStr[iIndex]->SetColor(m_CGreen);
						break;
					case ITEM_ATTRIB_UNIQUE:
						m_pStr[iIndex]->SetColor(m_CGold);
						break;
					case ITEM_ATTRIB_UPGRADE:
						m_pStr[iIndex]->SetColor(m_CIvory);
						break;
					default:
						m_pStr[iIndex]->SetColor(m_CWhite);
						break;
				}

				if ((e_ItemAttrib) (spItem->pItemExt->byMagicOrRare) != ITEM_ATTRIB_UNIQUE)
				{
					std::string strtemp;
					if (spItem->pItemExt->dwID % 10 != 0)
					{
						char szExtID[20] = {};
						sprintf(szExtID, "(+%d)", spItem->pItemExt->dwID % 10);
						strtemp = szExtID;
					}

					m_pstdstr[iIndex] = spItem->pItemBasic->szName + strtemp;
				}
				else
				{
					m_pstdstr[iIndex] = spItem->pItemExt->szHeader;
				}
			}
		}
		iIndex++;

		if ( (spItem->pItemBasic->byContable != UIITEM_TYPE_COUNTABLE) && (spItem->pItemBasic->byContable != UIITEM_TYPE_COUNTABLE_SMALL) )
		{
			m_pStr[iIndex]->SetStyle(UI_STR_TYPE_HALIGN, UISTYLE_STRING_ALIGNCENTER);
			e_ItemClass eIC = (e_ItemClass) (spItem->pItemBasic->byClass);
			CGameBase::GetTextByItemClass(eIC, m_pstdstr[iIndex]); // 아이템 종류에 따라 문자열 만들기..
			m_pStr[iIndex]->SetColor(m_CWhite);
			iIndex++;
		}

		e_Race eRace = (e_Race)spItem->pItemBasic->byNeedRace;
		if (eRace != RACE_ALL)
		{
			m_pStr[iIndex]->SetStyle(UI_STR_TYPE_HALIGN, UISTYLE_STRING_ALIGNCENTER);
			CGameBase::GetTextByRace(eRace, m_pstdstr[iIndex]); // 아이템을 찰수 있는 종족에 따른 문자열 만들기.
			if (SetTooltipTextColor(CGameBase::s_pPlayer->m_InfoBase.eRace, eRace))
				m_pStr[iIndex]->SetColor(m_CWhite);
			else
				m_pStr[iIndex]->SetColor(m_CRed);
			iIndex++;
		}
		ERROR_EXCEPTION

		if ((int)spItem->pItemBasic->byNeedClass != 0)
		{
			m_pStr[iIndex]->SetStyle(UI_STR_TYPE_HALIGN, UISTYLE_STRING_ALIGNCENTER);
			e_Class eClass = (e_Class) spItem->pItemBasic->byNeedClass;
			CGameBase::GetTextByClass(eClass, m_pstdstr[iIndex]); // 아이템을 찰수 있는 종족에 따른 문자열 만들기.

			switch (eClass)
			{
				case CLASS_KINDOF_WARRIOR:
					switch (CGameBase::s_pPlayer->m_InfoBase.eClass)
					{
						case CLASS_KA_WARRIOR:
						case CLASS_KA_BERSERKER:
						case CLASS_KA_GUARDIAN:
						case CLASS_EL_WARRIOR:
						case CLASS_EL_BLADE:
						case CLASS_EL_PROTECTOR:
							m_pStr[iIndex]->SetColor(m_CWhite);
							break;
						default:
							m_pStr[iIndex]->SetColor(m_CRed);
							break;
					}
					break;

				case CLASS_KINDOF_ROGUE:
					switch (CGameBase::s_pPlayer->m_InfoBase.eClass)
					{
						case CLASS_KA_ROGUE:
						case CLASS_KA_HUNTER:
						case CLASS_KA_PENETRATOR:
						case CLASS_EL_ROGUE:
						case CLASS_EL_RANGER:
						case CLASS_EL_ASSASIN:
							m_pStr[iIndex]->SetColor(m_CWhite);
							break;
						default:
							m_pStr[iIndex]->SetColor(m_CRed);
							break;
					}
					break;

				case CLASS_KINDOF_WIZARD:
					switch (CGameBase::s_pPlayer->m_InfoBase.eClass)
					{
						case CLASS_KA_WIZARD:
						case CLASS_KA_SORCERER:
						case CLASS_KA_NECROMANCER:
						case CLASS_EL_WIZARD:
						case CLASS_EL_MAGE:
						case CLASS_EL_ENCHANTER:
							m_pStr[iIndex]->SetColor(m_CWhite);
							break;
						default:
							m_pStr[iIndex]->SetColor(m_CRed);
							break;
					}
					break;

				case CLASS_KINDOF_PRIEST:
					switch (CGameBase::s_pPlayer->m_InfoBase.eClass)
					{
						case CLASS_KA_PRIEST:
						case CLASS_KA_SHAMAN:
						case CLASS_KA_DARKPRIEST:
						case CLASS_EL_PRIEST:
						case CLASS_EL_CLERIC:
						case CLASS_EL_DRUID:
							m_pStr[iIndex]->SetColor(m_CWhite);
							break;
						default:
							m_pStr[iIndex]->SetColor(m_CRed);
							break;
					}
					break;

				case CLASS_KINDOF_ATTACK_WARRIOR:
					switch (CGameBase::s_pPlayer->m_InfoBase.eClass)
					{
						case CLASS_KA_BERSERKER:
						case CLASS_EL_BLADE:
							m_pStr[iIndex]->SetColor(m_CWhite);
							break;
						default:
							m_pStr[iIndex]->SetColor(m_CRed);
							break;
					}
					break;

				case CLASS_KINDOF_DEFEND_WARRIOR:
					switch (CGameBase::s_pPlayer->m_InfoBase.eClass)
					{
						case CLASS_KA_GUARDIAN:
						case CLASS_EL_PROTECTOR:
							m_pStr[iIndex]->SetColor(m_CWhite);
							break;
						default:
							m_pStr[iIndex]->SetColor(m_CRed);
							break;
					}
					break;

				case CLASS_KINDOF_ARCHER:
					switch (CGameBase::s_pPlayer->m_InfoBase.eClass)
					{
						case CLASS_KA_HUNTER:
						case CLASS_EL_RANGER:
							m_pStr[iIndex]->SetColor(m_CWhite);
							break;
						default:
							m_pStr[iIndex]->SetColor(m_CRed);
							break;
					}
					break;

				case CLASS_KINDOF_ASSASSIN:
					switch (CGameBase::s_pPlayer->m_InfoBase.eClass)
					{
						case CLASS_KA_PENETRATOR:
						case CLASS_EL_ASSASIN:
							m_pStr[iIndex]->SetColor(m_CWhite);
							break;
						default:
							m_pStr[iIndex]->SetColor(m_CRed);
							break;
					}
					break;

				case CLASS_KINDOF_ATTACK_WIZARD:
					switch (CGameBase::s_pPlayer->m_InfoBase.eClass)
					{
						case CLASS_KA_SORCERER:
						case CLASS_EL_MAGE:
							m_pStr[iIndex]->SetColor(m_CWhite);
							break;
						default:
							m_pStr[iIndex]->SetColor(m_CRed);
							break;
					}
					break;

				case CLASS_KINDOF_PET_WIZARD:
					switch (CGameBase::s_pPlayer->m_InfoBase.eClass)
					{
						case CLASS_KA_NECROMANCER:
						case CLASS_EL_ENCHANTER:
							m_pStr[iIndex]->SetColor(m_CWhite);
							break;
						default:
							m_pStr[iIndex]->SetColor(m_CRed);
							break;
					}
					break;

				case CLASS_KINDOF_HEAL_PRIEST:
					switch (CGameBase::s_pPlayer->m_InfoBase.eClass)
					{
						case CLASS_KA_SHAMAN:
						case CLASS_EL_CLERIC:
							m_pStr[iIndex]->SetColor(m_CWhite);
							break;
						default:
							m_pStr[iIndex]->SetColor(m_CRed);
							break;
					}
					break;

				case CLASS_KINDOF_CURSE_PRIEST:
					switch (CGameBase::s_pPlayer->m_InfoBase.eClass)
					{
						case CLASS_KA_DARKPRIEST:
						case CLASS_EL_DRUID:
							m_pStr[iIndex]->SetColor(m_CWhite);
							break;
						default:
							m_pStr[iIndex]->SetColor(m_CRed);
							break;
					}
					break;

				default:
					if (SetTooltipTextColor(CGameBase::s_pPlayer->m_InfoBase.eClass, eClass))
						m_pStr[iIndex]->SetColor(m_CWhite);
					else
						m_pStr[iIndex]->SetColor(m_CRed);
					break;
			}					

			iIndex++;
		}
		ERROR_EXCEPTION

		if (spItem->pItemBasic->siDamage+spItem->pItemExt->siDamage != 0)
		{
			CGameBase::GetTextF(
				IDS_TOOLTIP_DAMAGE,
				&m_pstdstr[iIndex],
				spItem->pItemBasic->siDamage + spItem->pItemExt->siDamage);

			m_pStr[iIndex]->SetStyle(UI_STR_TYPE_HALIGN, UISTYLE_STRING_ALIGNLEFT);
			m_pStr[iIndex]->SetColor(m_CWhite);
			iIndex++;
		}
		ERROR_EXCEPTION

		if (spItem->pItemBasic->siAttackInterval * (float) ((float) spItem->pItemExt->siAttackIntervalPercentage / 100.0f) != 0)
		{
			float fValue = spItem->pItemBasic->siAttackInterval * (float) ((float) spItem->pItemExt->siAttackIntervalPercentage / 100.0f);

			if ((0 <= fValue) && (fValue <= 89))
				CGameBase::GetText(IDS_TOOLTIP_ATTACKINT_VERYFAST, &m_pstdstr[iIndex]);
			else if ((90 <= fValue) && (fValue <= 110))
				CGameBase::GetText(IDS_TOOLTIP_ATTACKINT_FAST, &m_pstdstr[iIndex]);
			else if ((111 <= fValue) && (fValue <= 130))
				CGameBase::GetText(IDS_TOOLTIP_ATTACKINT_NORMAL, &m_pstdstr[iIndex]);
			else if ((131 <= fValue) && (fValue <= 150))
				CGameBase::GetText(IDS_TOOLTIP_ATTACKINT_SLOW, &m_pstdstr[iIndex]);
			else
				CGameBase::GetText(IDS_TOOLTIP_ATTACKINT_VERYSLOW, &m_pstdstr[iIndex]);

			m_pStr[iIndex]->SetStyle(UI_STR_TYPE_HALIGN, UISTYLE_STRING_ALIGNLEFT);
			m_pStr[iIndex]->SetColor(m_CWhite);
			iIndex++;
		}
		ERROR_EXCEPTION

		// 공격시간 감소 없어짐..

		if (spItem->pItemBasic->siAttackRange != 0)
		{
			CGameBase::GetTextF(
				IDS_TOOLTIP_ATTACKRANGE,
				&m_pstdstr[iIndex],
				(float) spItem->pItemBasic->siAttackRange / 10.0f);

			m_pStr[iIndex]->SetStyle(UI_STR_TYPE_HALIGN, UISTYLE_STRING_ALIGNLEFT);
			m_pStr[iIndex]->SetColor(m_CWhite);
			iIndex++;
		}
		ERROR_EXCEPTION

		if( spItem->pItemExt->siHitRate != 0)
		{
			CGameBase::GetTextF(
				IDS_TOOLTIP_HITRATE_OVER,
				&m_pstdstr[iIndex],
				spItem->pItemExt->siHitRate);

			m_pStr[iIndex]->SetStyle(UI_STR_TYPE_HALIGN, UISTYLE_STRING_ALIGNLEFT);
			m_pStr[iIndex]->SetColor(m_CWhite);
			iIndex++;
		}
		ERROR_EXCEPTION

		if( spItem->pItemExt->siEvationRate != 0)
		{
			CGameBase::GetTextF(
				IDS_TOOLTIP_AVOIDRATE_OVER,
				&m_pstdstr[iIndex],
				spItem->pItemExt->siEvationRate);

			m_pStr[iIndex]->SetStyle(UI_STR_TYPE_HALIGN, UISTYLE_STRING_ALIGNLEFT);
			m_pStr[iIndex]->SetColor(m_CWhite);
			iIndex++;
		}
		ERROR_EXCEPTION

		if( spItem->pItemBasic->siWeight != 0)
		{
			CGameBase::GetTextF(
				IDS_TOOLTIP_WEIGHT,
				&m_pstdstr[iIndex],
				spItem->pItemBasic->siWeight * 0.1f);

			m_pStr[iIndex]->SetStyle(UI_STR_TYPE_HALIGN, UISTYLE_STRING_ALIGNLEFT);
			m_pStr[iIndex]->SetColor(m_CWhite);
			iIndex++;
		}
		ERROR_EXCEPTION

		if( spItem->pItemBasic->siMaxDurability+spItem->pItemExt->siMaxDurability != 1)
		{
			CGameBase::GetTextF(
				IDS_TOOLTIP_MAX_DURABILITY,
				&m_pstdstr[iIndex],
				spItem->pItemBasic->siMaxDurability + spItem->pItemExt->siMaxDurability);

			m_pStr[iIndex]->SetStyle(UI_STR_TYPE_HALIGN, UISTYLE_STRING_ALIGNLEFT);
			m_pStr[iIndex]->SetColor(m_CWhite);
			iIndex++;

			CGameBase::GetTextF(
				IDS_TOOLTIP_CUR_DURABILITY,
				&m_pstdstr[iIndex],
				spItem->iDurability);

			m_pStr[iIndex]->SetStyle(UI_STR_TYPE_HALIGN, UISTYLE_STRING_ALIGNLEFT);
			m_pStr[iIndex]->SetColor(m_CWhite);
			iIndex++;
		}
		ERROR_EXCEPTION

		if( spItem->pItemBasic->siDefense+spItem->pItemExt->siDefense != 0)
		{
			CGameBase::GetTextF(
				IDS_TOOLTIP_DEFENSE,
				&m_pstdstr[iIndex],
				spItem->pItemBasic->siDefense + spItem->pItemExt->siDefense);

			m_pStr[iIndex]->SetStyle(UI_STR_TYPE_HALIGN, UISTYLE_STRING_ALIGNLEFT);
			m_pStr[iIndex]->SetColor(m_CWhite);
			iIndex++;
		}
		ERROR_EXCEPTION

		if( spItem->pItemExt->siDefenseRateDagger != 0)
		{
			CGameBase::GetTextF(
				IDS_TOOLTIP_DEFENSE_RATE_DAGGER,
				&m_pstdstr[iIndex],
				spItem->pItemExt->siDefenseRateDagger);

			m_pStr[iIndex]->SetStyle(UI_STR_TYPE_HALIGN, UISTYLE_STRING_ALIGNLEFT);
			m_pStr[iIndex]->SetColor(m_CGreen);
			iIndex++;
		}
		ERROR_EXCEPTION

		if( spItem->pItemExt->siDefenseRateSword != 0)
		{
			CGameBase::GetTextF(
				IDS_TOOLTIP_DEFENSE_RATE_SWORD,
				&m_pstdstr[iIndex],
				spItem->pItemExt->siDefenseRateSword);

			m_pStr[iIndex]->SetStyle(UI_STR_TYPE_HALIGN, UISTYLE_STRING_ALIGNLEFT);
			m_pStr[iIndex]->SetColor(m_CGreen);
			iIndex++;
		}
		ERROR_EXCEPTION

		if( spItem->pItemExt->siDefenseRateBlow != 0)
		{
			CGameBase::GetTextF(
				IDS_TOOLTIP_DEFENSE_RATE_BLOW,
				&m_pstdstr[iIndex],
				spItem->pItemExt->siDefenseRateBlow);

			m_pStr[iIndex]->SetStyle(UI_STR_TYPE_HALIGN, UISTYLE_STRING_ALIGNLEFT);
			m_pStr[iIndex]->SetColor(m_CGreen);
			iIndex++;
		}
		ERROR_EXCEPTION

		if( spItem->pItemExt->siDefenseRateAxe != 0)
		{
			CGameBase::GetTextF(
				IDS_TOOLTIP_DEFENSE_RATE_AXE,
				&m_pstdstr[iIndex],
				spItem->pItemExt->siDefenseRateAxe);

			m_pStr[iIndex]->SetStyle(UI_STR_TYPE_HALIGN, UISTYLE_STRING_ALIGNLEFT);
			m_pStr[iIndex]->SetColor(m_CGreen);
			iIndex++;
		}
		ERROR_EXCEPTION

		if( spItem->pItemExt->siDefenseRateSpear != 0)
		{
			CGameBase::GetTextF(
				IDS_TOOLTIP_DEFENSE_RATE_SPEAR,
				&m_pstdstr[iIndex],
				spItem->pItemExt->siDefenseRateSpear);

			m_pStr[iIndex]->SetStyle(UI_STR_TYPE_HALIGN, UISTYLE_STRING_ALIGNLEFT);
			m_pStr[iIndex]->SetColor(m_CGreen);
			iIndex++;
		}
		ERROR_EXCEPTION

		if( spItem->pItemExt->siDefenseRateArrow != 0)
		{
			CGameBase::GetTextF(
				IDS_TOOLTIP_DEFENSE_RATE_ARROW,
				&m_pstdstr[iIndex],
				spItem->pItemExt->siDefenseRateArrow);

			m_pStr[iIndex]->SetStyle(UI_STR_TYPE_HALIGN, UISTYLE_STRING_ALIGNLEFT);
			m_pStr[iIndex]->SetColor(m_CGreen);
			iIndex++;
		}
		ERROR_EXCEPTION

		if( spItem->pItemExt->byDamageFire != 0)	// 화염속성
		{
			CGameBase::GetTextF(
				IDS_TOOLTIP_ATTRMAGIC1,
				&m_pstdstr[iIndex],
				spItem->pItemExt->byDamageFire);

			m_pStr[iIndex]->SetStyle(UI_STR_TYPE_HALIGN, UISTYLE_STRING_ALIGNLEFT);
			m_pStr[iIndex]->SetColor(m_CGreen);
			iIndex++;
		}
		ERROR_EXCEPTION

		if( spItem->pItemExt->byDamageIce != 0)	
		{
			CGameBase::GetTextF(
				IDS_TOOLTIP_ATTRMAGIC2,
				&m_pstdstr[iIndex],
				spItem->pItemExt->byDamageIce);

			m_pStr[iIndex]->SetStyle(UI_STR_TYPE_HALIGN, UISTYLE_STRING_ALIGNLEFT);
			m_pStr[iIndex]->SetColor(m_CGreen);
			iIndex++;
		}
		ERROR_EXCEPTION

		if( spItem->pItemExt->byDamageThuner != 0)	
		{
			CGameBase::GetTextF(
				IDS_TOOLTIP_ATTRMAGIC3,
				&m_pstdstr[iIndex],
				spItem->pItemExt->byDamageThuner);

			m_pStr[iIndex]->SetStyle(UI_STR_TYPE_HALIGN, UISTYLE_STRING_ALIGNLEFT);
			m_pStr[iIndex]->SetColor(m_CGreen);
			iIndex++;
		}
		ERROR_EXCEPTION

		if( spItem->pItemExt->byDamagePoison != 0)	
		{
			CGameBase::GetTextF(
				IDS_TOOLTIP_ATTRMAGIC4,
				&m_pstdstr[iIndex],
				spItem->pItemExt->byDamagePoison);

			m_pStr[iIndex]->SetStyle(UI_STR_TYPE_HALIGN, UISTYLE_STRING_ALIGNLEFT);
			m_pStr[iIndex]->SetColor(m_CGreen);
			iIndex++;
		}
		ERROR_EXCEPTION

		if( spItem->pItemExt->byStillHP != 0)	
		{
			CGameBase::GetTextF(
				IDS_TOOLTIP_ATTRMAGIC5,
				&m_pstdstr[iIndex],
				spItem->pItemExt->byStillHP);

			m_pStr[iIndex]->SetStyle(UI_STR_TYPE_HALIGN, UISTYLE_STRING_ALIGNLEFT);
			m_pStr[iIndex]->SetColor(m_CGreen);
			iIndex++;
		}
		ERROR_EXCEPTION

		if( spItem->pItemExt->byDamageMP != 0)	
		{
			CGameBase::GetTextF(
				IDS_TOOLTIP_ATTRMAGIC6,
				&m_pstdstr[iIndex],
				spItem->pItemExt->byDamageMP);

			m_pStr[iIndex]->SetStyle(UI_STR_TYPE_HALIGN, UISTYLE_STRING_ALIGNLEFT);
			m_pStr[iIndex]->SetColor(m_CGreen);
			iIndex++;
		}
		ERROR_EXCEPTION

		if( spItem->pItemExt->byStillMP != 0)	
		{
			CGameBase::GetTextF(
				IDS_TOOLTIP_ATTRMAGIC7,
				&m_pstdstr[iIndex],
				spItem->pItemExt->byStillMP);

			m_pStr[iIndex]->SetStyle(UI_STR_TYPE_HALIGN, UISTYLE_STRING_ALIGNLEFT);
			m_pStr[iIndex]->SetColor(m_CGreen);
			iIndex++;
		}
		ERROR_EXCEPTION

		if( spItem->pItemExt->siBonusStr != 0)
		{
			CGameBase::GetTextF(
				IDS_TOOLTIP_BONUSSTR,
				&m_pstdstr[iIndex],
				spItem->pItemExt->siBonusStr);

			m_pStr[iIndex]->SetStyle(UI_STR_TYPE_HALIGN, UISTYLE_STRING_ALIGNLEFT);
			m_pStr[iIndex]->SetColor(m_CGreen);
			iIndex++;
		}
		ERROR_EXCEPTION

		if( spItem->pItemExt->siBonusSta != 0)
		{
			CGameBase::GetTextF(
				IDS_TOOLTIP_BONUSSTA,
				&m_pstdstr[iIndex],
				spItem->pItemExt->siBonusSta);

			m_pStr[iIndex]->SetStyle(UI_STR_TYPE_HALIGN, UISTYLE_STRING_ALIGNLEFT);
			m_pStr[iIndex]->SetColor(m_CGreen);
			iIndex++;
		}
		ERROR_EXCEPTION

		if( spItem->pItemExt->siBonusHP != 0)
		{
			CGameBase::GetTextF(
				IDS_TOOLTIP_BONUSHP,
				&m_pstdstr[iIndex],
				spItem->pItemExt->siBonusHP);

			m_pStr[iIndex]->SetStyle(UI_STR_TYPE_HALIGN, UISTYLE_STRING_ALIGNLEFT);
			m_pStr[iIndex]->SetColor(m_CGreen);
			iIndex++;
		}
		ERROR_EXCEPTION

		if( spItem->pItemExt->siBonusDex != 0)
		{
			CGameBase::GetTextF(
				IDS_TOOLTIP_BONUSDEX,
				&m_pstdstr[iIndex],
				spItem->pItemExt->siBonusDex);

			m_pStr[iIndex]->SetStyle(UI_STR_TYPE_HALIGN, UISTYLE_STRING_ALIGNLEFT);
			m_pStr[iIndex]->SetColor(m_CGreen);
			iIndex++;
		}
		ERROR_EXCEPTION

		if( spItem->pItemExt->siBonusMSP != 0)
		{
			CGameBase::GetTextF(
				IDS_TOOLTIP_BONUSWIZ,
				&m_pstdstr[iIndex],
				spItem->pItemExt->siBonusMSP);

			m_pStr[iIndex]->SetStyle(UI_STR_TYPE_HALIGN, UISTYLE_STRING_ALIGNLEFT);
			m_pStr[iIndex]->SetColor(m_CGreen);
			iIndex++;
		}
		ERROR_EXCEPTION

		if( spItem->pItemExt->siBonusInt != 0)
		{
			CGameBase::GetTextF(
				IDS_TOOLTIP_BONUSINT,
				&m_pstdstr[iIndex],
				spItem->pItemExt->siBonusInt);

			m_pStr[iIndex]->SetStyle(UI_STR_TYPE_HALIGN, UISTYLE_STRING_ALIGNLEFT);
			m_pStr[iIndex]->SetColor(m_CGreen);
			iIndex++;
		}
		ERROR_EXCEPTION

		if( spItem->pItemExt->siBonusMagicAttak != 0)
		{
			CGameBase::GetTextF(
				IDS_TOOLTIP_BONUSMAGICATTACK,
				&m_pstdstr[iIndex],
				spItem->pItemExt->siBonusMagicAttak);

			m_pStr[iIndex]->SetStyle(UI_STR_TYPE_HALIGN, UISTYLE_STRING_ALIGNLEFT);
			m_pStr[iIndex]->SetColor(m_CGreen);
			iIndex++;
		}
		ERROR_EXCEPTION

		if( spItem->pItemExt->siRegistFire != 0)
		{
			CGameBase::GetTextF(
				IDS_TOOLTIP_REGISTFIRE,
				&m_pstdstr[iIndex],
				spItem->pItemExt->siRegistFire);

			m_pStr[iIndex]->SetStyle(UI_STR_TYPE_HALIGN, UISTYLE_STRING_ALIGNLEFT);
			m_pStr[iIndex]->SetColor(m_CGreen);
			iIndex++;
		}
		ERROR_EXCEPTION

		if( spItem->pItemExt->siRegistIce != 0)
		{
			CGameBase::GetTextF(
				IDS_TOOLTIP_REGISTICE,
				&m_pstdstr[iIndex],
				spItem->pItemExt->siRegistIce);

			m_pStr[iIndex]->SetStyle(UI_STR_TYPE_HALIGN, UISTYLE_STRING_ALIGNLEFT);
			m_pStr[iIndex]->SetColor(m_CGreen);
			iIndex++;
		}
		ERROR_EXCEPTION

		if( spItem->pItemExt->siRegistElec != 0)
		{
			CGameBase::GetTextF(
				IDS_TOOLTIP_REGISTELEC,
				&m_pstdstr[iIndex],
				spItem->pItemExt->siRegistElec);

			m_pStr[iIndex]->SetStyle(UI_STR_TYPE_HALIGN, UISTYLE_STRING_ALIGNLEFT);
			m_pStr[iIndex]->SetColor(m_CGreen);
			iIndex++;
		}
		ERROR_EXCEPTION

		if( spItem->pItemExt->siRegistMagic != 0)
		{
			CGameBase::GetTextF(
				IDS_TOOLTIP_REGISTMAGIC,
				&m_pstdstr[iIndex],
				spItem->pItemExt->siRegistMagic);

			m_pStr[iIndex]->SetStyle(UI_STR_TYPE_HALIGN, UISTYLE_STRING_ALIGNLEFT);
			m_pStr[iIndex]->SetColor(m_CGreen);
			iIndex++;
		}
		ERROR_EXCEPTION

		if( spItem->pItemExt->siRegistPoison != 0)
		{
			CGameBase::GetTextF(
				IDS_TOOLTIP_REGISTPOISON,
				&m_pstdstr[iIndex],
				spItem->pItemExt->siRegistPoison);

			m_pStr[iIndex]->SetStyle(UI_STR_TYPE_HALIGN, UISTYLE_STRING_ALIGNLEFT);
			m_pStr[iIndex]->SetColor(m_CGreen);
			iIndex++;
		}
		ERROR_EXCEPTION

		if( spItem->pItemExt->siRegistCurse != 0)
		{
			CGameBase::GetTextF(
				IDS_TOOLTIP_REGISTCURSE,
				&m_pstdstr[iIndex],
				spItem->pItemExt->siRegistCurse);

			m_pStr[iIndex]->SetStyle(UI_STR_TYPE_HALIGN, UISTYLE_STRING_ALIGNLEFT);
			m_pStr[iIndex]->SetColor(m_CGreen);
			iIndex++;
		}
		ERROR_EXCEPTION

		if( /*(spItem->pItemBasic->byAttachPoint == ITEM_LIMITED_EXHAUST) &&*/ spItem->pItemBasic->cNeedLevel+spItem->pItemExt->siNeedLevel > 1)
		{
			CGameBase::GetTextF(
				IDS_TOOLTIP_NEEDLEVEL,
				&m_pstdstr[iIndex],
				spItem->pItemBasic->cNeedLevel + spItem->pItemExt->siNeedLevel);

			if (SetTooltipTextColor(CGameBase::s_pPlayer->m_InfoBase.iLevel, spItem->pItemBasic->cNeedLevel + spItem->pItemExt->siNeedLevel))
				m_pStr[iIndex]->SetColor(m_CWhite);
			else
				m_pStr[iIndex]->SetColor(m_CRed);
			iIndex++;
		}
		ERROR_EXCEPTION

		if ((spItem->pItemBasic->byNeedRank + spItem->pItemExt->siNeedRank) > 0)
		{
			CGameBase::GetTextF(
				IDS_TOOLTIP_NEEDRANK,
				&m_pstdstr[iIndex],
				spItem->pItemBasic->byNeedRank + spItem->pItemExt->siNeedRank);

			m_pStr[iIndex]->SetStyle(UI_STR_TYPE_HALIGN, UISTYLE_STRING_ALIGNLEFT);

			if (SetTooltipTextColor(pInfoExt->iRank, spItem->pItemBasic->byNeedRank+spItem->pItemExt->siNeedRank))
				m_pStr[iIndex]->SetColor(m_CWhite);
			else
				m_pStr[iIndex]->SetColor(m_CRed);
			iIndex++;
		}
		ERROR_EXCEPTION

		if ((spItem->pItemBasic->byNeedTitle + spItem->pItemExt->siNeedTitle) > 0)
		{
			CGameBase::GetTextF(
				IDS_TOOLTIP_NEEDTITLE,
				&m_pstdstr[iIndex],
				// TODO: Use title name here.
				std::to_string(spItem->pItemBasic->byNeedTitle + spItem->pItemExt->siNeedTitle).c_str());

			m_pStr[iIndex]->SetStyle(UI_STR_TYPE_HALIGN, UISTYLE_STRING_ALIGNLEFT);

			if (SetTooltipTextColor(pInfoExt->iTitle, spItem->pItemBasic->byNeedTitle+spItem->pItemExt->siNeedTitle))
				m_pStr[iIndex]->SetColor(m_CWhite);
			else
				m_pStr[iIndex]->SetColor(m_CRed);

			iIndex++;
		}
		ERROR_EXCEPTION

		int iNeedValue;
		iNeedValue = spItem->pItemBasic->byNeedStrength;
		if (iNeedValue != 0)
			iNeedValue += spItem->pItemExt->siNeedStrength;
		if( iNeedValue > 0)
		{
			std::string szReduce;
			if (spItem->pItemExt->siNeedStrength < 0)
			{
				if (spItem->pItemExt->byMagicOrRare == ITEM_ATTRIB_UNIQUE
					|| spItem->pItemExt->byMagicOrRare == ITEM_ATTRIB_UPGRADE
					|| spItem->pItemExt->byMagicOrRare == ITEM_ATTRIB_UNIQUE_REVERSE
					|| spItem->pItemExt->byMagicOrRare == ITEM_ATTRIB_UPGRADE_REVERSE)
					CGameBase::GetText(IDS_TOOLTIP_REDUCE, &szReduce);
			}

			CGameBase::GetTextF(
				IDS_TOOLTIP_NEEDSTRENGTH,
				&m_pstdstr[iIndex],
				spItem->pItemBasic->byNeedStrength + spItem->pItemExt->siNeedStrength,
				szReduce.c_str());

			m_pStr[iIndex]->SetStyle(UI_STR_TYPE_HALIGN, UISTYLE_STRING_ALIGNLEFT);

			if (SetTooltipTextColor(pInfoExt->iStrength, spItem->pItemBasic->byNeedStrength+spItem->pItemExt->siNeedStrength))
				m_pStr[iIndex]->SetColor(m_CWhite);
			else
				m_pStr[iIndex]->SetColor(m_CRed);

			iIndex++;
		}
		ERROR_EXCEPTION

		iNeedValue = spItem->pItemBasic->byNeedStamina;
		if (iNeedValue != 0)
			iNeedValue += spItem->pItemExt->siNeedStamina;
		if( iNeedValue > 0)		
		{
			std::string szReduce;
			if (spItem->pItemExt->siNeedStamina < 0)
			{
				if (spItem->pItemExt->byMagicOrRare == ITEM_ATTRIB_UNIQUE
					|| spItem->pItemExt->byMagicOrRare == ITEM_ATTRIB_UPGRADE
					|| spItem->pItemExt->byMagicOrRare == ITEM_ATTRIB_UNIQUE_REVERSE
					|| spItem->pItemExt->byMagicOrRare == ITEM_ATTRIB_UPGRADE_REVERSE)
					CGameBase::GetText(IDS_TOOLTIP_REDUCE, &szReduce);
			}

			CGameBase::GetTextF(
				IDS_TOOLTIP_NEEDSTAMINA,
				&m_pstdstr[iIndex],
				spItem->pItemBasic->byNeedStamina + spItem->pItemExt->siNeedStamina,
				szReduce.c_str());

			m_pStr[iIndex]->SetStyle(UI_STR_TYPE_HALIGN, UISTYLE_STRING_ALIGNLEFT);

			if (SetTooltipTextColor(pInfoExt->iStamina, spItem->pItemBasic->byNeedStamina+spItem->pItemExt->siNeedStamina)) 
				m_pStr[iIndex]->SetColor(m_CWhite);
			else
				m_pStr[iIndex]->SetColor(m_CRed);

			iIndex++;
		}
		ERROR_EXCEPTION

		iNeedValue = spItem->pItemBasic->byNeedDexterity;
		if (iNeedValue != 0)
			iNeedValue += spItem->pItemExt->siNeedDexterity;
		if( iNeedValue > 0)		
		{
			std::string szReduce;
			if (spItem->pItemExt->siNeedDexterity < 0)
			{
				if (spItem->pItemExt->byMagicOrRare == ITEM_ATTRIB_UNIQUE
					|| spItem->pItemExt->byMagicOrRare == ITEM_ATTRIB_UPGRADE
					|| spItem->pItemExt->byMagicOrRare == ITEM_ATTRIB_UNIQUE_REVERSE
					|| spItem->pItemExt->byMagicOrRare == ITEM_ATTRIB_UPGRADE_REVERSE)
					CGameBase::GetText(IDS_TOOLTIP_REDUCE, &szReduce);
			}

			CGameBase::GetTextF(
				IDS_TOOLTIP_NEEDDEXTERITY,
				&m_pstdstr[iIndex],
				spItem->pItemBasic->byNeedDexterity + spItem->pItemExt->siNeedDexterity,
				szReduce.c_str());

			m_pStr[iIndex]->SetStyle(UI_STR_TYPE_HALIGN, UISTYLE_STRING_ALIGNLEFT);

			if (SetTooltipTextColor(pInfoExt->iDexterity, spItem->pItemBasic->byNeedDexterity+spItem->pItemExt->siNeedDexterity))
				m_pStr[iIndex]->SetColor(m_CWhite);
			else
				m_pStr[iIndex]->SetColor(m_CRed);

			iIndex++;
		}
		ERROR_EXCEPTION

		iNeedValue = spItem->pItemBasic->byNeedInteli;
		if (iNeedValue != 0)
			iNeedValue += spItem->pItemExt->siNeedInteli;
		if( iNeedValue > 0)			
		{
			std::string szReduce;
			if (spItem->pItemExt->siNeedInteli < 0)
			{
				if (spItem->pItemExt->byMagicOrRare == ITEM_ATTRIB_UNIQUE
					|| spItem->pItemExt->byMagicOrRare == ITEM_ATTRIB_UPGRADE
					|| spItem->pItemExt->byMagicOrRare == ITEM_ATTRIB_UNIQUE_REVERSE
					|| spItem->pItemExt->byMagicOrRare == ITEM_ATTRIB_UPGRADE_REVERSE)
					CGameBase::GetText(IDS_TOOLTIP_REDUCE, &szReduce);
			}

			CGameBase::GetTextF(
				IDS_TOOLTIP_NEEDINTELLI,
				&m_pstdstr[iIndex],
				spItem->pItemBasic->byNeedInteli + spItem->pItemExt->siNeedInteli,
				szReduce.c_str());

			m_pStr[iIndex]->SetStyle(UI_STR_TYPE_HALIGN, UISTYLE_STRING_ALIGNLEFT);

			if (SetTooltipTextColor(pInfoExt->iIntelligence, spItem->pItemBasic->byNeedInteli+spItem->pItemExt->siNeedInteli))
				m_pStr[iIndex]->SetColor(m_CWhite);
			else
				m_pStr[iIndex]->SetColor(m_CRed);

			iIndex++;
		}
		ERROR_EXCEPTION

		iNeedValue = spItem->pItemBasic->byNeedMagicAttack;
		if (iNeedValue != 0)
			iNeedValue += spItem->pItemExt->siNeedMagicAttack;
		if( iNeedValue > 0)			
		{
			std::string szReduce;
			if (spItem->pItemExt->siNeedMagicAttack < 0)
			{
				if (spItem->pItemExt->byMagicOrRare == ITEM_ATTRIB_UNIQUE
					|| spItem->pItemExt->byMagicOrRare == ITEM_ATTRIB_UPGRADE
					|| spItem->pItemExt->byMagicOrRare == ITEM_ATTRIB_UNIQUE_REVERSE
					|| spItem->pItemExt->byMagicOrRare == ITEM_ATTRIB_UPGRADE_REVERSE)
					CGameBase::GetText(IDS_TOOLTIP_REDUCE, &szReduce);
			}

			CGameBase::GetTextF(
				IDS_TOOLTIP_NEEDMAGICATTACK,
				&m_pstdstr[iIndex],
				spItem->pItemBasic->byNeedMagicAttack + spItem->pItemExt->siNeedMagicAttack,
				szReduce.c_str());

			m_pStr[iIndex]->SetStyle(UI_STR_TYPE_HALIGN, UISTYLE_STRING_ALIGNLEFT);

			if (SetTooltipTextColor(pInfoExt->iMagicAttak, spItem->pItemBasic->byNeedMagicAttack+spItem->pItemExt->siNeedMagicAttack))
				m_pStr[iIndex]->SetColor(m_CWhite);
			else
				m_pStr[iIndex]->SetColor(m_CRed);

			iIndex++;
		}
		ERROR_EXCEPTION

exceptions:;

		if (bPrice)
		{
			if (bBuy)	
			{
				CGameBase::GetTextF(
					IDS_TOOLTIP_BUY_PRICE,
					&m_pstdstr[iIndex],
					std::to_string(spItem->pItemBasic->iPrice* spItem->pItemExt->siPriceMultiply).c_str());

				m_pStr[iIndex]->SetStyle(UI_STR_TYPE_HALIGN, UISTYLE_STRING_ALIGNLEFT);

				if (SetTooltipTextColor(pInfoExt->iGold, spItem->pItemBasic->iPrice*spItem->pItemExt->siPriceMultiply))
					m_pStr[iIndex]->SetColor(m_CWhite);
				else
					m_pStr[iIndex]->SetColor(m_CRed);
			}
			else
			{	
				int iSellPrice = (spItem->pItemBasic->iPrice*spItem->pItemExt->siPriceMultiply/6);
				if (iSellPrice < 1)
					iSellPrice = 1;

				CGameBase::GetTextF(
					IDS_TOOLTIP_SELL_PRICE,
					&m_pstdstr[iIndex],
					std::to_string(iSellPrice).c_str());

				m_pStr[iIndex]->SetStyle(UI_STR_TYPE_HALIGN, UISTYLE_STRING_ALIGNLEFT);
				m_pStr[iIndex]->SetColor(m_CWhite);
			}

			iIndex++;			
		}

		for (int i = iIndex; i < MAX_TOOLTIP_COUNT; i++)
			m_pstdstr[iIndex].clear();
	}

	return iIndex;	// 임시..	반드시 1보다 크다..
}

void CUIImageTooltipDlg::DisplayTooltipsEnable(int xpos, int ypos, __IconItemSkill* spItem, bool bPrice, bool bBuy)
{
	if ( !spItem ) return;

	if ( !IsVisible() ) SetVisible(true);

	if ( (m_iPosXBack != xpos) || (m_iPosYBack != ypos) )
	{
		int iNum = CalcTooltipStringNumAndWrite(spItem, bPrice, bBuy);
		SetPosSomething(xpos, ypos, iNum);
	}

	Render();
}

