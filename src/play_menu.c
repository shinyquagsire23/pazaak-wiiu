#include "play_menu.h"

#define SHOW_ENEMY_HAND 0

//Screen transition state
int play_next_state;
bool play_transitioning_out = false;

bool player_standing = false;
bool enemy_standing = false;

bool player_full = false;
bool enemy_full = false;

bool player_turn = true;
bool player_drew_card = false;
bool player_played_card = false;

bool game_over = false;
bool player_won = false;
bool game_tied = false;

int player_total;
int enemy_total;

struct pazaak_card player_field[9];
struct pazaak_card enemy_field[9];

struct pazaak_card player_hand[4];
struct pazaak_card enemy_hand[4];

button_t *player_hand_buttons[4];
button_t *stand_button;
button_t *end_button;

int player_score = 0;
int enemy_score = 0;

GX2Texture *get_card_texture(struct pazaak_card card)
{
    switch(card.type)
    {
        default:
        case CARD_DRAW:
            return &textureDraws[card.value-1];
            break;
        case CARD_ADD:
            return &textureAdd[card.value-1];
            break;
        case CARD_SUB:
            return &textureSub[card.value-1];
            break;
        case CARD_FLIP:
            return &textureFlip[card.value-1];
            break;
        case CARD_SPECIAL:
            return &textureSpecial[card.value-1];
            break;
    }
}

void recalculate_card_totals()
{
    player_total = 0;
    enemy_total = 0;
    
    for(int i = 0; i < 9; i++)
    {
        if(player_field[i].type != CARD_SPECIAL)
        {
            player_total += player_field[i].value * (player_field[i].subtract ? -1 : 1);
        }
        else
        {
            switch(player_field[i].value)
            {
                case CARD_TIEBREAK:
                    player_total += player_field[i].value * (player_field[i].subtract ? -1 : 1);
                    break;
                
                case CARD_24:
                case CARD_36:
                    break;
                    
                case CARD_DOUBLE:
                    player_total *= 2;
                    break;
            }
        }
    }
    
    for(int i = 0; i < 9; i++)
    {
        enemy_total += enemy_field[i].value * (enemy_field[i].subtract ? -1 : 1);
    }
}

bool draw_card(struct pazaak_card *field)
{
    bool future_24 = false;
    bool future_36 = false;
    for(int i = 0; i < 9; i++)
    {
        //Apply negations with special cards as they are played
        if(field[i].type == CARD_SPECIAL)
        {
            if(field[i].value == CARD_24)
                future_24 = true;
            else if(field[i].value == CARD_36)
                future_36 = true;
        }
    
        if(field[i].value == 0)
        {
            field[i].value = (rand() % 10) + 1;
            
            //2&4 card played, new cards are negative
            if(future_24 && (field[i].value == 2 || field[i].value == 4))
                field[i].subtract = true;
                
            //3&6 card played, new cards are negative
            if(future_36 && (field[i].value == 3 || field[i].value == 6))
                field[i].subtract = true;
            
            recalculate_card_totals();
            return true;
        }
    }
    
    return false;
}

bool add_card(struct pazaak_card *field, struct pazaak_card card)
{
    for(int i = 0; i < 9; i++)
    {
        if(field[i].value == 0)
        {
            //Apply negations with special cards as they are played
            if(card.type == CARD_SPECIAL)
            {
                switch(card.value)
                {
                    case CARD_24:
                        for(int j = 0; j < 9; j++)
                        {
                            if(field[j].value == 2 || field[j].value == 4)
                                field[j].subtract = true;
                        }
                        break;
                    case CARD_36:
                        for(int j = 0; j < 9; j++)
                        {
                            if(field[j].value == 3 || field[j].value == 6)
                                field[j].subtract = true;
                        }
                        break;
                }
            }
        
            field[i].type = card.type;
            field[i].value = card.value;
            field[i].subtract = card.subtract;
            
            recalculate_card_totals();
            return true;
        }
    }
    
    return false;
}

