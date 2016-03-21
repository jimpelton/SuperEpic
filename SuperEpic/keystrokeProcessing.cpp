#include "keystrokeProcessing.h"
#include <stdlib.h>
#include <iostream>



keystrokeProcessing::keystrokeProcessing()
{
}


keystrokeProcessing::~keystrokeProcessing()
{
}

void keystrokeProcessing::processKeyboardInput(void)
{
	SDL_Event event;
	int quitFlag = 0;
	int imageSelectionMode = 1;
	int imageViewerMode = 0;
	int zoomLevel = 1;

	while (quitFlag == 0)
	{
		/* Poll for events */
		while (SDL_PollEvent(&event) != 0) {

			if (event.type == SDL_QUIT)
			{
				quitFlag = 1;
			}
			else
			{
				switch (event.type)
				{
					/* Keyboard event */
					/* Pass the event data onto PrintKeyInfo() */
					case SDL_KEYUP:
						//PrintKeyInfo(&event.key);
						switch (event.key.keysym.sym)
						{
						case SDLK_i:
							if (imageViewerMode)
							{
								// Do something in this case
								zoomLevel++;
								printf("Zooming IN! Zoom Level = %d\n", zoomLevel);
							}
							else if (imageSelectionMode)
							{
								//Do something in this case
								printf("Wrong mode for Single Image manipulations!\n");
							}
							else
							{
								printf("ERROR, UNHANDLED STATE!\n");
							}
							break;

						case SDLK_o:
							if (imageViewerMode)
							{
								//Do something in this case
								zoomLevel--;
								if (zoomLevel < 1)
								{
									printf("Going back to image selection screen!!\n");
									imageViewerMode = 0;
									imageSelectionMode = 1;
									zoomLevel = 1;
								}
								else
								{
									printf("Zooming OUT! Zoom Level = %d\n", zoomLevel);
								}
							}
							else if (imageSelectionMode)
							{
								//Do something in this case
							}
							else
							{
								printf("ERROR, UNHANDLED STATE!\n");
							}
							break;

						case SDLK_w:
							if (imageViewerMode)
							{
								//Do something in this case
								printf("Panning UP!\n");
							}
							else if (imageSelectionMode)
							{
								//Do something in this case
								printf("Wrong mode for Single Image manipulations!\n");
							}
							else
							{
								printf("ERROR, UNHANDLED STATE!\n");
							}
							break;

						case SDLK_a:
							if (imageViewerMode)
							{
								//Do something in this case
								printf("Panning LEFT!\n");
							}
							else if (imageSelectionMode)
							{
								//Do something in this case
								printf("Wrong mode for Single Image manipulations!\n");
							}
							else
							{
								printf("ERROR, UNHANDLED STATE!\n");
							}
							break;

						case SDLK_s:
							if (imageViewerMode)
							{
								//Do something in this case
								printf("Panning DOWN!\n");
							}
							else if (imageSelectionMode)
							{
								//Do something in this case
								printf("Wrong mode for Single Image manipulations!\n");
							}
							else
							{
								printf("ERROR, UNHANDLED STATE!\n");
							}
							break;

						case SDLK_d:
							if (imageViewerMode)
							{
								//Do something in this case
								printf("Panning RIGHT!\n");
							}
							else if (imageSelectionMode)
							{
								//Do something in this case
								printf("Wrong mode for Single Image manipulations!\n");
							}
							else
							{
								printf("ERROR, UNHANDLED STATE!\n");
							}
							break;

						case SDLK_1:
							if (imageViewerMode)
							{
								//Do something in this case
								printf("Wrong mode for image selection!\n");
							}
							else if (imageSelectionMode)
							{
								//Do something in this case
								printf("Selecting Image 1!\n");
								imageViewerMode = 1;
								imageSelectionMode = 0;
							}
							else
							{
								printf("ERROR, UNHANDLED STATE!\n");
							}
							break;

						case SDLK_2:
							if (imageViewerMode)
							{
								//Do something in this case
								printf("Wrong mode for image selection!\n");
							}
							else if (imageSelectionMode)
							{
								//Do something in this case
								printf("Selecting Image 2!\n");
								imageViewerMode = 1;
								imageSelectionMode = 0;
							}
							else
							{
								printf("ERROR, UNHANDLED STATE!\n");
							}
							break;

						case SDLK_3:
							if (imageViewerMode)
							{
								//Do something in this case
								printf("Wrong mode for image selection!\n");
							}
							else if (imageSelectionMode)
							{
								//Do something in this case
								printf("Selecting Image 3!\n");
								imageViewerMode = 1;
								imageSelectionMode = 0;
							}
							else
							{
								printf("ERROR, UNHANDLED STATE!\n");
							}
							break;

						case SDLK_4:
							if (imageViewerMode)
							{
								//Do something in this case
								printf("Wrong mode for image selection!\n");
							}
							else if (imageSelectionMode)
							{
								//Do something in this case
								printf("Selecting Image 4!\n");
								imageViewerMode = 1;
								imageSelectionMode = 0;
							}
							else
							{
								printf("ERROR, UNHANDLED STATE!\n");
							}
							break;

						case SDLK_5:
							if (imageViewerMode)
							{
								//Do something in this case
								printf("Wrong mode for image selection!\n");
							}
							else if (imageSelectionMode)
							{
								//Do something in this case
								printf("Selecting Image 5!\n");
								imageViewerMode = 1;
								imageSelectionMode = 0;
							}
							else
							{
								printf("ERROR, UNHANDLED STATE!\n");
							}
							break;

						case SDLK_k:
							// This case manually toggles the ImageViewer vs ImageSelector mode, in case we get lost somewhere....
							if (imageViewerMode)
							{
								//Do something in this case
								printf("WARNING: Manually Enabling Image Selection Mode!\n");
								imageSelectionMode = 1;
								zoomLevel = 1;
								imageViewerMode = 0;
							}
							else if (imageSelectionMode)
							{
								//Do something in this case
								printf("WARNING: Manually Enabling Image Viewer Mode!\n");
								imageViewerMode = 1;
								imageSelectionMode = 0;
							}
							else
							{
								printf("ERROR, NO WORKING STATE!!!!\n");
							}
							break;

						case SDLK_LEFT:
							// This case manually toggles the ImageViewer vs ImageSelector mode, in case we get lost somewhere....
							if (imageViewerMode)
							{
								//Do something in this case
								printf("Wrong mode for image Manipulation!\n");
							}
							else if (imageSelectionMode)
							{
								//Do something in this case
								printf("Advancing Image Collection to the LEFT!\n");
							}
							else
							{
								printf("ERROR, NO WORKING STATE!!!!\n");
							}
							break;

						case SDLK_RIGHT:
							// This case manually toggles the ImageViewer vs ImageSelector mode, in case we get lost somewhere....
							if (imageViewerMode)
							{
								//Do something in this case
								printf("Wrong mode for image Manipulation!\n");
							}
							else if (imageSelectionMode)
							{
								//Do something in this case
								printf("Advancing Image Collection to the RIGHT!\n");
							}
							else
							{
								printf("ERROR, NO WORKING STATE!!!!\n");
							}
							break;

						case SDLK_q:
							// This case manually toggles the ImageViewer vs ImageSelector mode, in case we get lost somewhere....
							if (imageViewerMode)
							{
								//Do something in this case
								printf("Wrong mode for image Manipulation!\n");
							}
							else if (imageSelectionMode)
							{
								//Do something in this case
								printf("Shutting down the program!\n");
								quitFlag = 1;
							}
							else
							{
								printf("ERROR, NO WORKING STATE!!!!\n");
							}
							break;
						default:
							break;

						}
						break;

					/* SDL_QUIT event (window close) */

					default:
						break;
				}
			}
		}
	}
	printf("Leaving keyboard handler!!\n");
}

