#ifndef __CSPRITE_H__
#define __CSPRITE_H__

#include <SDL/SDL.h>
#include "CSpriteBase.h"
#include "variables.h"

class CSprite
{
public:
    CSprite();
    ~CSprite() {}
    int  init(char *dir, SDL_Surface *screen);
    void draw();
    void drawMoving();
    void clearBG();
    void updateBG();

    void setFrame(int nr);
    int  getFrame();
    void setSpeed(float nr);
    float getSpeed();

    void toggleAnim();
    void startAnim();
    void stopAnim();
    void rewind();
    void xadd(int nr);
    void yadd(int nr);
    void xset(int nr);
    void yset(int nr);
    void set(int xx, int yy);
    int getX();
    int getY();

    int getw();
    int geth();

    // Manejo de coordenadas relativas
    void setCoord(int x, int y, int xoff, int yoff);
    int coordX();
    int coordY();

public:
    int mFrame;
    int mX, mY, mOldX, mOldY;
    bool mAnimating;
    int mDrawn;
    float mSpeed;
    long mLastupdate;
    CSpriteBase *mSpriteBase;
    SDL_Surface *mBackreplacement;
    SDL_Surface *mScreen;
};

#endif
