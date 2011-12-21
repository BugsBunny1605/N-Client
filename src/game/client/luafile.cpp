/* (c) MAP94. See www.n-lvl.com/ndc/nclient/ for more information. */
#include "lua.h"
#include "components/flow.h"
#include "components/particles.h"
#include "components/menus.h"

#include <game/generated/client_data.h>

#include <engine/serverbrowser.h>
#include <engine/textrender.h>
#include <engine/sound.h>
#include <engine/graphics.h>
#include <engine/storage.h>

#include <game/client/lineinput.h>
#include <game/client/components/menus.h>
#include <game/client/components/chat.h>
#include <game/client/components/countryflags.h>
#include <game/client/components/skins.h>

CLuaFile::CLuaFile()
{
    mem_zero(this, sizeof(CLuaFile));
    Close();
}

CLuaFile::~CLuaFile()
{
    #ifndef CONF_PLATFORM_MACOSX
    End();
    if (m_pLua)
        lua_close(m_pLua);
    m_pLua = 0;
    #endif
}

void CLuaFile::UiTick()
{
    if (!g_Config.m_ClLua)
        return;
    for (int i = 0; i < LUAMAXUIELEMENTS; i++)
    {
        if (m_aUiElements[i].m_Used)
        {
            m_aUiElements[i].Tick();
        }
    }
}

void CLuaFile::Tick()
{
    if (!g_Config.m_ClLua)
        return;

    ErrorFunc(m_pLua);

    FunctionPrepare("Tick");
    PushInteger((int)(time_get() * 10 / time_freq()));
    FunctionExec();

    ErrorFunc(m_pLua);
}

void CLuaFile::End()
{
    if (m_pLua == 0)
        return;

    //try to call the end function
    //Maybe the lua file need to save data eg. a ConfigFile
    FunctionExec("end");
}

int CLuaFile::Panic(lua_State *L)
{
    dbg_break();
    return 0;
}

void CLuaFile::Init(const char *pFile)
{
    //close first
    Close();
    //init ui
    for (int i = 0; i < LUAMAXUIELEMENTS; i++)
        m_aUiElements[i].m_pClient = m_pClient;

    str_copy(m_aFilename, pFile, sizeof(m_aFilename));

    m_pLua = lua_open();
    luaL_openlibs(m_pLua);

    lua_atpanic(m_pLua, &Panic);

    //include
    lua_register(m_pLua, "Include", this->Include);

    //Settings
    lua_register(m_pLua, "SetScriptUseSettingPage", this->SetScriptUseSettingPage);
    lua_register(m_pLua, "SetScriptTitle", this->SetScriptTitle);
    lua_register(m_pLua, "SetScriptInfo", this->SetScriptInfo);

    //Eventlistener stuff
    lua_register(m_pLua, "AddEventListener", this->AddEventListener);
    lua_register(m_pLua, "RemoveEventListener", this->RemoveEventListener);

    //menu browser
    lua_register(m_pLua, "SetMenuBrowserGameTypeColor", this->SetMenuBrowserGameTypeColor);
    lua_register(m_pLua, "GetMenuBrowserGameTypeName", this->GetMenuBrowserGameTypeName);

    //menu
    lua_register(m_pLua, "MenuActiv", this->MenuActiv);
    lua_register(m_pLua, "MenuGameActiv", this->MenuGameActiv);
    lua_register(m_pLua, "MenuPlayersActiv", this->MenuPlayersActiv);
    lua_register(m_pLua, "MenuServerInfoActiv", this->MenuServerInfoActiv);
    lua_register(m_pLua, "MenuCallVoteActiv", this->MenuCallVoteActiv);
    lua_register(m_pLua, "MenuServersActiv", this->MenuServersActiv);
    lua_register(m_pLua, "MenuMusicActiv", this->MenuMusicActiv);
    lua_register(m_pLua, "MenuDemosActiv", this->MenuDemosActiv);

    //mouse and keyboard
    lua_register(m_pLua, "GetMousePosMenu", this->GetMousePosMenu);
    lua_register(m_pLua, "SetMouseModeRelativ", this->SetMouseModeRelativ);
    lua_register(m_pLua, "SetMouseModeAbsolute", this->SetMouseModeAbsolute);

    //scoreboard
    lua_register(m_pLua, "ScoreboardAbortRender", this->ScoreboardAbortRender);

    //sendinfo
    lua_register(m_pLua, "SendPlayerInfo", this->SendPlayerInfo);

    //Chat
    lua_register(m_pLua, "ChatGetText", this->ChatGetText);
    lua_register(m_pLua, "ChatGetClientID", this->ChatGetClientID);
    lua_register(m_pLua, "ChatGetTeam", this->ChatGetTeam);
    lua_register(m_pLua, "ChatHide", this->ChatHide);

    //Kill
    lua_register(m_pLua, "KillGetKillerID", this->KillGetKillerID);
    lua_register(m_pLua, "KillGetVictimID", this->KillGetVictimID);
    lua_register(m_pLua, "KillGetWeapon", this->KillGetWeapon);

    //Player
    lua_register(m_pLua, "GetPlayerName", this->GetPlayerName);
    lua_register(m_pLua, "GetPlayerClan", this->GetPlayerClan);
    lua_register(m_pLua, "GetPlayerCountry", this->GetPlayerCountry);
    lua_register(m_pLua, "GetPlayerScore", this->GetPlayerScore);
    lua_register(m_pLua, "GetPlayerPing", this->GetPlayerPing);
    lua_register(m_pLua, "GetPlayerTeam", this->GetPlayerTeam);
    lua_register(m_pLua, "GetPlayerSkin", this->GetPlayerSkin);
    lua_register(m_pLua, "GetPlayerColorFeet", this->GetPlayerColorFeet);
    lua_register(m_pLua, "GetPlayerColorBody", this->GetPlayerColorBody);
    lua_register(m_pLua, "GetPlayerColorSkin", this->GetPlayerColorSkin);

    //Emote
    lua_register(m_pLua, "Emote", this->Emote);

    //lua_register(m_pLua, "CreateParticleEmitter", CreateParticleEmitter); //particleemitter gibt es noch nicht
    lua_register(m_pLua, "CreateParticle", this->CreateParticle);

    lua_register(m_pLua, "GetFlow", this->GetFlow);
    lua_register(m_pLua, "SetFlow", this->SetFlow);

    lua_register(m_pLua, "GetLocalCharacterId", this->GetLocalCharacterId);
    lua_register(m_pLua, "GetCharacterPos", this->GetCharacterPos);
    lua_register(m_pLua, "GetCharacterVel", this->GetCharacterVel);

    //Music
    lua_register(m_pLua, "MusicPlay", this->MusicPlay);
    lua_register(m_pLua, "MusicPause", this->MusicPause);
    lua_register(m_pLua, "MusicStop", this->MusicStop);
    lua_register(m_pLua, "MusicNext", this->MusicNext);
    lua_register(m_pLua, "MusicPrev", this->MusicPrev);
    lua_register(m_pLua, "MusicSetVol", this->MusicSetVol);
    lua_register(m_pLua, "MusicGetVol", this->MusicGetVol);
    lua_register(m_pLua, "MusicGetState", this->MusicGetState);

    lua_register(m_pLua, "GetConfigValue", this->GetConfigValue);
    lua_register(m_pLua, "SetConfigValue", this->SetConfigValue);

    lua_register(m_pLua, "GetControlValue", this->GetControlValue);
    lua_register(m_pLua, "SetControlValue", this->SetControlValue);
    lua_register(m_pLua, "UnSetControlValue", this->UnSetControlValue);

    //Console Print
    lua_register(m_pLua, "Print", this->Print);
    lua_register(m_pLua, "Console", this->Console);

    //States
    lua_register(m_pLua, "StateOnline", this->StateOnline);
    lua_register(m_pLua, "StateOffline", this->StateOffline);
    lua_register(m_pLua, "StateConnecting", this->StateConnecting);
    lua_register(m_pLua, "StateDemoplayback", this->StateDemoplayback);
    lua_register(m_pLua, "StateLoading", this->StateLoading);

    //Serverinfo
    lua_register(m_pLua, "GetGameType", this->GetGameType);
    lua_register(m_pLua, "IsTeamplay", this->IsTeamplay);

    //Get Net Error
    lua_register(m_pLua, "GetNetError", this->GetNetError);

    //Connect
    lua_register(m_pLua, "Connect", this->Connect);

    //collision
    lua_register(m_pLua, "IntersectLine", this->IntersectLine);
    lua_register(m_pLua, "GetTile", this->GetTile);
    lua_register(m_pLua, "GetMapWidth", this->GetMapWidth);
    lua_register(m_pLua, "GetMapHeight", this->GetMapHeight);

    //Chat
    lua_register(m_pLua, "ChatSend", this->ChatSend);
    lua_register(m_pLua, "ChatTeamSend", this->ChatTeamSend);

    //Ui
    lua_register(m_pLua, "UiDoButton", this->UiDoButton);
    lua_register(m_pLua, "UiDoEditBox", this->UiDoEditBox);
    lua_register(m_pLua, "UiDoLabel", this->UiDoLabel);
    lua_register(m_pLua, "UiDoRect", this->UiDoRect);
    lua_register(m_pLua, "UiDoImage", this->UiDoImage);
    lua_register(m_pLua, "UiDoLine", this->UiDoLine);
    lua_register(m_pLua, "UiDoSlider", this->UiDoSlider);
    lua_register(m_pLua, "UiRemoveElement", this->UiRemoveElement);
    lua_register(m_pLua, "UiGetText", this->UiGetText);
    lua_register(m_pLua, "UiSetText", this->UiSetText);
    lua_register(m_pLua, "UiGetColor", this->UiGetColor);
    lua_register(m_pLua, "UiSetColor", this->UiSetColor);
    lua_register(m_pLua, "UiGetRect", this->UiGetRect);
    lua_register(m_pLua, "UiSetRect", this->UiSetRect);
    lua_register(m_pLua, "UiGetScreenWidth", this->UiGetScreenWidth);
    lua_register(m_pLua, "UiGetScreenHeight", this->UiGetScreenHeight);
    lua_register(m_pLua, "UiGetGameTextureID", this->UiGetGameTextureID);
    lua_register(m_pLua, "UiGetParticleTextureID", this->UiGetParticleTextureID);
    lua_register(m_pLua, "UiGetFlagTextureID", this->UiGetFlagTextureID);
    lua_register(m_pLua, "NetSend", this->SendPacket);
    lua_register(m_pLua, "NetFetch", this->FetchPacket);

    //Texture
    lua_register(m_pLua, "TextureLoad", this->TextureLoad);
    lua_register(m_pLua, "TextureUnload", this->TextureUnload);

    //Texture
    lua_register(m_pLua, "FetchPacket", this->FetchPacket);
    lua_register(m_pLua, "SendPacket", this->SendPacket);

    lua_pushlightuserdata(m_pLua, this);
    lua_setglobal(m_pLua, "pLUA");

    lua_register(m_pLua, "errorfunc", this->ErrorFunc); //TODO: fix me
	lua_getglobal(m_pLua, "errorfunc");


    if (luaL_loadfile(m_pLua, m_aFilename) == 0)
    {
        lua_pcall(m_pLua, 0, LUA_MULTRET, 0);
        ErrorFunc(m_pLua);
		dbg_msg("Lua", "Error loading Lua file");
    }
    lua_getglobal(m_pLua, "errorfunc");
    ErrorFunc(m_pLua);
}

