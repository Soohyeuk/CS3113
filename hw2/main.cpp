/**
* Author: Soohyeuk Choi
* Assignment: Pong Clone
* Date due: 02/20/2026

* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

#include "CS3113/cs3113.h"


// Global Constants
constexpr int SCREEN_WIDTH  = 1200,
              SCREEN_HEIGHT = 675,
              FPS           = 60,
              SIZE          = 500 / 2,
              SPRITE_SIZE   = 150,
              SPEED         = 400;

constexpr float BALL_SPEED   = 500.0f,
                AI_SPEED     = 350.0f;
constexpr int   MAX_BALLS    = 3;

constexpr char    BG_COLOUR[]    = "#F8F1C8";
constexpr Vector2 ORIGIN         = { SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 },
                  BASE_SIZE      = { (float) SIZE, (float) SIZE },
                  DIVER_SIZE     = { (float) SPRITE_SIZE, (float) SPRITE_SIZE },
                  TRASH_SIZE = { (float) SPRITE_SIZE/2, (float) SPRITE_SIZE/2 },
                  INIT_POS  = { ORIGIN.x - 200.0f, ORIGIN.y },
                  RUPEE_INIT_POS = { ORIGIN.x + 250.0f, ORIGIN.y };


// Global Variables
enum GameState { PLAYING, GAME_OVER };
AppStatus  gAppStatus     = RUNNING;
GameState  gGameState     = PLAYING;
float      gAngle         = 0.0f,
           gPreviousTicks = 0.0f,
           gDeltaTime     = 0.0f;


Vector2 gDiverBluePos = { SCREEN_WIDTH - 100.0f, SCREEN_HEIGHT / 2.0f };
Vector2 gDiverYlwPos  = { 100.0f, SCREEN_HEIGHT / 2.0f };

Vector2 gTrashPos[MAX_BALLS];
Vector2 gTrashVel[MAX_BALLS];
int     gActiveBalls = 1;

//texture imports 
constexpr char BG[]  = "assets/Sinky_Sub_BG.png";
constexpr char ROCK[]  = "assets/Sinky_Sub_CloseRockkelp.png";
constexpr char FLOOR[]  = "assets/Sinky_Sub_Floor.png";

constexpr char DIVER_BLUE[] = "assets/diver_blue.png";
constexpr char DIVER_YLW[] = "assets/diver_yellow.png";
constexpr char TRASH[] = "assets/trash.png";

Texture2D  gRock,
           gFloor,
           gBG, 
           gDiverBlue,
           gDiverYlw,
           gTrash;

bool gAIEnabled = false;
float gAIDirection = 1.0f;  // 1 = down, -1 = up
int gScoreLeft  = 0;
int gScoreRight = 0;
constexpr int WIN_SCORE = 10;


unsigned int startTime;

// Function Declarations
void initialise();
void processInput();
void update();
void render();
void shutdown();

void resetBall(int i);
void resetAllBalls();

bool isColliding(const Vector2 *postionA,  const Vector2 *scaleA, const Vector2 *positionB, const Vector2 *scaleB);

// Function Definitions

/**
 * @brief Checks for a square collision between 2 Rectangle objects.
 * 
 * @see 
 * 
 * @param postionA The position of the first object
 * @param scaleA The scale of the first object
 * @param positionB The position of the second object
 * @param scaleB The scale of the second object
 * @return true if a collision is detected,
 * @return false if a collision is not detected
 */
bool isColliding(const Vector2 *postionA,  const Vector2 *scaleA, 
                 const Vector2 *positionB, const Vector2 *scaleB)
{
    float xDistance = fabs(postionA->x - positionB->x) - 
                      ((scaleA->x + scaleB->x) / 2.0f);
    float yDistance = fabs(postionA->y - positionB->y) - 
                      ((scaleA->y + scaleB->y) / 2.0f);

    if (xDistance < 0.0f && yDistance < 0.0f) return true;

    return false;
}

