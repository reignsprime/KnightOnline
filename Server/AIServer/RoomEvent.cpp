﻿// RoomEvent.cpp: implementation of the CRoomEvent class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "server.h"
#include "RoomEvent.h"
#include "ServerDlg.h"
#include "define.h"
//#include "Npc.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
extern CRITICAL_SECTION g_region_critical;

CRoomEvent::CRoomEvent()
{
	m_iZoneNumber = 0;
	m_sRoomNumber = 0;
	m_byStatus = 1;
	m_iInitMinX = 0;
	m_iInitMinZ = 0;
	m_iInitMaxX = 0;
	m_iInitMaxZ = 0;
	m_iEndMinX = 0;
	m_iEndMinZ = 0;
	m_iEndMaxX = 0;
	m_iEndMaxZ = 0;
	m_byCheck = 0;
	m_byRoomType = 0;
	m_pMain = (CServerDlg*) AfxGetApp()->GetMainWnd();

	Initialize();
}

CRoomEvent::~CRoomEvent()
{
	if (!m_mapRoomNpcArray.IsEmpty())
		m_mapRoomNpcArray.DeleteAllData();
}

void CRoomEvent::Initialize()
{
	m_fDelayTime = 0.0f;
	m_byLogicNumber = 1;

	for (int i = 0; i < MAX_CHECK_EVENT; i++)
	{
		m_Logic[i].sNumber = 0;
		m_Logic[i].sOption_1 = 0;
		m_Logic[i].sOption_2 = 0;
		m_Exec[i].sNumber = 0;
		m_Exec[i].sOption_1 = 0;
		m_Exec[i].sOption_2 = 0;
	}
}

void CRoomEvent::MainRoom(float fcurtime)
{
	// 조건 검색먼저 해야 겠지..
	BOOL bCheck = FALSE, bRunCheck = FALSE;
	char notify[50] = {};

	int event_num = m_Logic[m_byLogicNumber - 1].sNumber;

	bCheck = CheckEvent(event_num, fcurtime);
	if (bCheck)
	{
		event_num = m_Exec[m_byLogicNumber - 1].sNumber;
		bRunCheck = RunEvent(event_num);
		if (bRunCheck)
		{
			//wsprintf(notify, "** 알림 : [%d]방이 클리어 되어습니다. **", m_sRoomNumber);
			//m_pMain->SendSystemMsg( notify, m_iZoneNumber, PUBLIC_CHAT, SEND_ALL);
			m_byStatus = 3;
		}
	}
}

BOOL CRoomEvent::CheckEvent(int event_num, float fcurtime)
{
	int nMinute = 0, nOption_1 = 0, nOption_2 = 0;
	CNpc* pNpc = nullptr;
	BOOL bRetValue = FALSE;

	if (m_byLogicNumber == 0
		|| m_byLogicNumber > MAX_CHECK_EVENT)
	{
		TRACE(_T("### Check Event Fail :: array overflow = %d ###\n"), m_byLogicNumber);
		return FALSE;
	}

	switch (event_num)
	{
		// 특정 몬스터를 죽이는 경우
		case 1:
			nOption_1 = m_Logic[m_byLogicNumber - 1].sOption_1;
			pNpc = GetNpcPtr(nOption_1);
			if (pNpc != nullptr)
			{
				if (pNpc->m_byChangeType == 100)
					return TRUE;
			}
			else
			{
				TRACE(_T("### CheckEvent Error : monster nid = %d, logic=%d ###\n"), nOption_1, m_byLogicNumber);
			}
			//TRACE(_T("---Check Event : monster dead = %d \n"), nMonsterNid);
			break;

		// 모든 몬스터를 죽여라
		case 2:
			bRetValue = CheckMonsterCount(0, 0, 3);
			if (bRetValue)
			{
				TRACE(_T("모든 몬스터를 죽여라 죽임\n"));
				return TRUE;
			}
			break;

		// 몇분동안 버텨라
		case 3:
			nMinute = m_Logic[m_byLogicNumber - 1].sOption_1;
			nMinute = nMinute * 60;								// 분을 초로 변환

			// 제한시간 종료
			if (fcurtime >= m_fDelayTime + nMinute)
			{
				TRACE(_T("---Check Event : curtime=%.2f, starttime=%.2f \n"), fcurtime, m_fDelayTime);
				TRACE(_T("버티기 성공\n"));
				return TRUE;
			}
			//TRACE(_T("---Check Event : curtime=%.2f, starttime=%.2f \n"), fcurtime, m_fDelayTime);
			break;

		// 목표지점까지 이동
		case 4:
			break;

		// 특정몬스터를 옵션2의 마리수 만큼 죽여라
		case 5:
			nOption_1 = m_Logic[m_byLogicNumber - 1].sOption_1;
			nOption_2 = m_Logic[m_byLogicNumber - 1].sOption_2;
			bRetValue = CheckMonsterCount(nOption_1, nOption_2, 1);
			if (bRetValue)
			{
				TRACE(_T("특정몬스터(%d)를 %d마리 죽임\n"), nOption_1, nOption_2);
				return TRUE;
			}
			break;

		default:
			TRACE(_T("### Check Event Fail :: event number = %d ###\n"), event_num);
			break;
	}

	return FALSE;
}

