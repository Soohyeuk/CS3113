/**

* Author: Soohyeuk Choi

* Assignment: Pong Clone

* Date due: 02/14/2026

* I pledge that I have completed this assignment without

* collaborating with anyone else, in conformance with the

* NYU School of Engineering Policies and Procedures on

* Academic Misconduct.

**/


#include "CS3113/cs3113.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

// Enums
enum Direction { LEFT, RIGHT };       // turning left or right?

// Global Constants
// Global Constants
constexpr int   SCREEN_WIDTH  = 1200,
                SCREEN_HEIGHT = 675,
                FPS           = 60,
                SIZE          = 100;
constexpr float MAX_AMP       = 10.0f;

constexpr char    BG_COLOUR[] = "#B2AAC6";
constexpr Vector2 ORIGIN      = { SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 };
constexpr Vector2 BASE_SIZE   = { (float) SIZE, (float) SIZE };
constexpr float LIMIT_ANGLE = 20.0f; 

constexpr float FISH_AMP_X = 200.0f;
constexpr float FISH_AMP_Y = 100.0f;
constexpr float FISH_SPEED = 2.0f;

constexpr float JELLY_AMP_X   = 50.0f;
constexpr float JELLY_FREQ    = 3.0f;

constexpr float BUBBLE_OFFSET_X = 40.0f;
constexpr float BUBBLE_OFFSET_Y = -40.0f;


// Global Variables
AppStatus gAppStatus     = RUNNING;
Direction gDirection = RIGHT; 
float     gScaleFactor   = SIZE,
          gAngle         = 0.0f,
          gPulseTime     = 0.0f;
Vector2   gPosition      = ORIGIN; 
Vector2   gFishPos       = ORIGIN;
Vector2   gJellyfishPos  = {SCREEN_WIDTH / 6.0f, SCREEN_HEIGHT};
Vector2   gBubblePos     = ORIGIN;
Vector2   gScale         = BASE_SIZE;
float     gPreviousTicks = 0.0f;

//texture imports 
constexpr char BUBBLE[]  = "assets/bubble.png";
constexpr char FISH[]  = "assets/fish.png";
constexpr char JELLYFISH[]  = "assets/jellyfish.png";
constexpr char BG[]  = "assets/Sinky_Sub_BG.png";
constexpr char RUINS[]  = "assets/Sinky_Sub_CloseRockkelp.png";
constexpr char FLOOR[]  = "assets/Sinky_Sub_Floor.png";

Texture2D gBubble,
           gFish,
           gJellyfish,
           gRuins,
           gFloor,
           gBG;


// Function Declarations
void initialise();
void processInput();
void update();
void render();
void shutdown();

// Function Definitions
void initialise() {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Transformations");

    gBG = LoadTexture(BG);
    gBubble = LoadTexture(BUBBLE);
    gFish = LoadTexture(FISH);
    gJellyfish = LoadTexture(JELLYFISH);
    gRuins = LoadTexture(RUINS);
    gFloor = LoadTexture(FLOOR);

    SetTargetFPS(FPS);

}

void processInput() {
    if (WindowShouldClose()) gAppStatus = TERMINATED;
}

void update() 
{
    // Delta time
    float ticks = (float) GetTime();
    float deltaTime = ticks - gPreviousTicks;
    gPreviousTicks = ticks;

    gPulseTime += 1.0f * deltaTime;

    gScale = {
        BASE_SIZE.x + MAX_AMP * cos(gPulseTime),
        BASE_SIZE.y + MAX_AMP * cos(gPulseTime)
    };

    // --- FISH ---
    // x = A * sin(t)
    // y = B * sin(2*t) * 0.5 (or cos)
    gFishPos.x = ORIGIN.x + FISH_AMP_X * sin(gPulseTime * FISH_SPEED);
    gFishPos.y = ORIGIN.y + FISH_AMP_Y * sin(gPulseTime * FISH_SPEED * 2.0f) * 0.5f;


    // --- JELLYFISH ---
    //vertical Sine Wave with Ping-Pong
    static float jellySpeedY = -1.0f;
    gJellyfishPos.y += jellySpeedY;
    gJellyfishPos.x = (SCREEN_WIDTH / 6.0f) + JELLY_AMP_X * sin(gPulseTime * JELLY_FREQ);

    //top and bottom bound
    if (gJellyfishPos.y <= 0.0f) {
        gJellyfishPos.y = 0.0f;
        jellySpeedY *= -1.0f;
    }
    else if (gJellyfishPos.y >= SCREEN_HEIGHT) {
        gJellyfishPos.y = SCREEN_HEIGHT;
        jellySpeedY *= -1.0f; 
    }


    // --- BUBBLE ---
    //top right of jellyfish
    gBubblePos.x = gJellyfishPos.x + BUBBLE_OFFSET_X;
    gBubblePos.y = gJellyfishPos.y + BUBBLE_OFFSET_Y; 


    //angle movement
    gAngle += 1.0f * gDirection == RIGHT ? 1 : -1;

    if      (gAngle >  LIMIT_ANGLE) gDirection = LEFT;
    else if (gAngle < -LIMIT_ANGLE) gDirection = RIGHT;
}

