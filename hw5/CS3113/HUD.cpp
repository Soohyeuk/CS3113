#include "HUD.h"

HUD::HUD(int screenWidth, int screenHeight, int maxCortisol)
    : mScreenWidth(screenWidth), mScreenHeight(screenHeight),
      mMaxCortisol(maxCortisol) {}

void HUD::render(const RunState &run, const PhoneMinigame &phone) const
{
    renderCortisolBar(run.cortisol);
    if (run.emergency) renderEmergencyOverlay(run.emergencyTimeLeft);
    renderPhoneBadge(phone);
}

void HUD::renderCortisolBar(int cortisol) const
{
    const int barX = 20, barY = 50, barW = 240, barH = 22;
    DrawText("Cortisol", barX, 20, 24, WHITE);
    DrawRectangle(barX, barY, barW, barH, Color{ 40, 40, 40, 200 });
    int fillW = (int)(barW * (cortisol / (float) mMaxCortisol));
    Color fillColour = (cortisol >= 70) ? RED
                     : (cortisol >= 40) ? ORANGE
                                        : GREEN;
    DrawRectangle(barX, barY, fillW, barH, fillColour);
    DrawRectangleLines(barX, barY, barW, barH, WHITE);
    DrawText(TextFormat("%d / %d", cortisol, mMaxCortisol),
             barX + barW + 12, barY, 22, WHITE);
}

void HUD::renderEmergencyOverlay(float timeLeft) const
{
    int secs = (int) ceilf(timeLeft);
    if (secs < 0) secs = 0;
    const char *txt = TextFormat("0:%02d", secs);
    int fontSize = 120;
    int tw = MeasureText(txt, fontSize);
    int tx = (mScreenWidth  - tw) / 2;
    int ty = (mScreenHeight - fontSize) / 2;

    float pulse = 0.5f + 0.5f * sinf((float) GetTime() * 10.0f);
    unsigned char alpha = (unsigned char)(160 + 95 * pulse);
    Color red = { 255, 30, 30, alpha };

    DrawText(txt, tx + 3, ty + 3, fontSize, Color{ 0, 0, 0, 200 });
    DrawText(txt, tx, ty, fontSize, red);

    const char *warn = "REACH THE EXIT!";
    int wfs = 28;
    int ww  = MeasureText(warn, wfs);
    DrawText(warn, (mScreenWidth - ww) / 2, ty + fontSize + 10,
             wfs, Color{ 255, 255, 255, alpha });
}

void HUD::renderPhoneBadge(const PhoneMinigame &phone) const
{
    const int hudCX = mScreenWidth  - 55;
    const int hudCY = mScreenHeight - 55;
    const int hudR  = 34;
    DrawCircle(hudCX, hudCY, hudR + 2, Color{  0,   0,   0, 180 });
    DrawCircle(hudCX, hudCY, hudR,     Color{ 40, 180,  80, 230 });

    Texture2D phoneTex = phone.phoneTexture();
    if (phoneTex.id != 0)
    {
        const float iconH  = hudR * 1.5f;
        float       aspect = (float) phoneTex.width / (float) phoneTex.height;
        float       iconW  = iconH * aspect;
        Rectangle src = { 0, 0, (float) phoneTex.width, (float) phoneTex.height };
        Rectangle dst = { (float) hudCX, (float) hudCY, iconW, iconH };
        Vector2   org = { iconW / 2.0f, iconH / 2.0f };
        DrawTexturePro(phoneTex, src, dst, org, 0.0f, WHITE);
    }

    // P badge tucked at the bottom-right of the phone icon
    int badgeX = hudCX + hudR - 6;
    int badgeY = hudCY + hudR - 6;
    DrawCircle(badgeX, badgeY, 11, Color{ 0, 0, 0, 220 });
    int pW = MeasureText("P", 16);
    DrawText("P", badgeX - pW / 2, badgeY - 8, 16, WHITE);

    // Uses left indicator
    const char *usesText = TextFormat("%d left", phone.usesLeft());
    int uW = MeasureText(usesText, 16);
    DrawText(usesText, hudCX - uW / 2, hudCY + hudR + 4, 16, WHITE);
}
