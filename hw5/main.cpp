/**
* Author: Soohyeuk Choi
* Assignment: Your Life
* Date due: 04/24/2026, 2:00pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

#include "CS3113/ShaderProgram.h"
#include "CS3113/MenuScene.h"
#include "CS3113/MonologueScene.h"
#include "CS3113/LevelA.h"
#include "CS3113/LevelB.h"
#include "CS3113/LevelC.h"
#include "CS3113/WinScene.h"
#include "CS3113/LoseScene.h"
#include "CS3113/PhoneMinigame.h"
#include "CS3113/RunState.h"
#include "CS3113/HUD.h"

// ---- Constants ----
constexpr int SCREEN_WIDTH     = 1000,
              SCREEN_HEIGHT    = 600,
              FPS              = 120,
              NUMBER_OF_SCENES = 6;

constexpr Vector2 ORIGIN = { SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f };

constexpr float FIXED_TIMESTEP      = 1.0f / 60.0f;
constexpr int   MAX_CORTISOL        = 100;
constexpr int   CORTISOL_PER_HIT    = 10;
constexpr float EMERGENCY_DURATION  = 60.0f;
constexpr float HIT_COOLDOWN        = 0.5f;

// ---- Globals ----
AppStatus gAppStatus   = RUNNING;
float gPreviousTicks   = 0.0f,
      gTimeAccumulator = 0.0f;

Camera2D gCamera = { 0 };

Scene *gCurrentScene = nullptr;
std::vector<Scene*> gScenes = {};

MenuScene      *gMenuScene      = nullptr;
MonologueScene *gMonologueScene = nullptr;
LevelA         *gLevelA         = nullptr;
LevelB         *gLevelB         = nullptr;
LevelC         *gLevelC         = nullptr;
WinScene       *gWinScene       = nullptr;
LoseScene      *gLoseScene      = nullptr;

Effects       *gEffects = nullptr;
ShaderProgram  gShader;

RunState gRun;

Music gEmergencyMusic = { 0 };

// Shared level audio: loaded once at startup, referenced by all levels.
Music gBgm        = { 0 };
Sound gJumpSound  = { 0 };
Sound gStepSound  = { 0 };
Sound gStompSound = { 0 };

PhoneMinigame *gPhone = nullptr;
HUD           *gHud   = nullptr;

// ---- Function Declarations ----
void switchToScene(Scene *scene);
void showMonologue(std::vector<std::string> lines, int afterSceneID);
void initialise();
void processInput();
void update();
void render();
void shutdown();

// Configure and jump to the shared monologue scene. After the last line,
// the monologue transitions to `afterSceneID`.
void showMonologue(std::vector<std::string> lines, int afterSceneID)
{
    gMonologueScene->configure(std::move(lines), afterSceneID);
    switchToScene(gMonologueScene);
}

void switchToScene(Scene *scene)
{
    if (gCurrentScene != nullptr) gCurrentScene->shutdown();

    gCurrentScene = scene;
    gCurrentScene->initialise();

    gRun.emergency         = false;
    gRun.emergencyTimeLeft = 0.0f;
    StopMusicStream(gEmergencyMusic);

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
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Your Life");
    InitAudioDevice();

    gShader.load("assets/shaders/vertex.glsl", "assets/shaders/fragment.glsl");

    gMenuScene      = new MenuScene(ORIGIN, "#1a1a2e");
    gMonologueScene = new MonologueScene(ORIGIN, "#000000");
    gLevelA         = new LevelA(ORIGIN, "#163e1f");
    gLevelB         = new LevelB(ORIGIN, "#163e1f");
    gLevelC         = new LevelC(ORIGIN, "#163e1f");
    gWinScene       = new WinScene(ORIGIN, "#0a0a0a");
    gLoseScene      = new LoseScene(ORIGIN, "#0a0a0a");

    gScenes.push_back(gMenuScene);  // 0
    gScenes.push_back(gLevelA);     // 1
    gScenes.push_back(gLevelB);     // 2
    gScenes.push_back(gLevelC);     // 3
    gScenes.push_back(gWinScene);   // 4
    gScenes.push_back(gLoseScene);  // 5

    gCamera.offset   = ORIGIN;
    gCamera.rotation = 0.0f;
    gCamera.zoom     = 1.0f;

    gEffects = new Effects(ORIGIN, (float) SCREEN_WIDTH * 1.5f, (float) SCREEN_HEIGHT * 1.5f);
    gEffects->setEffectSpeed(2.0f);

    gPhone = new PhoneMinigame(SCREEN_WIDTH, SCREEN_HEIGHT);
    gPhone->load();

    gHud = new HUD(SCREEN_WIDTH, SCREEN_HEIGHT, MAX_CORTISOL);

    gEmergencyMusic  = LoadMusicStream("assets/game/emergency.wav");

    gBgm        = LoadMusicStream("assets/game/horror.wav");
    gJumpSound  = LoadSound("assets/game/jumo_sound.wav");
    gStepSound  = LoadSound("assets/game/step_sound.wav");
    gStompSound = LoadSound("assets/game/kill_sound.wav");

    switchToScene(gScenes[0]); // Start at the menu

    SetTargetFPS(FPS);
}

void processInput()
{
    if (IsKeyPressed(KEY_Q) || WindowShouldClose()) { gAppStatus = TERMINATED; return; }

    bool isGameplay = gCurrentScene->getState().xochitl != nullptr;

    // ---- Phone mini-game has exclusive input while open ----
    if (gPhone->isActive())
    {
        PhoneEvent event = gPhone->processInput();
        if (event == PHONE_EVENT_SUCCESS)
        {
            gRun.cortisol          = 0;
            gRun.emergency         = false;
            gRun.emergencyTimeLeft = 0.0f;
            StopMusicStream(gEmergencyMusic);
        }
        else if (event == PHONE_EVENT_FAIL)
        {
            gRun.cortisol = MAX_CORTISOL;
            if (!gRun.emergency)
            {
                gRun.emergency         = true;
                gRun.emergencyTimeLeft = EMERGENCY_DURATION;
                if (gCurrentScene && gCurrentScene->getState().bgm.stream.buffer)
                    PauseMusicStream(gCurrentScene->getState().bgm);
                PlayMusicStream(gEmergencyMusic);
            }
        }
        return;
    }

    // Open phone (only during gameplay)
    if (isGameplay && IsKeyPressed(KEY_P) && gPhone->canOpen())
    {
        gPhone->open();
        return;
    }

    // Scene-specific input (player movement, Enter key, etc.)
    gCurrentScene->processInput();

    //switch levels with number keys
    if (IsKeyPressed(KEY_ONE))   switchToScene(gScenes[1]);
    if (IsKeyPressed(KEY_TWO))   switchToScene(gScenes[2]);
    if (IsKeyPressed(KEY_THREE)) switchToScene(gScenes[3]);
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
        // Phone open: pause the world, only tick the mini-game.
        if (gPhone->isActive())
        {
            gPhone->update(FIXED_TIMESTEP);
            deltaTime -= FIXED_TIMESTEP;
            continue;
        }

        gCurrentScene->update(FIXED_TIMESTEP);
        deltaTime -= FIXED_TIMESTEP;

        // Camera follows player (top-down: both axes)
        if (gCurrentScene->getState().xochitl != nullptr)
        {
            Vector2 currentPlayerPosition = gCurrentScene->getState().xochitl->getPosition();
            panCamera(&gCamera, &currentPlayerPosition);

            // Clamp camera to map boundaries on both axes
            if (gCurrentScene->getState().map != nullptr)
            {
                float halfW = SCREEN_WIDTH  / 2.0f;
                float halfH = SCREEN_HEIGHT / 2.0f;
                Map *m = gCurrentScene->getState().map;
                float left   = m->getLeftBoundary();
                float right  = m->getRightBoundary();
                float top    = m->getTopBoundary();
                float bottom = m->getBottomBoundary();

                if (gCamera.target.x < left + halfW)   gCamera.target.x = left + halfW;
                if (gCamera.target.x > right - halfW)  gCamera.target.x = right - halfW;
                if (gCamera.target.y < top + halfH)    gCamera.target.y = top + halfH;
                if (gCamera.target.y > bottom - halfH) gCamera.target.y = bottom - halfH;
            }

            gEffects->update(FIXED_TIMESTEP, &gCamera.target);
        }
        else
        {
            Vector2 menuTarget = ORIGIN;
            gEffects->update(FIXED_TIMESTEP, &menuTarget);
        }
    }

    gTimeAccumulator = deltaTime;

    // Phone is open — freeze cortisol/emergency/transition logic.
    if (gPhone->isActive()) return;

    // Cortisol: tick up whenever the scene reports contact, but rate-limited
    // by a short cooldown so an enemy that keeps touching the player doesn't
    // drain cortisol in a single frame.
    if (gRun.hitCooldown > 0.0f) gRun.hitCooldown -= FIXED_TIMESTEP;

    if (gCurrentScene->getState().playerHit && gRun.hitCooldown <= 0.0f)
    {
        gRun.cortisol    += CORTISOL_PER_HIT;
        gRun.hitCooldown  = HIT_COOLDOWN;

        if (gRun.cortisol >= MAX_CORTISOL)
        {
            gRun.cortisol = MAX_CORTISOL;
            if (!gRun.emergency)
            {
                gRun.emergency         = true;
                gRun.emergencyTimeLeft = EMERGENCY_DURATION;
                if (gCurrentScene && gCurrentScene->getState().bgm.stream.buffer)
                    PauseMusicStream(gCurrentScene->getState().bgm);
                PlayMusicStream(gEmergencyMusic);
            }
        }
    }

    if (gRun.emergency)
    {
        UpdateMusicStream(gEmergencyMusic);
        gRun.emergencyTimeLeft -= FIXED_TIMESTEP;
        if (gRun.emergencyTimeLeft <= 0.0f)
        {
            gRun.emergencyTimeLeft = 0.0f;
            switchToScene(gScenes[5]); // Lose scene
            return;
        }
    }

    // Handle scene transitions
    int nextScene = gCurrentScene->getState().nextSceneID;
    if (nextScene >= 0)
    {
        // Opening monologue: leaving MenuScene for LevelA plays a short
        // dialog first, then continues into LevelA.
        if (gCurrentScene == gMenuScene && nextScene == 1)
        {
            showMonologue({
                "fuh I don't wanna shower. I don't wanna clean up my room.",
                "cortisol.. cortisol.. cortisol..",
            }, 1);
            return;
        }

        // Reset cortisol when returning to menu
        if (nextScene == 0)
        {
            gRun = RunState{};
            gPhone->resetUses();
        }
        // Moving to a gameplay level: if player was maxed out, drop to 50;
        // otherwise carry the current cortisol value into the next level.
        else if (nextScene >= 1 && nextScene <= 3 && gRun.cortisol >= MAX_CORTISOL)
        {
            gRun.cortisol    = 50;
            gRun.hitCooldown = 0.0f;
        }

        switchToScene(gScenes[nextScene]);
        return;
    }
}

void render()
{
    BeginDrawing();
    BeginMode2D(gCamera);

    bool isGameplay = gCurrentScene->getState().xochitl != nullptr;

    if (isGameplay)
    {
        gShader.setVector2("lightPosition", gCurrentScene->getState().xochitl->getPosition());
        gShader.setFloat("uTime", (float)GetTime());
        gShader.setFloat("uEmergency", gRun.emergency ? 1.0f : 0.0f);
        gShader.begin();
    }

    gCurrentScene->render();

    if (isGameplay)
    {
        gShader.end();
    }

    gEffects->render();
    EndMode2D();

    // HUD — cortisol bar, emergency overlay, phone badge. Hidden while
    // the phone modal is open so it doesn't overlap.
    if (isGameplay && !gPhone->isActive()) gHud->render(gRun, *gPhone);

    // ---- Phone modal ----
    gPhone->render();

    EndDrawing();
}

void shutdown()
{
    delete gMenuScene;
    delete gMonologueScene;
    delete gLevelA;
    delete gLevelB;
    delete gLevelC;
    delete gWinScene;
    delete gLoseScene;

    for (int i = 0; i < NUMBER_OF_SCENES; i++) gScenes[i] = nullptr;

    delete gEffects;
    gEffects = nullptr;

    if (gPhone != nullptr)
    {
        gPhone->unload();
        delete gPhone;
        gPhone = nullptr;
    }

    delete gHud;
    gHud = nullptr;

    UnloadMusicStream(gEmergencyMusic);

    UnloadMusicStream(gBgm);
    UnloadSound(gJumpSound);
    UnloadSound(gStepSound);
    UnloadSound(gStompSound);

    gShader.unload();

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
