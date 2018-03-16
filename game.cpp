#include "game.h"

#include "globals.h"
#include "physics.h"
#include "wall.h"
#include "math_extra.h"

#define BLACK 0x000000
#define WHITE 0xFFFFFF


/** Erases the ball from the screen by drawing over it with the background color. */
void erase_ball(Ball* ball)
{
    // TODO: Draw background color over curriously drawn ball location
    uLCD.filled_circle(ball->x, ball->y, radius, BLACK);
}

/** Draws the ball on the screen at the updated location (according to the state) */
void draw_ball(Ball* ball, Physics* state)
{
    // TODO: Save that updated ball position for later erasing
    ball->x = (int)state->px;
    ball->y = (int)state->py;
    // TODO: Draw ball in its updated location
    uLCD.filled_circle(ball->x, ball->y, radius, WHITE);
}

/** Assigns gravity values from acc to 'next' Physics state */
void do_gravity(Physics* state, GameInputs inputs)
{
    state->ax = (float)inputs.ax * -30.0 * state->scale;
    state->ay = (float)inputs.ay * 30.0 *  state->scale;
}

/** Used to draw the goal */
void draw_goal(Goal* goal)
{
    if (goal->should_draw) {
        uLCD.filled_circle(goal->x, goal->y, radius + 2, BLUE);
        goal->should_draw = 0;
    }
}

/** Used to check goal conditions and determine if redraw is necessary*/
int do_goal(Physics* state, Goal* goal)
{
    // TODO: DETERMINE IF GAME WON OR NOT and if goal should be redrawn or not
    if (in_range(state->px, goal->x - radius * 2 - 3, goal->x + radius * 2 + 3) && in_range(state->py, goal->y - radius * 2 - 3, goal->y + radius * 2 + 3)) {
        goal->should_draw = 1;
    }
    if (in_range(state->px, goal->x - 2, goal->x + 2) && in_range(state->py, goal->y - 2, goal->y + 2)) {
        return 1; //end level state reached
    }
    return 0;
}

/** Used to draw potholes */
void draw_pothole(Pothole* pothole)
{
    if (pothole->should_draw) {
        uLCD.filled_circle(pothole->x, pothole->y, pothole->radius, RED);
        pothole->should_draw = 0;
    }
}

/** Used to check fall and restart conditions */
void do_pothole(Physics* state, Pothole* pothole)
{
    // TODO: DETERMINE IF GAME RESTART OR NOT and if pothole should be redrawn or not
    if (in_range(state->px, pothole->x - (pothole->radius * 2) - 3, pothole->x + (pothole->radius * 2) + 3) && in_range(state->py, pothole->y - (pothole->radius * 2) - 3, pothole->y + (pothole->radius * 2) + 3)) {
        pothole->should_draw = 1;
    }
    int rng = radius + pothole->radius;
    if (in_range(state->px, pothole->x - rng, pothole->x + rng) && in_range(state->py, pothole->y - rng, pothole->y + rng)) {
        state->px = state->ix;
        state->py = state->iy; //restart level state reached
        state->potholeDrops++; //update pothole stat
    }
}

/** Used to draw portals */
void draw_portal(Portal* portal)
{
    if (portal->should_draw) {
        uLCD.filled_circle(portal->x1, portal->y1, radius + 2, GREEN);
        uLCD.filled_circle(portal->x2, portal->y2, radius + 2, GREEN);
        portal->should_draw = 0;
    }
}