BOOL  CRoomEvent::RunEvent(int event_num)
{
	char notify[50] = {};
	CNpc* pNpc = nullptr;
	int nOption_1 = 0, nOption_2 = 0;
	BOOL bRetValue = FALSE;
	switch (event_num)
	{
		// 다른 몬스터의 출현
		case 1:
			nOption_1 = m_Exec[m_byLogicNumber - 1].sOption_1;
			pNpc = GetNpcPtr(nOption_1);
			if (pNpc != nullptr)
			{
				pNpc->m_byChangeType = 3;	// 몬스터 출현해주세여...
				pNpc->SetLive(&m_pMain->m_Iocport);
			}
			else
			{
				TRACE(_T("### RunEvent Error : 몬스터 출현 할 수 없당 = %d, logic=%d ###\n"), nOption_1, m_byLogicNumber);
			}

			// 방이 클리어
			if (m_byCheck == m_byLogicNumber)
				return TRUE;
			
			m_byLogicNumber++;
			break;

		// 문이 열림
		case 2:
			nOption_1 = m_Exec[m_byLogicNumber - 1].sOption_1;
			pNpc = GetNpcPtr(nOption_1);
			if (pNpc == nullptr)
			{
				TRACE(_T("### RunEvent Error : 문 담당 몬스터 출현 할 수 없당 = %d, logic=%d ###\n"), nOption_1, m_byLogicNumber);
			}

			//wsprintf(notify, "** 알림 : [%d] 문이 열립니다 **", m_sRoomNumber);
			//m_pMain->SendSystemMsg( notify, m_iZoneNumber, PUBLIC_CHAT, SEND_ALL);

			// 방이 클리어
			if (m_byCheck == m_byLogicNumber)
				return TRUE;
			
			m_byLogicNumber++;
			break;

		// 다른 몬스터로 변환
		case 3:
			// 방이 클리어
			if (m_byCheck == m_byLogicNumber)
				return TRUE;
			break;

		// 특정몬스터 옵션2의 마리수만큼 출현
		case 4:
			nOption_1 = m_Exec[m_byLogicNumber - 1].sOption_1;
			nOption_2 = m_Exec[m_byLogicNumber - 1].sOption_2;
			bRetValue = CheckMonsterCount(nOption_1, nOption_2, 2);

			//wsprintf(notify, "** 알림 : [%d, %d] 몬스터 출현 **", nOption_1, nOption_2);
			//m_pMain->SendSystemMsg( notify, m_iZoneNumber, PUBLIC_CHAT, SEND_ALL);

			// 방이 클리어
			if (m_byCheck == m_byLogicNumber)
				return TRUE;
			
			m_byLogicNumber++;
			break;

		// 특정몬스터 옵션2의 마리수만큼 출현
		case 100:
			nOption_1 = m_Exec[m_byLogicNumber - 1].sOption_1;
			nOption_2 = m_Exec[m_byLogicNumber - 1].sOption_2;

			TRACE(_T("RunEvent - room=%d, option1=%d, option2=%d\n"), m_sRoomNumber, nOption_1, nOption_2);
			if (nOption_1 != 0)
				EndEventSay(nOption_1, nOption_2);

			// 방이 클리어
			if (m_byCheck == m_byLogicNumber)
				return TRUE;
			
			m_byLogicNumber++;
			break;

		default:
			TRACE(_T("### RunEvent Fail :: event number = %d ###\n"), event_num);
			break;
	}

	return FALSE;
}

