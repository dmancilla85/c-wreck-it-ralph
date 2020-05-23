#ifndef __CSPRITEBASE_H__
#define __CSPRITEBASE_H__

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>

// Sprite frame básico
struct CSpriteFrame
{
    SDL_Surface *image;
    int pause;
};

// Carga las imagenes indicadas por el archivo 'info'
// y genera un arreglo.
class CSpriteBase
{
public:
    CSpriteBase();
    int init(char *dir);

    CSpriteFrame *mAnim;
    int mBuilt, mNumframes, mW, mH;
};

#endif
