// Include header files for platform
#include "mbed.h"

// Include header files for mazerunner project
#include "globals.h"
#include "math_extra.h"
#include "physics.h"
#include "game.h"
#include "wall.h"
#include "doublely_linked_list.h"

// Hardware initialization
DigitalIn left_pb(p21);  // push button
DigitalIn right_pb(p22); // push button
DigitalIn up_pb(p23);    // push button
DigitalIn down_pb(p24);  // push button
PwmOut myLED(p25);       // RGB LED
Speaker mySpeaker(p18);  // Speaker
uLCD_4DGL uLCD(p9,p10,p11); // LCD (serial tx, serial rx, reset pin;)
Serial pc(USBTX,USBRX);     // used by Accelerometer
MMA8452 acc(p28, p27, 100000); // Accelerometer
TMP36 myTMP36(p15);      // Temperature sensor 

// Level creation method declarations
DLinkedList* create_first_level();
DLinkedList* create_second_level();
DLinkedList* create_third_level();

// Parameters. Declared in globals.h
const float mass = 0.001;
const int radius = 4;
const float bounce = 0.5;
//Guard facing up sprite
 int uSpr[100] = {_,o,_,_,_,_,_,_,o,_,
                  X,o,X,_,_,_,_,X,o,X,
                  X,o,X,_,_,_,_,X,o,X,
                  X,o,X,_,_,_,_,X,o,X,
                  X,X,X,X,X,X,X,X,X,X,
                  _,X,X,X,o,o,X,X,X,_,
                  _,_,X,X,o,o,X,X,_,_,
                  _,_,_,X,X,X,X,_,_,_,
                  _,_,_,_,_,_,_,_,_,_,
                  _,_,_,_,_,_,_,_,_,_};
//Gurad facing down sprite
 int lSpr[100] = {_,X,X,X,X,_,_,_,_,_,
                  o,o,o,o,X,X,_,_,_,_,
                  _,X,X,X,X,X,X,_,_,_,
                  _,_,_,_,X,X,X,X,_,_,
                  _,_,_,_,X,o,o,X,_,_,
                  _,_,_,_,X,o,o,X,_,_,
                  _,_,_,_,X,X,X,X,_,_,
                  _,X,X,X,X,X,X,_,_,_,
                  o,o,o,o,X,X,_,_,_,_,
                  _,X,X,X,X,_,_,_,_,_};
//Guard's clear sprite
 int cSpr[100] = {_,_,_,_,_,_,_,_,_,_,
                  _,_,_,_,_,_,_,_,_,_,
                  _,_,_,_,_,_,_,_,_,_,
                  _,_,_,_,_,_,_,_,_,_,
                  _,_,_,_,_,_,_,_,_,_,
                  _,_,_,_,_,_,_,_,_,_,
                  _,_,_,_,_,_,_,_,_,_,
                  _,_,_,_,_,_,_,_,_,_,
                  _,_,_,_,_,_,_,_,_,_,
                  _,_,_,_,_,_,_,_,_,_};
//Initialize score
int score = 20010;
  
