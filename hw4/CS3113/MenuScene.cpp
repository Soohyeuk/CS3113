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

    const char *title = "A SAMURAI THAT CAN'T USE SWORD, SO HE STOMPS INSTEAD";
    int titleFontSize = 30;
    int titleWidth = MeasureText(title, titleFontSize);
    DrawText(title, (int)(mOrigin.x - titleWidth / 2), (int)(mOrigin.y - 60), titleFontSize, YELLOW);

    const char *prompt = "Press ENTER to start";
    int promptFontSize = 28;
    int promptWidth = MeasureText(prompt, promptFontSize);
    DrawText(prompt, (int)(mOrigin.x - promptWidth / 2), (int)(mOrigin.y ), promptFontSize, LIGHTGRAY);
}

void MenuScene::shutdown() {}
