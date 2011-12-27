/* (c) MAP94. See www.n-lvl.com/ndc/nclient/ for more information. */
#ifndef GAME_LUA_H
#define GAME_LUA_H

#include "gameclient.h"
#include <engine/shared/config.h>
#include <engine/config.h>
#include <base/tl/array.h>
#include <base/tl/sorted_array.h>

extern "C" { // lua
    #define LUA_CORE /* make sure that we don't try to import these functions */
    #include <engine/external/lua/lua.h>
    #include <engine/external/lua/lualib.h> /* luaL_openlibs */
    #include <engine/external/lua/lauxlib.h> /* luaL_loadfile */
}

class CLuaEventListener
{
    struct CLuaListenerData
    {
        class CLuaFile *m_pLuaFile;
        char m_aLuaFunction[256];
        char m_aEvent[256];
        bool operator==(const CLuaListenerData &Other) { return this == &Other; }
    };

    array<CLuaListenerData> m_aListeners;
public:
    void AddEventListener(class CLuaFile *pLuaFile, char *pEvent, char *pLuaFunction);
    void RemoveEventListener(class CLuaFile *pLuaFile, char *pEvent);

    void OnEvent(char *pEvent);

    CLuaEventListener();
    ~CLuaEventListener();

    //values
    //Menu Browser GameType OnRender Color
    char *m_pBrowserActivGameTypeName;
    vec4 m_BrowserActivGameTypeColor;

    //Scoreboard On Render
    bool m_ScoreboardSkipRender;

    //Chat OnChat
    char *m_pChatText;
    int m_ChatClientID;
    int m_ChatTeam;
    bool m_ChatHide;

    //Kill
    int m_KillKillerID;
    int m_KillVictimID;
    int m_KillWeapon;

    //OnStateChange
    int m_StateOld;

    //OnNetData
    char *m_pNetData;
};

class CLuaBinding
{
public:
    CGameClient *m_pClient;
    CLuaBinding(CGameClient *pClient);
    ~CLuaBinding();
    //Control binding
    int m_ControlDirectionPre;
    int m_ControlDirection;
    bool m_ControlDirectionIsSet;
    int m_ControlJumpPre;
    int m_ControlJump;
    bool m_ControlJumpIsSet;
    int m_ControlHookPre;
    int m_ControlHook;
    bool m_ControlHookIsSet;
    int m_ControlFirePre;
    int m_ControlFire;
    bool m_ControlFireIsSet;
    int m_ControlWeaponPre;
    int m_ControlWeapon;
    bool m_ControlWeaponIsSet;
    int m_ControlTargetXPre;
    int m_ControlTargetX;
    bool m_ControlTargetXIsSet;
    int m_ControlTargetYPre;
    int m_ControlTargetY;
    bool m_ControlTargetYIsSet;
};

class CLuaUi
{
public:
    CLuaUi();
    ~CLuaUi();
    //button
    int m_Checked;
    CUIRect m_Rect;
    //edit box
    float m_FontSize;
    bool m_Hidden;

    //Label
    int m_Align;

    //slider
    float m_Value;
    int m_Direction;

    int m_Corners;
    int m_Rounding;
    vec4 m_Color;

    char m_pText[256];


    //some system things
    int m_Type;
    enum
    {
        LUAUIBUTTON = 1,
        LUAUIEDITBOX,
        LUAUILABEL,
        LUAUIRECT,
        LUAUIIMAGE,
        LUAUILINE,
        LUAUISLIDER,
    };
    int m_Id;
    float m_Offset; //for edit boxes
    char m_pCallback[256];
    CGameClient *m_pClient;
    class CLuaFile *m_pLuaFile;
    int m_RegPoint;

    int m_TextureID;
    int m_SpriteID;

    bool m_Used;
    void Tick();

    vec4 ButtonColorMul(const void *pID);
    int DoButton_Menu(const void *pID, const char *pText, int Checked, const CUIRect *pRect, int Corners, vec4 Color);
    int DoEditBox(void *pID, const CUIRect *pRect, char *pStr, unsigned StrSize, float FontSize, float *Offset, bool Hidden, int Corners, vec4 Color);
    int DoImage(int *pID, int TextureID, int SpriteID, const CUIRect *pRect);
	float DoScrollbarV(const void *pID, const CUIRect *pRect, float Current, vec4 Color);
	float DoScrollbarH(const void *pID, const CUIRect *pRect, float Current, vec4 Color);

};

