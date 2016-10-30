#ifndef GUI_H
#define GUI_H

#include <wut.h>
#include <memory.h>
#include <gx2/texture.h>
#include <vpad/input.h>

#include "draw.h"
#include "font.h"
#include "texture.h"

enum available_states
{
    STATE_LOADING = 0,
    STATE_MAIN_MENU,
};

//Common GUI Textures
GX2Texture textureBackdrop;
GX2Texture textureStand;
GX2Texture textureBack;
GX2Texture textureEndTurn;
GX2Texture textureDraws[10];
GX2Texture textureAdd[6];
GX2Texture textureSub[6];
GX2Texture textureFlip[6];
GX2Texture textureSpecial[4];
GX2Texture fontTexture;

//State variables
VPADStatus vpad;
float tpXPos;
float tpYPos;
bool tpTouched;
bool app_is_running;

int current_state;

void gui_init();
void gui_transition_update();
void gui_transition_in(float start_scale, float start_xshift, float start_yshift, float frames);
void gui_transition_out(float end_scale, float end_xshift, float end_yshift, float frames);
bool gui_transition_done();

#endif
