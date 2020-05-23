#ifndef __RALPH_H__
#define __RALPH_H__

#include "Edificio.h"

class Ralph
{
public:
    Ralph();
    virtual ~Ralph() {}
    int init(SDL_Surface*);
    // Dibujar en edificio
    void draw(Edificio &edificio);
    // Definir posicion relativa en matriz
    void set(int x, int y);
    // Definir desplazamiento
    void setOffset(int x, int y);
    // Definir altura (piso del edificio)
    void setAltura(int y);
    // Definir dificultad de juego
   // void setDificultad(dificultad dif);
    // Definir coeficiente de dificultad
    /*void setCoefDif(float coef);
    void addCoefDif(float coef);
    float getCoefDif();
    int  getX();
    int  getY();*/
    // Esta atacando (si/no)
    void setAttacking(bool at);
    bool isAttacking();
protected:
    CSprite sprite;
    int     xOffset;
    int     yOffset;
    long    lastMove;
    bool    attacking;  // Variable que indica si esta atacando
    //float   coefDif;    // Coef. para dificultad
    Mix_Chunk *attack;  // Sonido para ataque
private:
};

#endif
