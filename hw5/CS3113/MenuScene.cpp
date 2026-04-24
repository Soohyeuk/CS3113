#include "MenuScene.h"

MenuScene::MenuScene()                                        : Scene { {0.0f}, nullptr   } {}
MenuScene::MenuScene(Vector2 origin, const char *bgHexCode)   : Scene { origin, bgHexCode } {}
MenuScene::~MenuScene() { shutdown(); }

void MenuScene::initialise()
{
    mGameState.nextSceneID = -1;
    mGameState.xochitl     = nullptr;
    mGameState.map         = nullptr;
}

void MenuScene::processInput()
{
    if (IsKeyPressed(KEY_ENTER))
    {
        mGameState.nextSceneID = 1; // Go to LevelA
    }
}

void MenuScene::update(float deltaTime) {}

void MenuScene::render()
{
    ClearBackground(ColorFromHex(mBGColourHexCode));

    const char *title = "YOUR LIFE";
    int titleFontSize = 64;
    int titleWidth = MeasureText(title, titleFontSize);
    DrawText(title, (int)(mOrigin.x - titleWidth / 2), (int)(mOrigin.y - 160), titleFontSize, YELLOW);

    const char *prompt = "Press ENTER to start";
    int promptFontSize = 28;
    int promptWidth = MeasureText(prompt, promptFontSize);
    DrawText(prompt, (int)(mOrigin.x - promptWidth / 2), (int)(mOrigin.y - 80), promptFontSize, LIGHTGRAY);

    // Instructions
    const char *heading = "How to play";
    int hfs = 24;
    int hw  = MeasureText(heading, hfs);
    DrawText(heading, (int)(mOrigin.x - hw / 2), (int)(mOrigin.y - 20), hfs, WHITE);

    const char *lines[] = {
        "WASD  -  move",
        "P     -  doomscroll (risk / reward)",
        "SPACE -  lock the scroll in the green zone",
        "Avoid hazards.  Cortisol carries between levels.",
        "Hit 100 cortisol and you have 60s to reach the exit.",
    };
    int lfs = 20;
    int lineH = 26;
    int startY = (int)(mOrigin.y + 20);
    for (int i = 0; i < 5; i++)
    {
        int lw = MeasureText(lines[i], lfs);
        DrawText(lines[i], (int)(mOrigin.x - lw / 2), startY + i * lineH,
                 lfs, Color{ 210, 210, 210, 255 });
    }
}

void MenuScene::shutdown() {}
