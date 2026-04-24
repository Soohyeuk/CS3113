#include "LoseScene.h"

LoseScene::LoseScene()                                        : Scene { {0.0f}, nullptr   } {}
LoseScene::LoseScene(Vector2 origin, const char *bgHexCode)   : Scene { origin, bgHexCode } {}
LoseScene::~LoseScene() { shutdown(); }

void LoseScene::initialise()
{
    mGameState.nextSceneID = -1;
    mGameState.xochitl     = nullptr;
    mGameState.map         = nullptr;
}

void LoseScene::processInput()
{
    if (IsKeyPressed(KEY_ENTER))
    {
        mGameState.nextSceneID = 0; // Back to menu
    }
}

void LoseScene::update(float deltaTime) {}

void LoseScene::render()
{
    ClearBackground(ColorFromHex(mBGColourHexCode));

    const char *text = "Cortisolmaxxing";
    int textWidth = MeasureText(text, 70);
    DrawText(text, (int)(mOrigin.x - textWidth / 2), (int)(mOrigin.y - 60), 70, RED);

    const char *sub = "Press ENTER for menu";
    int subWidth = MeasureText(sub, 28);
    DrawText(sub, (int)(mOrigin.x - subWidth / 2), (int)(mOrigin.y + 40), 28, WHITE);
}

void LoseScene::shutdown() {}