void CLuaFile::Close()
{
    //kill lua
    if (m_pLua)
        lua_close(m_pLua);
    m_pLua = 0;

    //clear
    mem_zero(m_aUiElements, sizeof(m_aUiElements));
    mem_zero(m_aTitle, sizeof(m_aTitle));
    mem_zero(m_aInfo, sizeof(m_aInfo));
    mem_zero(m_aFilename, sizeof(m_aFilename));
    m_HaveSettings = 0;
    m_FunctionVarNum = 0;
}

int CLuaFile::ErrorFunc(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)(int)lua_touserdata(L, -1);

    lua_pop(L,1);

	int depth = 0;
	int frameskip = 1;
	lua_Debug frame;

    if (lua_tostring(L, -1) == 0)
        return 0;

    dbg_msg("Lua", pSelf->m_aFilename);
    dbg_msg("Lua", lua_tostring(L, -1));

    dbg_msg("Lua", "Backtrace:");
    while(lua_getstack(L, depth, &frame) == 1)
    {
        depth++;

        lua_getinfo(L, "nlSf", &frame);

        /* check for functions that just report errors. these frames just confuses more then they help */
        if(frameskip && str_comp(frame.short_src, "[C]") == 0 && frame.currentline == -1)
            continue;
        frameskip = 0;

        /* print stack frame */
        dbg_msg("Lua", "%s(%d): %s %s", frame.short_src, frame.currentline, frame.name, frame.namewhat);
    }
    lua_pop(L, 1); // remove error message
    lua_gc(L, LUA_GCCOLLECT, 0);
    return 0;
}


void CLuaFile::ConfigClose()
{
    FunctionExec("ConfigClose");
}

void CLuaFile::PushString(const char *pString)
{
    if (m_pLua == 0)
        return;
    lua_pushstring(m_pLua, pString);
    m_FunctionVarNum++;
}

void CLuaFile::PushInteger(int value)
{
    if (m_pLua == 0)
        return;
    lua_pushinteger(m_pLua, value);
    m_FunctionVarNum++;
}

void CLuaFile::PushFloat(float value)
{
    if (m_pLua == 0)
        return;
    lua_pushnumber(m_pLua, value);
    m_FunctionVarNum++;
}

void CLuaFile::PushBoolean(bool value)
{
    if (m_pLua == 0)
        return;
    lua_pushboolean(m_pLua, value);
    m_FunctionVarNum++;
}

void CLuaFile::PushParameter(const char *pString)
{
    if (m_pLua == 0)
        return;
    if (StrIsInteger(pString))
    {
        PushInteger(str_toint(pString));
    }
    else if (StrIsFloat(pString))
    {
        PushInteger(str_tofloat(pString));
    }
    else
    {
        PushString(pString);
    }

}

bool CLuaFile::FunctionExist(const char *pFunctionName)
{
    if (m_pLua == 0)
        return false;
    lua_getglobal(m_pLua, pFunctionName);
    return lua_isfunction(m_pLua, lua_gettop(m_pLua));
}

void CLuaFile::FunctionPrepare(const char *pFunctionName)
{
    if (m_pLua == 0 || m_aFilename[0] == 0)
        return;

    lua_pushstring (m_pLua, pFunctionName);
    lua_gettable (m_pLua, LUA_GLOBALSINDEX);
    m_FunctionVarNum = 0;
}

void CLuaFile::FunctionExec(const char *pFunctionName)
{
    if (m_pLua == 0)
        return;
    if (m_aFilename[0] == 0)
        return;

    if (pFunctionName)
    {
        if (FunctionExist(pFunctionName) == false)
            return;
        lua_pushstring (m_pLua, pFunctionName);
        lua_gettable (m_pLua, LUA_GLOBALSINDEX);
    }
    lua_pcall(m_pLua, m_FunctionVarNum, LUA_MULTRET, 0);
    ErrorFunc(m_pLua);
    m_FunctionVarNum = 0;
}


//functions

int CLuaFile::Include(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)(int)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    if (!lua_isstring(L, 1))
        return 0;
    if (luaL_loadfile(L, lua_tostring(L, 1)) == 0)
    {
        lua_pcall(L, 0, LUA_MULTRET, 0);
    }

    return 0;
}

int CLuaFile::SetScriptUseSettingPage(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)(int)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    if (!lua_isnumber(L, 1))
        return 0;
    pSelf->m_HaveSettings = lua_tointeger(L, 1);
    return 0;
}

int CLuaFile::SetScriptTitle(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)(int)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    if (!lua_isstring(L, 1))
        return 0;
    str_copy(pSelf->m_aTitle, lua_tostring(L, 1), sizeof(pSelf->m_aTitle));
    return 0;
}

int CLuaFile::SetScriptInfo(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)(int)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    if (!lua_isstring(L, 1))
        return 0;
    str_copy(pSelf->m_aInfo, lua_tostring(L, 1), sizeof(pSelf->m_aInfo));
    return 0;
}

int CLuaFile::AddEventListener(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)(int)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);


    if (!lua_isstring(L, 1) && !lua_isstring(L, 2))
        return 0;
    pSelf->m_pLuaHandler->m_EventListener.AddEventListener(pSelf, (char *)lua_tostring(L, 1), (char *)lua_tostring(L, 2));
    return 0;
}

int CLuaFile::RemoveEventListener(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)(int)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);


    if (!lua_isstring(L, 1))
        return 0;
    pSelf->m_pLuaHandler->m_EventListener.RemoveEventListener(pSelf, (char *)lua_tostring(L, 1));
    return 0;
}

int CLuaFile::SetMenuBrowserGameTypeColor(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)(int)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    if (lua_isnumber(L, 1) && lua_isnumber(L, 2) && lua_isnumber(L, 3) && lua_isnumber(L, 4))
        pSelf->m_pLuaHandler->m_EventListener.m_BrowserActivGameTypeColor = vec4(lua_tonumber(L, 1), lua_tonumber(L, 2), lua_tonumber(L, 3), lua_tonumber(L, 4));
    return 0;
}

int CLuaFile::GetMenuBrowserGameTypeName(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)(int)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    lua_pushstring(L, pSelf->m_pLuaHandler->m_EventListener.m_pBrowserActivGameTypeName);
    return 1;
}

int CLuaFile::ScoreboardAbortRender(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)(int)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    pSelf->m_pLuaHandler->m_EventListener.m_ScoreboardSkipRender = true;
    return 0;
}

int CLuaFile::ChatGetText(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)(int)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    lua_pushstring(L, pSelf->m_pLuaHandler->m_EventListener.m_pChatText);
    return 1;
}

int CLuaFile::ChatGetClientID(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)(int)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    lua_pushinteger(L, pSelf->m_pLuaHandler->m_EventListener.m_ChatClientID);
    return 1;
}

int CLuaFile::ChatGetTeam(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)(int)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    lua_pushinteger(L, pSelf->m_pLuaHandler->m_EventListener.m_ChatTeam);
    return 1;
}