/** Used to check fall and teleport conditions */
void do_portal(Physics* state, Portal* portal)
{
    // TODO: DETERMINE IF BALL TELEPORT OR NOT and if portal should be redrawn or not
    if ((in_range(state->px, portal->x1 - (radius * 2) - 3, portal->x1 + (radius * 2) + 3) && in_range(state->py, portal->y1 - (radius * 2) - 3, portal->y1 + (radius * 2) + 3))
            || (in_range(state->px, portal->x2 - (radius * 2) - 3, portal->x2 + (radius * 2) + 3) && in_range(state->py, portal->y2 - (radius * 2) - 3, portal->y2 + (radius * 2) + 3))) {
        portal->should_draw = 1;
    }
    if (in_range(state->px, portal->x1 - 8, portal->x1 + 8) && in_range(state->py, portal->y1 - 8, portal->y1 + 8)) {
        state->px = portal->x2 - 10;
        state->py = portal->y2; //teleport state reached
        state->vx = -15.0;
        state->vy = 0;
        state->portalJumps++;   //update portal stat
    } else if (in_range(state->px, portal->x2 - 8, portal->x2 + 8) && in_range(state->py, portal->y2 - 8, portal->y2 + 8)) {
        state->px = portal->x1 - 10;
        state->py = portal->y1; //teleport state reached
        state->vx = -15.0;
        state->vy = 0;
        state->portalJumps++;   //update portal stat
    }
}

/** Used to draw slow zones */
void draw_slow_zone(SlowZone* s_zone)
{
    if (s_zone->should_draw) {
        uLCD.filled_rectangle(s_zone->x1, s_zone->y1, s_zone->x2, s_zone->y2, DGREY);
        s_zone->should_draw = 0;
    }
}

/** Used to check slow status conditions */
void do_slow_zone(Physics* state, SlowZone* s_zone)
{
    if (in_range(state->px, s_zone->x1 - 6, s_zone->x2 + 6) && in_range(state->py, s_zone->y1, 128)) {
        s_zone->should_draw = 1;
    } //draw condition 1
    if (state->leftFromZone) {
        state->leftFromZone = false;
        s_zone->should_draw = 1;
    } //draw condition 2
    if (state->slowed) { // slowed status active
        state->ax *= 0.4;
        state->ay *= 0.4;
        state->vx = clamp(state->vx, 10);
        state->vy = clamp(state->vy, 10);
    } //regular slow zone effect update
    if (!state->slowed && ((int)state->py >= s_zone->y1)) {
        if ((int)state->px == s_zone->x1) {
            // slowed status conditions met
            state->slowed = 1;
            s_zone->fromLeft = 1;
            state->ax *= 0.4;
            state->ay *= 0.4;
            state->vx *= 0.4;
            state->vy *= 0.4;
        }
        if((int)state->px == s_zone->x2) {
            // slowed status conditions met
            state->slowed = 1;
            s_zone->fromRight = 1;
            state->ax *= 0.4;
            state->ay *= 0.4;
            state->vx *= 0.4;
            state->vy *= 0.4;
        }
    } //entering the slow zone
    if (((int)state->px > s_zone->x2) || ((int)state->px < s_zone->x1)) {
        state->slowed = 0;
        s_zone->fromRight = 0;
    } //leaving the slow zone
}

/** Erases the guard */
void erase_guard(Guard* guard)
{
    uLCD.BLIT(guard->left_x, guard->top_y, 10, 10, guard->ClearSprite);
    guard->facingLeft = !(guard->facingLeft);
    guard->should_draw = 1;
}

/** Used to draw guard */
void draw_guard(Guard* guard)
{
    if (guard->should_draw && guard->facingLeft) {
        uLCD.BLIT(guard->left_x, guard->top_y, 10, 10, guard->LeftSprite);
        guard->should_draw = 0;
    } else if (guard->should_draw && !guard->facingLeft) {
        uLCD.BLIT(guard->left_x, guard->top_y, 10, 10, guard->UpSprite);
        guard->should_draw = 0;
    }
}