void game_state_advance()
{
    recalculate_card_totals();
    
    if(player_turn && enemy_total > 20)
        game_over = true;
    
    if(player_standing && enemy_standing)
        game_over = true;
    
    if(game_over)
    {   
        if(player_total > enemy_total && player_total <= 20)
            player_won = true;
            
        if(enemy_total > 20)
            player_won = true;
            
        //If all 9 slots are filled and the amount is under 20,
        //automatic victory
        bool player_full = true;
        for(int i = 0; i < 9; i++)
        {
            if(player_field[i].value == 0)
                player_full = false;
        }
        
        bool enemy_full = true;
        for(int i = 0; i < 9; i++)
        {
            if(enemy_field[i].value == 0)
                enemy_full = false;
        }
        
        if(player_total <= 20 && player_full)
            player_won = true;
            
        if(enemy_total <= 20 && enemy_full)
            player_won = false;
            
        if(player_total == enemy_total)
        {
            bool enemy_tiebreaker = false;
            bool player_tiebreaker = false;
            for(int i = 0; i < 9; i++)
            {
                if(enemy_field[i].type == CARD_SPECIAL && enemy_field[i].value == CARD_TIEBREAK && enemy_field[i+1].value == 0)
                    enemy_tiebreaker = true;
            }
            
            for(int i = 0; i < 9; i++)
            {
                if(player_field[i].type == CARD_SPECIAL && player_field[i].value == CARD_TIEBREAK && player_field[i+1].value == 0)
                    player_tiebreaker = true;
            }
            
            if(enemy_tiebreaker && !player_tiebreaker)
            {
                player_won = false;
                game_tied = false;
            }
            else if(player_tiebreaker && !enemy_tiebreaker)
            {
                player_won = true;
                game_tied = false;
            }
            else
                game_tied = true;
        }
    }
}

void player_state_advance()
{
    if(player_standing)
        player_turn = false;
    else if(player_turn && !player_drew_card)
    {
        bool full = !draw_card(player_field);
        player_drew_card = true;
        
        if(full)
        {
            //We filled the 9 slots without going over 20, force
            //victory
            if(player_total <= 20)
            {
                enemy_standing = true;
                player_standing = true;
            }
        }
    }
}

void ai_advance()
{
    if(!player_turn && !game_over)
    {
        if(player_total > 20 && player_standing)
            enemy_standing = true;
        else if(!enemy_standing)
        {
            if(!draw_card(enemy_field))
            {
                //We filled the 9 slots without going over 20, force
                //victory
                if(enemy_total <= 20)
                {
                    enemy_standing = true;
                    player_standing = true;
                    player_turn = true;
                    return;
                }
            }
            
            bool played_card = false;
            bool full = false;
            
            //We're over 20, search for cards which can help
            //us get under that.
            //TODO: Special card logic can get under 20
            if(enemy_total > 20)
            {
                for(int i = 0; i < 4; i++)
                {
                    if((enemy_hand[i].subtract || enemy_hand[i].type == CARD_FLIP) && enemy_hand[i].value)
                    {
                        if(enemy_total - enemy_hand[i].value > 20)
                            continue;
                    
                        enemy_hand[i].subtract = true;
                        full = !add_card(enemy_field, enemy_hand[i]);
                        
                        enemy_hand[i].type = CARD_DRAW;
                        enemy_hand[i].value = 0;
                        enemy_hand[i].subtract = false;
                        
                        if(enemy_total >= 18 && (player_standing && player_total <= enemy_total))
                            enemy_standing = true;
                        
                        played_card = true;
                        break;
                    }
                }
            }
            
            //Look for cards which can add and get us close to 20
            if(!played_card && enemy_total < 20)
            {
                int best_value = 0;
                for(int i = 0; i < 4; i++)
                {
                    if(enemy_hand[i].type == CARD_SPECIAL) continue;
                
                    if(!enemy_hand[i].subtract && enemy_hand[i].value + enemy_total > best_value && enemy_hand[i].value + enemy_total <= 20)
                    {
                        best_value = enemy_hand[i].value + enemy_total;
                    }
                }
                
                for(int i = 0; i < 4; i++)
                {
                    if(enemy_hand[i].type == CARD_SPECIAL) continue;
                
                    if(enemy_hand[i].value + enemy_total == best_value && best_value >= 19)
                    {
                        full = !add_card(enemy_field, enemy_hand[i]);
                        
                        enemy_hand[i].type = CARD_DRAW;
                        enemy_hand[i].value = 0;
                        enemy_hand[i].subtract = false;
                        
                        enemy_standing = true;
                        
                        played_card = true;
                        break;
                    }
                }
            }
            
            if(full)
            {
                //We filled the 9 slots without going over 20, force
                //victory
                if(enemy_total <= 20)
                {
                    enemy_standing = true;
                    player_standing = true;
                }
            }
            
            //If the player's total is lower and they already
            //stood, we win.
            if(player_total < enemy_total && player_standing)
                enemy_standing = true;
            
            //We lost
            if(enemy_total >= 20)
                enemy_standing = true;
        }
        
        player_turn = true;
    }
}