int CLuaFile::ChatHide(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)(int)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    pSelf->m_pLuaHandler->m_EventListener.m_ChatHide = true;
    return 0;
}

int CLuaFile::KillGetKillerID(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)(int)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    lua_pushinteger(L, pSelf->m_pLuaHandler->m_EventListener.m_KillKillerID);
    return 1;
}

int CLuaFile::KillGetVictimID(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)(int)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    lua_pushinteger(L, pSelf->m_pLuaHandler->m_EventListener.m_KillVictimID);
    return 1;
}

int CLuaFile::KillGetWeapon(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)(int)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    lua_pushinteger(L, pSelf->m_pLuaHandler->m_EventListener.m_KillWeapon);
    return 1;
}

int CLuaFile::GetPlayerName(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)(int)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    if (lua_isnumber(L, 1))
    {
        if (lua_tointeger(L, 1) >= 0 && lua_tointeger(L, 1) < MAX_CLIENTS)
        {
            if (pSelf->m_pClient->m_aClients[lua_tointeger(L, 1)].m_aName[0])
                lua_pushstring(L, pSelf->m_pClient->m_aClients[lua_tointeger(L, 1)].m_aName);
            else
                lua_pushnil(L);
        }
        else
            lua_pushnil(L);
    }
    else
        lua_pushnil(L);
    return 1;
}

int CLuaFile::GetPlayerClan(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)(int)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    if (lua_isnumber(L, 1))
    {
        if (lua_tointeger(L, 1) >= 0 && lua_tointeger(L, 1) < MAX_CLIENTS)
        {
            lua_pushstring(L, pSelf->m_pClient->m_aClients[lua_tointeger(L, 1)].m_aClan);
            return 1;
        }
    }
    return 0;
}

int CLuaFile::GetPlayerCountry(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)(int)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    if (lua_isnumber(L, 1))
    {
        if (lua_tointeger(L, 1) >= 0 && lua_tointeger(L, 1) < MAX_CLIENTS)
        {
            lua_pushinteger(L, pSelf->m_pClient->m_aClients[lua_tointeger(L, 1)].m_Country);
            return 1;
        }
    }
    return 0;
}

int CLuaFile::GetPlayerScore(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)(int)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    if (lua_isnumber(L, 1))
    {
        if (lua_tointeger(L, 1) >= 0 && lua_tointeger(L, 1) < MAX_CLIENTS)
        {
            const CNetObj_PlayerInfo *pInfo = pSelf->m_pClient->m_Snap.m_paPlayerInfos[lua_tointeger(L, 1)];
            if (pInfo)
            {
                lua_pushinteger(L, pInfo->m_Score);
                return 1;
            }
            lua_pushinteger(L, 0);
            return 1;
        }
    }
    return 0;
}

int CLuaFile::GetPlayerPing(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)(int)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    if (lua_isnumber(L, 1))
    {
        if (lua_tointeger(L, 1) >= 0 && lua_tointeger(L, 1) < MAX_CLIENTS)
        {
            const CNetObj_PlayerInfo *pInfo = pSelf->m_pClient->m_Snap.m_paPlayerInfos[lua_tointeger(L, 1)];
            if (pInfo)
            {
                lua_pushinteger(L, pInfo->m_Latency);
                return 1;
            }
        }
    }
    return 0;
}

int CLuaFile::GetPlayerTeam(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)(int)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    if (lua_isnumber(L, 1))
    {
        if (lua_tointeger(L, 1) >= 0 && lua_tointeger(L, 1) < MAX_CLIENTS)
        {
            const CNetObj_PlayerInfo *pInfo = pSelf->m_pClient->m_Snap.m_paPlayerInfos[lua_tointeger(L, 1)];
            if (pInfo)
            {
                lua_pushinteger(L, pInfo->m_Team);
                return 1;
            }
        }
    }
    return 0;
}

int CLuaFile::GetPlayerSkin(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)(int)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    if (lua_isnumber(L, 1))
    {
        if (lua_tointeger(L, 1) >= 0 && lua_tointeger(L, 1) < MAX_CLIENTS)
        {
            lua_pushstring(L, pSelf->m_pClient->m_aClients[lua_tointeger(L, 1)].m_aSkinName);
            return 1;
        }
    }
    return 0;
}

int CLuaFile::GetPlayerColorFeet(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)(int)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    if (lua_isnumber(L, 1))
    {
        if (lua_tointeger(L, 1) >= 0 && lua_tointeger(L, 1) < MAX_CLIENTS)
        {
            lua_pushnumber(L, pSelf->m_pClient->m_pSkins->GetColorV3(pSelf->m_pClient->m_aClients[lua_tointeger(L, 1)].m_ColorFeet).r);
            lua_pushnumber(L, pSelf->m_pClient->m_pSkins->GetColorV3(pSelf->m_pClient->m_aClients[lua_tointeger(L, 1)].m_ColorFeet).g);
            lua_pushnumber(L, pSelf->m_pClient->m_pSkins->GetColorV3(pSelf->m_pClient->m_aClients[lua_tointeger(L, 1)].m_ColorFeet).b);
            lua_pushnumber(L, 1.0f);
            return 1;
        }
    }
    return 0;
}

int CLuaFile::GetPlayerColorBody(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)(int)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    if (lua_isnumber(L, 1))
    {
        if (lua_tointeger(L, 1) >= 0 && lua_tointeger(L, 1) < MAX_CLIENTS)
        {
            lua_pushnumber(L, pSelf->m_pClient->m_pSkins->GetColorV3(pSelf->m_pClient->m_aClients[lua_tointeger(L, 1)].m_ColorBody).r);
            lua_pushnumber(L, pSelf->m_pClient->m_pSkins->GetColorV3(pSelf->m_pClient->m_aClients[lua_tointeger(L, 1)].m_ColorBody).g);
            lua_pushnumber(L, pSelf->m_pClient->m_pSkins->GetColorV3(pSelf->m_pClient->m_aClients[lua_tointeger(L, 1)].m_ColorBody).b);
            lua_pushnumber(L, 1.0f);
            return 1;
        }
    }
    return 0;
}

int CLuaFile::GetPlayerColorSkin(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)(int)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    if (lua_isnumber(L, 1))
    {
        if (lua_tointeger(L, 1) >= 0 && lua_tointeger(L, 1) < MAX_CLIENTS)
        {
            if (pSelf->m_pClient->m_aClients[lua_tointeger(L, 1)].m_UseCustomColor)
            {
                lua_pushnumber(L, pSelf->m_pClient->m_pSkins->GetColorV3(pSelf->m_pClient->m_aClients[lua_tointeger(L, 1)].m_ColorBody).r);
                lua_pushnumber(L, pSelf->m_pClient->m_pSkins->GetColorV3(pSelf->m_pClient->m_aClients[lua_tointeger(L, 1)].m_ColorBody).g);
                lua_pushnumber(L, pSelf->m_pClient->m_pSkins->GetColorV3(pSelf->m_pClient->m_aClients[lua_tointeger(L, 1)].m_ColorBody).b);
                lua_pushnumber(L, 1.0f);
                return 1;
            }
            else
            {
                const CSkins::CSkin *s = pSelf->m_pClient->m_pSkins->Get(pSelf->m_pClient->m_aClients[lua_tointeger(L, 1)].m_SkinID);
                if (s)
                {
                    lua_pushnumber(L, s->m_BloodColor.r);
                    lua_pushnumber(L, s->m_BloodColor.g);
                    lua_pushnumber(L, s->m_BloodColor.b);
                    lua_pushnumber(L, 1.0f);
                    return 1;
                }
            }
        }
    }
    return 0;
}


int CLuaFile::UiGetScreenWidth(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)(int)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    CUIRect Screen = *pSelf->m_pClient->UI()->Screen();
    lua_pushnumber(L, Screen.w);
    return 1;
}

int CLuaFile::UiGetScreenHeight(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)(int)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    CUIRect Screen = *pSelf->m_pClient->UI()->Screen();
    lua_pushnumber(L, Screen.h);
    return 1;
}

int CLuaFile::MusicPlay(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)(int)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    pSelf->m_pClient->Sound()->m_MusicPlaying = true;
    pSelf->m_pClient->m_Music->m_MusicListActivated = true;
    return 0;
}

int CLuaFile::MusicPause(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)(int)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    pSelf->m_pClient->Sound()->m_MusicPlaying = pSelf->m_pClient->Sound()->m_MusicPlaying ^ 1;
    if (pSelf->m_pClient->m_Music->m_MusicFirstPlay)
        pSelf->m_pClient->m_Music->m_MusicListActivated = true;
    return 0;
}

int CLuaFile::MusicStop(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)(int)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    pSelf->m_pClient->Sound()->m_MusicPlaying = false;
    pSelf->m_pClient->m_Music->m_MusicListActivated = false;
    return 0;
}

int CLuaFile::MusicNext(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)(int)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    pSelf->m_pClient->Sound()->m_MusicPlayIndex++;
    return 0;
}

int CLuaFile::MusicPrev(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)(int)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    pSelf->m_pClient->Sound()->m_MusicPlayIndex--;
    return 0;
}

int CLuaFile::MusicSetVol(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)(int)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    g_Config.m_SndMusicVolume = max(0, min(lua_tointeger(L, 1), 100));
    return 0;
}