CNpc* CRoomEvent::GetNpcPtr(int sid)
{
	CNpc* pNpc = nullptr;
	int* pIDList = nullptr;
	int nMonsterid = 0, count = 0;

	EnterCriticalSection(&g_region_critical);

	int nMonster = m_mapRoomNpcArray.GetSize();
	if (nMonster == 0)
	{
		TRACE(_T("### RoomEvent-GetNpcPtr() : monster empty ###\n"));
		LeaveCriticalSection(&g_region_critical);
		return nullptr;
	}

	auto Iter1 = m_mapRoomNpcArray.begin();
	auto Iter2 = m_mapRoomNpcArray.end();

	pIDList = new int[nMonster];
	for (; Iter1 != Iter2; Iter1++)
	{
		nMonsterid = *((*Iter1).second);
		pIDList[count] = nMonsterid;
		count++;
	}
	LeaveCriticalSection(&g_region_critical);

	for (int i = 0; i < nMonster; i++)
	{
		nMonsterid = pIDList[i];
		if (nMonsterid < 0)
			continue;

		pNpc = m_pMain->m_arNpc.GetData(nMonsterid);
		if (pNpc == nullptr)
			continue;

		if (pNpc->m_sSid == sid)
		{
			delete[] pIDList;
			pIDList = nullptr;
			return pNpc;
		}
	}

	delete[] pIDList;
	pIDList = nullptr;

	return nullptr;
}

BOOL CRoomEvent::CheckMonsterCount(int sid, int count, int type)
{
	int nMonsterCount = 0;
	CNpc* pNpc = nullptr;
	int* pIDList = nullptr;
	int nMonsterid = 0, nTotalMonster = 0;
	BOOL bRetValue = FALSE;

	EnterCriticalSection(&g_region_critical);

	int nMonster = m_mapRoomNpcArray.GetSize();
	if (nMonster == 0)
	{
		TRACE(_T("### RoomEvent-GetNpcPtr() : monster empty ###\n"));
		LeaveCriticalSection(&g_region_critical);
		return FALSE;
	}

	auto Iter1 = m_mapRoomNpcArray.begin();
	auto Iter2 = m_mapRoomNpcArray.end();

	pIDList = new int[nMonster];
	for (; Iter1 != Iter2; Iter1++)
	{
		nMonsterid = *((*Iter1).second);
		pIDList[nTotalMonster] = nMonsterid;
		nTotalMonster++;
	}
	LeaveCriticalSection(&g_region_critical);

	for (int i = 0; i < nMonster; i++)
	{
		nMonsterid = pIDList[i];
		if (nMonsterid < 0)
			continue;

		pNpc = m_pMain->m_arNpc.GetData(nMonsterid);
		if (pNpc == nullptr)
			continue;

		if (type == 4)
		{
			if (pNpc->m_byRegenType == 2)
				pNpc->m_byRegenType = 0;

			pNpc->m_byChangeType = 0;
		}
		// 모든 몬스터를 죽었는지를 판단
		else if (type == 3)
		{
			if (pNpc->m_byDeadType == 100)
				nMonsterCount++;

			if (nMonsterCount == nMonster)
				bRetValue = TRUE;
		}
		else if (pNpc->m_sSid == sid)
		{
			// 특정 몬스터가 마리수 만큼 죽었는지를 판단
			if (type == 1)
			{
				if (pNpc->m_byChangeType == 100)
					nMonsterCount++;

				if (nMonsterCount == count)
					bRetValue = TRUE;
			}
			// 특정 몬스터를 마리수 만큼 출현 시켜라,,
			else if (type == 2)
			{
				pNpc->m_byChangeType = 3;
				nMonsterCount++;

				if (nMonsterCount == count)
					bRetValue = TRUE;
			}
		}
	}

	delete[] pIDList;
	pIDList = nullptr;

	return bRetValue;
}

