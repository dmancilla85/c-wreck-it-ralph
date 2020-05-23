#ifndef __PATO_H__
#define __PATO_H__

#include "CSprite.h"
#include "variables.h"



class Pato
{
public:
    Pato();
    virtual ~Pato() {}
    int init(SDL_Surface*);
    void draw();

    // Definir posicion relativa en matriz
    void set(int x, int y);
    void setPos(int x, int y);
    // Definir desplazamiento
    void setOffset(int x, int y);
    // Definir altura (piso del edificio)
    void setAltura(int y);
    // Definir dificultad de juego
    void setDificultad(dificultad dif);
    // Definir coeficiente de dificultad
    void setCoefDif(float coef);
    void addCoefDif(float coef);
    float getCoefDif();
    int  getX();
    int  getY();
    // Saber si impacto contra el objetivo
    bool isView();
    // Indicar que impacto contra el objetivo
    void setView(bool sh);
    // Saber si comenzo a moverse
    bool isStarted();
    // Indicar que se esta moviendo
    void setStart(bool st);

protected:
    CSprite sprite;
    long    lastMove;
    int     xOffset, yOffset;
    float   coefDif;
    bool    start;
    bool    shock;
private:
};

#endif
