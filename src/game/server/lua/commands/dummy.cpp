/* (c) MAP94 and Patafix. See www.n-lvl.com/ndc/nclient/ for more information. */
#include "../lua.h"

int CLuaFile::DummyCreate(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    if (!lua_isnumber(L, 1))
        return 0;

    if(lua_tointeger(L, 1) >= 0 && lua_tointeger(L, 1) < MAX_CLIENTS && pSelf->m_pServer->m_apPlayers[lua_tointeger(L, 1)] && pSelf->m_pServer->m_apPlayers[lua_tointeger(L, 1)]->GetCharacter())
    {
        lua_pushnumber(L, pSelf->m_pServer->m_apPlayers[lua_tointeger(L, 1)]->GetCharacter()->m_Pos.x);
        lua_pushnumber(L, pSelf->m_pServer->m_apPlayers[lua_tointeger(L, 1)]->GetCharacter()->m_Pos.y);
        return 2;
    }

    lua_pushnumber(L, 0);
    lua_pushnumber(L, 0);
    return 2;
}

int CLuaFile::IsDummy(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    if (!lua_isnumber(L, 1))
        return 0;

    if(lua_tointeger(L, 1) >= 0 && lua_tointeger(L, 1) < MAX_CLIENTS && pSelf->m_pServer->m_apPlayers[lua_tointeger(L, 1)])
    {
        lua_pushboolean(L, pSelf->m_pServer->m_apPlayers[lua_tointeger(L, 1)]->m_IsDummy);
        return 1;
    }
    return 0;
}