int CLuaFile::MusicGetVol(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)(int)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    lua_pushinteger(L, g_Config.m_SndMusicVolume);
    return 1;
}

int CLuaFile::MusicGetState(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)(int)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    lua_pushinteger(L, pSelf->m_pClient->Sound()->m_MusicPlaying);
    return 1;
}

int CLuaFile::MusicGetPlayedIndex(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)(int)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    lua_pushinteger(L, pSelf->m_pClient->Sound()->m_MusicPlayIndex);
    return 1;
}

int CLuaFile::SendPlayerInfo(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)(int)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    pSelf->m_pClient->SendInfo(false);
    return 0;
}

int CLuaFile::Console(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)(int)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    if (lua_tointeger(L, 1))
    {
            pSelf->m_pClient->Console()->Print(lua_tointeger(L, 1), lua_tostring(L, 2), lua_tostring(L, 3));
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

    if (lua_isnumber(L, 1))
    {
        CNetMsg_Cl_Emoticon Msg;
        Msg.m_Emoticon = lua_tonumber(L, 1);
        pSelf->m_pClient->Client()->SendPackMsg(&Msg, MSGFLAG_VITAL);
    }
    return 0;
}


int CLuaFile::GetConfigValue(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)(int)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    if (!lua_isstring(L, 1))
        return 0;
    if (str_comp_nocase(lua_tostring(L, 1), "PlayerName") == 0)
    {
        lua_pushstring(L, g_Config.m_PlayerName);
        return 1;
    }
    if (str_comp_nocase(lua_tostring(L, 1), "MouseDeadzone") == 0)
    {
        lua_pushinteger(L, g_Config.m_ClMouseDeadzone);
        return 1;
    }
    if (str_comp_nocase(lua_tostring(L, 1), "MouseFollowfactor") == 0)
    {
        lua_pushinteger(L, g_Config.m_ClMouseFollowfactor);
        return 1;
    }
    if (str_comp_nocase(lua_tostring(L, 1), "MouseMaxDistance") == 0)
    {
        lua_pushinteger(L, g_Config.m_ClMouseMaxDistance);
        return 1;
    }
    if (str_comp_nocase(lua_tostring(L, 1), "PlayerColorBody") == 0)
    {
        lua_pushinteger(L, g_Config.m_PlayerColorBody);
        return 1;
    }
    if (str_comp_nocase(lua_tostring(L, 1), "PlayerColorFeet") == 0)
    {
        lua_pushinteger(L, g_Config.m_PlayerColorFeet);
        return 1;
    }
    if (str_comp_nocase(lua_tostring(L, 1), "Nameplates") == 0)
    {
        lua_pushinteger(L, g_Config.m_ClNameplates);
        return 1;
    }
    if (str_comp_nocase(lua_tostring(L, 1), "WarningTeambalance") == 0)
    {
        lua_pushinteger(L, g_Config.m_ClWarningTeambalance);
        return 1;
    }
    return 0;
}

int CLuaFile::SetConfigValue(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)(int)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    if (!lua_isstring(L, 1))
        return 0;
    if (str_comp_nocase(lua_tostring(L, 1), "PlayerName") == 0 && lua_isstring(L, 2))
    {
        str_copy(g_Config.m_PlayerName, lua_tostring(L, 2), sizeof(g_Config.m_PlayerName));
    }
    if (str_comp_nocase(lua_tostring(L, 1), "MouseDeadzone") == 0 && lua_isnumber(L, 2))
    {
        g_Config.m_ClMouseDeadzone = lua_tointeger(L, 2);
    }
    if (str_comp_nocase(lua_tostring(L, 1), "MouseFollowfactor") == 0 && lua_isnumber(L, 2))
    {
        g_Config.m_ClMouseFollowfactor = lua_tointeger(L, 2);
    }
    if (str_comp_nocase(lua_tostring(L, 1), "MouseMaxDistance") == 0 && lua_isnumber(L, 2))
    {
        g_Config.m_ClMouseMaxDistance = lua_tointeger(L, 2);
    }
    if (str_comp_nocase(lua_tostring(L, 1), "PlayerColorBody") == 0 && lua_isnumber(L, 2))
    {
        g_Config.m_PlayerColorBody = lua_tointeger(L, 2);
    }
    if (str_comp_nocase(lua_tostring(L, 1), "PlayerColorFeet") == 0 && lua_isnumber(L, 2))
    {
        g_Config.m_PlayerColorFeet = lua_tointeger(L, 2);
    }
    if (str_comp_nocase(lua_tostring(L, 1), "Nameplates") == 0 && lua_isnumber(L, 2))
    {
        g_Config.m_ClNameplates = lua_tointeger(L, 2);
    }
    if (str_comp_nocase(lua_tostring(L, 1), "WarningTeambalance") == 0 && lua_isnumber(L, 2))
    {
        g_Config.m_ClWarningTeambalance = lua_tointeger(L, 2);
    }
    return 0;
}

int CLuaFile::GetControlValue(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)(int)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    if (!lua_isstring(L, 1))
        return 0;
    if (str_comp_nocase(lua_tostring(L, 1), "Direction") == 0)
    {
        lua_pushnumber(L, pSelf->m_pClient->m_pLuaBinding->m_ControlDirectionPre);
    }
    if (str_comp_nocase(lua_tostring(L, 1), "Fire") == 0)
    {
        lua_pushnumber(L, pSelf->m_pClient->m_pLuaBinding->m_ControlFirePre);
    }
    if (str_comp_nocase(lua_tostring(L, 1), "Hook") == 0)
    {
        lua_pushnumber(L, pSelf->m_pClient->m_pLuaBinding->m_ControlHookPre);
    }
    if (str_comp_nocase(lua_tostring(L, 1), "Jump") == 0)
    {
        lua_pushnumber(L, pSelf->m_pClient->m_pLuaBinding->m_ControlJumpPre);
    }
    if (str_comp_nocase(lua_tostring(L, 1), "Weapon") == 0)
    {
        lua_pushnumber(L, pSelf->m_pClient->m_pLuaBinding->m_ControlWeaponPre);
    }
    if (str_comp_nocase(lua_tostring(L, 1), "TargetX") == 0)
    {
        lua_pushnumber(L, pSelf->m_pClient->m_pLuaBinding->m_ControlTargetXPre);
    }
    if (str_comp_nocase(lua_tostring(L, 1), "TargetY") == 0)
    {
        lua_pushnumber(L, pSelf->m_pClient->m_pLuaBinding->m_ControlTargetYPre);
    }
    return 1;
}

int CLuaFile::SetControlValue(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)(int)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    if (!lua_isstring(L, 1) && !lua_isnumber(L, 2))
        return 0;
    if (str_comp_nocase(lua_tostring(L, 1), "Direction") == 0)
    {
        pSelf->m_pClient->m_pLuaBinding->m_ControlDirection = lua_tointeger(L, 2);
        pSelf->m_pClient->m_pLuaBinding->m_ControlDirectionIsSet = true;
    }
    if (str_comp_nocase(lua_tostring(L, 1), "Fire") == 0)
    {
        pSelf->m_pClient->m_pLuaBinding->m_ControlFire = lua_tointeger(L, 2);
        pSelf->m_pClient->m_pLuaBinding->m_ControlFireIsSet = true;
    }
    if (str_comp_nocase(lua_tostring(L, 1), "Hook") == 0)
    {
        pSelf->m_pClient->m_pLuaBinding->m_ControlHook = lua_tointeger(L, 2);
        pSelf->m_pClient->m_pLuaBinding->m_ControlHookIsSet = true;
    }
    if (str_comp_nocase(lua_tostring(L, 1), "Jump") == 0)
    {
        pSelf->m_pClient->m_pLuaBinding->m_ControlJump = lua_tointeger(L, 2);
        pSelf->m_pClient->m_pLuaBinding->m_ControlJumpIsSet = true;
    }
    if (str_comp_nocase(lua_tostring(L, 1), "Weapon") == 0)
    {
        pSelf->m_pClient->m_pLuaBinding->m_ControlWeapon = lua_tointeger(L, 2);
        pSelf->m_pClient->m_pLuaBinding->m_ControlWeaponIsSet = true;
    }
    if (str_comp_nocase(lua_tostring(L, 1), "TargetX") == 0)
    {
        pSelf->m_pClient->m_pLuaBinding->m_ControlTargetX = lua_tointeger(L, 2);
        pSelf->m_pClient->m_pLuaBinding->m_ControlTargetXIsSet = true;
    }
    if (str_comp_nocase(lua_tostring(L, 1), "TargetY") == 0)
    {
        pSelf->m_pClient->m_pLuaBinding->m_ControlTargetY = lua_tointeger(L, 2);
        pSelf->m_pClient->m_pLuaBinding->m_ControlTargetYIsSet = true;
    }
    return 0;
}

