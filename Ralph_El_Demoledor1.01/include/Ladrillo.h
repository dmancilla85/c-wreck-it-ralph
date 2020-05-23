#ifndef __LADRILLO_H__
#define __LADRILLO_H__

#include "CSprite.h"

class Ladrillo
{
public:
    Ladrillo();
    virtual ~Ladrillo() { }

    // Cargar imagen en Surface
    int init(SDL_Surface*, int, int);

    // Definir posicion relativa en matriz
    void set(int x, int y);
    // Definir posicion relativa en matriz
    void setPos(int x, int y);

    // Definir desplazamiento
    void setOffset(int x, int y);
    // Dibujar en pantalla
    void draw();

    // Definir dificultad de juego
    void setDificultad(dificultad dif);
    // Definir coeficiente de dificultad
    void setCoefDif(float coef);
    void addCoefDif(float coef);
    float getCoefDif();
    // Obtener coordenadas relativas
    int  getX();
    int  getY();
    // Saber si impacto contra el objetivo
    bool isShocked();
    // Indicar que impacto contra el objetivo
    void setShock(bool sh);
protected:
    CSprite sprite;
    long    lastMove;           // Timer
    int     xOffset, yOffset;   // Desplazamiento relativo
    float   coefDif;            // Coeficiente para dificultad
    bool    shock;              // Impactado
private:
};

#endif