void renderObject(const Texture2D *texture, const Vector2 *position, 
                  const Vector2 *scale)
{
    // Whole texture (UV coordinates)
    Rectangle textureArea = {
        // top-left corner
        0.0f, 0.0f,

        // 
    // Blue diver (right) — IJKLbottom-right corner (of texture)
        static_cast<float>(texture->width),
        static_cast<float>(texture->height)
    };

    // Destination rectangle – centred on gPosition
    Rectangle destinationArea = {
        position->x,
        position->y,
        static_cast<float>(scale->x),
        static_cast<float>(scale->y)
    };

    // Origin inside the source texture (centre of the texture)
    Vector2 originOffset = {
        static_cast<float>(scale->x) / 2.0f,
        static_cast<float>(scale->y) / 2.0f
    };

    // Render the texture on screen
    DrawTexturePro(
        *texture, 
        textureArea, destinationArea, originOffset,
        gAngle, WHITE
    );
}

void resetBall(int i)
{
    float yOffset = 0.0f;
    if (i == 1) yOffset = -40.0f;
    if (i == 2) yOffset =  40.0f;

    gTrashPos[i] = { SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f + yOffset };

    //changing the direction of the ball 
    float horizontalDir;
    if (gTrashVel[i].x > 0)
        horizontalDir = -1.0f;   //right to left
    else
        horizontalDir =  1.0f; 

    float verticalSpeed = 200.0f;
    if (i == 1) verticalSpeed = 150.0f;
    if (i == 2) verticalSpeed = 250.0f;

    gTrashVel[i] = { horizontalDir * BALL_SPEED, verticalSpeed };
}

void resetAllBalls()
{
    for (int i = 0; i < MAX_BALLS; i++)
    {
        gTrashPos[i] = { SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f };

        //changing the direction of the ball 
        float horizontalDir;
        if (gTrashVel[i].x > 0)
            horizontalDir = -1.0f;   //right to left
        else
            horizontalDir =  1.0f; 

        float verticalSpeed = 200.0f;
        if (i == 1) verticalSpeed = 150.0f;
        if (i == 2) verticalSpeed = 250.0f;

        gTrashVel[i] = { horizontalDir * BALL_SPEED, verticalSpeed };
    }
}

// Helper: clamp a diver, so they dont leave the screen 
void clampDiverToHalf(bool isBlue, Vector2 *pos)
{
    float halfDiver = SPRITE_SIZE / 3.0f;

    if (pos->y - halfDiver < 0) pos->y = halfDiver;
    if (pos->y + halfDiver > SCREEN_HEIGHT) pos->y = SCREEN_HEIGHT - halfDiver;

    if (isBlue)
    {
        //right half
        if (pos->x - halfDiver < SCREEN_WIDTH / 2.0f) pos->x = SCREEN_WIDTH / 2.0f + halfDiver;
        if (pos->x + halfDiver > SCREEN_WIDTH) pos->x = SCREEN_WIDTH - halfDiver;
    }
    else
    {
        //left half
        if (pos->x - halfDiver < 0) pos->x = halfDiver;
        if (pos->x + halfDiver > SCREEN_WIDTH / 2.0f) pos->x = SCREEN_WIDTH / 2.0f - halfDiver;
    }
}

void initialise()
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Pong Clone");

    startTime = time(NULL);

    gRock = LoadTexture(ROCK);
    gFloor = LoadTexture(FLOOR);
    gBG = LoadTexture(BG);
    gDiverBlue = LoadTexture(DIVER_BLUE);
    gDiverYlw = LoadTexture(DIVER_YLW);
    gTrash = LoadTexture(TRASH);

    resetAllBalls();

    SetTargetFPS(FPS);
}