void render() {
    BeginDrawing();

    // Origin inside the source texture (centre of the texture)
    Vector2 objectOrigin = {
        static_cast<float>(gScale.x) / 2.0f,
        static_cast<float>(gScale.y) / 2.0f
    };

    // background
    Rectangle backgroundArea = {
        0.0f, 0.0f,
        static_cast<float>(gBG.width),
        static_cast<float>(gBG.height)
    };
    DrawTexturePro(gBG, backgroundArea, {0,0, SCREEN_WIDTH, SCREEN_HEIGHT}, {0,0}, 0.0f, WHITE);

    // rockkelp
    Rectangle rockkelp = {
        0.0f, 0.0f, 
        static_cast<float>(gRuins.width),
        static_cast<float>(gRuins.height)
    };
    DrawTexturePro(gRuins, rockkelp, {0,0, SCREEN_WIDTH, SCREEN_HEIGHT}, {0,0}, 0.0f, WHITE);


    // floor
    Rectangle floorArea = {
        0.0f, 0.0f, 
        static_cast<float>(gFloor.width),
        static_cast<float>(gFloor.height)
    };
    DrawTexturePro(gFloor, floorArea, {0,0, SCREEN_WIDTH, SCREEN_HEIGHT}, {0,0}, 0.0f, WHITE);



    ///////////////
    ////sprites////
    ///////////////

    // fish
    Rectangle fishArea = {
        0.0f, 0.0f,
        static_cast<float>(gFish.width)/4,
        static_cast<float>(gFish.height)
    };
    Rectangle fishDest = {
        gFishPos.x,
        gFishPos.y,
        static_cast<float>(gScale.x),
        static_cast<float>(gScale.y)
    };
    DrawTexturePro(gFish, fishArea, fishDest, objectOrigin, gAngle, WHITE);

    // jellyfish
    Rectangle jellyfishArea = {
        0.0f, 0.0f,
        static_cast<float>(gJellyfish.width)/4,
        static_cast<float>(gJellyfish.height)
    };
    Rectangle jellyfishDest = {
        gJellyfishPos.x,
        gJellyfishPos.y,
        static_cast<float>(gScale.x),
        static_cast<float>(gScale.y)
    };
    DrawTexturePro(gJellyfish, jellyfishArea, jellyfishDest, objectOrigin, gAngle, WHITE);

    // bubble
    Rectangle bubbleArea = {
        0.0f, 0.0f,
        static_cast<float>(gBubble.width),
        static_cast<float>(gBubble.height)
    };
    
    float bubbleScaleFactor = 0.15f + 0.05f * sin(gPulseTime * 5.0f);
    
    Rectangle bubbleDest = {
        gBubblePos.x,
        gBubblePos.y,
        static_cast<float>(gScale.x) * bubbleScaleFactor,
        static_cast<float>(gScale.y) * bubbleScaleFactor
    };
    Vector2 bubbleOrigin = {
        static_cast<float>(gScale.x) * bubbleScaleFactor / 2.0f,
        static_cast<float>(gScale.y) * bubbleScaleFactor / 2.0f
    };
    DrawTexturePro(gBubble, bubbleArea, bubbleDest, bubbleOrigin, gAngle, WHITE);

    EndDrawing();
}

void shutdown() { 
    CloseWindow();
}

int main(void) {
    initialise();

    while (gAppStatus == RUNNING)
    {
        processInput();
        update();
        render();
    }

    shutdown();

    return 0;
}