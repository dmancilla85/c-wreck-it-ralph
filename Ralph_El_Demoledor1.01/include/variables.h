#ifndef __VARIABLES_H__
#define __VARIABLES_H__

#include <array>
#include <arpa/inet.h>
#include <ctime>
#include <cmath>
#include <SDL/SDL.h>
#include <SDL/SDL_mixer.h>
#include <SDL/SDL_image.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <pthread.h>
#include <netinet/in.h>
#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <semaphore.h>
#include <fcntl.h>
#include <error.h>
#include <errno.h>
#include <list>
#include <iterator>
#include <sstream>
#include <fstream>
#include <vector>

// Constantes de pantalla
#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480
#define COLOR_PALETTE_16 16

// Constantes de localización
#define VENTANA_PISO 		5
#define PISOS        		3
#define START_POS_X 		230
#define START_POS_Y 		290
#define WINDOW_LX           20
#define WINDOW_LY           49
#define SPACE_BETWEEN_X		10
#define SPACE_BETWEEN_Y		25
#define MOVE_X		        (SPACE_BETWEEN_X + WINDOW_LX + 6)
#define MOVE_Y		        (SPACE_BETWEEN_Y + WINDOW_LY + 5)

// Constantes del juego en sí
#define VIDAS_DEFAULT		3
#define VIDAS_MAXIMO		6
#define PUNTO_X_VENTANA 	100
#define PUNTO_X_TORTA		300
#define PUNTO_X_NIVEL		1000

// Ubicación de sprites
#define SPRITE_RALPH "data/ralph"
#define SPRITE_FELIX "data/felix"
#define SPRITE_PATO "data/duck"
#define SPRITE_LADRILLO "data/brick"
#define SPRITE_TARTA "data/pie"
#define SPRITE_VENTANA "data/ventana"

#define TAMBUF 255
#define LIBRE 0
#define OCUPADO 1
#define DESCONECTADO -1
#define DIBUJAR_PROYECTIL_PMSG "dp %d %d|" //dibujarProyectil

#define IMPACTO_FELIX_PMSG  "if %d|" //impactoFelix
#define DIBUJAR_GAVIOTA_PMSG "dg %d %d|" //dibujarGaviota <fila> <posicion Y>
#define MOVER_FELIX_PMSG "mf %d %d %d|" //moverFelix <numFelix> <fila> <columna>
#define CAMBIAR_ESTADO_FELIX_PMSG "cf %d %d|" //cambiarEstadoFelix <numFelix> <estado>
#define DIBUJAR_TORTA_PMSG "dt %d %d|" //dibujarTorta <fila> <columna>
#define ELIMINAR_TORTA_PMSG "et %d %d|" //eliminarTorta <fila> <columna>
#define ARREGLAR_VENTANA_PMSG "av %d %d %d|" //arreglarVentana <numPlayer> <fila> <columna>
#define MOVER_RALPH_PMSG "mr %d|" //moverRalph <columna>
#define RALPH_ATACA_PMSG "ra|"
#define START_PMSG "cv %d %s %d" //cantidadVidas | nombreOponente | nj
#define END_PARTIDA_PMSG "ep %d" //endPartida
#define END_CLIENT_RMSG "END" //Finalizacion del cliente
#define MOVER_FELIX_RMSG "mf" //Comando que manda el cliente para mover a felix
#define SUBIR_NIVEL_PMSG "sn %d|" //Comando que manda el cliente para mover a felix
#define ARREGLAR_VENTANA_RMSG "av" //Comando que manda el cliente para arreglar una ventana.

#define FELIX_HEIGHT 1
#define FELIX_WIDTH 1
#define FELIX_FILA_INICIAL 0
#define FELIX1_COLUMNA_INICIAL 0
#define FELIX2_COLUMNA_INICIAL VENTANA_PISO-1
#define PROYECTIL_WIDTH 1
#define PROYECTIL_HEIGHT 1
#define GAVIOTA_WIDTH 60
#define GAVIOTA_HEIGHT 44
#define MOVER_IZQUIERDA 'L'
#define MOVER_DERECHA 'R'
#define MOVER_ARRIBA 'U'
#define MOVER_ABAJO 'D'

#define ESTADO_INVULNERABLE 1
#define ESTADO_VULNERABLE 0
#define TIEMPO_MAXIMO_INVULNERABILIDAD 5
#define RALPH_POSICION_INICIAL VENTANA_PISO/2

#define COEF_APARICION_GAVIOTA 10
#define COEF_APARICION_TORTA 1
#define COEF_VELOCIDAD_TIRO_RALPH 1
#define COEF_MOVIMIENTO_GAVIOTA 0.5
#define COEF_FRECUENCIA_TIRO_RALPH 5


enum dificultad
{
    FACIL,
    MEDIO,
    DIFICIL
};

#define COEF_FACIL 1.5
#define COEF_NORMAL 1
#define COEF_DIFICIL 0.5

#define WIN_NORMAL   0
#define WIN_NORMAL1  1
#define WIN_BROKEN   2
#define WIN_BROKEN2  3
#define WIN_ESPECIAL 4

// Estados del juego
#define INTRO 				0 // Pantalla de intro.
#define MENU 				1 // Pantalla de menu (no se usa)
#define LOADING_LEVEL 	2 // Cargando nivel
#define IN_GAME 			3 // Jugando
#define GAME_OVER 		4 // Derrota
#define VICTORIA 			5 // Victoria
#define EMPATE 			6
#define WAITING			7 // Esperando contrincante
#define FINAL 			8
#define CRASH			9
#define CRASH_GAME 		10

#define FRAMES_PER_SECOND       10

#define WHITE SDL_Color {255,255,255}


#endif
