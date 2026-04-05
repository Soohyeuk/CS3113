/**
* Author: Soohyeuk Choi
* Assignment: Rise of the AI
* Date due: 2026-04-04, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

#include "CS3113/MenuScene.h"
#include "CS3113/LevelA.h"
#include "CS3113/LevelB.h"
#include "CS3113/LevelC.h"
#include "CS3113/WinScene.h"
#include "CS3113/LoseScene.h"

// ---- Constants ----
constexpr int SCREEN_WIDTH     = 1000,
              SCREEN_HEIGHT    = 600,
              FPS              = 120,
              NUMBER_OF_SCENES = 6;

constexpr Vector2 ORIGIN = { SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f };

constexpr float FIXED_TIMESTEP = 1.0f / 60.0f;
constexpr int   MAX_LIVES      = 3;

// ---- Globals ----
AppStatus gAppStatus   = RUNNING;
float gPreviousTicks   = 0.0f,
      gTimeAccumulator = 0.0f;

Camera2D gCamera = { 0 };

Scene *gCurrentScene = nullptr;
std::vector<Scene*> gScenes = {};

MenuScene *gMenuScene  = nullptr;
LevelA    *gLevelA     = nullptr;
LevelB    *gLevelB     = nullptr;
LevelC    *gLevelC     = nullptr;
WinScene  *gWinScene   = nullptr;
LoseScene *gLoseScene  = nullptr;

int gLives = MAX_LIVES;

// ---- Function Declarations ----
void switchToScene(Scene *scene);
void initialise();
void processInput();
void update();
void render();
void shutdown();

void switchToScene(Scene *scene)
{
    if (gCurrentScene != nullptr) gCurrentScene->shutdown();

    gCurrentScene = scene;
    gCurrentScene->initialise();

    // Reset camera for the new scene
    if (gCurrentScene->getState().xochitl != nullptr)
    {
        gCamera.target = gCurrentScene->getState().xochitl->getPosition();
    }
    else
    {
        gCamera.target = ORIGIN;
    }
}

void initialise()
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Shadow Runner");
    InitAudioDevice();

    gMenuScene = new MenuScene(ORIGIN, "#1a1a2e");  
    gLevelA    = new LevelA(ORIGIN, "#163e1f");      
    gLevelB    = new LevelB(ORIGIN, "#163e1f");      
    gLevelC    = new LevelC(ORIGIN, "#163e1f");   
    gWinScene  = new WinScene(ORIGIN, "#0a0a0a");   
    gLoseScene = new LoseScene(ORIGIN, "#0a0a0a");   

    gScenes.push_back(gMenuScene);  // 0
    gScenes.push_back(gLevelA);     // 1
    gScenes.push_back(gLevelB);     // 2
    gScenes.push_back(gLevelC);     // 3
    gScenes.push_back(gWinScene);   // 4
    gScenes.push_back(gLoseScene);  // 5

    switchToScene(gScenes[0]); // Start at menu

    gCamera.offset   = ORIGIN;
    gCamera.rotation = 0.0f;
    gCamera.zoom     = 1.0f;

    SetTargetFPS(FPS);
}

void processInput()
{
    // Scene-specific input (player movement, Enter key, etc.)
    gCurrentScene->processInput();

    //switch levels with number keys
    if (IsKeyPressed(KEY_ONE))   switchToScene(gScenes[1]);
    if (IsKeyPressed(KEY_TWO))   switchToScene(gScenes[2]);
    if (IsKeyPressed(KEY_THREE)) switchToScene(gScenes[3]);

    if (IsKeyPressed(KEY_Q) || WindowShouldClose()) gAppStatus = TERMINATED;
}

void update()
{
    float ticks = (float) GetTime();
    float deltaTime = ticks - gPreviousTicks;
    gPreviousTicks  = ticks;

    deltaTime += gTimeAccumulator;

    if (deltaTime < FIXED_TIMESTEP)
    {
        gTimeAccumulator = deltaTime;
        return;
    }

    while (deltaTime >= FIXED_TIMESTEP)
    {
        gCurrentScene->update(FIXED_TIMESTEP);
        deltaTime -= FIXED_TIMESTEP;

        // Camera follows player (horizontal only)
        if (gCurrentScene->getState().xochitl != nullptr)
        {
            Vector2 currentPlayerPosition = {
                gCurrentScene->getState().xochitl->getPosition().x,
                ORIGIN.y
            };
            panCamera(&gCamera, &currentPlayerPosition);

            // Clamp camera to map boundaries
            if (gCurrentScene->getState().map != nullptr)
            {
                float halfW = SCREEN_WIDTH / 2.0f;
                float left  = gCurrentScene->getState().map->getLeftBoundary();
                float right = gCurrentScene->getState().map->getRightBoundary();

                if (gCamera.target.x < left + halfW)  gCamera.target.x = left + halfW;
                if (gCamera.target.x > right - halfW) gCamera.target.x = right - halfW;
            }
        }
    }

    gTimeAccumulator = deltaTime;

    // Handle player death
    if (gCurrentScene->getState().playerHit)
    {
        gLives--;
        if (gLives <= 0)
        {
            switchToScene(gScenes[5]); // Lose scene
            return;
        }
        else
        {
            // Respawn: re-initialise the current level
            gCurrentScene->shutdown();
            gCurrentScene->initialise();
            if (gCurrentScene->getState().xochitl != nullptr)
                gCamera.target = gCurrentScene->getState().xochitl->getPosition();
            return;
        }
    }

    // Handle scene transitions
    int nextScene = gCurrentScene->getState().nextSceneID;
    if (nextScene >= 0)
    {
        // Reset lives when returning to menu
        if (nextScene == 0) gLives = MAX_LIVES;

        switchToScene(gScenes[nextScene]);
        return;
    }
}

void render()
{
    BeginDrawing();
    BeginMode2D(gCamera);

    gCurrentScene->render();

    EndMode2D();

    //draw lives in screen space (only during gameplay levels)
    if (gCurrentScene->getState().xochitl != nullptr)
    {
        DrawText(TextFormat("Lives: %d", gLives), 20, 20, 30, WHITE);
    }

    EndDrawing();
}

void shutdown()
{
    // Current scene cleanup is handled by switchToScene / destructors
    delete gMenuScene;
    delete gLevelA;
    delete gLevelB;
    delete gLevelC;
    delete gWinScene;
    delete gLoseScene;

    CloseAudioDevice();
    CloseWindow();
}

int main(void)
{
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
