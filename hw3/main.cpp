/**
* Author: Soohyeuk Choi
* Assignment: Lunar Lander
* Date due: 03/14/2026
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

#include "CS3113/Entity.h"

constexpr int SCREEN_WIDTH  = 1000,
              SCREEN_HEIGHT = 600,
              FPS           = 120;

constexpr float GRAVITY        = 120.0f,
                THRUST         = 300.0f,
                DRAG           = 2.0f,
                DRAG_Y_FACTOR  = 0.3f,
                FIXED_TIMESTEP = 1.0f / 60.0f;

constexpr float MAX_FUEL          = 100.0f,
                FUEL_CONSUMPTION  = 20.0f;

constexpr float PLATFORM_SIZE = 90.0f,
                GROUND_Y      = 550.0f;

constexpr int NUM_SAFE_PLATFORMS = 2,
              NUM_DANGER_GROUND  = 11,
              NUM_GROUND         = NUM_SAFE_PLATFORMS + NUM_DANGER_GROUND,
              MOVING_IDX         = NUM_GROUND,
              NUM_PLATFORMS       = NUM_GROUND + 1,
              NUM_TRASH          = 6,
              NUM_ENTITIES       = NUM_PLATFORMS + NUM_TRASH;

// ——— Game state ———
enum GameState { PLAYING, WON, LOST };

AppStatus  gAppStatus       = RUNNING;
GameState  gGameState       = PLAYING;
float      gPreviousTicks   = 0.0f,
           gTimeAccumulator = 0.0f,
           gFuel            = MAX_FUEL,
           gMovingTime      = 0.0f;
bool       gThrusting       = false;

Entity    *gPlayer   = nullptr;
Entity    *gEntities = nullptr;   // all collidable entities in one array
Texture2D  gBackground;

void initialise();
void processInput();
void update();
void render();
void shutdown();

void initialise()
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Sinky Sub");

    gBackground = LoadTexture("assets/Sinky_Sub_BG.png");

    // ——— Player (diver) ———
    gPlayer = new Entity(
        {SCREEN_WIDTH / 2.0f, 80.0f},
        {80.0f, 96.0f},
        "assets/diver_yellow.png",
        PLAYER
    );
    gPlayer->setAcceleration({0.0f, GRAVITY});
    gPlayer->setColliderDimensions({50.0f, 60.0f});

    // ——— All collidable entities in one array ———
    gEntities = new Entity[NUM_ENTITIES];

    //safe landing pads (WINNING_PLATFORM) — centre of ground
    float safeX[] = {455.0f, 545.0f};
    for (int i = 0; i < NUM_SAFE_PLATFORMS; i++)
    {
        gEntities[i].setTexture("assets/Sinky_Sub_Floor.png");
        gEntities[i].setEntityType(WINNING_PLATFORM);
        gEntities[i].setScale({PLATFORM_SIZE, PLATFORM_SIZE});
        gEntities[i].setColliderDimensions({PLATFORM_SIZE, PLATFORM_SIZE / 6.0f});
        gEntities[i].setColliderOffset({0.0f, PLATFORM_SIZE * 5.0f / 12.0f});
        gEntities[i].setPosition({safeX[i], GROUND_Y});
    }

    //danger ground tiles (BLOCK) — rocks on both sides
    float dangerX[] = {5.0f, 95.0f, 185.0f, 275.0f, 365.0f,
                       635.0f, 725.0f, 815.0f, 905.0f, 995.0f, 1085.0f};
    for (int i = 0; i < NUM_DANGER_GROUND; i++)
    {
        int idx = NUM_SAFE_PLATFORMS + i;
        gEntities[idx].setTexture("assets/Sinky_Sub_CloseRockKelp.png");
        gEntities[idx].setEntityType(BLOCK);
        gEntities[idx].setScale({PLATFORM_SIZE, PLATFORM_SIZE});
        gEntities[idx].setColliderDimensions({PLATFORM_SIZE, PLATFORM_SIZE / 3.0f});
        gEntities[idx].setColliderOffset({0.0f, PLATFORM_SIZE / 3.0f});
        gEntities[idx].setPosition({dangerX[i], GROUND_Y});
    }

    //moving safe platform (WINNING_PLATFORM)
    gEntities[MOVING_IDX].setTexture("assets/Sinky_Sub_Floor.png");
    gEntities[MOVING_IDX].setEntityType(WINNING_PLATFORM);
    gEntities[MOVING_IDX].setScale({PLATFORM_SIZE, PLATFORM_SIZE});
    gEntities[MOVING_IDX].setColliderDimensions({PLATFORM_SIZE, PLATFORM_SIZE / 6.0f});
    gEntities[MOVING_IDX].setColliderOffset({0.0f, PLATFORM_SIZE * 5.0f / 12.0f});
    gEntities[MOVING_IDX].setPosition({500.0f, 350.0f});

    //floating trash obstacles (BLOCK) — appended after platforms
    const char *trashFiles[] = {
        "assets/trash1.png", "assets/trash2.png", "assets/trash3.png", "assets/trash2.png", "assets/trash3.png",
        "assets/trash1.png"
    };
    Vector2 trashPos[] = {
        {280.0f, 280.0f}, {650.0f, 220.0f}, {180.0f, 400.0f},
        {780.0f, 350.0f}, {350.0f, 350.0f},
        {550.0f, 430.0f}
    };

    for (int i = 0; i < NUM_TRASH; i++)
    {
        int idx = NUM_PLATFORMS + i;
        gEntities[idx].setTexture(trashFiles[i]);
        gEntities[idx].setEntityType(BLOCK);
        gEntities[idx].setScale({70.0f, 70.0f});
        gEntities[idx].setColliderDimensions({60.0f, 60.0f});
        gEntities[idx].setPosition(trashPos[i]);
    }

    SetTargetFPS(FPS);
}

void processInput()
{
    if (IsKeyPressed(KEY_Q) || WindowShouldClose())
    {
        gAppStatus = TERMINATED;
        return;
    }

    if (gGameState != PLAYING) return;

    gThrusting = false;

    Vector2 vel = gPlayer->getVelocity();

    //drag opposes current velocity; gravity always pulls down
    float accX = -vel.x * DRAG;
    float accY = GRAVITY - vel.y * DRAG * DRAG_Y_FACTOR;

    if (gFuel > 0.0f)
    {
        if (IsKeyDown(KEY_LEFT))
            { accX -= THRUST; gThrusting = true; }

        if (IsKeyDown(KEY_RIGHT))
            { accX += THRUST; gThrusting = true; }

        if (IsKeyDown(KEY_UP))
            { accY -= THRUST; gThrusting = true; }
    }

    gPlayer->setAcceleration({accX, accY});
}

void update()
{
    float ticks     = (float)GetTime();
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
        if (gGameState != PLAYING)
        {
            deltaTime -= FIXED_TIMESTEP;
            continue;
        }

        //consume fuel while thrusting
        if (gThrusting)
        {
            gFuel -= FUEL_CONSUMPTION * FIXED_TIMESTEP;
            if (gFuel < 0.0f) gFuel = 0.0f;
        }

        //animate moving platform (sine wave left-right)
        gMovingTime += FIXED_TIMESTEP;
        gEntities[MOVING_IDX].setPosition({
            500.0f + sinf(gMovingTime * 1.0f) * 200.0f,
            350.0f
        });

        //physics update
        gPlayer->update(FIXED_TIMESTEP, gEntities, NUM_ENTITIES);

        //check win / lose by scanning which entity the player collided with
        if (gPlayer->isCollidingBottom() || gPlayer->isCollidingTop() ||
            gPlayer->isCollidingLeft()   || gPlayer->isCollidingRight())
        {
            Vector2 pPos = gPlayer->getPosition();
            Vector2 pCol = gPlayer->getColliderDimensions();

            for (int i = 0; i < NUM_ENTITIES; i++)
            {
                if (!gEntities[i].isActive()) continue;

                Vector2 ePos = gEntities[i].getPosition();
                Vector2 eCol = gEntities[i].getColliderDimensions();
                Vector2 eOff = gEntities[i].getColliderOffset();

                float xDist = fabs(pPos.x - (ePos.x + eOff.x));
                float yDist = fabs(pPos.y - (ePos.y + eOff.y));
                float xThresh = (pCol.x + eCol.x) / 2.0f + 2.0f;
                float yThresh = (pCol.y + eCol.y) / 2.0f + 2.0f;

                if (xDist < xThresh && yDist < yThresh)
                {
                    EntityType type = gEntities[i].getEntityType();
                    if (type == BLOCK)
                        { gGameState = LOST; break; }
                    else if (type == WINNING_PLATFORM && gPlayer->isCollidingBottom())
                        { gGameState = WON; break; }
                }
            }
        }

        //out of bounds = lose
        Vector2 pos = gPlayer->getPosition();
        if (pos.y > SCREEN_HEIGHT + 50 ||
            pos.x < -50 || pos.x > SCREEN_WIDTH + 50)
            gGameState = LOST;

        deltaTime -= FIXED_TIMESTEP;
    }

    gTimeAccumulator = deltaTime;
}

void render()
{
    BeginDrawing();
    ClearBackground(ColorFromHex("#0e2a47"));

    // Background image stretched to fill window
    DrawTexturePro(
        gBackground,
        {0, 0, (float)gBackground.width, (float)gBackground.height},
        {0, 0, (float)SCREEN_WIDTH, (float)SCREEN_HEIGHT},
        {0, 0}, 0.0f, WHITE
    );

    for (int i = 0; i < NUM_ENTITIES; i++) gEntities[i].render();
    gPlayer->render();

    // ——— Fuel bar UI ———
    constexpr int BAR_W = 200, BAR_H = 20, BAR_X = 20, BAR_Y = 40;
    float ratio = gFuel / MAX_FUEL;
    Color fuelColor = ratio > 0.5f ? GREEN : (ratio > 0.25f ? YELLOW : RED);

    DrawText("FUEL", BAR_X, BAR_Y - 22, 20, WHITE);
    DrawRectangle(BAR_X, BAR_Y, BAR_W, BAR_H, DARKGRAY);
    DrawRectangle(BAR_X, BAR_Y, (int)(BAR_W * ratio), BAR_H, fuelColor);
    DrawRectangleLines(BAR_X, BAR_Y, BAR_W, BAR_H, WHITE);

    // ——— End-game messages ———
    if (gGameState == WON)
    {
        const char *msg = "Mission Accomplished!";
        int w = MeasureText(msg, 40);
        DrawText(msg, SCREEN_WIDTH / 2 - w / 2, SCREEN_HEIGHT / 2 - 20, 40, GREEN);
    }
    else if (gGameState == LOST)
    {
        const char *msg = "Mission Failed!";
        int w = MeasureText(msg, 40);
        DrawText(msg, SCREEN_WIDTH / 2 - w / 2, SCREEN_HEIGHT / 2 - 20, 40, RED);
    }

    EndDrawing();
}

void shutdown()
{
    delete   gPlayer;
    delete[] gEntities;

    UnloadTexture(gBackground);
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
