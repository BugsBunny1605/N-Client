#ifndef GAME_CLIENT_COMPONENTS_LUARENDER_H
#define GAME_CLIENT_COMPONENTS_LUARENDER_H
#include <game/client/component.h>

class CLuaRender : public CComponent
{
	int m_Level;	
public:
	CLuaRender();
	virtual void OnRender();
};

#endif

