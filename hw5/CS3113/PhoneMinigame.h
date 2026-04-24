#ifndef PHONE_MINIGAME_H
#define PHONE_MINIGAME_H

#include "cs3113.h"

enum PhoneResult { PHONE_NONE, PHONE_SUCCESS, PHONE_FAIL };
enum PhoneEvent  { PHONE_EVENT_NONE, PHONE_EVENT_SUCCESS, PHONE_EVENT_FAIL };

class PhoneMinigame
{
private:
    Texture2D mPhoneTex        = { 0 };
    Texture2D mCatTex          = { 0 };
    Texture2D mDoomTex         = { 0 };
    Texture2D mGlassesTex      = { 0 };
    Texture2D mHighCortisolTex = { 0 };
    Texture2D mLowCortisolTex  = { 0 };

    bool        mActive   = false;
    PhoneResult mResult   = PHONE_NONE;
    float       mBarY     = 0.0f;
    float       mBarDir   = 1.0f;
    int         mUsesLeft = 2;

    std::vector<std::string> mMonologue;
    int  mMonologueIdx    = 0;
    bool mMonologueActive = false;

    int mScreenWidth;
    int mScreenHeight;

    static constexpr float BAR_TOP    = 230.0f;
    static constexpr float BAR_BOTTOM = 390.0f;
    static constexpr float BAR_SPEED  = 260.0f;
    static constexpr float SWEET_TOP  = 295.0f;
    static constexpr float SWEET_BOT  = 315.0f;

public:
    PhoneMinigame(int screenWidth, int screenHeight);

    void load();
    void unload();

    bool isActive()   const { return mActive;       }
    int  usesLeft()   const { return mUsesLeft;     }
    bool canOpen()    const { return mUsesLeft > 0; }
    void resetUses(int n = 2) { mUsesLeft = n;      }

    Texture2D phoneTexture() const { return mPhoneTex; }

    void open();
    PhoneEvent processInput();
    void update(float deltaTime);
    void render() const;
};

#endif
