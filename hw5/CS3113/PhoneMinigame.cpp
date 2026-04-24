#include "PhoneMinigame.h"

PhoneMinigame::PhoneMinigame(int screenWidth, int screenHeight)
    : mScreenWidth(screenWidth), mScreenHeight(screenHeight) {}

void PhoneMinigame::load()
{
    mPhoneTex        = LoadTexture("assets/game/phone.png");
    mCatTex          = LoadTexture("assets/game/cat.png");
    mDoomTex         = LoadTexture("assets/game/doom.png");
    mGlassesTex      = LoadTexture("assets/game/Glasses.png");
    mHighCortisolTex = LoadTexture("assets/game/high_cortisol.png");
    mLowCortisolTex  = LoadTexture("assets/game/low_cortisol.png");
}

void PhoneMinigame::unload()
{
    UnloadTexture(mPhoneTex);
    UnloadTexture(mCatTex);
    UnloadTexture(mDoomTex);
    UnloadTexture(mGlassesTex);
    UnloadTexture(mHighCortisolTex);
    UnloadTexture(mLowCortisolTex);
}

void PhoneMinigame::open()
{
    if (mUsesLeft <= 0) return;

    mUsesLeft--;
    mActive          = true;
    mResult          = PHONE_NONE;
    mBarY            = BAR_TOP;
    mBarDir          = 1.0f;
    mMonologue       = { "I'm tired. Let's just doomscroll." };
    mMonologueIdx    = 0;
    mMonologueActive = true;
}

PhoneEvent PhoneMinigame::processInput()
{
    if (!mActive) return PHONE_EVENT_NONE;

    // Monologue takes priority: Enter advances lines, no bar input.
    if (mMonologueActive)
    {
        if (IsKeyPressed(KEY_ENTER))
        {
            mMonologueIdx++;
            if (mMonologueIdx >= (int) mMonologue.size())
                mMonologueActive = false;
        }
        return PHONE_EVENT_NONE;
    }

    if (mResult == PHONE_NONE)
    {
        if (IsKeyPressed(KEY_SPACE))
        {
            bool inZone = (mBarY >= SWEET_TOP && mBarY <= SWEET_BOT);
            if (inZone)
            {
                mResult    = PHONE_SUCCESS;
                mMonologue = { "W doomscroll sesh" };
            }
            else
            {
                mResult    = PHONE_FAIL;
                mMonologue = { "fuh I need a j*b" };
            }
            mMonologueIdx    = 0;
            mMonologueActive = true;
            return (mResult == PHONE_SUCCESS) ? PHONE_EVENT_SUCCESS
                                              : PHONE_EVENT_FAIL;
        }
    }
    else
    {
        // Result shown and monologue finished — any key closes.
        if (GetKeyPressed() != 0) mActive = false;
    }
    return PHONE_EVENT_NONE;
}

void PhoneMinigame::update(float deltaTime)
{
    if (!mActive) return;
    // Freeze the bar while a monologue is showing, or after a result.
    if (mResult != PHONE_NONE || mMonologueActive) return;

    mBarY += mBarDir * BAR_SPEED * deltaTime;
    if (mBarY >= BAR_BOTTOM)
    {
        mBarY   = BAR_BOTTOM;
        mBarDir = -1.0f;
    }
    else if (mBarY <= BAR_TOP)
    {
        mBarY   = BAR_TOP;
        mBarDir =  1.0f;
    }
}