class CLuaFile
{
public:
    CLuaFile();
    ~CLuaFile();
    class CLua *m_pLuaHandler;
    CGameClient *m_pClient;
    void UiTick();
    void Tick();
    void End();
    void Close();
    void Init(const char *pFile);

    //Some Error and Lua stuff
    //Error
    static int ErrorFunc(lua_State *L);
    static int Panic(lua_State *L); //lua panic function
    lua_State *m_pLua;

    enum
    {
        LUAMAXUIELEMENTS = 256,
    };

    CLuaUi m_aUiElements[LUAMAXUIELEMENTS];

    char *GetScriptName() {return m_aFilename;};

    array<int> m_lTextures;
    array<int> m_lSounds;

    //Settings
    void ConfigClose(); //Helper function
    bool m_HaveSettings;
    char m_aTitle[64];
    char m_aFilename[256];
    char m_aInfo[256];

    bool FunctionExist(const char *pFunctionName);
    void FunctionExec(const char *pFunctionName = 0);
    void FunctionPrepare(const char *pFunctionName);
    void PushString(const char *pString);
    void PushInteger(int value);
    void PushFloat(float value);
    void PushBoolean(bool value);
    void PushParameter(const char *pString);
    int m_FunctionVarNum;

    //Functions:
    //Settings
    static inline int SetScriptUseSettingPage(lua_State *L);
    static inline int SetScriptTitle(lua_State *L);
    static inline int SetScriptInfo(lua_State *L);

    //Eventlistener stuff
    static inline int AddEventListener(lua_State *L);
    static inline int RemoveEventListener(lua_State *L);

    //Menu Browser Things
    static inline int SetMenuBrowserGameTypeColor(lua_State *L);
    static inline int GetMenuBrowserGameTypeName(lua_State *L);

    //Menu
    static inline int MenuActiv(lua_State *L);
    static inline int MenuGameActiv(lua_State *L);
    static inline int MenuPlayersActiv(lua_State *L);
    static inline int MenuServerInfoActiv(lua_State *L);
    static inline int MenuCallVoteActiv(lua_State *L);
    static inline int MenuServersActiv(lua_State *L);
    static inline int MenuMusicActiv(lua_State *L);
    static inline int MenuDemosActiv(lua_State *L);

    //Mouse and Keyboard
    static inline int GetMousePosMenu(lua_State *L);
    static inline int SetMouseModeRelativ(lua_State *L);
    static inline int SetMouseModeAbsolute(lua_State *L);

    //Scoreboard
    static inline int ScoreboardAbortRender(lua_State *L);

    //Chat
    static inline int ChatGetText(lua_State *L);
    static inline int ChatGetClientID(lua_State *L);
    static inline int ChatGetTeam(lua_State *L);
    static inline int ChatHide(lua_State *L);

    //Kill
    static inline int KillGetKillerID(lua_State *L);
    static inline int KillGetVictimID(lua_State *L);
    static inline int KillGetWeapon(lua_State *L);

    //
    //Include
    static inline int Include(lua_State *L);

    //Sendinfo
    static inline int SendPlayerInfo(lua_State *L);

    //emote
    static inline int Emote(lua_State *L);

    //Character
    static inline int GetLocalCharacterId(lua_State *L);
    static inline int GetCharacterPos(lua_State *L);
    static inline int GetCharacterVel(lua_State *L);

    //collision
    static inline int IntersectLine(lua_State *L);
    static inline int GetTile(lua_State *L);
    static inline int GetMapWidth(lua_State *L);
    static inline int GetMapHeight(lua_State *L);
    static inline int SetTile(lua_State *L);

	//layers
	static inline int GetNumGroups(lua_State *L);
	static inline int GetNumLayers(lua_State *L);
	static inline int GetGroupNumLayers(lua_State *L);
	static inline int GetLayerType(lua_State *L);
	static inline int GetLayerFlags(lua_State *L);
	static inline int GetLayerTileFlags(lua_State *L);
	static inline int GetLayerTileIndex(lua_State *L);
	static inline int SetLayerTileFlags(lua_State *L);
	static inline int SetLayerTileIndex(lua_State *L);
	static inline int GetLayerSize(lua_State *L);
	static inline int RenderTilemapGenerateSkip(lua_State *L);

    static inline int CreateParticle(lua_State *L);

    //Flow
    static inline int GetFlow(lua_State *L);
    static inline int SetFlow(lua_State *L);

    //Console Print
    static inline int Print(lua_State *L);
    static inline int Console(lua_State *L);

