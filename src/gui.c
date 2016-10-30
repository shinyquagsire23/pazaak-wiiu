#include "gui.h"

//Common GUI Textures
GX2Texture textureBackdrop;
GX2Texture textureStand;
GX2Texture textureEndTurn;
GX2Texture textureBack;
GX2Texture textureDraws[10];
GX2Texture textureAdd[6];
GX2Texture textureSub[6];
GX2Texture textureFlip[6];
GX2Texture textureSpecial[4];
GX2Texture fontTexture;

//Stuff for state
VPADStatus vpad;
float tpXPos = 0.0f;
float tpYPos = 0.0f;
bool tpTouched = false;
bool app_is_running = true;

//Transition state
bool gui_transitioning = true;
bool gui_transitioning_in = true;
int gui_transition_stationary_frames = 0;
float gui_transition_scale = 0.9f;
float gui_transition_alpha = 0.0f;
float gui_transition_xshift = 0.0f;
float gui_transition_yshift = 0.0f;
float gui_transition_scale_delta = 0.0f;
float gui_transition_alpha_delta = 0.0f;
float gui_transition_xshift_delta = 0.0f;
float gui_transition_yshift_delta = 0.0f;

int current_state = STATE_LOADING;

void gui_init()
{
    current_state = STATE_LOADING;

    load_img_texture(&textureBackdrop, "backdrop.png");
    load_img_texture(&textureStand, "stand.png");
    load_img_texture(&textureEndTurn, "end.png");
    
    load_img_texture_mask(&textureBack, "cards/pazaakBack.png", "cards/mask.png");
    for(int i = 0; i < 10; i++)
    {
        char temp[0x200];
        snprintf(temp, 0x200, "cards/pazaak%u.png", i+1);
        load_img_texture_mask(&textureDraws[i], temp, "cards/mask.png");
    }
    for(int i = 0; i < 6; i++)
    {
        char temp[0x200];
        snprintf(temp, 0x200, "cards/pazaakP%u.png", i+1);
        load_img_texture_mask(&textureAdd[i], temp, "cards/mask.png");
    }
    for(int i = 0; i < 6; i++)
    {
        char temp[0x200];
        snprintf(temp, 0x200, "cards/pazaakM%u.png", i+1);
        load_img_texture_mask(&textureSub[i], temp, "cards/mask.png");
    }
    for(int i = 0; i < 6; i++)
    {
        char temp[0x200];
        snprintf(temp, 0x200, "cards/pazaakPM%u.png", i+1);
        load_img_texture_mask(&textureFlip[i], temp, "cards/mask.png");
    }
    load_img_texture_mask(&textureSpecial[0], "cards/pazaakG1.png", "cards/mask.png");
    load_img_texture_mask(&textureSpecial[1], "cards/pazaakG24.png", "cards/mask.png");
    load_img_texture_mask(&textureSpecial[2], "cards/pazaakG36.png", "cards/mask.png");
    load_img_texture_mask(&textureSpecial[3], "cards/pazaakGD.png", "cards/mask.png");
    
    current_state = STATE_MAIN_MENU;
}

void gui_transition_update()
{
    if(gui_transitioning)
    {
        if(gui_transitioning_in)
        {
            gui_transition_scale += gui_transition_scale_delta;
            gui_transition_alpha += gui_transition_alpha_delta;
            gui_transition_xshift += gui_transition_xshift_delta;
            gui_transition_yshift += gui_transition_yshift_delta;
            
            if(gui_transition_scale > 1.0f)
                gui_transition_scale = 1.0f;
            if(gui_transition_alpha > 1.0f)
                gui_transition_alpha = 1.0f;
                
            if((gui_transition_yshift_delta < 0.0f && gui_transition_yshift < 0.0f) || (gui_transition_yshift_delta > 0.0f && gui_transition_yshift > 0.0f))
                gui_transition_yshift = 0.0f;
            if((gui_transition_xshift_delta < 0.0f && gui_transition_xshift < 0.0f) || (gui_transition_xshift_delta > 0.0f && gui_transition_xshift > 0.0f))
                gui_transition_xshift = 0.0f;
                
            if(gui_transition_scale >= 1.0f && gui_transition_alpha >= 1.0f)
            {
                gui_transition_stationary_frames++;
                if(gui_transition_stationary_frames > 10)
                {
                    gui_transitioning = false;
                    gui_transition_stationary_frames = 0;
                }
            }
        }
        else
        {
            gui_transition_scale += gui_transition_scale_delta;
            gui_transition_alpha += gui_transition_alpha_delta;
            gui_transition_xshift += gui_transition_xshift_delta;
            gui_transition_yshift += gui_transition_yshift_delta;
            
            if(gui_transition_alpha < 0.0f)
                gui_transition_alpha = 0.0f;
                
            if(gui_transition_alpha <= 0.0f)
            {
                gui_transition_stationary_frames++;
                if(gui_transition_stationary_frames > 10)
                {
                    gui_transitioning = false;
                    gui_transition_stationary_frames = 0;
                }
            }
        }
    }
    
    draw_set_global_alpha(gui_transition_alpha);
    draw_set_global_scale(gui_transition_scale);
    draw_set_global_xshift(gui_transition_xshift);
    draw_set_global_yshift(gui_transition_yshift);
}

void gui_transition_in(float start_scale, float start_xshift, float start_yshift, float frames)
{
    gui_transition_scale = start_scale;  
    gui_transition_alpha = 0.0f;
    gui_transition_xshift = start_xshift;
    gui_transition_yshift = start_yshift;
    gui_transition_scale_delta = (1.0f - gui_transition_scale) / frames;
    gui_transition_alpha_delta = (1.0f - gui_transition_alpha) / frames;
    gui_transition_xshift_delta = (0.0f - gui_transition_xshift) / frames;
    gui_transition_yshift_delta = (0.0f - gui_transition_yshift) / frames;
    
    gui_transitioning = true;
    gui_transitioning_in = true;
}

void gui_transition_out(float end_scale, float end_xshift, float end_yshift, float frames)
{
    gui_transition_alpha = 1.0f;
    gui_transition_xshift = 0.0f;
    gui_transition_yshift = 0.0f;
    gui_transition_scale_delta = (end_scale - gui_transition_scale) / frames;
    gui_transition_alpha_delta = (0.0f - gui_transition_alpha) / frames;
    gui_transition_xshift_delta = (end_xshift - gui_transition_xshift) / frames;
    gui_transition_yshift_delta = (end_yshift - gui_transition_yshift) / frames;

    gui_transitioning = true;
    gui_transitioning_in = false;
}

bool gui_transition_done()
{
    return !gui_transitioning;
}