void processInput() 
{
    if (IsKeyPressed(KEY_Q) || WindowShouldClose()) gAppStatus = TERMINATED;

    //restart from the ending screen
    if (gGameState == GAME_OVER && IsKeyPressed(KEY_R))
    {
        gScoreLeft = 0;
        gScoreRight = 0;
        gDiverBluePos = { SCREEN_WIDTH - 100.0f, SCREEN_HEIGHT / 2.0f };
        gDiverYlwPos  = { 100.0f, SCREEN_HEIGHT / 2.0f };
        resetAllBalls();
        gGameState = PLAYING;
        return;
    }

    //AI with T button
    if (IsKeyPressed(KEY_T)) gAIEnabled = !gAIEnabled;

    //number of balls 
    if (IsKeyPressed(KEY_ONE))   { gActiveBalls = 1; resetAllBalls(); }
    if (IsKeyPressed(KEY_TWO))   { gActiveBalls = 2; resetAllBalls(); }
    if (IsKeyPressed(KEY_THREE)) { gActiveBalls = 3; resetAllBalls(); }


    if (!gAIEnabled)
    {
        if (IsKeyDown(KEY_I)) gDiverBluePos.y -= SPEED * gDeltaTime;
        if (IsKeyDown(KEY_K)) gDiverBluePos.y += SPEED * gDeltaTime;
        if (IsKeyDown(KEY_J)) gDiverBluePos.x -= SPEED * gDeltaTime;
        if (IsKeyDown(KEY_L)) gDiverBluePos.x += SPEED * gDeltaTime;
    }

    if (IsKeyDown(KEY_W)) gDiverYlwPos.y -= SPEED * gDeltaTime;
    if (IsKeyDown(KEY_S)) gDiverYlwPos.y += SPEED * gDeltaTime;
    if (IsKeyDown(KEY_A)) gDiverYlwPos.x -= SPEED * gDeltaTime;
    if (IsKeyDown(KEY_D)) gDiverYlwPos.x += SPEED * gDeltaTime;
}

void update() 
{
    // Delta time
    float ticks = (float) GetTime();
    gDeltaTime = ticks - gPreviousTicks;
    gPreviousTicks = ticks;

    if (gGameState == GAME_OVER) return;
    
    //clamp divers
    clampDiverToHalf(true, &gDiverBluePos);
    clampDiverToHalf(false, &gDiverYlwPos);

    // AI movement to be justs up and down
    if (gAIEnabled)
    {
        float halfDiverAI = SPRITE_SIZE / 3.0f;
        gDiverBluePos.y += gAIDirection * AI_SPEED * gDeltaTime;
        
        //bounce at top and bottom
        if (gDiverBluePos.y - halfDiverAI <= 0.0f)
        {
            gDiverBluePos.y = halfDiverAI;
            gAIDirection = 1.0f;  //go down
        }
        if (gDiverBluePos.y + halfDiverAI >= SCREEN_HEIGHT)
        {
            gDiverBluePos.y = SCREEN_HEIGHT - halfDiverAI;
            gAIDirection = -1.0f;  //go up
        }
    }

    float quarter_trash = SPRITE_SIZE / 4.0f;
    for (int i = 0; i < gActiveBalls; i++)
    {
        gTrashPos[i].x += gTrashVel[i].x * gDeltaTime;
        gTrashPos[i].y += gTrashVel[i].y * gDeltaTime;

        //bounce off top and bottom walls
        if (gTrashPos[i].y - quarter_trash <= 0.0f)
        {
            gTrashPos[i].y = quarter_trash;
            gTrashVel[i].y = fabs(gTrashVel[i].y);
        }
        if (gTrashPos[i].y + quarter_trash >= SCREEN_HEIGHT)
        {
            gTrashPos[i].y = SCREEN_HEIGHT - quarter_trash;
            gTrashVel[i].y = -fabs(gTrashVel[i].y);
        }

        //collision check for yellow driver with the trash
        if (isColliding(&gTrashPos[i], &TRASH_SIZE, &gDiverYlwPos, &DIVER_SIZE))
        {
            gTrashVel[i].x = fabs(gTrashVel[i].x); //send right
            gTrashVel[i].y = -gTrashVel[i].y;      //reverse vertical
        }

        //collision check for blue with the trash
        if (isColliding(&gTrashPos[i], &TRASH_SIZE, &gDiverBluePos, &DIVER_SIZE))
        {
            gTrashVel[i].x = -fabs(gTrashVel[i].x); //send left
            gTrashVel[i].y = -gTrashVel[i].y;       //reverse vertical
        }

        //scoring system
        if (gTrashPos[i].x + quarter_trash < 0.0f)
        {
            gScoreRight++;
            if (gScoreRight >= WIN_SCORE) gGameState = GAME_OVER;
            else resetBall(i);
        }
        if (gTrashPos[i].x - quarter_trash > SCREEN_WIDTH)
        {
            gScoreLeft++;
            if (gScoreLeft >= WIN_SCORE) gGameState = GAME_OVER;
            else resetBall(i);
        }
    }
}

