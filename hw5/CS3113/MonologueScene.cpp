#include "MonologueScene.h"

MonologueScene::MonologueScene()
    : Scene{ {0.0f}, nullptr },
      mCurrentLine(0), mAfterSceneID(0),
      mCharacterTex{ 0 }, mTexLoaded(false) {}

MonologueScene::MonologueScene(Vector2 origin, const char *bgHexCode)
    : Scene{ origin, bgHexCode },
      mCurrentLine(0), mAfterSceneID(0),
      mCharacterTex{ 0 }, mTexLoaded(false) {}

MonologueScene::~MonologueScene() { shutdown(); }

void MonologueScene::configure(std::vector<std::string> lines, int afterSceneID)
{
    mLines        = std::move(lines);
    mAfterSceneID = afterSceneID;
    mCurrentLine  = 0;
}

void MonologueScene::initialise()
{
    mGameState.nextSceneID = -1;
    mGameState.xochitl     = nullptr;
    mGameState.map         = nullptr;

    if (!mTexLoaded)
    {
        mCharacterTex = LoadTexture("assets/game/Glasses.png");
        mTexLoaded    = true;
    }
    mCurrentLine = 0;
}

void MonologueScene::processInput()
{
    if (IsKeyPressed(KEY_ENTER))
    {
        mCurrentLine++;
        if (mCurrentLine >= (int) mLines.size())
            mGameState.nextSceneID = mAfterSceneID;
    }
}

void MonologueScene::update(float deltaTime) { (void) deltaTime; }

void MonologueScene::render()
{
    ClearBackground(BLACK);

    // Dialog box (bottom third-ish of the screen)
    const int boxX = 40, boxY = 340, boxW = 920, boxH = 220;
    DrawRectangle(boxX, boxY, boxW, boxH, Color{ 18, 18, 28, 245 });
    DrawRectangleLines(boxX, boxY, boxW, boxH, Color{ 210, 210, 230, 255 });

    // Character portrait on the left
    const int portraitSize = 180;
    const int portraitX    = boxX + 20;
    const int portraitY    = boxY + (boxH - portraitSize) / 2;
    if (mTexLoaded && mCharacterTex.id != 0)
    {
        Rectangle src = { 0, 0,
                          (float) mCharacterTex.width,
                          (float) mCharacterTex.height };
        Rectangle dst = { (float) portraitX, (float) portraitY,
                          (float) portraitSize, (float) portraitSize };
        DrawTexturePro(mCharacterTex, src, dst, { 0, 0 }, 0.0f, WHITE);
    }

    // Line text to the right of the portrait
    if (mCurrentLine >= 0 && mCurrentLine < (int) mLines.size())
    {
        const char *line  = mLines[mCurrentLine].c_str();
        const int   fs    = 24;
        const int   textX = portraitX + portraitSize + 24;
        const int   textY = boxY + 40;
        DrawText(line, textX, textY, fs, WHITE);
    }

    // Progress + Enter hint
    const char *hint = "press ENTER";
    const int   hfs  = 18;
    const int   hw   = MeasureText(hint, hfs);
    DrawText(hint, boxX + boxW - hw - 20, boxY + boxH - hfs - 16,
             hfs, LIGHTGRAY);
}

void MonologueScene::shutdown()
{
    if (mTexLoaded)
    {
        UnloadTexture(mCharacterTex);
        mCharacterTex = { 0 };
        mTexLoaded    = false;
    }
}