/** Used to check caught conditions */
void do_guard(Physics* state, Guard* guard)
{
    //first check draw conditions (within 3 pixels of guard), which are the same as the pothole conditions
    if (in_range(state->px, 110, 127) && in_range(state->py, 110, 127)) {
        guard->should_draw = 1;
        state->px = state->ix;
        state->py = state->iy; //restart level state reached
        state->guardCatches++; //update guard stat
        return;//if touched, no need to check if guard can actually see the ball
    }
    //TODO: guard must also send it back to beginning if it sees it from a certain distance (can use wall next to goal to determine this distance)
    if ((!guard->facingLeft && in_range(state->px, 110, 127) && in_range(state->py, 52, 127)) || (guard->facingLeft && in_range(state->px, 36, 127) && in_range(state->py, 110, 127))) {
        if (in_range(state->px, 75, 101)) {
            state->leftFromZone = true;
        }
        state->px = state->ix;
        state->py = state->iy; //restart level state reached
        state->guardCatches++; //update guard stat
    }
}

/** Used to save states */
void do_save(Physics* state, SaveRestore* sr)
{
    SaveState * new_save = (SaveState*)malloc(sizeof(SaveState));
    new_save->px = state->px;
    new_save->py = state->py;
    new_save->slowed = state->slowed;
    new_save->usedStop = state->usedStop;
    insertHead(sr->saves, (void*)new_save);
}

/** Used to restore states */
void do_restore(Physics* state, SaveRestore* sr)
{
    SaveState * save = (SaveState*)getHead(sr->saves);
    if (save) {
        state->px = save->px;
        state->py = save->py;
        state->vx = 0.0;
        state->vy = 0.0;
        state->slowed = save->slowed;
        state->usedStop = save->usedStop;
        deleteForward(sr->saves);
    }
}

/** Reads inputs to the game, such as accelerometer and buttons */
GameInputs read_inputs()
{
    GameInputs inputs = {0};

    // TODO: Get acceleration vector from accelerometer acc
    acc.readXYZGravity(&inputs.ay,&inputs.ax,&inputs.az);
    inputs.ax = inputs.ax * -2;
    inputs.ay = inputs.ay * 2;

    // TODO: Read buttons (*_pb, lowercase)
    inputs.Advance1 = left_pb;
    inputs.Advance2 = right_pb;
    inputs.Save = up_pb;
    inputs.Restore = down_pb;
    return inputs;
}

int update_game(DLinkedList* arena, Physics* curr, GameInputs inputs, float delta)
{
    int result;
    ///////////////////////////////
    // Prepare for physics update
    ///////////////////////////////
    // Make a copy of the current state for modification
    Physics next = *curr;
    // No acceleration unless the ArenaElements apply them. (Newton's 1st law)
    next.ax = next.ay = 0;

    // Loop over all arena elements
    ArenaElement* elem = (ArenaElement*)getHead(arena);
    do {
        switch(elem->type) {
            case WALL:
                do_wall(&next, curr, (Wall*) elem, delta);
                break;
            case GOAL:
                result = do_goal(&next, (Goal*) elem);
                if (!inputs.Advance1 && !inputs.Advance2) {
                    result = 1;
                }
                break;
            case POTHOLE:
                do_pothole(&next, (Pothole*) elem);
                break;
            case PORTAL:
                do_portal(&next, (Portal*)elem);
                break;
            case GUARD:
                do_guard(&next, (Guard*) elem);
                break;
            case GRAVITY:
                do_gravity(&next, inputs);
                break;
            case SLOW_ZONE:
                do_slow_zone(&next, (SlowZone*) elem);
                break;
            case BALL:
                if (!inputs.Advance1 && !curr->usedStop && inputs.Advance2) {
                    next.vx = 0;
                    next.vy = 0;
                    next.usedStop = true;
                } else {
                    forward_euler(&next, delta);
                }
                break;
            case SAVE_RESTORE:
                if (!inputs.Save){
                    do_save(&next, (SaveRestore*)elem);
                    wait(0.2);
                }
                else if (!inputs.Restore){
                    do_restore(&next, (SaveRestore*)elem);
                    wait(0.2);
                }
                break;
            default:
                break;
        }
    } while(elem = (ArenaElement*)getNext(arena));

    // Last thing! Update state, so it will be saved for the next iteration.
    *curr = next;

    // Zero means we aren't done yet
    return result;
}