void stand_event(button_t *button)
{
    if(player_turn)
    {
        player_standing = true;
        
        OSReport("stand\n");
    }
}

void end_event(button_t *button)
{
    if(player_turn && !player_standing)
    {
        game_state_advance();
        if(player_total >= 20 || enemy_total > 20)
            player_standing = true;
            
        bool full = true;
        for(int i = 0; i < 9; i++)
        {
            if(player_field[i].value == 0)
                full = false;
        }
        
        if(full)
        {
            //We filled the 9 slots without going over 20, force
            //victory
            if(player_total <= 20)
            {
                enemy_standing = true;
                player_standing = true;
            }
        }

        player_turn = false;
        player_played_card = false;
        player_drew_card = false;
    }
}

void hand_event(button_t *button)
{
    if(player_turn && !player_standing && !player_played_card)
    {
        bool flip_subtract = true;
        float touched_y = button->touch_y - button->y_pos;
        if(touched_y > (button->height/4)*3)
        {
            flip_subtract = false;
        }
        
        if(player_hand[(int)button->extra_data].type == CARD_FLIP || (player_hand[(int)button->extra_data].type == CARD_SPECIAL && player_hand[(int)button->extra_data].value == CARD_TIEBREAK))
            player_hand[(int)button->extra_data].subtract = flip_subtract;
    
        add_card(player_field, player_hand[(int)button->extra_data]);
        
        player_hand[(int)button->extra_data].value = 0;
        player_hand[(int)button->extra_data].type = CARD_DRAW;
        player_hand[(int)button->extra_data].subtract = false;
        
        player_played_card = true;
    }
}

bool full_reset = true;
void play_reset()
{
    if(game_over)
    {
        if(player_won)
        {
            player_score++;
        }
        else if(!player_won && !game_tied)
        { 
            enemy_score++;
        }
    }
    
    if(player_score >= 3 || enemy_score >= 3)
    {
        for(int i = 0; i < 4; i++)
        {
            button_destroy(player_hand_buttons[i]);
        }
        
        full_reset = true;
    }

    game_over = false;
    player_turn = true;
    player_played_card = false;
    player_drew_card = false;
    player_standing = false;
    enemy_standing = false;
    
    player_won = false;
    game_tied = false;

    //Init player field
    player_total = 0;
    for(int i = 0; i < 9; i++)
    {
        player_field[i].type = CARD_DRAW;
        player_field[i].value = 0;
        player_field[i].subtract = false;
    }
    
    
    //Init enemy field
    enemy_total = 0;
    for(int i = 0; i < 9; i++)
    {
        enemy_field[i].type = CARD_DRAW;
        enemy_field[i].value = 0;
        enemy_field[i].subtract = false;
    }
    
    if(full_reset)
    {
        for(int i = 0; i < 4; i++)
        {
            player_hand[i].type = (rand() % 4) + 1;
            player_hand[i].subtract = false;
            switch(player_hand[i].type)
            {
                
                case CARD_SUB:
                    player_hand[i].subtract = true;
                case CARD_ADD:
                case CARD_FLIP:
                    player_hand[i].value = (rand() % 6)+1;
                    break;
                case CARD_SPECIAL:
                    player_hand[i].value = (rand() % 4)+1;
                    break;
            }
        }
        
        for(int i = 0; i < 4; i++)
        {
            enemy_hand[i].type = (rand() % 4) + 1;
            switch(enemy_hand[i].type)
            {
                
                case CARD_SUB:
                    enemy_hand[i].subtract = true;
                case CARD_ADD:
                case CARD_FLIP:
                    enemy_hand[i].value = (rand() % 6)+1;
                    break;
                case CARD_SPECIAL:
                    enemy_hand[i].value = (rand() % 4)+1;
                    break;
            }
        }
        
        //Init hand buttons
        for(int i = 0; i < 4; i++)
        {
            //render_texture(, (float)(i) * 430.0f + 100.0f, 400.0f, 400.0f, 554.0f);
            player_hand_buttons[i] = button_instantiate((float)(i) * 430.0f + 100.0f, 400.0f, 1.025f);
            button_add_texture(player_hand_buttons[i], get_card_texture(player_hand[i]), 0,0,400.0f, 554.0f,false,false);
            button_add_inflate_release_event(player_hand_buttons[i], hand_event);
            
            player_hand_buttons[i]->extra_data = (void*)i;
        }
        
        player_score = 0;
        enemy_score = 0;
        full_reset = false;
    }
}