void PhoneMinigame::render() const
{
    if (!mActive) return;

    // Black backdrop over the world.
    DrawRectangle(0, 0, mScreenWidth, mScreenHeight, Color{ 0, 0, 0, 245 });

    // Pick which texture fills the phone slot: the phone itself while
    // the mini-game is running, then cat / doom once a result is locked.
    const Texture2D *screenTex = &mPhoneTex;
    if      (mResult == PHONE_SUCCESS) screenTex = &mCatTex;
    else if (mResult == PHONE_FAIL)    screenTex = &mDoomTex;

    const float screenH = 540.0f;
    float       aspect  = (screenTex->height > 0)
                        ? (float) screenTex->width / (float) screenTex->height
                        : 0.5f;
    const float screenW = screenH * aspect;

    if (screenTex->id != 0)
    {
        Rectangle src = { 0, 0, (float) screenTex->width, (float) screenTex->height };
        Rectangle dst = { mScreenWidth / 2.0f, mScreenHeight / 2.0f, screenW, screenH };
        Vector2   org = { screenW / 2.0f, screenH / 2.0f };
        DrawTexturePro(*screenTex, src, dst, org, 0.0f, WHITE);

        const Texture2D *sideTex = nullptr;
        if      (mResult == PHONE_SUCCESS) sideTex = &mLowCortisolTex;
        else if (mResult == PHONE_FAIL)    sideTex = &mHighCortisolTex;

        if (sideTex != nullptr && sideTex->id != 0)
        {
            float phoneLeftEdge = dst.x - org.x;
            const float sideH   = 250.0f;
            float sideAspect    = (float) sideTex->width / (float) sideTex->height;
            float sideW         = sideH * sideAspect;
            float padding       = -18.0f;

            Rectangle sideSrc = { 0, 0, (float) sideTex->width, (float) sideTex->height };
            Rectangle sideDst = { phoneLeftEdge - padding - sideW / 2.0f,
                                  mScreenHeight / 2.0f, sideW, sideH };
            Vector2   sideOrg = { sideW / 2.0f, sideH / 2.0f };

            DrawTexturePro(*sideTex, sideSrc, sideDst, sideOrg, 0.0f, WHITE);
        }
    }

    // Vertical progress track — only while the mini-game is active
    // (not during intro/result monologues).
    if (mResult == PHONE_NONE && !mMonologueActive)
    {
        const float trackX = mScreenWidth / 2.0f;
        const float trackW = 18.0f;
        DrawRectangle((int)(trackX - trackW / 2), (int) BAR_TOP,
                      (int) trackW, (int)(BAR_BOTTOM - BAR_TOP),
                      Color{ 20, 20, 20, 235 });
        DrawRectangle((int)(trackX - trackW / 2), (int) SWEET_TOP,
                      (int) trackW, (int)(SWEET_BOT - SWEET_TOP),
                      Color{ 40, 200, 90, 245 });

        const float indicatorW = trackW * 2.0f;
        DrawRectangle((int)(trackX - indicatorW / 2), (int) mBarY - 3,
                      (int) indicatorW, 6, WHITE);
    }

    // Bottom hint line — varies by phase.
    const char *hint = nullptr;
    if      (mMonologueActive)      hint = "press ENTER";
    else if (mResult == PHONE_NONE) hint = "SPACE to lock";
    else                            hint = "press any button to resume";
    int hfs = 18;
    int hw  = MeasureText(hint, hfs);
    DrawText(hint, (mScreenWidth - hw) / 2, mScreenHeight - 34,
             hfs, Color{ 220, 220, 220, 230 });

    // Monologue dialog overlay — sits on top of the phone sprite.
    if (mMonologueActive &&
        mMonologueIdx >= 0 &&
        mMonologueIdx < (int) mMonologue.size())
    {
        const int boxX = 40, boxY = 400, boxW = 920, boxH = 150;
        DrawRectangle(boxX, boxY, boxW, boxH, Color{ 18, 18, 28, 245 });
        DrawRectangleLines(boxX, boxY, boxW, boxH, Color{ 210, 210, 230, 255 });

        const int portraitSize = 120;
        const int portraitX    = boxX + 15;
        const int portraitY    = boxY + (boxH - portraitSize) / 2;
        if (mGlassesTex.id != 0)
        {
            Rectangle src = { 0, 0,
                              (float) mGlassesTex.width,
                              (float) mGlassesTex.height };
            Rectangle dst = { (float) portraitX, (float) portraitY,
                              (float) portraitSize, (float) portraitSize };
            DrawTexturePro(mGlassesTex, src, dst, { 0, 0 }, 0.0f, WHITE);
        }

        const char *line  = mMonologue[mMonologueIdx].c_str();
        const int   fs    = 22;
        const int   textX = portraitX + portraitSize + 20;
        const int   textY = boxY + (boxH - fs) / 2;
        DrawText(line, textX, textY, fs, WHITE);
    }
}
