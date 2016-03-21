#pragma once

#include "SDL.h"
class keystrokeProcessing
{
public:
	keystrokeProcessing();
	~keystrokeProcessing();
	void processKeyboardInput(void);
	
private:
	void PrintKeyInfo(SDL_KeyboardEvent * key);
	void PrintModifiers(SDL_Keymod mod);
};


