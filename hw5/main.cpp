/**
* Author: Soohyeuk Choi
* Assignment: Cortisol
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

int   gCortisol         = 0;
float gHitCooldownTimer = 0.0f; // seconds until next hit can register

bool  gEmergency         = false;
float gEmergencyTimeLeft = 0.0f;
Music gEmergencyMusic    = { 0 };

// Shared level audio: loaded once at startup, referenced by all levels.
Music gBgm        = { 0 };
Sound gJumpSound  = { 0 };
Sound gStepSound  = { 0 };
Sound gStompSound = { 0 };

// ---- Phone (doomscroll mini-game) ----
enum PhoneResult { PHONE_NONE, PHONE_SUCCESS, PHONE_FAIL };

Texture2D    gPhoneTex      = { 0 };
Texture2D    gCatTex        = { 0 };
Texture2D    gDoomTex       = { 0 };
Texture2D    gGlassesTex    = { 0 };
Texture2D    gHighCortisolTex = { 0 };
Texture2D    gLowCortisolTex  = { 0 };
bool         gPhoneActive   = false;
PhoneResult  gPhoneResult   = PHONE_NONE;
float        gPhoneBarY     = 0.0f;
float        gPhoneBarDir   = 1.0f;
int          gPhoneUsesLeft = 2;

// Inline monologue overlay — paused-dialog, Enter to advance.
// Used inside the phone modal (and re-usable anywhere else).
std::vector<std::string> gPhoneMonologue;
int                      gPhoneMonologueIdx    = 0;
bool                     gPhoneMonologueActive = false;

// Phone overlay layout (screen space)
constexpr float PHONE_BAR_TOP     = 230.0f;
constexpr float PHONE_BAR_BOTTOM  = 390.0f;
constexpr float PHONE_BAR_X       = SCREEN_WIDTH / 2.0f;
constexpr float PHONE_BAR_SPEED   = 260.0f; // px/sec
constexpr float PHONE_SWEET_TOP   = 295.0f;
constexpr float PHONE_SWEET_BOT   = 315.0f;

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

    gEmergency         = false;
    gEmergencyTimeLeft = 0.0f;
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

    gPhoneTex   = LoadTexture("assets/game/phone.png");
    gCatTex     = LoadTexture("assets/game/cat.png");
    gDoomTex    = LoadTexture("assets/game/doom.png");
    gGlassesTex = LoadTexture("assets/game/Glasses.png");
    gHighCortisolTex = LoadTexture("assets/game/high_cortisol.png");
    gLowCortisolTex  = LoadTexture("assets/game/low_cortisol.png");

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
    if (gPhoneActive)
    {
        // Monologue takes priority: Enter advances lines, no bar input.
        if (gPhoneMonologueActive)
        {
            if (IsKeyPressed(KEY_ENTER))
            {
                gPhoneMonologueIdx++;
                if (gPhoneMonologueIdx >= (int) gPhoneMonologue.size())
                    gPhoneMonologueActive = false;
            }
            return;
        }

        if (gPhoneResult == PHONE_NONE)
        {
            if (IsKeyPressed(KEY_SPACE))
            {
                bool inZone = (gPhoneBarY >= PHONE_SWEET_TOP &&
                               gPhoneBarY <= PHONE_SWEET_BOT);
                if (inZone)
                {
                    gPhoneResult = PHONE_SUCCESS;
                    gCortisol    = 0;
                    gEmergency         = false;
                    gEmergencyTimeLeft = 0.0f;
                    gPhoneMonologue       = { "W doomscroll sesh" };
                }
                else
                {
                    gPhoneResult = PHONE_FAIL;
                    gCortisol    = MAX_CORTISOL;
                    if (!gEmergency)
                    {
                        gEmergency         = true;
                        gEmergencyTimeLeft = EMERGENCY_DURATION;
                        if (gCurrentScene && gCurrentScene->getState().bgm.stream.buffer)
                            PauseMusicStream(gCurrentScene->getState().bgm);
                        PlayMusicStream(gEmergencyMusic);
                    }
                    gPhoneMonologue       = { "fuh I need a j*b" };
                }
                gPhoneMonologueIdx    = 0;
                gPhoneMonologueActive = true;
            }
        }
        else
        {
            // Result shown and monologue finished — any key closes.
            if (GetKeyPressed() != 0) gPhoneActive = false;
        }
        return;
    }

    // Open phone (only during gameplay and not already in emergency/paused)
    if (isGameplay && IsKeyPressed(KEY_P))
    {
        if (gPhoneUsesLeft <= 0) return;

        gPhoneUsesLeft--;
        gPhoneActive = true;
        gPhoneResult = PHONE_NONE;
        gPhoneBarY   = PHONE_BAR_TOP;
        gPhoneBarDir = 1.0f;
        gPhoneMonologue       = { "I'm tired. Let's just doomscroll." };
        gPhoneMonologueIdx    = 0;
        gPhoneMonologueActive = true;
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
        if (gPhoneActive)
        {
            // Freeze the bar while a monologue is showing.
            if (gPhoneResult == PHONE_NONE && !gPhoneMonologueActive)
            {
                gPhoneBarY += gPhoneBarDir * PHONE_BAR_SPEED * FIXED_TIMESTEP;
                if (gPhoneBarY >= PHONE_BAR_BOTTOM)
                {
                    gPhoneBarY   = PHONE_BAR_BOTTOM;
                    gPhoneBarDir = -1.0f;
                }
                else if (gPhoneBarY <= PHONE_BAR_TOP)
                {
                    gPhoneBarY   = PHONE_BAR_TOP;
                    gPhoneBarDir =  1.0f;
                }
            }
            // After a result, the phone stays open until the player presses a
            // key (handled in processInput). No auto-close.
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
    if (gPhoneActive) return;

    // Cortisol: tick up whenever the scene reports contact, but rate-limited
    // by a short cooldown so an enemy that keeps touching the player doesn't
    // drain cortisol in a single frame.
    if (gHitCooldownTimer > 0.0f) gHitCooldownTimer -= FIXED_TIMESTEP;

    if (gCurrentScene->getState().playerHit && gHitCooldownTimer <= 0.0f)
    {
        gCortisol         += CORTISOL_PER_HIT;
        gHitCooldownTimer  = HIT_COOLDOWN;

        if (gCortisol >= MAX_CORTISOL)
        {
            gCortisol = MAX_CORTISOL;
            if (!gEmergency)
            {
                gEmergency         = true;
                gEmergencyTimeLeft = EMERGENCY_DURATION;
                if (gCurrentScene && gCurrentScene->getState().bgm.stream.buffer)
                    PauseMusicStream(gCurrentScene->getState().bgm);
                PlayMusicStream(gEmergencyMusic);
            }
        }
    }

    if (gEmergency)
    {
        UpdateMusicStream(gEmergencyMusic);
        gEmergencyTimeLeft -= FIXED_TIMESTEP;
        if (gEmergencyTimeLeft <= 0.0f)
        {
            gEmergencyTimeLeft = 0.0f;
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
            gCortisol     = 0;
            gHitCooldownTimer = 0.0f;
            gPhoneUsesLeft = 2;
        }
        // Moving to a gameplay level: if player was maxed out, drop to 50;
        // otherwise carry the current cortisol value into the next level.
        else if (nextScene >= 1 && nextScene <= 3 && gCortisol >= MAX_CORTISOL)
        {
            gCortisol     = 50;
            gHitCooldownTimer = 0.0f;
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
        gShader.setFloat("uEmergency", gEmergency ? 1.0f : 0.0f);
        gShader.begin();
    }

    gCurrentScene->render();

    if (isGameplay)
    {
        gShader.end();
    }

    gEffects->render();
    EndMode2D();

    // Draw cortisol bar in screen space (only during gameplay levels,
    // and hidden while the phone mini-game is open to keep things clean).
    if (isGameplay && !gPhoneActive)
    {
        const int barX = 20, barY = 50, barW = 240, barH = 22;
        DrawText("Cortisol", barX, 20, 24, WHITE);
        DrawRectangle(barX, barY, barW, barH, Color{40, 40, 40, 200});
        int fillW = (int)(barW * (gCortisol / (float) MAX_CORTISOL));
        Color fillColour = (gCortisol >= 70) ? RED
                         : (gCortisol >= 40) ? ORANGE
                                             : GREEN;
        DrawRectangle(barX, barY, fillW, barH, fillColour);
        DrawRectangleLines(barX, barY, barW, barH, WHITE);
        DrawText(TextFormat("%d / %d", gCortisol, MAX_CORTISOL),
                 barX + barW + 12, barY, 22, WHITE);

        if (gEmergency)
        {
            int secs = (int) ceilf(gEmergencyTimeLeft);
            if (secs < 0) secs = 0;
            const char *txt = TextFormat("0:%02d", secs);
            int fontSize = 120;
            int tw = MeasureText(txt, fontSize);
            int tx = (SCREEN_WIDTH  - tw) / 2;
            int ty = (SCREEN_HEIGHT - fontSize) / 2;

            float pulse = 0.5f + 0.5f * sinf((float) GetTime() * 10.0f);
            unsigned char alpha = (unsigned char)(160 + 95 * pulse);
            Color red = { 255, 30, 30, alpha };

            DrawText(txt, tx + 3, ty + 3, fontSize, Color{0, 0, 0, 200});
            DrawText(txt, tx, ty, fontSize, red);

            const char *warn = "REACH THE EXIT!";
            int wfs = 28;
            int ww = MeasureText(warn, wfs);
            DrawText(warn, (SCREEN_WIDTH - ww) / 2, ty + fontSize + 10,
                     wfs, Color{255, 255, 255, alpha});
        }

        // ---- Phone HUD (bottom-right: phone icon on green circle, P badge) ----
        const int hudCX = SCREEN_WIDTH  - 55;
        const int hudCY = SCREEN_HEIGHT - 55;
        const int hudR  = 34;
        DrawCircle(hudCX, hudCY, hudR + 2, Color{ 0, 0, 0, 180 });
        DrawCircle(hudCX, hudCY, hudR,     Color{ 40, 180, 80, 230 });

        if (gPhoneTex.id != 0)
        {
            const float iconH  = hudR * 1.5f;
            float       aspect = (float) gPhoneTex.width / (float) gPhoneTex.height;
            float       iconW  = iconH * aspect;
            Rectangle src = { 0, 0, (float) gPhoneTex.width, (float) gPhoneTex.height };
            Rectangle dst = { (float) hudCX, (float) hudCY, iconW, iconH };
            Vector2   org = { iconW / 2.0f, iconH / 2.0f };
            DrawTexturePro(gPhoneTex, src, dst, org, 0.0f, WHITE);
        }

        // P badge tucked at the bottom-right of the phone icon
        int badgeX = hudCX + hudR - 6;
        int badgeY = hudCY + hudR - 6;
        DrawCircle(badgeX, badgeY, 11, Color{ 0, 0, 0, 220 });
        int pW = MeasureText("P", 16);
        DrawText("P", badgeX - pW / 2, badgeY - 8, 16, WHITE);

        // Uses left indicator
        const char *usesText = TextFormat("%d left", gPhoneUsesLeft);
        int uW = MeasureText(usesText, 16);
        DrawText(usesText, hudCX - uW / 2, hudCY + hudR + 4, 16, WHITE);
    }

    // ---- Phone modal ----
    if (gPhoneActive)
    {
        // Black backdrop over the world.
        DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Color{ 0, 0, 0, 245 });

        // Pick which texture fills the phone slot: the phone itself while
        // the mini-game is running, then cat / doom once a result is locked.
        Texture2D *screenTex = &gPhoneTex;
        if (gPhoneResult == PHONE_SUCCESS) screenTex = &gCatTex;
        else if (gPhoneResult == PHONE_FAIL) screenTex = &gDoomTex;

        const float screenH = 540.0f;
        float       aspect  = (screenTex->height > 0)
                            ? (float) screenTex->width / (float) screenTex->height
                            : 0.5f;
        const float screenW = screenH * aspect;

        if (screenTex->id != 0)
        {
            Rectangle src = { 0, 0, (float) screenTex->width, (float) screenTex->height };
            Rectangle dst = { SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f, screenW, screenH };
            Vector2   org = { screenW / 2.0f, screenH / 2.0f };
            DrawTexturePro(*screenTex, src, dst, org, 0.0f, WHITE);

            Texture2D *sideTex = nullptr;
            if (gPhoneResult == PHONE_SUCCESS) sideTex = &gLowCortisolTex;
            else if (gPhoneResult == PHONE_FAIL) sideTex = &gHighCortisolTex;

            if (sideTex != nullptr && sideTex->id != 0)
            {
                float phoneLeftEdge = dst.x - org.x;
                const float sideH = 250.0f; 
                float sideAspect = (float)sideTex->width / (float)sideTex->height;
                float sideW = sideH * sideAspect;
                float padding = -18.0f;

                Rectangle sideSrc = { 0, 0, (float) sideTex->width, (float) sideTex->height };
                Rectangle sideDst = { phoneLeftEdge - padding - sideW / 2.0f, SCREEN_HEIGHT / 2.0f, sideW, sideH };
                Vector2   sideOrg = { sideW / 2.0f, sideH / 2.0f };

                DrawTexturePro(*sideTex, sideSrc, sideDst, sideOrg, 0.0f, WHITE);
            }
        }

        // Vertical progress track — only while the mini-game is active
        // (not during intro/result monologues).
        if (gPhoneResult == PHONE_NONE && !gPhoneMonologueActive)
        {
            const float trackX = PHONE_BAR_X;
            const float trackW = 18.0f;
            DrawRectangle((int)(trackX - trackW/2), (int) PHONE_BAR_TOP,
                          (int) trackW, (int)(PHONE_BAR_BOTTOM - PHONE_BAR_TOP),
                          Color{ 20, 20, 20, 235 });
            DrawRectangle((int)(trackX - trackW/2), (int) PHONE_SWEET_TOP,
                          (int) trackW, (int)(PHONE_SWEET_BOT - PHONE_SWEET_TOP),
                          Color{ 40, 200, 90, 245 });

            const float indicatorW = trackW * 2.0f;
            DrawRectangle((int)(trackX - indicatorW/2), (int) gPhoneBarY - 3,
                          (int) indicatorW, 6, WHITE);
        }

        // Bottom hint line — varies by phase.
        const char *hint = nullptr;
        if (gPhoneMonologueActive)         hint = "press ENTER";
        else if (gPhoneResult == PHONE_NONE) hint = "SPACE to lock";
        else                               hint = "press any button to resume";
        int hfs = 18;
        int hw  = MeasureText(hint, hfs);
        DrawText(hint, (SCREEN_WIDTH - hw) / 2, SCREEN_HEIGHT - 34,
                 hfs, Color{ 220, 220, 220, 230 });

        // Monologue dialog overlay — sits on top of the phone sprite.
        if (gPhoneMonologueActive &&
            gPhoneMonologueIdx >= 0 &&
            gPhoneMonologueIdx < (int) gPhoneMonologue.size())
        {
            const int boxX = 40, boxY = 400, boxW = 920, boxH = 150;
            DrawRectangle(boxX, boxY, boxW, boxH, Color{ 18, 18, 28, 245 });
            DrawRectangleLines(boxX, boxY, boxW, boxH, Color{ 210, 210, 230, 255 });

            const int portraitSize = 120;
            const int portraitX    = boxX + 15;
            const int portraitY    = boxY + (boxH - portraitSize) / 2;
            if (gGlassesTex.id != 0)
            {
                Rectangle src = { 0, 0,
                                  (float) gGlassesTex.width,
                                  (float) gGlassesTex.height };
                Rectangle dst = { (float) portraitX, (float) portraitY,
                                  (float) portraitSize, (float) portraitSize };
                DrawTexturePro(gGlassesTex, src, dst, { 0, 0 }, 0.0f, WHITE);
            }

            const char *line  = gPhoneMonologue[gPhoneMonologueIdx].c_str();
            const int   fs    = 22;
            const int   textX = portraitX + portraitSize + 20;
            const int   textY = boxY + (boxH - fs) / 2;
            DrawText(line, textX, textY, fs, WHITE);
        }
    }

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

    UnloadTexture(gPhoneTex);
    UnloadTexture(gCatTex);
    UnloadTexture(gDoomTex);
    UnloadTexture(gGlassesTex);
    UnloadTexture(gHighCortisolTex);
    UnloadTexture(gLowCortisolTex);

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
