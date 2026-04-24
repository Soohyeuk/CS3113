#ifndef CS3113_H
#define CS3113_H
#define LOG(argument) std::cout << argument << '\n'

#include "raylib.h"
#include "rlgl.h"
#include "raymath.h"
#include <math.h>
#include <time.h>
#include <stdio.h>
#include <map>
#include <vector>
#include <string>
#include <iostream>

enum AppStatus   { TERMINATED, RUNNING };
enum TextureType { SINGLE, ATLAS       };

// Shared game audio: loaded once in main and referenced by all levels.
extern Music gBgm;
extern Sound gJumpSound;
extern Sound gStepSound;
extern Sound gStompSound;

Color ColorFromHex(const char *hex);
void Normalise(Vector2 *vector);
float GetLength(const Vector2 vector);
Rectangle getUVRectangle(const Texture2D *texture, int index, int rows, int cols);
void panCamera(Camera2D *camera, const Vector2 *targetPosition);


#endif // CS3113_H