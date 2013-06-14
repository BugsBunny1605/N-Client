/* (c) MAP94 and Patafix. See www.n-lvl.com/ndc/nclient/ for more information. */
/*DGI:Doc-Gen-Info*/
/*DGI:Type:Server*/
/*DGI:Exception:errorfunc*/
/*DGI:Event:OnWeaponFire*/
/*DGI:Event:OnJump*/
/*DGI:Event:OnJump*/
/*DGI:Event:OnDie*/
/*DGI:Event:OnExplosion*/
/*DGI:Event:OnClientEnter*/
/*DGI:Event:OnClientConnect*/
/*DGI:Event:OnChat*/
/*DGI:Event:OnPlayerJoinTeam*/
/*DGI:Event:OnNetData*/
/*DGI:Event:OnCanSpawn*/
/*DGI:Event:OnEntity*/
/*DGI:Event:OnConsole*/
#include <string.h>
#include <time.h>

#include "lua.h"

#include <game/luaglobal.h>

#include "commands/character.cpp"
#include "commands/chat.cpp"
#include "commands/collision.cpp"
#include "commands/console.cpp"
#include "commands/config.cpp"
#include "commands/game.cpp"
#include "commands/events.cpp"
#include "commands/message.cpp"
#include "commands/player.cpp"
#include "commands/entities.cpp"
#include "commands/dummy.cpp"
#include "commands/mysql.cpp"

#define NON_HASED_VERSION
#include <game/version.h>
#undef NON_HASED_VERSION



CLuaFile::CLuaFile()
{
    MySQLInit(); //start mysql thread
    m_MySQLConnected = false;
    m_IncrementalQueryId = 0;
    m_pLua = 0;
    m_pLuaHandler = 0;
    m_pServer = 0;
    Close();
}

CLuaFile::~CLuaFile()
{
    m_MySQLThread.m_Running = false;
    lock_wait(m_MySQLThread.m_MySSQLLock);
    lock_release(m_MySQLThread.m_MySSQLLock);
    lock_destroy(m_MySQLThread.m_MySSQLLock);
#ifndef CONF_PLATFORM_MACOSX
    End();
    if (m_pLua)
        lua_close(m_pLua);
    m_pLua = 0;
#endif
}
void CLuaFile::Tick()
{
    if (!g_Config.m_SvLua)
        return;

    ErrorFunc(m_pLua);
    MySQLTick(); //garbage collector -> clear old results that aren't fetched by lua
    m_pLuaShared->Tick();

    if (!FunctionExist("Tick"))
        return;

    FunctionPrepare("Tick");
    PushInteger((int)(time_get() * 1000 / time_freq()));
    PushInteger(m_pServer->Server()->Tick());
    FunctionExec();

	if (m_pServer->Server()->Tick() % (m_pServer->Server()->TickSpeed() * 60) == 0)
		dbg_msg("lua", "%i kiB", lua_gc(m_pLua, LUA_GCCOUNT, 0));

    lua_gc(m_pLua, LUA_GCCOLLECT, 1000);

    ErrorFunc(m_pLua);
}
void CLuaFile::TickDefered()
{
    if (!g_Config.m_SvLua)
        return;

    ErrorFunc(m_pLua);

    if (!FunctionExist("TickDefered"))
        return;

    FunctionPrepare("TickDefered");
    PushInteger((int)(time_get() * 1000 / time_freq()));
    PushInteger(m_pServer->Server()->Tick());
    FunctionExec();

    ErrorFunc(m_pLua);
}
void CLuaFile::PostTick()
{
    if (!g_Config.m_SvLua)
        return;

    ErrorFunc(m_pLua);

    if (!FunctionExist("PostTick"))
        return;

    FunctionPrepare("PostTick");
    PushInteger((int)(time_get() * 1000 / time_freq()));
    PushInteger(m_pServer->Server()->Tick());
    FunctionExec();

    ErrorFunc(m_pLua);
}