int CLuaFile::UnSetControlValue(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)(int)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    if (!lua_isstring(L, 1))
        return 0;
    if (str_comp_nocase(lua_tostring(L, 1), "Direction") == 0)
    {
        pSelf->m_pClient->m_pLuaBinding->m_ControlDirection = 0;
        pSelf->m_pClient->m_pLuaBinding->m_ControlDirectionIsSet = false;
    }
    if (str_comp_nocase(lua_tostring(L, 1), "Fire") == 0)
    {
        pSelf->m_pClient->m_pLuaBinding->m_ControlFire = 0;
        pSelf->m_pClient->m_pLuaBinding->m_ControlFireIsSet = false;
    }
    if (str_comp_nocase(lua_tostring(L, 1), "Hook") == 0)
    {
        pSelf->m_pClient->m_pLuaBinding->m_ControlHook = 0;
        pSelf->m_pClient->m_pLuaBinding->m_ControlHookIsSet = false;
    }
    if (str_comp_nocase(lua_tostring(L, 1), "Jump") == 0)
    {
        pSelf->m_pClient->m_pLuaBinding->m_ControlJump = 0;
        pSelf->m_pClient->m_pLuaBinding->m_ControlJumpIsSet = false;
    }
    if (str_comp_nocase(lua_tostring(L, 1), "Weapon") == 0)
    {
        pSelf->m_pClient->m_pLuaBinding->m_ControlWeapon = 0;
        pSelf->m_pClient->m_pLuaBinding->m_ControlWeaponIsSet = false;
    }
    if (str_comp_nocase(lua_tostring(L, 1), "TargetX") == 0)
    {
        pSelf->m_pClient->m_pLuaBinding->m_ControlTargetX = 0;
        pSelf->m_pClient->m_pLuaBinding->m_ControlTargetXIsSet = false;
    }
    if (str_comp_nocase(lua_tostring(L, 1), "TargetY") == 0)
    {
        pSelf->m_pClient->m_pLuaBinding->m_ControlTargetY = 0;
        pSelf->m_pClient->m_pLuaBinding->m_ControlTargetYIsSet = false;
    }
    return 0;
}

int CLuaFile::GetFlow(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)(int)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    vec2 tmp = pSelf->m_pClient->m_pFlow->Get(vec2(lua_tonumber(L, 1), lua_tonumber(L, 2)));
    lua_pushnumber(L, tmp.x);
    lua_pushnumber(L, tmp.y);
    return 2;
}

int CLuaFile::SetFlow(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)(int)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    pSelf->m_pClient->m_pFlow->Add(vec2(lua_tonumber(L, 1), lua_tonumber(L, 2)), vec2(lua_tonumber(L, 3), lua_tonumber(L, 4)), lua_tonumber(L, 5));
    return 0;
}

int CLuaFile::GetCharacterPos(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)(int)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    if (!lua_isnumber(L, 1))
        return 0;
    lua_pushnumber(L, pSelf->m_pClient->m_Snap.m_aCharacters[lua_tointeger(L, 1)].m_Cur.m_X);
    lua_pushnumber(L, pSelf->m_pClient->m_Snap.m_aCharacters[lua_tointeger(L, 1)].m_Cur.m_Y);
    return 2;
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
    lua_pushnumber(L, pSelf->m_pClient->m_Snap.m_aCharacters[lua_tointeger(L, 1)].m_Cur.m_X - pSelf->m_pClient->m_Snap.m_aCharacters[lua_tointeger(L, 1)].m_Prev.m_X);
    lua_pushnumber(L, pSelf->m_pClient->m_Snap.m_aCharacters[lua_tointeger(L, 1)].m_Cur.m_Y - pSelf->m_pClient->m_Snap.m_aCharacters[lua_tointeger(L, 1)].m_Prev.m_Y);
    return 2;
}

int CLuaFile::GetLocalCharacterId(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)(int)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    lua_pushnumber(L, pSelf->m_pClient->m_Snap.m_LocalClientID);
    return 1;
}

int CLuaFile::IntersectLine(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)(int)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    vec2 Pos1 = vec2(lua_tonumber(L, 1), lua_tonumber(L, 2));
    vec2 Pos2 = vec2(lua_tonumber(L, 3), lua_tonumber(L, 4));
    lua_pushnumber(L, pSelf->m_pClient->Collision()->IntersectLine(Pos1, Pos2, 0, 0));
    return 1;
}

int CLuaFile::GetTile(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)(int)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    lua_pushnumber(L, pSelf->m_pClient->Collision()->GetTileRaw(lua_tonumber(L, 1), lua_tonumber(L, 2)));
    return 1;
}

int CLuaFile::GetMapWidth(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)(int)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    lua_pushnumber(L, pSelf->m_pClient->Collision()->GetWidth());
    return 1;
}

int CLuaFile::GetMapHeight(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)(int)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    lua_pushnumber(L, pSelf->m_pClient->Collision()->GetHeight());
    return 1;
}

int CLuaFile::ChatSend(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)(int)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    if (lua_isstring(L, 1))
    {
        pSelf->m_pClient->m_pChat->Say(0, lua_tostring(L, 1));
    }
    return 0;
}

int CLuaFile::ChatTeamSend(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)(int)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    if (lua_isstring(L, 1))
    {
        pSelf->m_pClient->m_pChat->Say(1, lua_tostring(L, 1));
    }
    return 0;
}

int CLuaFile::CreateParticle(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)(int)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    CParticle p;
    p.SetDefault();
    if (lua_isnumber(L, 1))
        p.m_Spr = lua_tonumber(L, 1);

    if (lua_isnumber(L, 2))
        p.m_Texture = lua_tonumber(L, 2);

    if (lua_isnumber(L, 3) && lua_isnumber(L, 4))
        p.m_Pos = vec2(lua_tonumber(L, 3), lua_tonumber(L, 4));

    if (lua_isnumber(L, 5) && lua_isnumber(L, 6))
        p.m_Vel = vec2(lua_tonumber(L, 5), lua_tonumber(L, 6));

    if (lua_isnumber(L, 7))
        p.m_LifeSpan = lua_tonumber(L, 7);

    if (lua_isnumber(L, 8))
        p.m_Rot = lua_tonumber(L, 8);

    if (lua_isnumber(L, 9))
        p.m_Rotspeed = lua_tonumber(L, 9);

    if (lua_isnumber(L, 10))
        p.m_StartSize = lua_tonumber(L, 10);

    if (lua_isnumber(L, 11))
        p.m_EndSize = lua_tonumber(L, 11);

    if (lua_isnumber(L, 12))
        p.m_Friction = lua_tonumber(L, 12);

    if (lua_isnumber(L, 13))
        p.m_Gravity.x = lua_tonumber(L, 13);

    if (lua_isnumber(L, 14))
        p.m_Gravity.y = lua_tonumber(L, 14);

    if (lua_isnumber(L, 15))
        p.m_FlowAffected = lua_tonumber(L, 15);

    if (lua_isnumber(L, 16) && lua_isnumber(L, 17) && lua_isnumber(L, 18) && lua_isnumber(L, 19))
        p.m_Color = vec4(lua_tonumber(L, 16), lua_tonumber(L, 17), lua_tonumber(L, 18), lua_tonumber(L, 19));

    if (lua_isnumber(L, 20) && lua_isnumber(L, 21) && lua_isnumber(L, 22) && lua_isnumber(L, 23))
        p.m_ColorEnd = vec4(lua_tonumber(L, 20), lua_tonumber(L, 21), lua_tonumber(L, 22), lua_tonumber(L, 23));

    //Fire ^^
    //CallParent("CreateParticle", 7, 815 - (10-math.random()*20), 1410, 50-math.random()*100, -440, 0.5 + math.random() * 0.5, pi * 10 * (math.random() - 0.5), pi * 2, 36, 0, 0.8, 0, math.random() * -500, 1, 1, 1, 1, 1)
    //CallParent("CreateParticle", 5, 815 - (10-math.random()*20), 1410, 50-math.random()*100, -440, 1 + math.random() * 0.5, pi * 10 * (math.random() - 0.5), pi * 2, 10, 20, 0.8, 0, math.random() * -500, 1, 1, 1, 1, 0.5, 1, 1, 1, 0)
    //CallParent("CreateParticle", 5, 815 - (10-math.random()*20), 1410, 50-math.random()*100, -440, 1 + math.random() * 0.5, pi * 10 * (math.random() - 0.5), pi * 2, 10, 20, 0.8, 0, math.random() * -500, 1, 1, 1, 1, 0.5, 1, 1, 1, 0)

    //lua_pushnumber(L, EmitterId);

	if (pSelf->m_pClient->Client()->GameTick())
        pSelf->m_pClient->m_pParticles->Add(CParticles::GROUP_GENERAL, &p);
    return 0;
}

int CLuaFile::Print(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)(int)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    if (lua_isstring(L, 1) && lua_isstring(L, 2))
        dbg_msg(lua_tostring(L, 1), lua_tostring(L, 2));
    return 0;
}

int CLuaFile::GetGameType(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)(int)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    CServerInfo CurrentServerInfo;
	pSelf->m_pClient->Client()->GetServerInfo(&CurrentServerInfo);
    lua_pushstring(L, CurrentServerInfo.m_aGameType);
    return 1;
}

int CLuaFile::IsTeamplay(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)(int)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    if (pSelf->m_pClient->m_Snap.m_pGameInfoObj)
        lua_pushboolean(L, pSelf->m_pClient->m_Snap.m_pGameInfoObj->m_GameFlags&GAMEFLAG_TEAMS);
    else
        lua_pushboolean(L, false);
    return 1;
}