    //States
    static inline int StateGetOld(lua_State *L);
    static inline int StateGet(lua_State *L);
    static inline int StateOnline(lua_State *L);
    static inline int StateOffline(lua_State *L);
    static inline int StateConnecting(lua_State *L);
    static inline int StateLoading(lua_State *L);
    static inline int StateDemoplayback(lua_State *L);

    //Serverinfo
    static inline int GetGameType(lua_State *L);
    static inline int IsTeamplay(lua_State *L);

    //Get Net Error
    static inline int GetNetError(lua_State *L);

    //Connect
    static inline int Connect(lua_State *L);

    //Chat
    static inline int ChatSend(lua_State *L);
    static inline int ChatTeamSend(lua_State *L);

    //Player
    static inline int GetPlayerName(lua_State *L);
    static inline int GetPlayerClan(lua_State *L);
    static inline int GetPlayerCountry(lua_State *L);
    static inline int GetPlayerScore(lua_State *L);
    static inline int GetPlayerPing(lua_State *L);
    static inline int GetPlayerTeam(lua_State *L);
    static inline int GetPlayerSkin(lua_State *L);
    static inline int GetPlayerColorFeet(lua_State *L);
    static inline int GetPlayerColorBody(lua_State *L);
    static inline int GetPlayerColorSkin(lua_State *L);

    //Ui
    static inline int UiDoButton(lua_State *L);
    static inline int UiDoEditBox(lua_State *L);
    static inline int UiDoLabel(lua_State *L);
    static inline int UiDoRect(lua_State *L);
    static inline int UiDoImage(lua_State *L);
    static inline int UiDoLine(lua_State *L);
    static inline int UiDoSlider(lua_State *L);
    static inline int UiGetText(lua_State *L);
    static inline int UiSetText(lua_State *L);
    static inline int UiGetColor(lua_State *L);
    static inline int UiSetColor(lua_State *L);
    static inline int UiGetRect(lua_State *L);
    static inline int UiSetRect(lua_State *L);
    static inline int UiRemoveElement(lua_State *L);
    static inline int UiGetScreenWidth(lua_State *L);
    static inline int UiGetScreenHeight(lua_State *L);
    static inline int UiGetGameTextureID(lua_State *L);
    static inline int UiGetParticleTextureID(lua_State *L);
    static inline int UiGetFlagTextureID(lua_State *L);

    //Texture
    static inline int TextureLoad(lua_State *L);
    static inline int TextureUnload(lua_State *L);
    static inline int RenderTexture(lua_State *L);

    //Music
    static inline int MusicPlay(lua_State *L);
    static inline int MusicPause(lua_State *L);
    static inline int MusicStop(lua_State *L);
    static inline int MusicNext(lua_State *L);
    static inline int MusicPrev(lua_State *L);
    static inline int MusicSetVol(lua_State *L);
    static inline int MusicGetVol(lua_State *L);
    static inline int MusicGetState(lua_State *L);
    static inline int MusicGetPlayedIndex(lua_State *L);

    //Config
    static inline int GetConfigValue(lua_State *L);
    static inline int SetConfigValue(lua_State *L);

    //Control
    static inline int GetControlValue(lua_State *L);
    static inline int SetControlValue(lua_State *L);
    static inline int UnSetControlValue(lua_State *L);

    //Sound
    static inline int LoadWvFile(lua_State *L);
    static inline int PlayWv(lua_State *L);

    //LuaNetWork
    static inline int FetchPacket(lua_State *L);
    static inline int SendPacket(lua_State *L);

    //Replace Texture
    static inline int ReplaceGameTexture(lua_State *L);
};

class CLua
{
public:
    CGameClient *m_pClient;
    CLua(CGameClient *pClient);
    ~CLua();
    void Tick();
    bool Init(const char *pFile);
    void End();
    void Close();

    CLuaFile m_aLuaFiles[MAX_LUA_FILES];
    CLuaEventListener m_EventListener;

    //Mouse
    bool m_MouseModeAbsolute;

    //Search the file and execs the function
    void ConfigClose(char *pFilename);

    int GetFileId(char *pFilename);

    int m_OriginalGameTexture;
};


//helper functions
static int StrIsInteger(const char *pStr)
{
	while(*pStr)
	{
		if(!(*pStr >= '0' && *pStr <= '9'))
			return 0;
		pStr++;
	}
	return 1;
}
static int StrIsFloat(const char *pStr)
{
	while(*pStr)
	{
		if(!(*pStr >= '0' && *pStr <= '9' || *pStr == '.'))
			return 0;
		pStr++;
	}
	return 1;
}

#endif