void CLuaFile::End()
{
    if (m_pLua == 0)
        return;

    //try to call the atexit function
    //Maybe the lua file need to save data eg. a ConfigFile
    FunctionExec("atexit");
    m_pLuaShared->Clear();
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

    str_copy(m_aFilename, pFile, sizeof(m_aFilename));

    m_pLua = luaL_newstate();
	dbg_msg("lua", "%i kiB (loaded state)", lua_gc(m_pLua, LUA_GCCOUNT, 0));
    luaL_openlibs(m_pLua);
	dbg_msg("lua", "%i kiB (loaded libs)", lua_gc(m_pLua, LUA_GCCOUNT, 0));

    lua_atpanic(m_pLua, &Panic);

	#define REGISTER_LUA(name) lua_register(m_pLua, ToLower(#name), this->name)
    //include
    REGISTER_LUA(Include);
    luaL_dostring(m_pLua, "package.path = \"./lua/?.lua;./lua/lib/?.lua\"\n");

    //config
    REGISTER_LUA(SetScriptUseSettingPage);
    REGISTER_LUA(SetScriptTitle);
    REGISTER_LUA(SetScriptInfo);

    //events
    REGISTER_LUA(AddEventListener);
    REGISTER_LUA(RemoveEventListener);

    //player
    REGISTER_LUA(GetPlayerIP);
    REGISTER_LUA(GetPlayerSpectateID);
    REGISTER_LUA(GetPlayerName);
    REGISTER_LUA(GetPlayerClan);
    REGISTER_LUA(GetPlayerCountry);
    REGISTER_LUA(GetPlayerScore);
    REGISTER_LUA(GetPlayerPing);
    REGISTER_LUA(GetPlayerTeam);
    REGISTER_LUA(GetPlayerSkin);
    REGISTER_LUA(GetPlayerColorFeet);
    REGISTER_LUA(GetPlayerColorBody);
    REGISTER_LUA(SetPlayerScore);
    REGISTER_LUA(SetPlayerName);
    REGISTER_LUA(SetPlayerTeam);
    REGISTER_LUA(SetPlayerClan);
    REGISTER_LUA(SetPlayerCountry);
    REGISTER_LUA(SetPlayerSpectateID);

    REGISTER_LUA(SetPlayerColorBody);
    REGISTER_LUA(SetPlayerColorFeet);

    //character
    REGISTER_LUA(Emote);
    REGISTER_LUA(GetCharacterPos);
    REGISTER_LUA(GetCharacterVel);
    REGISTER_LUA(SetCharacterPos);
    REGISTER_LUA(SetCharacterVel);

    //config
    REGISTER_LUA(GetConfigValue);
    REGISTER_LUA(SetConfigValue);

    //console
    REGISTER_LUA(Print);
    REGISTER_LUA(Console);

    //game
    REGISTER_LUA(GetGameType);
    REGISTER_LUA(IsTeamplay);

    //message    
    REGISTER_LUA(SendPacket);
    REGISTER_LUA(AddModFile);
    REGISTER_LUA(DeleteModFile);
    REGISTER_LUA(SendFile);


    //collision
    REGISTER_LUA(IntersectLine);
    REGISTER_LUA(GetTile);
    REGISTER_LUA(SetTile);
    REGISTER_LUA(GetMapWidth);
    REGISTER_LUA(GetMapHeight);

    //Chat
    REGISTER_LUA(SendBroadcast);
    REGISTER_LUA(SendChat);
    REGISTER_LUA(SendChatTarget);

    //Entities
    REGISTER_LUA(EntityFind);
    REGISTER_LUA(EntityGetCharacterId);
    REGISTER_LUA(EntityGetPos);
    REGISTER_LUA(EntitySetPos);
    REGISTER_LUA(EntityDestroy);
    REGISTER_LUA(ProjectileFind);
    REGISTER_LUA(ProjectileGetWeapon);
    REGISTER_LUA(ProjectileGetOwner);
    REGISTER_LUA(ProjectileGetPos);
    REGISTER_LUA(ProjectileGetDir);
    REGISTER_LUA(ProjectileGetLifespan);
    REGISTER_LUA(ProjectileGetExplosive);
    REGISTER_LUA(ProjectileGetSoundImpact);
    REGISTER_LUA(ProjectileGetStartTick);
    REGISTER_LUA(ProjectileSetWeapon);
    REGISTER_LUA(ProjectileSetOwner);
    REGISTER_LUA(ProjectileSetStartPos);
    REGISTER_LUA(ProjectileSetDir);
    REGISTER_LUA(ProjectileSetLifespan);
    REGISTER_LUA(ProjectileSetExplosive);
    REGISTER_LUA(ProjectileSetSoundImpact);
    REGISTER_LUA(ProjectileSetStartTick);
    REGISTER_LUA(ProjectileCreate);
    REGISTER_LUA(LaserCreate);


    //game
    REGISTER_LUA(CreateExplosion);
    REGISTER_LUA(CreateDeath);
    REGISTER_LUA(CreateDamageIndicator);
    REGISTER_LUA(CreateHammerHit);
    REGISTER_LUA(CreateSound);

    //tunings
    REGISTER_LUA(GetTuning);
    REGISTER_LUA(SetTuning);

    REGISTER_LUA(CharacterSetInputDirection);
    REGISTER_LUA(CharacterSetInputJump);
    REGISTER_LUA(CharacterSetInputWeapon);
    REGISTER_LUA(CharacterSetInputTarget);
    REGISTER_LUA(CharacterSetInputHook);
    REGISTER_LUA(CharacterSetInputFire);
    REGISTER_LUA(CharacterGetCoreJumped);
    REGISTER_LUA(CharacterSpawn);
    REGISTER_LUA(CharacterIsAlive);
    REGISTER_LUA(CharacterKill);
    REGISTER_LUA(CharacterIsGrounded);
    REGISTER_LUA(CharacterIncreaseHealth);
    REGISTER_LUA(CharacterIncreaseArmor);
    REGISTER_LUA(CharacterSetAmmo);
    REGISTER_LUA(CharacterGetAmmo);
    REGISTER_LUA(CharacterGetInputTarget);
    REGISTER_LUA(CharacterGetActiveWeapon);
    REGISTER_LUA(CharacterSetActiveWeapon);
    REGISTER_LUA(CharacterDirectInput);
    REGISTER_LUA(CharacterPredictedInput);
    REGISTER_LUA(CharacterGetHealth);
    REGISTER_LUA(CharacterGetArmor);
    REGISTER_LUA(CharacterSetHealth);
    REGISTER_LUA(CharacterSetArmor);
    REGISTER_LUA(CharacterTakeDamage);

    REGISTER_LUA(SendCharacterInfo);

    REGISTER_LUA(SetAutoRespawn);

    REGISTER_LUA(Win);
    REGISTER_LUA(SetGametype);

    REGISTER_LUA(DummyCreate);
    REGISTER_LUA(IsDummy);

    //version
    REGISTER_LUA(CheckVersion);
    REGISTER_LUA(GetVersion);

    REGISTER_LUA(CreateDirectory);
    REGISTER_LUA(GetDate);

    REGISTER_LUA(GetTick);
    REGISTER_LUA(GetTickSpeed);

    //MySQL
    REGISTER_LUA(MySQLConnect);
    REGISTER_LUA(MySQLEscapeString);
    REGISTER_LUA(MySQLSelectDatabase);
    REGISTER_LUA(MySQLIsConnected);
    REGISTER_LUA(MySQLQuery);
    REGISTER_LUA(MySQLClose);
    REGISTER_LUA(MySQLFetchResults);
	
	#undef REGISTER_LUA
	
    m_pLuaShared = new CLuaShared<CLuaFile>(this);

    lua_pushlightuserdata(m_pLua, this);
    lua_setglobal(m_pLua, "pLUA");

    lua_register(m_pLua, ToLower("errorfunc"), this->ErrorFunc);

	dbg_msg("lua", "%i kiB (loaded fx)", lua_gc(m_pLua, LUA_GCCOUNT, 0));
    if (luaL_loadfile(m_pLua, m_aFilename) == 0)
    {
        lua_pcall(m_pLua, 0, LUA_MULTRET, 0);
        ErrorFunc(m_pLua);
		dbg_msg("lua", "%i kiB (loaded file)", lua_gc(m_pLua, LUA_GCCOUNT, 0));
    }
    else
    {
        ErrorFunc(m_pLua);
        dbg_msg("lua", "fail to load file: %s", pFile);
        Close();
        return;
    }
}

