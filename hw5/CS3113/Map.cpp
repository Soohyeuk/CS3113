#include "Map.h"

Map::Map(int mapColumns, int mapRows, unsigned int *levelData,
         const char *textureFilePath, float tileSize, int textureColumns,
         int textureRows, Vector2 origin,
         std::unordered_set<unsigned int> solidTiles,
         unsigned int floorTile) :
         mMapColumns {mapColumns}, mMapRows {mapRows},
         mTextureAtlas { LoadTexture(textureFilePath) },
         mLevelData {levelData }, mTileSize {tileSize},
         mTextureColumns {textureColumns}, mTextureRows {textureRows},
         mOrigin {origin}, mSolidTiles { std::move(solidTiles) },
         mFloorTile { floorTile }
{
    // Pixel-art atlases: disable bilinear filtering so adjacent atlas tiles
    // don't bleed into each other at the seams.
    SetTextureFilter(mTextureAtlas, TEXTURE_FILTER_POINT);
    build();
}

Map::~Map() { UnloadTexture(mTextureAtlas); }

void Map::build()
{
    // Calculate map boundaries in world coordinates
    mLeftBoundary   = mOrigin.x - (mMapColumns * mTileSize) / 2.0f;
    mRightBoundary  = mOrigin.x + (mMapColumns * mTileSize) / 2.0f;
    mTopBoundary    = mOrigin.y - (mMapRows * mTileSize) / 2.0f;
    mBottomBoundary = mOrigin.y + (mMapRows * mTileSize) / 2.0f;

    // Precompute texture areas for each tile. Use integer tile dimensions so
    // source rects don't overlap adjacent atlas rows/columns (which would
    // leak neighbour pixels at the seams — e.g. 511/31 = 16.48 as a float).
    const int tileW = mTextureAtlas.width  / mTextureColumns;
    const int tileH = mTextureAtlas.height / mTextureRows;
    for (int row = 0; row < mTextureRows; row++)
    {
        for (int col = 0; col < mTextureColumns; col++)
        {
            Rectangle textureArea = {
                (float)(col * tileW),
                (float)(row * tileH),
                (float) tileW,
                (float) tileH
            };

            mTextureAreas.push_back(textureArea);
        }
    }
}

// Tile value in the level data is used directly as a 1-based index into the
// texture atlas. 0 means "empty" (nothing drawn). Solidity is determined by
// the per-map `mSolidTiles` set supplied at construction. When `mFloorTile`
// is non-zero it is drawn beneath every cell so transparent decor tiles
// show the floor instead of the scene's clear colour.
void Map::render()
{
    const bool hasFloor = mFloorTile > 0 && mFloorTile <= mTextureAreas.size();

    for (int row = 0; row < mMapRows; row++)
    {
        for (int col = 0; col < mMapColumns; col++)
        {
            Rectangle destinationArea = {
                mLeftBoundary + col * mTileSize,
                mTopBoundary  + row * mTileSize,
                mTileSize,
                mTileSize
            };

            if (hasFloor)
            {
                DrawTexturePro(
                    mTextureAtlas,
                    mTextureAreas[mFloorTile - 1],
                    destinationArea,
                    {0.0f, 0.0f}, 0.0f, WHITE
                );
            }

            unsigned int tile = mLevelData[row * mMapColumns + col];
            if (tile == 0 || tile > mTextureAreas.size()) continue;
            if (tile == mFloorTile) continue; // already drawn as base floor

            DrawTexturePro(
                mTextureAtlas,
                mTextureAreas[tile - 1],
                destinationArea,
                {0.0f, 0.0f}, 0.0f, WHITE
            );
        }
    }
}

bool Map::isSolidTileAt(Vector2 position, float *xOverlap, float *yOverlap)
{
    *xOverlap = 0.0f;
    *yOverlap = 0.0f;

    if (position.x < mLeftBoundary || position.x > mRightBoundary ||
        position.y < mTopBoundary  || position.y > mBottomBoundary)
        return false;

    int tileXIndex = floor((position.x - mLeftBoundary) / mTileSize);
    int tileYIndex = floor((position.y - mTopBoundary)  / mTileSize);

    if (tileXIndex < 0 || tileXIndex >= mMapColumns ||
        tileYIndex < 0 || tileYIndex >= mMapRows)
        return false;

    unsigned int tile = mLevelData[tileYIndex * mMapColumns + tileXIndex];
    if (mSolidTiles.find(tile) == mSolidTiles.end()) return false;

    float tileCentreX = mLeftBoundary + tileXIndex * mTileSize + mTileSize / 2.0f;
    float tileCentreY = mTopBoundary  + tileYIndex * mTileSize + mTileSize / 2.0f;

    *xOverlap = fmaxf(0.0f, (mTileSize / 2.0f) - fabs(position.x - tileCentreX));
    *yOverlap = fmaxf(0.0f, (mTileSize / 2.0f) - fabs(position.y - tileCentreY));

    return true;
}

unsigned int Map::getTileAt(Vector2 position) const
{
    if (position.x < mLeftBoundary || position.x > mRightBoundary ||
        position.y < mTopBoundary  || position.y > mBottomBoundary)
        return 0;

    int tileXIndex = (int) floor((position.x - mLeftBoundary) / mTileSize);
    int tileYIndex = (int) floor((position.y - mTopBoundary)  / mTileSize);

    if (tileXIndex < 0 || tileXIndex >= mMapColumns ||
        tileYIndex < 0 || tileYIndex >= mMapRows)
        return 0;

    return mLevelData[tileYIndex * mMapColumns + tileXIndex];
}