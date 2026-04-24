#include "Scene.h"

Scene::Scene() : mOrigin{{}} {}

Scene::Scene(Vector2 origin, const char *bgHexCode) : mOrigin{origin}, mBGColourHexCode {bgHexCode}
{
    ClearBackground(ColorFromHex(bgHexCode));
}

void Scene::processInput()
{
    if (mGameState.xochitl == nullptr) return;

    constexpr double STEP_SOUND_INTERVAL = 0.20;
    static double lastStepSoundTime = 0.0;

    mGameState.xochitl->resetMovement();

    if      (IsKeyDown(KEY_A)) mGameState.xochitl->moveLeft();
    else if (IsKeyDown(KEY_D)) mGameState.xochitl->moveRight();

    if      (IsKeyDown(KEY_W)) mGameState.xochitl->moveUp();
    else if (IsKeyDown(KEY_S)) mGameState.xochitl->moveDown();


    bool isWalking =
        (IsKeyDown(KEY_A) || IsKeyDown(KEY_D) || IsKeyDown(KEY_W) || IsKeyDown(KEY_S));

    if (isWalking && mGameState.stepSound.frameCount > 0)
    {
        double now = GetTime();
        if (now - lastStepSoundTime >= STEP_SOUND_INTERVAL)
        {
            PlaySound(mGameState.stepSound);
            lastStepSoundTime = now;
        }
    }


    if (GetLength(mGameState.xochitl->getMovement()) > 1.0f)
        mGameState.xochitl->normaliseMovement();
}