int CLuaFile::GetNetError(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)(int)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    lua_pushstring(L, pSelf->m_pClient->Client()->ErrorString());
    return 1;
}

int CLuaFile::Connect(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)(int)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    if (lua_isstring(L, 1))
        pSelf->m_pClient->Client()->Connect(lua_tostring(L, 1));
    else
        pSelf->m_pClient->Client()->Connect(g_Config.m_UiServerAddress);
    return 0;
}

int CLuaFile::StateGetOld(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)(int)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    lua_pushboolean(L, pSelf->m_pLuaHandler->m_EventListener.m_StateOld);
    return 1;
}

int CLuaFile::StateGet(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)(int)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    lua_pushboolean(L, pSelf->m_pClient->Client()->State());
    return 1;
}

int CLuaFile::StateOnline(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)(int)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    lua_pushboolean(L, pSelf->m_pClient->Client()->State() == IClient::STATE_ONLINE);
    return 1;
}

int CLuaFile::StateOffline(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)(int)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    lua_pushboolean(L, pSelf->m_pClient->Client()->State() == IClient::STATE_OFFLINE);
    return 1;
}

int CLuaFile::StateConnecting(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)(int)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    lua_pushboolean(L, pSelf->m_pClient->Client()->State() == IClient::STATE_CONNECTING);
    return 1;
}

int CLuaFile::StateLoading(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)(int)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    lua_pushboolean(L, pSelf->m_pClient->Client()->State() == IClient::STATE_LOADING);
    return 1;
}

int CLuaFile::StateDemoplayback(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)(int)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    lua_pushboolean(L, pSelf->m_pClient->Client()->State() == IClient::STATE_DEMOPLAYBACK);
    return 1;
}

int CLuaFile::MenuActiv(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)(int)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    lua_pushboolean(L, pSelf->m_pClient->m_pMenus->IsActive());
    return 1;
}

int CLuaFile::MenuGameActiv(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)(int)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    lua_pushboolean(L, pSelf->m_pClient->m_pMenus->IsActive() && pSelf->m_pClient->m_pMenus->GetGamePage() == CMenus::PAGE_GAME);
    return 1;
}

int CLuaFile::MenuPlayersActiv(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)(int)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    lua_pushboolean(L, pSelf->m_pClient->m_pMenus->IsActive() && pSelf->m_pClient->m_pMenus->GetGamePage() == CMenus::PAGE_PLAYERS);
    return 1;
}

int CLuaFile::MenuServerInfoActiv(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)(int)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    lua_pushboolean(L, pSelf->m_pClient->m_pMenus->IsActive() && pSelf->m_pClient->m_pMenus->GetGamePage() == CMenus::PAGE_SERVER_INFO);
    return 1;
}

int CLuaFile::MenuCallVoteActiv(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)(int)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    lua_pushboolean(L, pSelf->m_pClient->m_pMenus->IsActive() && pSelf->m_pClient->m_pMenus->GetGamePage() == CMenus::PAGE_CALLVOTE);
    return 1;
}

int CLuaFile::MenuServersActiv(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)(int)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    lua_pushboolean(L, pSelf->m_pClient->m_pMenus->IsActive() && pSelf->m_pClient->m_pMenus->GetGamePage() == CMenus::PAGE_SERVERS);
    return 1;
}

int CLuaFile::MenuMusicActiv(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)(int)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    lua_pushboolean(L, pSelf->m_pClient->m_pMenus->IsActive() && pSelf->m_pClient->m_pMenus->GetGamePage() == CMenus::PAGE_MUSIC);
    return 1;
}

int CLuaFile::MenuDemosActiv(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)(int)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    lua_pushboolean(L, pSelf->m_pClient->m_pMenus->IsActive() && pSelf->m_pClient->m_pMenus->GetGamePage() == CMenus::PAGE_DEMOS);
    return 1;
}

int CLuaFile::GetMousePosMenu(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)(int)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    lua_pushnumber(L, pSelf->m_pClient->m_pMenus->GetMousePos().x);
    lua_pushnumber(L, pSelf->m_pClient->m_pMenus->GetMousePos().y);
    return 1;
}

int CLuaFile::SetMouseModeRelativ(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)(int)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    pSelf->m_pClient->m_pLua->m_MouseModeAbsolute = false;
    return 1;
}

int CLuaFile::SetMouseModeAbsolute(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)(int)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    pSelf->m_pClient->m_pLua->m_MouseModeAbsolute = true;
    return 1;
}

int CLuaFile::UiRemoveElement(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)(int)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    if (!lua_isnumber(L, 1))
        return 0;

    int i = lua_tonumber(L, 1);
    if (i >= 0 && i < LUAMAXUIELEMENTS)
    {
        pSelf->m_aUiElements[i].m_Used = false;
    }
    return 0;
}

int CLuaFile::UiGetText(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)(int)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    if (!lua_isnumber(L, 1))
        return 0;

    int i = lua_tonumber(L, 1);
    if (i >= 0 && i < LUAMAXUIELEMENTS)
    {
        lua_pushstring(L, pSelf->m_aUiElements[i].m_pText);
        return 1;
    }
    return 0;
}

int CLuaFile::UiSetText(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)(int)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    if (!lua_isnumber(L, 1))
        return 0;
    if (!lua_isstring(L, 2))
        return 0;

    int i = lua_tonumber(L, 1);
    if (i >= 0 && i < LUAMAXUIELEMENTS)
    {
        str_copy(pSelf->m_aUiElements[i].m_pText, lua_tostring(L, 2), sizeof(pSelf->m_aUiElements[i].m_pText));
    }
    return 0;
}

int CLuaFile::UiGetColor(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)(int)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    if (!lua_isnumber(L, 1))
        return 0;

    int i = lua_tonumber(L, 1);
    if (i >= 0 && i < LUAMAXUIELEMENTS)
    {
        lua_pushnumber(L, pSelf->m_aUiElements[i].m_Color.r);
        lua_pushnumber(L, pSelf->m_aUiElements[i].m_Color.g);
        lua_pushnumber(L, pSelf->m_aUiElements[i].m_Color.b);
        lua_pushnumber(L, pSelf->m_aUiElements[i].m_Color.a);
        return 4;
    }
    return 0;
}

int CLuaFile::UiSetColor(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)(int)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    if (!lua_isnumber(L, 1))
        return 0;
    if (!lua_isnumber(L, 2))
        return 0;
    if (!lua_isnumber(L, 3))
        return 0;
    if (!lua_isnumber(L, 4))
        return 0;
    if (!lua_isnumber(L, 5))
        return 0;

    int i = lua_tonumber(L, 1);
    if (i >= 0 && i < LUAMAXUIELEMENTS)
    {
        pSelf->m_aUiElements[i].m_Color.r = lua_tonumber(L, 2);
        pSelf->m_aUiElements[i].m_Color.g = lua_tonumber(L, 3);
        pSelf->m_aUiElements[i].m_Color.b = lua_tonumber(L, 4);
        pSelf->m_aUiElements[i].m_Color.a = lua_tonumber(L, 5);
    }
    return 0;
}

int CLuaFile::UiGetRect(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)(int)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    if (!lua_isnumber(L, 1))
        return 0;

    int i = lua_tonumber(L, 1);
    if (i >= 0 && i < LUAMAXUIELEMENTS)
    {
        lua_pushnumber(L, pSelf->m_aUiElements[i].m_Rect.x);
        lua_pushnumber(L, pSelf->m_aUiElements[i].m_Rect.y);
        lua_pushnumber(L, pSelf->m_aUiElements[i].m_Rect.w);
        lua_pushnumber(L, pSelf->m_aUiElements[i].m_Rect.h);
        return 4;
    }
    return 0;
}

int CLuaFile::UiSetRect(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)(int)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    if (!lua_isnumber(L, 1))
        return 0;
    if (!lua_isnumber(L, 2))
        return 0;
    if (!lua_isnumber(L, 3))
        return 0;
    if (!lua_isnumber(L, 4))
        return 0;
    if (!lua_isnumber(L, 5))
        return 0;

    int i = lua_tonumber(L, 1);
    if (i >= 0 && i < LUAMAXUIELEMENTS)
    {
        pSelf->m_aUiElements[i].m_Rect.x = lua_tonumber(L, 2);
        pSelf->m_aUiElements[i].m_Rect.y = lua_tonumber(L, 3);
        pSelf->m_aUiElements[i].m_Rect.w = lua_tonumber(L, 4);
        pSelf->m_aUiElements[i].m_Rect.h = lua_tonumber(L, 5);
    }
    return 0;
}

