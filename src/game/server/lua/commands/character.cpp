/* (c) MAP94 and Patafix. See www.n-lvl.com/ndc/nclient/ for more information. */
#include "../lua.h"


int CLuaFile::GetCharacterPos(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)(int)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    if (!lua_isnumber(L, 1))
        return 0;
	
	 if(pSelf->m_pServer->m_apPlayers[lua_tointeger(L, 1)] && pSelf->m_pServer->m_apPlayers[lua_tointeger(L, 1)]->GetCharacter())
		{	
				lua_pushnumber(L, pSelf->m_pServer->m_World.m_Core.m_apCharacters[lua_tointeger(L, 1)]->m_Pos.x);
				lua_pushnumber(L, pSelf->m_pServer->m_World.m_Core.m_apCharacters[lua_tointeger(L, 1)]->m_Pos.y);
			//lua_pushnumber(L, pSelf->m_pServer->m_World.m_Core.m_aCharacters[lua_tointeger(L, 1)].m_Vel.y);
		}
	
    lua_pushnumber(L, 0);
    lua_pushnumber(L, 0);
    return 2;
}
int CLuaFile::SetCharacterPos(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)(int)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    if (lua_isnumber(L, 1) && lua_isnumber(L, 2) && lua_isnumber(L, 3))
    {
		int Id = lua_tointeger(L, 1);
		if(Id < 0 || Id > MAX_CLIENTS)
			return 0;
		
		if(pSelf->m_pServer->m_apPlayers[Id] && pSelf->m_pServer->m_apPlayers[Id]->GetCharacter())
		{		
			pSelf->m_pServer->m_World.m_Core.m_apCharacters[Id]->m_Pos.x = lua_tointeger(L, 2);
			pSelf->m_pServer->m_World.m_Core.m_apCharacters[Id]->m_Pos.y = lua_tointeger(L, 3);
			return 1;
		}
	}
	return 0;
}

int CLuaFile::GetCharacterVel(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)(int)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    if (!lua_isnumber(L, 1))
        return 0;
		
    if(pSelf->m_pServer->m_apPlayers[lua_tointeger(L, 1)] && pSelf->m_pServer->m_apPlayers[lua_tointeger(L, 1)]->GetCharacter())
		{	
			lua_pushnumber(L, pSelf->m_pServer->m_World.m_Core.m_apCharacters[lua_tointeger(L, 1)]->m_Vel.x);
			lua_pushnumber(L, pSelf->m_pServer->m_World.m_Core.m_apCharacters[lua_tointeger(L, 1)]->m_Vel.y);
			
			//lua_pushnumber(L, pSelf->m_pServer->m_World.m_Core.m_aCharacters[lua_tointeger(L, 1)].m_Vel.y);
		}	
    lua_pushnumber(L, 0);
    lua_pushnumber(L, 0);
    return 2;
}
int CLuaFile::SetCharacterVel(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)(int)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    if (lua_isnumber(L, 1) && lua_isnumber(L, 2) && lua_isnumber(L, 3))
    {
		int Id = lua_tointeger(L, 1);
		if(Id < 0 || Id > MAX_CLIENTS)
			return 0;
		
		if(pSelf->m_pServer->m_apPlayers[Id] && pSelf->m_pServer->m_apPlayers[Id]->GetCharacter())
		{		
			pSelf->m_pServer->m_World.m_Core.m_apCharacters[Id]->m_Vel.x = lua_tointeger(L, 2);
			pSelf->m_pServer->m_World.m_Core.m_apCharacters[Id]->m_Vel.y = lua_tointeger(L, 3);
			return 1;
		}
	}
	return 0;
}

int CLuaFile::Emote(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)(int)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    if (lua_isnumber(L, 1) && lua_isnumber(L, 2) && lua_isnumber(L, 3))
    {
       if(pSelf->m_pServer->m_apPlayers[lua_tointeger(L, 1)] && pSelf->m_pServer->m_apPlayers[lua_tointeger(L, 1)]->GetCharacter())
	   {
			pSelf->m_pServer->m_apPlayers[lua_tointeger(L, 1)]->GetCharacter()->SetEmote(lua_tointeger(L, 2), lua_tointeger(L, 3));
			return 1;
	   }
    }
    return 0;
}