void render()
{
    BeginDrawing();

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
        static_cast<float>(gRock.width),
        static_cast<float>(gRock.height)
    };
    DrawTexturePro(gRock, rockkelp, {0,0, SCREEN_WIDTH, SCREEN_HEIGHT}, {0,0}, 0.0f, WHITE);


    // floor
    Rectangle floorArea = {
        0.0f, 0.0f, 
        static_cast<float>(gFloor.width),
        static_cast<float>(gFloor.height)
    };
    DrawTexturePro(gFloor, floorArea, {0,0, SCREEN_WIDTH, SCREEN_HEIGHT}, {0,0}, 0.0f, WHITE);

    //render all active balls
    for (int i = 0; i < gActiveBalls; i++)
    {
        renderObject(&gTrash, &gTrashPos[i], &TRASH_SIZE);
    }
    renderObject(&gDiverBlue, &gDiverBluePos, &DIVER_SIZE);
    renderObject(&gDiverYlw, &gDiverYlwPos, &DIVER_SIZE);

    //drawing texts - used outside sources to learn how to format texts and draw them
    const char *scoreText = TextFormat("%d    %d", gScoreLeft, gScoreRight);
    int textWidth = MeasureText(scoreText, 40);
    DrawText(scoreText, (SCREEN_WIDTH - textWidth) / 2, 20, 40, WHITE);

    const char *commands = "WASD: Yellow | IJKL: Blue | 1/2/3: Balls | T: AI | Q: Quit";
    int cmdWidth = MeasureText(commands, 18);
    DrawText(commands, (SCREEN_WIDTH - cmdWidth) / 2, SCREEN_HEIGHT - 30, 18, WHITE);

    if (gAIEnabled)
    {
        const char *aiText = "[AI ON]";
        int aiWidth = MeasureText(aiText, 20);
        DrawText(aiText, SCREEN_WIDTH - aiWidth - 10, 20, 20, YELLOW);
    }

    //game over screen 
    if (gGameState == GAME_OVER)
    {
        DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, { 0, 0, 0, 180 });
        const char *winner = (gScoreLeft >= WIN_SCORE) ? "YELLOW DIVER WINS!" : "BLUE DIVER WINS!";
        int winWidth = MeasureText(winner, 50);
        DrawText(winner, (SCREEN_WIDTH - winWidth) / 2, SCREEN_HEIGHT / 2 - 50, 50, WHITE);

        const char *restart = "Press R to restart or Q to quit";
        int restartWidth = MeasureText(restart, 24);
        DrawText(restart, (SCREEN_WIDTH - restartWidth) / 2, SCREEN_HEIGHT / 2 + 20, 24, WHITE);
    }

    EndDrawing();
}

void shutdown() 
{ 
    CloseWindow(); 
    UnloadTexture(gRock);
    UnloadTexture(gFloor);
    UnloadTexture(gBG);
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