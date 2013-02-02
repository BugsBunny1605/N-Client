#include "luainput.h"

CLuaInput::CLuaInput(void)
{
	m_Level = 1;
	//str_format(m_aEventString, sizeof(m_aEventString), "OnKeyLevel%i", Level);
}

bool CLuaInput::OnInput(IInput::CEvent Event)
{
	if(m_Level > 11)
		m_Level = 1;
    int EventID = m_pClient->m_pLua->m_pEventListener->CreateEventStack();
    m_pClient->m_pLua->m_pEventListener->GetParameters(EventID)->FindFree()->Set(Event.m_Key);
    m_pClient->m_pLua->m_pEventListener->GetParameters(EventID)->FindFree()->Set(Event.m_Unicode);
    m_pClient->m_pLua->m_pEventListener->GetParameters(EventID)->FindFree()->Set(Event.m_Flags);
	m_pClient->m_pLua->m_pEventListener->GetParameters(EventID)->FindFree()->Set(m_Level++);
    m_pClient->m_pLua->m_pEventListener->OnEvent("OnInput");

    if (m_pClient->m_pLua->m_pEventListener->GetReturns(EventID)->m_aVars[0].IsNumeric() && m_pClient->m_pLua->m_pEventListener->GetReturns(EventID)->m_aVars[0].GetInteger() == 1)
        return true;
    return false;
}

