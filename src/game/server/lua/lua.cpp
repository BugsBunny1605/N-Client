/* (c) MAP94 and Patafix. See www.n-lvl.com/ndc/nclient/ for more information. */
#include "lua.h"
#include "string.h"/*
#include "components/flow.h"
#include "components/particles.h"

#include <game/generated/client_data.h>
#include <game/client/lineinput.h>
#include <game/client/components/menus.h>
#include <game/client/components/chat.h>*/

#include <engine/serverbrowser.h>
#include <engine/textrender.h>
#include <engine/sound.h>
void CLua::Tick()
{
    for (int i = 0; i < MAX_LUA_FILES; i++)
    {
        if (m_aLuaFiles[i].GetScriptName()[0] == 0 && m_pServer->m_pLuaCore->GetFileName(i)[0])
        {
            char aFilename[256];
            str_format(aFilename, sizeof(aFilename), "lua/%s", m_pServer->m_pLuaCore->GetFileName(i));
            m_aLuaFiles[i].Init(aFilename);
        }
        else if (m_aLuaFiles[i].GetScriptName()[0] && m_pServer->m_pLuaCore->GetFileName(i)[0] == 0)
            m_aLuaFiles[i].Close();
        else if (m_aLuaFiles[i].GetScriptName()[0])
            m_aLuaFiles[i].Tick();
    }
}
void CLua::TickDefered()
{
    for (int i = 0; i < MAX_LUA_FILES; i++)
    {
       if (m_aLuaFiles[i].GetScriptName()[0] && m_pServer->m_pLuaCore->GetFileName(i)[0] == 0)
            m_aLuaFiles[i].Close();
        else if (m_aLuaFiles[i].GetScriptName()[0])
            m_aLuaFiles[i].TickDefered();
    }
}
void CLua::PostTick()
{
    for (int i = 0; i < MAX_LUA_FILES; i++)
    {
		if (m_aLuaFiles[i].GetScriptName()[0] && m_pServer->m_pLuaCore->GetFileName(i)[0] == 0)
            m_aLuaFiles[i].Close();
        else if (m_aLuaFiles[i].GetScriptName()[0])
            m_aLuaFiles[i].PostTick();
    }
}

void CLua::End()
{
    for (int i = 0; i < MAX_LUA_FILES; i++)
    {
        m_aLuaFiles[i].End();
    }
}

CLua::CLua(CGameContext *pServer)
{
    mem_zero(this, sizeof(CLua));
    Close();

    m_pServer = pServer;

    for (int i = 0; i < MAX_LUA_FILES; i++)
    {
        m_aLuaFiles[i].m_pServer = pServer;
        m_aLuaFiles[i].m_pLuaHandler = this;
    }
}

CLua::~CLua()
{
    Close();
}

void CLua::Close()
{
    End();
}

int CLua::GetFileId(char *pFilename)
{
    for (int i = 0; i < MAX_LUA_FILES; i++)
    {
        if (str_comp(m_aLuaFiles[i].GetScriptName(), pFilename) == 0)
            return i;
    }
    return -1;
}

void CLua::ConfigClose(char *pFilename)
{
    int Id = GetFileId(pFilename);
    if (Id == -1)
        return;
    m_aLuaFiles[Id].ConfigClose();
}
