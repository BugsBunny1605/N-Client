#ifndef GAME_CLIENT_COMPONENTS_LUAINPUT_H
#define GAME_CLIENT_COMPONENTS_LUAINPUT_H
#include <game/client/component.h>

class CLuaInput : public CComponent
{
	int m_Level;	
public:
	CLuaInput();
	virtual bool OnInput(IInput::CEvent Event);
};

#endif