void CLuaFile::Close()
{
    //kill lua
    if (m_pLua)
        lua_close(m_pLua);
    m_pLua = 0;

    //clear
    mem_zero(m_aTitle, sizeof(m_aTitle));
    mem_zero(m_aInfo, sizeof(m_aInfo));
    mem_zero(m_aFilename, sizeof(m_aFilename));
    m_HaveSettings = 0;
    m_FunctionVarNum = 0;
}

int CLuaFile::ErrorFunc(lua_State *L)
{
    lua_getglobal(L, "pLUA");
    CLuaFile *pSelf = (CLuaFile *)lua_touserdata(L, -1);

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

void CLuaFile::PushData(const char *pData, int Size)
{
    if (m_pLua == 0)
        return;
    lua_pushlstring(m_pLua, pData, Size);
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
    bool Ret = false;
    if (m_pLua == 0)
        return false;
    lua_getglobal(m_pLua, ToLower(pFunctionName));
    Ret = lua_isfunction(m_pLua, -1);
    lua_pop(m_pLua, 1);
    return Ret;
}

void CLuaFile::FunctionPrepare(const char *pFunctionName)
{
    if (m_pLua == 0 || m_aFilename[0] == 0)
        return;

    lua_getglobal(m_pLua, ToLower(pFunctionName));
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
        FunctionPrepare(pFunctionName);
    }
    if (lua_pcall(m_pLua, m_FunctionVarNum, LUA_MULTRET, 0))
        ErrorFunc(m_pLua);
    m_FunctionVarNum = 0;
}


