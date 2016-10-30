#ifndef ICON_MENU_H
#define ICON_MENU_H

#include <wut.h>
#include <memory.h>
#include <math.h>
#include <coreinit/debug.h>
#include <gx2/texture.h>

#include "draw.h"
#include "font.h"
#include "gui.h"
#include "button.h"

enum pazaak_card_type
{
    CARD_DRAW = 0,
    CARD_ADD,
    CARD_SUB,
    CARD_FLIP,
    CARD_SPECIAL,
};

enum pazaak_special_type
{
    CARD_TIEBREAK = 1,
    CARD_24,
    CARD_36,
    CARD_DOUBLE,
};

struct pazaak_card
{
    int value;
    int type;
    bool subtract;
} pazaak_card;

void play_menu_init();
void play_menu_update();
void play_menu_draw(bool screen);
void play_menu_draw_icons();

#endif