int run_game(DLinkedList* arena, Physics* state)
{
    // Initialize game loop timers
    int tick, phys_tick, draw_tick, Gdraw_tick, drawG;
    float tempC, tempI;
    Timer timer;
    timer.start();
    tick = timer.read_ms();
    phys_tick = tick;
    draw_tick = tick;
    Gdraw_tick = tick;

    // Initialize debug counters
    int count = 0;
    int count2 = 0;

    // Initial draw of the game
    uLCD.background_color(BLACK);
    uLCD.cls();
    
    //Initialize room temp
    tempI = myTMP36.read();

    ///////////////////
    // Main game loop
    ///////////////////
    while(1) {
        // Read timer to determine how long the last loop took
        tick = timer.read_ms();

        ///////////////////
        // Physics Update
        ///////////////////
        // Rate limit: 1 ms
        int diff = tick - phys_tick;
        if (diff < 1) continue;
        phys_tick = tick;

        // Compute elapsed time in milliseconds
        float delta = diff*1e-3;

        // Read inputs
        GameInputs inputs = read_inputs();

        // Update game state
        int done = update_game(arena, state, inputs, delta);
        if (done) return done;

        //see if current game state should be saved
        //if (!save){


        // Debug: Count physics updates
        count2++;

        //////////////////
        // Render update
        //////////////////
        // Rate limit: 40ms
        if(tick - draw_tick < 40) continue;
        draw_tick = tick;

        //guard changes position every 2 seconds
        if (tick - Gdraw_tick > 5000) {
            drawG = 1;
            Gdraw_tick = tick;
        }

        // Erase moving stuff
        ArenaElement* elem = (ArenaElement*)getHead(arena);
        do {
            switch(elem->type) {
                case BALL:
                    erase_ball((Ball*) elem);
                    break;
                case GUARD:
                    if (drawG) {
                        erase_guard((Guard*) elem);
                        drawG = 0;
                    }
                    break;
                default:
                    break;
            }
        } while(elem = (ArenaElement*)getNext(arena));

        // Draw everything
        elem = (ArenaElement*)getHead(arena);
        do {
            switch(elem->type) {
                case WALL:
                    draw_wall((Wall*) elem);
                    break;
                case POTHOLE:
                    draw_pothole((Pothole*) elem);
                    break;
                case PORTAL:
                    draw_portal((Portal*) elem);
                    break;
                case SLOW_ZONE:
                    draw_slow_zone((SlowZone*) elem);
                    break;
                case GUARD:
                    draw_guard((Guard*) elem);
                    break;
                case GOAL:
                    draw_goal((Goal*) elem);
                    break;
                case BALL:
                    draw_ball((Ball*) elem, state);
                    break;
                default:
                    break;
            }
        } while(elem = (ArenaElement*)getNext(arena));

        ///////////////
        // Debug info
        ///////////////
        // Displays rate info in the top corner
        //  First number is total time to update and render this frame
        //  Second number is how many physics iterations between drawing frames
        //  Only displayed every 10th render update (roughly 2.5 Hz)
        // TODO: Take this out before you turn your code in!
        if ((count = (count+1)%20) == 0) {
            uLCD.locate(0, 0);
            //uLCD.printf("%d %d \r\n", timer.read_ms()-tick, count2);
            if (score < 105) {
                score = 0;
                uLCD.printf("0   ");
            } else {
                //read temperature
                tempC = myTMP36.read();
                if (tempC > tempI + 3.0){
                    score -= 105;
                }
                else {
                    score -= 15;
                }
                uLCD.printf("%d  ", score);
            }
            if (state->slowed) {
                myLED.write(0.5);
            } else {
                myLED.write(0.0);
            }
        }
        // Reset physics iteration counter after every render update
        count2 = 0;
    }
}