//functions

int CLuaFile::Include(lua_State *L)
{
    LUA_FUNCTION_HEADER

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
    LUA_FUNCTION_HEADER

    if (!lua_isnumber(L, 1))
        return 0;
    pSelf->m_HaveSettings = lua_tointeger(L, 1);
    return 0;
}

int CLuaFile::SetScriptTitle(lua_State *L)
{
    LUA_FUNCTION_HEADER

    if (!lua_isstring(L, 1))
        return 0;
    str_copy(pSelf->m_aTitle, lua_tostring(L, 1), sizeof(pSelf->m_aTitle));
    return 0;
}

int CLuaFile::SetScriptInfo(lua_State *L)
{
    LUA_FUNCTION_HEADER

    if (!lua_isstring(L, 1))
        return 0;
    str_copy(pSelf->m_aInfo, lua_tostring(L, 1), sizeof(pSelf->m_aInfo));
    return 0;
}

int CLuaFile::CheckVersion(lua_State *L)
{
    LUA_FUNCTION_HEADER

    if (lua_isstring(L, 1))
        lua_pushboolean(L, str_comp(GAME_LUA_VERSION, lua_tostring(L, 1)) == 0);
    else
        lua_pushboolean(L, false);
    return 1;
}

int CLuaFile::GetVersion(lua_State *L)
{
    LUA_FUNCTION_HEADER

    lua_pushstring(L, GAME_LUA_VERSION);
    return 1;
}

int CLuaFile::GetTick(lua_State *L)
{
    LUA_FUNCTION_HEADER

    lua_pushnumber(L, pSelf->m_pServer->Server()->Tick());
    return 1;
}

int CLuaFile::GetTickSpeed(lua_State *L)
{
    LUA_FUNCTION_HEADER

    lua_pushnumber(L, pSelf->m_pServer->Server()->TickSpeed());
    return 1;
}

int CLuaFile::CreateDirectory(lua_State *L)
{
    LUA_FUNCTION_HEADER
    if(!lua_isstring(L, 1))
        return 0;

    lua_pushboolean(L, fs_makedir(lua_tostring(L, 1)));
    return 1;
}

int CLuaFile::GetDate (lua_State *L) //from loslib.c
{
    const char *s = luaL_optstring(L, 1, "%c");
    time_t t = luaL_opt(L, (time_t)luaL_checknumber, 2, time(NULL));
    struct tm tmr, *stm;
    if (*s == '!')    /* UTC? */
    {
        stm = l_gmtime(&t, &tmr);
        s++;  /* skip `!' */
    }
    else
        stm = l_localtime(&t, &tmr);
    if (stm == NULL)  /* invalid date? */
        lua_pushnil(L);
    else if (strcmp(s, "*t") == 0)
    {
        lua_createtable(L, 0, 9);  /* 9 = number of fields */
        setfield(L, "sec", stm->tm_sec);
        setfield(L, "min", stm->tm_min);
        setfield(L, "hour", stm->tm_hour);
        setfield(L, "day", stm->tm_mday);
        setfield(L, "month", stm->tm_mon+1);
        setfield(L, "year", stm->tm_year+1900);
        setfield(L, "wday", stm->tm_wday+1);
        setfield(L, "yday", stm->tm_yday+1);
        setboolfield(L, "isdst", stm->tm_isdst);
    }
    else
    {
        char cc[4];
        luaL_Buffer b;
        cc[0] = '%';
        luaL_buffinit(L, &b);
        while (*s)
        {
            if (*s != '%')  /* no conversion specifier? */
                luaL_addchar(&b, *s++);
            else
            {
                size_t reslen;
                char buff[200];  /* should be big enough for any conversion result */
                s = checkoption(L, s + 1, cc);
                reslen = strftime(buff, sizeof(buff), cc, stm);
                luaL_addlstring(&b, buff, reslen);
            }
        }
        luaL_pushresult(&b);
    }
    return 1;
}