/** Main() is where you start your implementation */
int main()
{
    ////////////////////////////
    // Power-on initialization
    ////////////////////////////
    // Turn up the serial data rate so we don't lag
    uLCD.baudrate(3000000);
    pc.baud(115200);

    // Initialize the buttons
    // Each *_pb variable is 0 when pressed
    left_pb.mode(PullUp);
    right_pb.mode(PullUp);
    up_pb.mode(PullUp);
    down_pb.mode(PullUp);

    // Other hardware initialization here (SD card, speaker, etc.)
    myLED = 0;
    
    // This first list will hold each level as a GameLevel struct pointer
    DLinkedList* levels = create_dlinkedlist(); 
    
    //initial game menu, here can select difficulty, which causes acceleration to be faster aka harder to control the ball
    float difficulty = 4.0;
    uLCD.cls();
    uLCD.printf("Welcome! \n\nChoose Difficulty:\n\n1. Easy\n2. Medium\n3. Hard\n\n\nPress and hold oneof the rightmost 3buttons to choose.");
    while (left_pb && right_pb && up_pb){
    }
    if (!left_pb){
        uLCD.cls();
        uLCD.printf("You chose Hard.\nGame will begin.");
        wait(2.0);
        difficulty = 1.0;
    }
    else if (!right_pb){
        uLCD.cls();
        uLCD.printf("You chose Medium.\nGame will begin.");
        wait(2.0);
        difficulty = 2.5;
    }   
    else if (!up_pb){
        uLCD.cls();
        uLCD.printf("You chose Easy.\nGame will begin.");
        wait(2.0);
    }
        
    int total_potholeDrops = 0;
    int total_portalJumps = 0;
    int total_guardCatches = 0;
    // The following creates the first level of the game:
    DLinkedList* arena1 = create_first_level();
    Physics state1 = {0};
    state1.px = 15.0;        // Position and Initial Position are equal 
    state1.ix = 15.0; 
    state1.py = 70.0;
    state1.iy = 70.0; 
    state1.vx = 0.0;         // Initially unmoving
    state1.vy = 0.0;
    state1.slowed = 0;       // No slow status yet
    state1.leftFromZone = false; //originally not left from slow zone
    state1.scale = difficulty;
    state1.usedStop = false;
    state1.potholeDrops = 0; // Initially no game  stats have been updated 
    state1.portalJumps = 0;
    state1.guardCatches = 0;
        
    //Create the struct and assign its values for this level:
    GameLevel* level_one = (GameLevel*) malloc(sizeof(GameLevel));
    level_one->state = state1;
    level_one->arena = arena1;
    insertTail(levels, (void*)level_one);
    
        
    // The following creates the second level of the game:
    DLinkedList* arena2 = create_second_level();
    Physics state2 = {0};
    state2.px = 95.0;        // Position and Initial Position are equal
    state2.ix = 95.0; 
    state2.py = 15.0;
    state2.iy = 15.0; 
    state2.vx = 0.0;         // Initially unmoving
    state2.vy = 0.0;
    state2.slowed = 0;       // No slow status yet
    state2.leftFromZone = false; //originally not left from slow zone
    state2.scale = difficulty;
    state2.usedStop = false;
    state2.potholeDrops = 0; // Initially no game  stats have been updated 
    state2.portalJumps = 0;
    state2.guardCatches = 0;
        
    //Create the struct and assign its values for this level:
    GameLevel* level_two = (GameLevel*) malloc(sizeof(GameLevel));
    level_two->state = state2;
    level_two->arena = arena2;
    insertTail(levels, (void*)level_two);
    
    
    // The following creates the third level of the game: 
    DLinkedList* arena3 = create_third_level();
    Physics state3 = {0};
    state3.px = 43.0;        // Position and Initial Position are equal
    state3.ix = 43.0; 
    state3.py = 70.0;
    state3.iy = 70.0;   
    state3.vx = 0.0;         // Initially unmoving
    state3.vy = 0.0;       
    state3.slowed = 0;       // No slow status yet 
    state3.leftFromZone = false; //originally not left from slow zone
    state3.scale = difficulty; //scale set according to difficulty chosen 
    state3.usedStop = false; //Starts with one insta stop feature
    state3.potholeDrops = 0; // Initially no game  stats have been updated 
    state3.portalJumps = 0;
    state3.guardCatches = 0;
        
    //Create the struct and assign its values for this level:
    GameLevel* level_three = (GameLevel*) malloc(sizeof(GameLevel));
    level_three->state = state3;
    level_three->arena = arena3;
    insertTail(levels, (void*)level_three);
    
    //point lvl at the first level
    GameLevel* lvl = (GameLevel*)getHead(levels);
    
    ///////////////
    // Reset loop
    ///////////////
    // This is where control between major phases of the game happens
    // This is a good place to add choose levels, add the game menu, etc.
    do {
        // Delegate to the game loop to execute the level
        // run_game() is in game.cpp
        run_game(lvl->arena, &(lvl->state));
        //save stats from level
        total_potholeDrops = total_potholeDrops + ((lvl->state).potholeDrops); 
        total_portalJumps = total_portalJumps + ((lvl->state).portalJumps);
        total_guardCatches = total_guardCatches + ((lvl->state).guardCatches);
        // Destroy the SaveRestore list once we're done with this level
        destroyList(((SaveRestore*)getTail(lvl->arena))->saves);
        // Destory the arena and entities once we're done with the level
        destroyList(lvl->arena);
        // Level cleared, clear screen to ready for possible next level
        uLCD.cls();
        uLCD.printf("Level Cleared!");
        mySpeaker.PlayNote(969.0, 0.1, 1.0);
        mySpeaker.PlayNote(800.0, 0.1, 1.0);
        mySpeaker.PlayNote(969.0, 0.1, 1.0);
        wait(0.5);
    } while(lvl = (GameLevel*)getNext(levels));
    
    // Destroy the level list
    // It's okay to do this now, since the DLL of each arena in the level struct has already been deallocated 
    destroyList(levels);
    uLCD.cls();
    uLCD.locate(0, 0);
    uLCD.printf("Game Finished! \n\n\nScore:      %d \n\nDrops:      %d\n\nTeleports:  %d\n\nCaught:     %d", score, total_potholeDrops, total_portalJumps, total_guardCatches);
    mySpeaker.PlayNote(300.0,0.1,1.0);
    mySpeaker.PlayNote(500.0,0.1,1.0);
    mySpeaker.PlayNote(700.0,0.1,1.0);
    mySpeaker.PlayNote(800.0,0.2,1.0);
    mySpeaker.PlayNote(800.0,0.2,1.0);
}

