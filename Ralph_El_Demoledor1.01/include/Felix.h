#ifndef __FELIX_H__
#define __FELIX_H__

#include "Edificio.h"

enum felix_state
{
    FLX_QUIETO,
    FLX_SALTANDO,
    FLX_REPARANDO,
    FLX_MUERTO,
    FLX_POWER,
    FLX_HERIDO
};

class Felix
{
public:
    Felix();
    virtual ~Felix() {}
    int init(SDL_Surface*, int);
    void draw();

    // Definir posicion relativa en matriz
    void set(int x, int y);
    int  getX();
    int  getY();
    // Definir desplazamiento
    void setOffset(int x, int y);

    // Manejo de vidas
    void pierdeVida();
    void setVida(int vi);
    void ganaVida();
    int getVida();

    // Reset vidas (y define en estado QUIETO)
    void resetVida();
    // Indica que ha muerto
    void muere();

    // Manejo de puntajes
    void resetPuntaje();
    void anotarPuntaje(int puntos);
    int getPuntaje();
    // Saber si impacto contra el objetivo
    bool isFixing();
    void setFixing(bool sh);
    // Saber si impacto contra el objetivo
    bool isJumping();
    void setJumping(bool sh);
    // Saber si impacto contra el objetivo
    bool isDead();
    void setDead(bool sh);
    // Indica que tiene el poder especial
    bool gotPower();
    void setPower(bool pow);

    void setSonido(bool sonidoState);

protected:
    CSprite sprite;
    long    lastMove;
    long 	startPower;
    int 	vidas;
    int xOffset, yOffset;
    bool fixing;
    bool sonido;
    bool jumping;
    bool power;
    bool dead;
    bool herido;
    int puntaje;
    // Sonidos de accion
    Mix_Chunk *jump;
    Mix_Chunk *fix;
    Mix_Chunk *death;
    Mix_Chunk *hurt;
    Mix_Chunk *powOn;
    Mix_Chunk *powOff;

private:
};

#endif