void play_menu_init()
{    
     gui_transition_in(1.0f, 0.0f, 0.0f, 30.0f);
     
     play_reset();
     
     //Init buttons
     stand_button = button_instantiate(120.0f, -20.0f, 1.025f);
     button_add_texture(stand_button, &textureStand, 0.0f, 0.0f, 350.0f, 350.0f, false, false);
     button_add_inflate_release_event(stand_button, stand_event);
     
     end_button = button_instantiate(520.0f, -20.0f, 1.025f);
     button_add_texture(end_button, &textureEndTurn, 0.0f, 0.0f, 350.0f, 350.0f, false, false);
     button_add_inflate_release_event(end_button, end_event);
}

void play_menu_update()
{
    if(play_transitioning_out && gui_transition_done())
    {
        play_transitioning_out = false;
        /*
            _menu_init();
            current_state = _MENU;
            
            gui_transition_in(1.0f, 0.0f, 60.0f, 30.0f);
        */
    }
    
    if(!game_over)
    {
        game_state_advance();
        player_state_advance();
        ai_advance();
    }
    else
    {
        if(tpTouched)
        {
            play_reset();
        }
    }
    
    //Button updates, etc
    button_update(stand_button, tpXPos, tpYPos, tpTouched);
    button_update(end_button, tpXPos, tpYPos, tpTouched);
    
    for(int i = 0; i < 4; i++)
    {
        if(player_hand[i].value)
        {
            button_update(player_hand_buttons[i], tpXPos, tpYPos, tpTouched);
        }
    }
}

void play_menu_draw(bool screen)
{
    float player_shift_x = 160.0f;
    float player_shift_y = 112.0f;
    float enemy_shift_x = 1130.0f;
    float enemy_shift_y = 112.0f;
    
    if(screen)
    {
        char player_total_str[0x10];
        char enemy_total_str[0x10];
        char score_str[0x20];
        snprintf(player_total_str, 0x10, "%d", player_total);
        snprintf(enemy_total_str, 0x10, "%d", enemy_total);
        snprintf(score_str, 0x20, "%d   %d", player_score, enemy_score);
        
        font_draw_string_height((float)(TARGET_WIDTH / 4.0) - (float)(font_measure_string_width_height(100, player_total_str) / 2.0f), (float)TARGET_HEIGHT - 100.0f, 100, player_total_str);
        font_draw_string_height((float)(TARGET_WIDTH / 4.0) - (float)(font_measure_string_width_height(100, enemy_total_str) / 2.0f) + (float)(TARGET_WIDTH/2), (float)TARGET_HEIGHT - 100.0f, 100, enemy_total_str);
        
        font_draw_string_height((float)(TARGET_WIDTH / 2.0) - (float)(font_measure_string_width_height(100, score_str) / 2.0f), (float)TARGET_HEIGHT - 100.0f, 100, score_str);
    
        //Player Field
        for(int i = 0; i < 9; i++)
        {
            if(player_field[i].value)
            {
                render_texture(get_card_texture(player_field[i]), (float)(i%3) * 215.0f + player_shift_x, (float)TARGET_HEIGHT - player_shift_y - (float)((i/3)+1) * 285.0f, 200.0f, 277.0f);
            }
        }
        
        //Enemy Field
        for(int i = 0; i < 9; i++)
        {
            if(enemy_field[i].value)
            {
                render_texture(get_card_texture(enemy_field[i]), (float)(i%3) * 215.0f + enemy_shift_x, (float)TARGET_HEIGHT - enemy_shift_y - (float)((i/3)+1) * 285.0f, 200.0f, 277.0f);
            }
        }
    }
    else
    {
        //Player Hand
        for(int i = 0; i < 4; i++)
        {
            if(player_hand[i].value)
            {
                button_draw(player_hand_buttons[i]);
            }
        }
        
        //Enemy Hand
        for(int i = 0; i < 4; i++)
        {
            if(enemy_hand[i].value)
            {
                render_texture(SHOW_ENEMY_HAND ? get_card_texture(enemy_hand[i]) : &textureBack, (float)i * 220.0f + 1000.0f, 50.0f, 200.0f, 277.0f);
            }
        }
    
        button_draw(stand_button);
        button_draw(end_button);
        
        if(game_over)
        {
            font_draw_string_height((float)(TARGET_WIDTH / 2.0) - (float)(font_measure_string_width_height(400, player_won ? "You Won!" : (game_tied ? "Tie" : "You Lost!")) / 2.0f), (float)TARGET_HEIGHT/2 - 100.0f, 400, player_won ? "You Won!" : (game_tied ? "Tie" : "You Lost!"));
        }
    }
}