/** Creates the first level. */
DLinkedList* create_first_level()
{
    DLinkedList* arena = create_dlinkedlist();

    // Initialize the walls
    Wall* walls[7];
    walls[0] = create_wall(HORIZONTAL, 0, 0, 127, bounce);  // top
    walls[1] = create_wall(HORIZONTAL, 0, 127, 127, bounce);// bottom
    walls[2] = create_wall(VERTICAL, 0, 0, 127, bounce);    // left
    walls[3] = create_wall(VERTICAL, 127, 0, 127, bounce);  // right
    walls[4] = create_wall(VERTICAL, 64, 0, 90, bounce);    // WALL IN MIDDLE
    walls[5] = create_wall(VERTICAL, 32, 47, 80, bounce);   // WALL LEFT
    walls[6] = create_wall(HORIZONTAL, 87, 64, 40, bounce); // WALL RIGHT
    
    
    // Add the walls to the arena
    for (int i = 0; i < 7; i++)
        insertTail(arena, (void*)walls[i]);
    
    // Initialize the goal
    Goal* goal = (Goal*) malloc(sizeof(Goal));
    goal->type = GOAL;
    goal->x = 100;
    goal->y = 20;
    goal->should_draw = 1;
    
    // Add the goal to the arena
    insertTail(arena, (void*)goal);
    
    // Initialize a pothole
    Pothole* pothole = (Pothole*) malloc(sizeof(Pothole));
    pothole->type = POTHOLE;
    pothole->x = 80;
    pothole->y = 100;
    pothole->should_draw = 1;
    pothole->radius = 6;
    
    // Add the pothole to the arena
    insertTail(arena, (void*)pothole);
    
    // Initialize the gravity element 
    Gravity* gravity = (Gravity*) malloc(sizeof(Gravity));
    gravity->type = GRAVITY;
    
    // Add gravity to the arena
    insertTail(arena, (void*)gravity);
    
    // Initialize the ball
    Ball* ball = (Ball*) malloc(sizeof(Ball));
    ball->type = BALL;
    
    // Add ball to the arena 
    // NOTE: The ball should always be last in the arena list, so that the other 
    // ArenaElements have a chance to compute the Physics updates before the 
    // ball applies forward euler method.
    insertTail(arena, (void*)ball);
    
    // Initialize a SaveRestore, last element because i said so
    SaveRestore* save_restore = (SaveRestore*)malloc(sizeof(SaveRestore));
    save_restore->type = SAVE_RESTORE;
    save_restore->saves = create_dlinkedlist();
    
    // Add SaveRestore to the arena
    insertTail(arena, (void*)save_restore);
    
    return arena;
}

