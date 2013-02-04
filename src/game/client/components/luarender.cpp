#include "luarender.h"

CLuaRender::CLuaRender()
{
	m_Level = 1;
	//str_format(m_aEventString, sizeof(m_aEventString), "OnRenderLevel%i", Level);
}

void CLuaRender::OnRender()
{
	if(m_Level> 20)
		m_Level=1;
    int EventID=m_pClient->m_pLua->m_pEventListener->CreateEventStack();
	m_pClient->m_pLua->m_pEventListener->GetParameters(EventID)->FindFree()->Set(m_Level++);
    m_pClient->m_pLua->m_pEventListener->OnEvent("OnRender");
}
