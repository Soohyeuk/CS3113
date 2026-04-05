#include "WinScene.h"

WinScene::WinScene()                                        : Scene { {0.0f}, nullptr   } {}
WinScene::WinScene(Vector2 origin, const char *bgHexCode)   : Scene { origin, bgHexCode } {}
WinScene::~WinScene() { shutdown(); }

void WinScene::initialise()
{
    mGameState.nextSceneID = -1;
    mGameState.xochitl     = nullptr;
    mGameState.map         = nullptr;
}

void WinScene::processInput()
{
    if (IsKeyPressed(KEY_ENTER))
    {
        mGameState.nextSceneID = 0; // Back to menu
    }
}

void WinScene::update(float deltaTime) {}

void WinScene::render()
{
    ClearBackground(ColorFromHex(mBGColourHexCode));

    const char *text = "YOU WIN!";
    int textWidth = MeasureText(text, 70);
    DrawText(text, (int)(mOrigin.x - textWidth / 2), (int)(mOrigin.y - 60), 70, GREEN);

    const char *sub = "Press ENTER for menu";
    int subWidth = MeasureText(sub, 28);
    DrawText(sub, (int)(mOrigin.x - subWidth / 2), (int)(mOrigin.y + 40), 28, WHITE);
}

void WinScene::shutdown() {}