/** Creates the second level. */
DLinkedList* create_second_level()
{
    DLinkedList* arena = create_dlinkedlist();

    // Initialize the walls
    Wall* walls[15];
    walls[0] = create_wall(HORIZONTAL, 0, 0, 127, bounce);   // top
    walls[1] = create_wall(HORIZONTAL, 0, 127, 127, bounce); // bottom
    walls[2] = create_wall(VERTICAL, 0, 0, 127, bounce);     // left
    walls[3] = create_wall(VERTICAL, 127, 0, 127, bounce);   // right
    walls[4] = create_wall(VERTICAL, 84, 0, 38, bounce);     // WALL A
    walls[5] = create_wall(HORIZONTAL, 84, 38, 22, bounce);  // WALL B
    walls[6] = create_wall(VERTICAL, 105, 39, 18, bounce);   // WALL C
    walls[7] = create_wall(HORIZONTAL, 65, 56, 41, bounce);  // WALL D
    walls[8] = create_wall(VERTICAL, 65, 56, 44, bounce);    // WALL E
    walls[9] = create_wall(HORIZONTAL, 65, 100, 41, bounce); // WALL F
    walls[10] = create_wall(HORIZONTAL, 85, 78, 42, bounce); // WALL G
    walls[11] = create_wall(HORIZONTAL, 24, 66, 41, bounce); // WALL H
    walls[12] = create_wall(VERTICAL, 24, 67, 34, bounce);   // WALL I
    walls[13] = create_wall(HORIZONTAL, 24, 100, 41, bounce);// WALL J
    walls[14] = create_wall(HORIZONTAL, 0, 42, 70, bounce);  // WALL K
    
    
    // Add the walls to the arena
    for (int i = 0; i < 15; i++)
        insertTail(arena, (void*)walls[i]);
    
    // Initialize the goal
    Goal* goal = (Goal*) malloc(sizeof(Goal));
    goal->type = GOAL;
    goal->x = 15;
    goal->y = 20;
    goal->should_draw = 1;
    
    // Add the goal to the arena
    insertTail(arena, (void*)goal);
    
    // Initialize a pothole
    Pothole* pothole1 = (Pothole*) malloc(sizeof(Pothole));
    pothole1->type = POTHOLE;
    pothole1->x = 45;
    pothole1->y = 23;
    pothole1->should_draw = 1;
    pothole1->radius = 8;
    
    // Add the pothole to the arena
    insertTail(arena, (void*)pothole1);
    
    // Initialize another pothole
    Pothole* pothole2 = (Pothole*) malloc(sizeof(Pothole));
    pothole2->type = POTHOLE;
    pothole2->x = 6;
    pothole2->y = 121;
    pothole2->should_draw = 1;
    pothole2->radius = 5;
    
    // Add the pothole to the arena
    insertTail(arena, (void*)pothole2);
    
    // Initialize the portal
    Portal* portal = (Portal*) malloc(sizeof(Portal));
    portal->type = PORTAL;
    portal->x1 = 50;
    portal->y1 = 85; //portal in box
    portal->x2 = 112;
    portal->y2 = 112;//portal in bottom left corner
    portal->should_draw = 1;
    
    // Add portal to the arena
    insertTail(arena, (void*)portal);

    // Initialize the gravity element 
    Gravity* gravity = (Gravity*) malloc(sizeof(Gravity));
    gravity->type = GRAVITY;
    
    // Add gravity to the arena
    insertTail(arena, (void*)gravity);
    
    // Initialize the ball
    Ball* ball = (Ball*) malloc(sizeof(Ball));
    ball->type = BALL;
    
    // Add ball to the arena 
    // NOTE: The ball should always be last in the arena list, so that the other 
    // ArenaElements have a chance to compute the Physics updates before the 
    // ball applies forward euler method.
    insertTail(arena, (void*)ball);
    
    // Initialize a SaveRestore, last element because i said so 
    SaveRestore* save_restore = (SaveRestore*)malloc(sizeof(SaveRestore));
    save_restore->type = SAVE_RESTORE;
    save_restore->saves = create_dlinkedlist();
    
    // Add SaveRestore to the arena
    insertTail(arena, (void*)save_restore);
    
    return arena;
}