/* Print all information about a key event */
void keystrokeProcessing::PrintKeyInfo(SDL_KeyboardEvent *key) {
	/* Is it a release or a press? */
	if (key->type == SDL_KEYUP)
	{
		//printf("Release:- ");
	}
	else
	{
		//printf("Press:- ");
	}

	/* Print the name of the key */
	printf(", Name: %s", SDL_GetKeyName(key->keysym.sym));
	/* We want to print the unicode info, but we need to make */
	/* sure its a press event first (remember, release events */
	/* don't have unicode info                                */
	if (key->type == SDL_KEYDOWN) {
		/* If the Unicode value is less than 0x80 then the    */
		/* unicode value can be used to get a printable       */
		/* representation of the key, using (char)unicode.    */
		printf(", Unicode: ");
		if (key->keysym.sym < 0x80 && key->keysym.sym > 0) {
			printf("%c (0x%04X)", (char)key->keysym.sym,
				key->keysym.sym);
		}
		else {
			printf("? (0x%04X)", key->keysym.sym);
		}
	}
	printf("\n");
	/* Print modifier info */
	//PrintModifiers((SDL_Keymod)key->keysym.mod);
}

/* Print modifier info */
void keystrokeProcessing::PrintModifiers(SDL_Keymod mod) {
	printf("Modifers: ");

	/* If there are none then say so and return */
	if (mod == KMOD_NONE) {
		printf("None\n");
		return;
	}

	/* Check for the presence of each SDLMod value */
	/* This looks messy, but there really isn't    */
	/* a clearer way.                              */
	if (mod & KMOD_NUM)
	{
		printf("NUMLOCK ");
	}

	if (mod & KMOD_CAPS)
	{
		printf("CAPSLOCK ");
	}

	if (mod & KMOD_LCTRL)
	{
		printf("LCTRL ");
	}

	if (mod & KMOD_RCTRL)
	{
		printf("RCTRL ");
	}

	if (mod & KMOD_RSHIFT)
	{
		printf("RSHIFT ");
	} 

	if (mod & KMOD_LSHIFT)
	{
		printf("LSHIFT ");
	}

	if (mod & KMOD_RALT)
	{
		printf("RALT ");
	}

	if (mod & KMOD_LALT)
	{
		printf("LALT ");
	}

	if (mod & KMOD_CTRL)
	{
		printf("CTRL ");
	}

	if (mod & KMOD_SHIFT)
	{
		printf("SHIFT ");
	}

	if (mod & KMOD_ALT)
	{
		printf("ALT ");
	}

	printf("\n");
}