int CLuaFile::UiDoButton(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)(int)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    if (!lua_isnumber(L, 1))
        return 0;
    if (!lua_isnumber(L, 2))
        return 0;
    if (!lua_isnumber(L, 3))
        return 0;
    if (!lua_isnumber(L, 4))
        return 0;
    if (!lua_isnumber(L, 5))
        return 0;
    if (!lua_isstring(L, 6))
        return 0;
    if (!lua_isstring(L, 7))
        return 0;

    int i = 0;
    for (i = 0; i < LUAMAXUIELEMENTS; i++)
    {
        if (pSelf->m_aUiElements[i].m_Used == 0)
        {
            break;
        }
    }
    if (i >= LUAMAXUIELEMENTS)
        return 0;

    pSelf->m_aUiElements[i].m_Used = true;
    pSelf->m_aUiElements[i].m_Rect.x = lua_tonumber(L, 1);
    pSelf->m_aUiElements[i].m_Rect.y = lua_tonumber(L, 2);
    pSelf->m_aUiElements[i].m_Rect.w = lua_tonumber(L, 3);
    pSelf->m_aUiElements[i].m_Rect.h = lua_tonumber(L, 4);
    pSelf->m_aUiElements[i].m_RegPoint = lua_tonumber(L, 5);
    str_copy(pSelf->m_aUiElements[i].m_pText, lua_tostring(L, 6), sizeof(pSelf->m_aUiElements[i].m_pText));
    str_copy(pSelf->m_aUiElements[i].m_pCallback, lua_tostring(L, 7), sizeof(pSelf->m_aUiElements[i].m_pCallback));
    if (lua_isnumber(L, 8))
        pSelf->m_aUiElements[i].m_Checked = lua_tonumber(L, 8);
    else
        pSelf->m_aUiElements[i].m_Checked = 0;

    if (lua_isnumber(L, 9))
        pSelf->m_aUiElements[i].m_Corners = lua_tonumber(L, 9);
    else
        pSelf->m_aUiElements[i].m_Corners = CUI::CORNER_ALL;

    if (lua_isnumber(L, 10) && lua_isnumber(L, 11) && lua_isnumber(L, 12) && lua_isnumber(L, 13))
        pSelf->m_aUiElements[i].m_Color = vec4(lua_tonumber(L, 10), lua_tonumber(L, 11), lua_tonumber(L, 12), lua_tonumber(L, 13));
    else
        pSelf->m_aUiElements[i].m_Color = vec4(1.0f, 1.0f, 1.0f, 0.5f);

    pSelf->m_aUiElements[i].m_pClient = pSelf->m_pClient;
    pSelf->m_aUiElements[i].m_pLuaFile = pSelf;
    pSelf->m_aUiElements[i].m_Type = CLuaUi::LUAUIBUTTON;

    lua_pushinteger(L, i);

    return 1;
}

int CLuaFile::UiDoEditBox(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)(int)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    if (!lua_isnumber(L, 1))
        return 0;
    if (!lua_isnumber(L, 2))
        return 0;
    if (!lua_isnumber(L, 3))
        return 0;
    if (!lua_isnumber(L, 4))
        return 0;
    if (!lua_isnumber(L, 5))
        return 0;

    int i = 0;
    for (i = 0; i < LUAMAXUIELEMENTS; i++)
    {
        if (pSelf->m_aUiElements[i].m_Used == 0)
        {
            break;
        }
    }
    if (i >= LUAMAXUIELEMENTS)
        return 0;

    pSelf->m_aUiElements[i].m_Used = true;
    pSelf->m_aUiElements[i].m_Rect.x = lua_tonumber(L, 1);
    pSelf->m_aUiElements[i].m_Rect.y = lua_tonumber(L, 2);
    pSelf->m_aUiElements[i].m_Rect.w = lua_tonumber(L, 3);
    pSelf->m_aUiElements[i].m_Rect.h = lua_tonumber(L, 4);
    pSelf->m_aUiElements[i].m_RegPoint = lua_tonumber(L, 5);

    if (lua_isstring(L, 6))
        str_copy(pSelf->m_aUiElements[i].m_pText, lua_tostring(L, 6), sizeof(pSelf->m_aUiElements[i].m_pText));
    else
        pSelf->m_aUiElements[i].m_pText[0] = 0;

    if (lua_isnumber(L, 7))
        pSelf->m_aUiElements[i].m_FontSize = lua_tonumber(L, 7);
    else
        pSelf->m_aUiElements[i].m_FontSize = 14.0f;

    if (lua_isnumber(L, 8))
        pSelf->m_aUiElements[i].m_Hidden = lua_toboolean(L, 8);
    else
        pSelf->m_aUiElements[i].m_Hidden = false;

    if (lua_isnumber(L, 9))
        pSelf->m_aUiElements[i].m_Corners = lua_tonumber(L, 9);
    else
        pSelf->m_aUiElements[i].m_Corners = CUI::CORNER_ALL;

    if (lua_isnumber(L, 10) && lua_isnumber(L, 11) && lua_isnumber(L, 12) && lua_isnumber(L, 13))
        pSelf->m_aUiElements[i].m_Color = vec4(lua_tonumber(L, 10), lua_tonumber(L, 11), lua_tonumber(L, 12), lua_tonumber(L, 13));
    else
        pSelf->m_aUiElements[i].m_Color = vec4(1.0f, 1.0f, 1.0f, 0.5f);

    pSelf->m_aUiElements[i].m_pClient = pSelf->m_pClient;
    pSelf->m_aUiElements[i].m_pLuaFile = pSelf;
    pSelf->m_aUiElements[i].m_Type = CLuaUi::LUAUIEDITBOX;

    lua_pushinteger(L, i);

    return 1;
}

int CLuaFile::UiDoLabel(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)(int)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    if (!lua_isnumber(L, 1))
        return 0;
    if (!lua_isnumber(L, 2))
        return 0;
    if (!lua_isnumber(L, 3))
        return 0;
    if (!lua_isnumber(L, 4))
        return 0;
    if (!lua_isnumber(L, 5))
        return 0;


    int i = 0;
    for (i = 0; i < LUAMAXUIELEMENTS; i++)
    {
        if (pSelf->m_aUiElements[i].m_Used == 0)
        {
            break;
        }
    }
    if (i >= LUAMAXUIELEMENTS)
        return 0;

    pSelf->m_aUiElements[i].m_Used = true;
    pSelf->m_aUiElements[i].m_Rect.x = lua_tonumber(L, 1);
    pSelf->m_aUiElements[i].m_Rect.y = lua_tonumber(L, 2);
    pSelf->m_aUiElements[i].m_Rect.w = lua_tonumber(L, 3);
    pSelf->m_aUiElements[i].m_Rect.h = lua_tonumber(L, 4);
    pSelf->m_aUiElements[i].m_RegPoint = lua_tonumber(L, 5);

    if (lua_isstring(L, 6))
        str_copy(pSelf->m_aUiElements[i].m_pText, lua_tostring(L, 6), sizeof(pSelf->m_aUiElements[i].m_pText));
    else
        pSelf->m_aUiElements[i].m_pText[0] = 0;

    if (lua_isnumber(L, 7))
        pSelf->m_aUiElements[i].m_FontSize = lua_tonumber(L, 7);
    else
        pSelf->m_aUiElements[i].m_FontSize = 14.0f;

    if (lua_isnumber(L, 8))
        pSelf->m_aUiElements[i].m_Align = lua_tonumber(L, 8);
    else
        pSelf->m_aUiElements[i].m_Align = 0;

    if (lua_isnumber(L, 9) && lua_isnumber(L, 10) && lua_isnumber(L, 11) && lua_isnumber(L, 12))
        pSelf->m_aUiElements[i].m_Color = vec4(lua_tonumber(L, 9), lua_tonumber(L, 10), lua_tonumber(L, 11), lua_tonumber(L, 12));
    else
        pSelf->m_aUiElements[i].m_Color = vec4(1, 1, 1, 1);

    pSelf->m_aUiElements[i].m_pClient = pSelf->m_pClient;
    pSelf->m_aUiElements[i].m_pLuaFile = pSelf;
    pSelf->m_aUiElements[i].m_Type = CLuaUi::LUAUILABEL;

    lua_pushinteger(L, i);

    return 1;
}

int CLuaFile::UiDoRect(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)(int)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    if (!lua_isnumber(L, 1))
        return 0;
    if (!lua_isnumber(L, 2))
        return 0;
    if (!lua_isnumber(L, 3))
        return 0;
    if (!lua_isnumber(L, 4))
        return 0;
    if (!lua_isnumber(L, 5))
        return 0;


    int i = 0;
    for (i = 0; i < LUAMAXUIELEMENTS; i++)
    {
        if (pSelf->m_aUiElements[i].m_Used == 0)
        {
            break;
        }
    }
    if (i >= LUAMAXUIELEMENTS)
        return 0;

    pSelf->m_aUiElements[i].m_Used = true;
    pSelf->m_aUiElements[i].m_Rect.x = lua_tonumber(L, 1);
    pSelf->m_aUiElements[i].m_Rect.y = lua_tonumber(L, 2);
    pSelf->m_aUiElements[i].m_Rect.w = lua_tonumber(L, 3);
    pSelf->m_aUiElements[i].m_Rect.h = lua_tonumber(L, 4);
    pSelf->m_aUiElements[i].m_RegPoint = lua_tonumber(L, 5);

    if (lua_isnumber(L, 6))
        pSelf->m_aUiElements[i].m_Corners = lua_tonumber(L, 6);
    else
        pSelf->m_aUiElements[i].m_Corners = CUI::CORNER_ALL;

    if (lua_isnumber(L, 7))
        pSelf->m_aUiElements[i].m_Rounding = lua_tonumber(L, 7);
    else
        pSelf->m_aUiElements[i].m_Rounding = 5.0f;

    if (lua_isnumber(L, 8) && lua_isnumber(L, 9) && lua_isnumber(L, 10) && lua_isnumber(L, 11))
        pSelf->m_aUiElements[i].m_Color = vec4(lua_tonumber(L, 8), lua_tonumber(L, 9), lua_tonumber(L, 10), lua_tonumber(L, 11));
    else
        pSelf->m_aUiElements[i].m_Color = vec4(0.0f, 0.0f, 0.0f, 0.5f);

    pSelf->m_aUiElements[i].m_pClient = pSelf->m_pClient;
    pSelf->m_aUiElements[i].m_pLuaFile = pSelf;
    pSelf->m_aUiElements[i].m_Type = CLuaUi::LUAUIRECT;

    lua_pushinteger(L, i);

    return 1;
}