void CRoomEvent::InitializeRoom()
{
	m_byStatus = 1;
	m_fDelayTime = 0.0f;
	m_byLogicNumber = 1;

	CheckMonsterCount(0, 0, 4);	// 몬스터의 m_byChangeType=0으로 초기화 
}

void CRoomEvent::EndEventSay(int option1, int option2)
{
	char notify[256] = {};
	char send_buff[128] = {};
	int send_index = 0;

	std::string buff;

	switch (option1)
	{
		// 클리어 상태에서 클라이언트에 내려줄 내용
		case 1:
			switch (option2)
			{
				case 1:
					::_LoadStringFromResource(IDS_KARUS_CATCH_1, buff);
					sprintf(notify, buff.c_str());
					break;

				case 2:
					::_LoadStringFromResource(IDS_KARUS_CATCH_2, buff);
					sprintf(notify, buff.c_str());
					break;

				case 11:
					::_LoadStringFromResource(IDS_ELMORAD_CATCH_1, buff);
					sprintf(notify, buff.c_str());
					break;

				case 12:
					::_LoadStringFromResource(IDS_ELMORAD_CATCH_2, buff);
					sprintf(notify, buff.c_str());
					break;
			}

			m_pMain->SendSystemMsg(notify, m_iZoneNumber, WAR_SYSTEM_CHAT, SEND_ALL);
			break;

		// 클리어 상태에서 클라이언트에 내려줄 내용와 적국으로 갈 수 있는 이벤트 존 열어주기
		case 2:
			if (option2 == KARUS_ZONE)
			{
				::_LoadStringFromResource(IDS_KARUS_PATHWAY, buff);
				sprintf(notify, buff.c_str());

				SetByte(send_buff, AG_BATTLE_EVENT, send_index);
				SetByte(send_buff, BATTLE_MAP_EVENT_RESULT, send_index);
				SetByte(send_buff, KARUS_ZONE, send_index);
				m_pMain->Send(send_buff, send_index, m_iZoneNumber);
			}
			else if (option2 == ELMORAD_ZONE)
			{
				::_LoadStringFromResource(IDS_ELMORAD_PATHWAY, buff);
				sprintf(notify, buff.c_str());

				SetByte(send_buff, AG_BATTLE_EVENT, send_index);
				SetByte(send_buff, BATTLE_MAP_EVENT_RESULT, send_index);
				SetByte(send_buff, ELMORAD_ZONE, send_index);
				m_pMain->Send(send_buff, send_index, m_iZoneNumber);
			}

			m_pMain->SendSystemMsg(notify, m_iZoneNumber, WAR_SYSTEM_CHAT, SEND_ALL);
			break;

		// 클리어 상태에서 클라이언트에 내려줄 내용와 승리팀을 알려준다.
		case 3:
			if (option2 == KARUS_ZONE)
			{
				SetByte(send_buff, AG_BATTLE_EVENT, send_index);
				SetByte(send_buff, BATTLE_EVENT_RESULT, send_index);
				SetByte(send_buff, KARUS_ZONE, send_index);
				m_pMain->Send(send_buff, send_index, m_iZoneNumber);
			}
			else if (option2 == ELMORAD_ZONE)
			{
				SetByte(send_buff, AG_BATTLE_EVENT, send_index);
				SetByte(send_buff, BATTLE_EVENT_RESULT, send_index);
				SetByte(send_buff, ELMORAD_ZONE, send_index);
				m_pMain->Send(send_buff, send_index, m_iZoneNumber);
			}
			break;
	}
}