/** Creates the third level. */
DLinkedList* create_third_level()
{
    DLinkedList* arena = create_dlinkedlist();

    // Initialize the walls
    Wall* walls[11];
    walls[0] = create_wall(HORIZONTAL, 0, 0, 127, bounce);   // top
    walls[1] = create_wall(HORIZONTAL, 0, 127, 127, bounce); // bottom
    walls[2] = create_wall(VERTICAL, 0, 0, 127, bounce);     // left
    walls[3] = create_wall(VERTICAL, 127, 0, 127, bounce);   // right
    walls[4] = create_wall(VERTICAL, 50, 0, 34, bounce);     // WALL A
    walls[5] = create_wall(HORIZONTAL, 16, 34, 86, bounce);  // WALL B
    walls[6] = create_wall(VERTICAL, 101, 34, 47, bounce);   // WALL C
    walls[7] = create_wall(HORIZONTAL, 0, 80, 102, bounce);  // WALL D
    walls[8] = create_wall(VERTICAL, 32, 35, 45, bounce);    // WALL E
    walls[9] = create_wall(HORIZONTAL, 0, 50, 18, bounce);   // WALL F
    walls[10] = create_wall(VERTICAL, 32, 100, 27, bounce);  // WALL G
    
    // Add the walls to the arena
    for (int i = 0; i < 11; i++)
        insertTail(arena, (void*)walls[i]);
    
    // Initialize the goal
    Goal* goal = (Goal*) malloc(sizeof(Goal));
    goal->type = GOAL;
    goal->x = 20;
    goal->y = 110;
    goal->should_draw = 1;
    
    // Add the goal to the arena
    insertTail(arena, (void*)goal);

    // Initialize a pothole
    Pothole* pothole1 = (Pothole*) malloc(sizeof(Pothole));
    pothole1->type = POTHOLE;
    pothole1->x = 60;
    pothole1->y = 95;
    pothole1->should_draw = 1;
    pothole1->radius = 7;
    
    // Add the pothole to the arena
    insertTail(arena, (void*)pothole1);
    
    // Initialize another pothole
    Pothole* pothole2 = (Pothole*) malloc(sizeof(Pothole));
    pothole2->type = POTHOLE;
    pothole2->x = 107;
    pothole2->y = 69;
    pothole2->should_draw = 1;
    pothole2->radius = 5;
    
    // Add the pothole to the arena
    insertTail(arena, (void*)pothole2);
    
    // Initialize another pothole
    Pothole* pothole3 = (Pothole*) malloc(sizeof(Pothole));
    pothole3->type = POTHOLE;
    pothole3->x = 121;
    pothole3->y = 39;
    pothole3->should_draw = 1;
    pothole3->radius = 5;
    
    // Add the pothole to the arena
    insertTail(arena, (void*)pothole3);
    
    // Initialize another pothole
    Pothole* pothole4 = (Pothole*) malloc(sizeof(Pothole));
    pothole4->type = POTHOLE;
    pothole4->x = 59;
    pothole4->y = 25;
    pothole4->should_draw = 1;
    pothole4->radius = 8;
    
    // Add the pothole to the arena
    insertTail(arena, (void*)pothole4);
    
    // Initialize another pothole
    Pothole* pothole5 = (Pothole*) malloc(sizeof(Pothole));
    pothole5->type = POTHOLE;
    pothole5->x = 25;
    pothole5->y = 73;
    pothole5->should_draw = 1;
    pothole5->radius = 6;
    
    // Add the pothole to the arena
    insertTail(arena, (void*)pothole5);
    
    // Initialize the portal
    Portal* portalJ = (Portal*) malloc(sizeof(Portal));
    portalJ->type = PORTAL;
    portalJ->x1 = 62;
    portalJ->y1 = 45; //portal in box
    portalJ->x2 = 43;
    portalJ->y2 = 17;//portal in top left corner
    portalJ->should_draw = 1;
    
    // Add portal to the arena
    insertTail(arena, (void*)portalJ);
    
    // Initialize another portal
    Portal* portalK = (Portal*) malloc(sizeof(Portal));
    portalK->type = PORTAL;
    portalK->x1 = 86;
    portalK->y1 = 67; //portal in box
    portalK->x2 = 93;
    portalK->y2 = 27;//portal in top right corner
    portalK->should_draw = 1;
    
    // Add portal to the arena
    insertTail(arena, (void*)portalK);
    
    // Initialize another portal
    Portal* portalL = (Portal*) malloc(sizeof(Portal));
    portalL->type = PORTAL;
    portalL->x1 = 7;
    portalL->y1 = 73; //portal in mid left
    portalL->x2 = 93;
    portalL->y2 = 27;//portal in top right corner
    portalL->should_draw = 1;
    
    // Add portal to the arena
    insertTail(arena, (void*)portalL);
    
    // Initialize a guard
    Guard* guard = (Guard*) malloc(sizeof(Guard));
    guard->type = GUARD;
    guard->facingLeft = true;
    guard->UpSprite = uSpr;
    guard->LeftSprite = lSpr;
    guard->ClearSprite = cSpr;
    guard->left_x = 116;
    guard->top_y = 116;
    guard->should_draw = 1; 
    
    // Add guard to the arena
    insertTail(arena, (void*)guard);

    // Initialize the gravity element 
    Gravity* gravity = (Gravity*) malloc(sizeof(Gravity));
    gravity->type = GRAVITY;
    
    // Add gravity to the arena
    insertTail(arena, (void*)gravity);
    
    // Initialize the slow zone
    SlowZone* slow_zone = (SlowZone*) malloc(sizeof(SlowZone));
    slow_zone->type = SLOW_ZONE;
    slow_zone->x1 = 75;
    slow_zone->y1 = 81;
    slow_zone->x2 = 101;
    slow_zone->y2 = 126;
    slow_zone->should_draw = 1;
    
    // Add slow zone to the arena 
    insertTail(arena, (void*)slow_zone);
    
    // Initialize the ball
    Ball* ball = (Ball*) malloc(sizeof(Ball));
    ball->type = BALL;
    
    // Add ball to the arena 
    // NOTE: The ball should always be last in the arena list, so that the other  
    // ArenaElements have a chance to compute the Physics updates before the 
    // ball applies forward euler method. 
    insertTail(arena, (void*)ball);
    
    // Initialize a SaveRestore, last element because i said so 
    SaveRestore* save_restore = (SaveRestore*)malloc(sizeof(SaveRestore));
    save_restore->type = SAVE_RESTORE;
    save_restore->saves = create_dlinkedlist();
    
    // Add SaveRestore to the arena
    insertTail(arena, (void*)save_restore);
    
    return arena;
}