int CLuaFile::UiDoImage(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)(int)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    if (!lua_isnumber(L, 1))
        return 0;
    if (!lua_isnumber(L, 2))
        return 0;
    if (!lua_isnumber(L, 3))
        return 0;
    if (!lua_isnumber(L, 4))
        return 0;
    if (!lua_isnumber(L, 5))
        return 0;
    if (!lua_isnumber(L, 6))
        return 0;
    if (!lua_isnumber(L, 7))
        return 0;
    if (!lua_isstring(L, 8))
        return 0;



    int i = 0;
    for (i = 0; i < LUAMAXUIELEMENTS; i++)
    {
        if (pSelf->m_aUiElements[i].m_Used == 0)
        {
            break;
        }
    }
    if (i >= LUAMAXUIELEMENTS)
        return 0;

    pSelf->m_aUiElements[i].m_Used = true;
    pSelf->m_aUiElements[i].m_Rect.x = lua_tonumber(L, 1);
    pSelf->m_aUiElements[i].m_Rect.y = lua_tonumber(L, 2);
    pSelf->m_aUiElements[i].m_Rect.w = lua_tonumber(L, 3);
    pSelf->m_aUiElements[i].m_Rect.h = lua_tonumber(L, 4);
    pSelf->m_aUiElements[i].m_RegPoint = lua_tonumber(L, 5);

    pSelf->m_aUiElements[i].m_TextureID = lua_tonumber(L, 6);
    pSelf->m_aUiElements[i].m_SpriteID = lua_tonumber(L, 7);
    str_copy(pSelf->m_aUiElements[i].m_pCallback, lua_tostring(L, 8), sizeof(pSelf->m_aUiElements[i].m_pCallback));

    pSelf->m_aUiElements[i].m_pClient = pSelf->m_pClient;
    pSelf->m_aUiElements[i].m_pLuaFile = pSelf;
    pSelf->m_aUiElements[i].m_Type = CLuaUi::LUAUIIMAGE;

    lua_pushinteger(L, i);

    return 1;
}

int CLuaFile::UiDoLine(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)(int)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    if (!lua_isnumber(L, 1)) //x1
        return 0;
    if (!lua_isnumber(L, 2)) //y1
        return 0;
    if (!lua_isnumber(L, 3)) //x2
        return 0;
    if (!lua_isnumber(L, 4)) //y2
        return 0;
    if (!lua_isnumber(L, 5))
        return 0;
    if (!lua_isnumber(L, 6))
        return 0;
    if (!lua_isnumber(L, 7))
        return 0;
    if (!lua_isnumber(L, 8))
        return 0;



    int i = 0;
    for (i = 0; i < LUAMAXUIELEMENTS; i++)
    {
        if (pSelf->m_aUiElements[i].m_Used == 0)
        {
            break;
        }
    }
    if (i >= LUAMAXUIELEMENTS)
        return 0;

    pSelf->m_aUiElements[i].m_Used = true;
    pSelf->m_aUiElements[i].m_Rect.x = lua_tonumber(L, 1);
    pSelf->m_aUiElements[i].m_Rect.y = lua_tonumber(L, 2);
    pSelf->m_aUiElements[i].m_Rect.w = lua_tonumber(L, 3);
    pSelf->m_aUiElements[i].m_Rect.h = lua_tonumber(L, 4);

    pSelf->m_aUiElements[i].m_Color = vec4(lua_tonumber(L, 5), lua_tonumber(L, 6), lua_tonumber(L, 7), lua_tonumber(L, 8));

    pSelf->m_aUiElements[i].m_pClient = pSelf->m_pClient;
    pSelf->m_aUiElements[i].m_pLuaFile = pSelf;
    pSelf->m_aUiElements[i].m_Type = CLuaUi::LUAUILINE;

    lua_pushinteger(L, i);

    return 1;
}

int CLuaFile::UiDoSlider(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)(int)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    if (!lua_isnumber(L, 1))
        return 0;
    if (!lua_isnumber(L, 2))
        return 0;
    if (!lua_isnumber(L, 3))
        return 0;
    if (!lua_isnumber(L, 4))
        return 0;
    if (!lua_isnumber(L, 5))
        return 0;
    if (!lua_isnumber(L, 6))
        return 0;
    if (!lua_isstring(L, 7))
        return 0;


    int i = 0;
    for (i = 0; i < LUAMAXUIELEMENTS; i++)
    {
        if (pSelf->m_aUiElements[i].m_Used == 0)
        {
            break;
        }
    }
    if (i >= LUAMAXUIELEMENTS)
        return 0;

    pSelf->m_aUiElements[i].m_Used = true;
    pSelf->m_aUiElements[i].m_Rect.x = lua_tonumber(L, 1);
    pSelf->m_aUiElements[i].m_Rect.y = lua_tonumber(L, 2);
    pSelf->m_aUiElements[i].m_Rect.w = lua_tonumber(L, 3);
    pSelf->m_aUiElements[i].m_Rect.h = lua_tonumber(L, 4);
    pSelf->m_aUiElements[i].m_RegPoint = lua_tonumber(L, 5);

    pSelf->m_aUiElements[i].m_Value = lua_tonumber(L, 6);
    str_copy(pSelf->m_aUiElements[i].m_pCallback, lua_tostring(L, 7), sizeof(pSelf->m_aUiElements[i].m_pCallback));

    if (lua_isnumber(L, 8) && lua_isnumber(L, 9) && lua_isnumber(L, 10) && lua_isnumber(L, 11))
        pSelf->m_aUiElements[i].m_Color = vec4(lua_tonumber(L, 8), lua_tonumber(L, 9), lua_tonumber(L, 10), lua_tonumber(L, 11));
    else
        pSelf->m_aUiElements[i].m_Color = vec4(1,1,1,0.25f);

    if (lua_isnumber(L, 12))
        pSelf->m_aUiElements[i].m_Direction = lua_tonumber(L, 12);
    else
        pSelf->m_aUiElements[i].m_Direction = 0;

    pSelf->m_aUiElements[i].m_pClient = pSelf->m_pClient;
    pSelf->m_aUiElements[i].m_pLuaFile = pSelf;
    pSelf->m_aUiElements[i].m_Type = CLuaUi::LUAUISLIDER;

    lua_pushinteger(L, i);

    return 1;
}

int CLuaFile::UiGetGameTextureID(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)(int)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    lua_pushinteger(L, g_pData->m_aImages[IMAGE_GAME].m_Id);
    return 1;
}

int CLuaFile::UiGetParticleTextureID(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)(int)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    lua_pushinteger(L, g_pData->m_aImages[IMAGE_PARTICLES].m_Id);
    return 1;
}

int CLuaFile::UiGetFlagTextureID(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)(int)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    if (!lua_isnumber(L, 1))
        return 0;

    lua_pushinteger(L, pSelf->m_pClient->m_pCountryFlags->GetByCountryCode(lua_tonumber(L, 1))->m_Texture);
    return 1;
}

int CLuaFile::TextureLoad(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)(int)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    if (!lua_isstring(L, 1))
        return 0;

    lua_pushinteger(L, pSelf->m_pClient->Graphics()->LoadTexture(lua_tostring(L, 1), IStorage::TYPE_ALL, CImageInfo::FORMAT_AUTO, IGraphics::TEXLOAD_NORESAMPLE));
    return 1;
}

int CLuaFile::TextureUnload(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)(int)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    if (!lua_isnumber(L, 1))
        return 0;

    pSelf->m_pClient->Graphics()->UnloadTexture(lua_tointeger(L, 1));
    return 0;
}

int CLuaFile::SendPacket(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)(int)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    if(lua_isnil(L, 1))
        return 0;

	char aData[2000]=" ";
	str_append(aData, (char *)lua_tostring(L, 1), 2000);	
    CMsgPacker P(NETMSG_LUA_DATA);
    P.AddString(aData, 2000);

    pSelf->m_pClient->Client()->SendMsgEx(&P, MSGFLAG_VITAL|MSGFLAG_FLUSH, true);

    return 0;
}

int CLuaFile::FetchPacket(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)(int)lua_touserdata(L, -1);
    lua_Debug Frame;
    lua_getstack(L, 1, &Frame);
    lua_getinfo(L, "nlSf", &Frame);

    if (!pSelf->m_pLuaHandler->m_EventListener.m_pNetData)
        return 0;
	lua_pushstring(L, pSelf->m_pLuaHandler->m_EventListener.m_pNetData);
    return 1;
}
