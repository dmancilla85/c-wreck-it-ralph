#include "include/variables.h"

using namespace std;


typedef struct
{
    //Estructura de clientes
    int orden;
    int socket;
    char nombre[255];
    int estado;
    int puntaje;
    int cantVidas;
    int cantPartidasGanadas;
}  Jugador;



typedef struct
{
    //Estructura de partida.
    Jugador jugador1;
    Jugador jugador2;
    int tiempoInmunidad;
    int ganador;
    int puntaje1;
    int puntaje2;
    pid_t pid;
}  Partida;

typedef struct
{
    int x;
    int y;
} PosicionFisica;

typedef struct
{
    int fila;
    int columna;
} Posicion;

typedef struct
{
    float width;
    float height;
} Size;

typedef struct
{
    int estado; //Invulnerable, u otros estados a definir
    time_t tInicioVulnerabilidad;
    Posicion posicion; //Si es felix, ventana en la que está parado, si es una ventana, su posición.
    PosicionFisica posicionFisica; //Si es felix, ventana en la que está parado, si es una ventana, su posición.
    Size size; // Tamaño que me sirve para calcular las colisiones. RECORDAR que
    // el tiro de Ralph va en línea recta usando la X de la columna de ventanas donde está parado.
} Felix, Ralph, Ventana, Gaviota, Torta, Proyectil, Objeto;


Partida * partidaMC;
time_t *tiempoMC;

Partida partidaActual;
char semEscrituraName[30]="/semEscritura";
char semLecturaName[30]="/semLectura";
char semStartName[30]="/semStart";
char semLiveName[30]="/semLive";
char shmName[30]="/shMemory";
char shmNameLive[30]="/shMemoryLive";

sem_t * semEscritura, * semLectura, * semStart, *semLive;

pthread_mutex_t mutexProyectiles, mutexGaviotas, mutexTorta;
pthread_mutex_t semSyncMessage;

char syncMessage[1000];

pthread_t 	player1Thread_tid,
player2Thread_tid,
randomShoot_tid,
randomTorta_tid,
randomGaviota_tid,
sync_tid,estoyVivo_tid;
bool    	player1TSet = false,
        	player2TSet=false,
			randomShootTSet=false,
			randomTortaTSet=false,
			randomGaviotaTSet=false,
			syncTSet=false,
			estoyVivoTSet=false;
vector<Ventana> ventanasList;

int mapaDeJuego[PISOS][VENTANA_PISO]= {{1,2,4,2,0},{1,3,1,2,0},{1,0,3,0,1}};

vector<Torta> tortasList;
vector<Proyectil> proyectilesList;
vector<Gaviota> gaviotasList;
Felix felix1;
Felix felix2;
Ralph ralph;
int numeroNivel=1;
float fVelocidadTiroRalph,
fFrecuenciaTiroRalph,
fVelocidadMovimientoRalph,
fVelocidadMovimientoGaviota,
fFrecuenciaAparicionGaviota,
fFrecuenciaAparicionTorta;
bool 	apareceGaviota=false;
int 	unidadesMovimientoProyectil = 15;
int 	unidadesMovimientoGaviota = 15;
int 	cantidadFilasNivel= PISOS;

/*AGREGADO LUIS*/
int jugador1 = 1, jugador2 = 2, jugador1Avisado=0, jugador2Avisado=0;
/*AGREGADO LUIS*/


//Funciones
void inicializarMemoriaCompartida();
void inicializarPosiciones();
void leerEstadoInicial();
void sincronizarClientes();
void inicializarNivel(int);
void juego();
void * playerListener(void * );
void *sync(void *);
void moverProyectiles();
void moverGaviotas();
void controlarInvulnerabilidad();
int fueraDePantalla(PosicionFisica);
void *randomShoot(void*);
void *randomTorta(void*);
void lanzarProyectil();
void moverRalph();
void *randomGaviota(void*);
void spawnearGaviota();
void spawnearTorta();
void moverFelix(int, char);
void impactoFelix(int);
void controlarVentanas();
void controlarBuff(int);
void controlarGanador();
void cambiarEstadoFelix(int , int );
void eliminarTorta(int);
void arreglarVentana(int , Posicion );
void broadcastMessage(char *);
bool huboImpactoProyectil(Proyectil , Felix);
bool huboImpactoGaviota(Gaviota , Felix);
void concatenarSyncMessage(char *);
PosicionFisica devolverPosicionFisica(Posicion);
void inicializarVentanas();
void subirNivel();
void exitForzado();
static void finalizarPartida(int);
void finalizarTorneo();
vector<string> split(string , char);
vector<string> split(string , char , vector<string> );
void *vivo(void *);

int main(int argc, char *argv[])
{
    //Manejamos las señales de terminación
    signal(SIGINT, finalizarPartida);
    signal(SIGTERM, finalizarPartida);
    signal(SIGUSR1, finalizarPartida);
    atexit(exitForzado);

    //Inicializamos el servidor.
    inicializarMemoriaCompartida();
    printf("Inicialice memoria compartida\n");
    fflush(stdout);
    pthread_create(&estoyVivo_tid, NULL, vivo, NULL);
    leerEstadoInicial();

    printf("Lei estado inicial\n");
    fflush(stdout);
    sincronizarClientes();
    printf("Sincronice clientes\n");
    fflush(stdout);

    inicializarNivel(1);
    printf("Inicialice nivel\n");
    fflush(stdout);
    printf("Llamando a juego()\n");
    fflush(stdout);

    juego();

    return EXIT_SUCCESS;
}

void *vivo(void *p)
{
    estoyVivoTSet = true;
    int valorSemaforo;
    time_t tiempoLeido;
    while (1)
    {
        sem_getvalue(semLive, &valorSemaforo);
        if (valorSemaforo >0)
        {
            tiempoLeido = *tiempoMC;
            if (difftime( time(NULL),tiempoLeido) > 5)
            {
                printf("El torneo se cerro abruptamente ;)\n");
                finalizarTorneo();
            }
        }
        sleep(3);
    }

}

void inicializarMemoriaCompartida()
{
    // Inicializo la memoria compartida y los semáforos correspondientes, al igual
    // que en el server del torneo pero con el pid del padre, para que se llamen igual.
    fflush(stdout);
    int pidInt = getppid(), fd, fdLive ;
    char pid[6];
    sprintf(pid, "%d", pidInt);
    strcat(shmName, pid);
    strcat(shmNameLive, pid);

    fflush(stdout);
    if ( ( fd = shm_open(shmName, O_RDWR, 0777 ) ) < 0 )
    {
        printf("Error abriendo la memoria compartida.\n");
        fflush(stdout);
        finalizarPartida(0);
    }

    if (ftruncate(fd, sizeof(Partida)) == -1)
    {
        printf("Error estableciendo el tamaño.\n");
        fflush(stdout);
        finalizarPartida(0);
    }
    partidaMC = (Partida *) mmap(0, sizeof(Partida), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    if (partidaMC == MAP_FAILED)
    {
        printf("Error mapeando a memoria.\n");
        fflush(stdout);
        finalizarPartida(0);
    }
    //memoria para leer el log del tiempo del servidor
    if ( ( fdLive = shm_open(shmNameLive , O_RDWR, 0777 ) ) < 0 )
    {
        printf("Error abriendo la memoria compartida.\n");
        fflush(stdout);
        finalizarPartida(0);
    }

    if (ftruncate(fdLive, sizeof(time_t)) == -1)
    {
        printf("Error estableciendo el tamaño.\n");
        fflush(stdout);
        finalizarPartida(0);
    }
    tiempoMC = (time_t *) mmap(0, sizeof(time_t), PROT_READ | PROT_WRITE, MAP_SHARED, fdLive, 0);

    if (tiempoMC == MAP_FAILED)
    {
        printf("Error mapeando a memoria.\n");
        fflush(stdout);
        finalizarPartida(0);
    }
    // Se utilizan los mismos nombres del servidor para los semáforos
    strcat(semEscrituraName, pid);
    strcat(semLecturaName, pid);
    strcat(semLiveName, pid);
    semEscritura = sem_open( semEscrituraName, O_CREAT, 0777);
    semLectura = sem_open( semLecturaName, O_CREAT, 0777);
    semLive = sem_open(semLiveName, O_CREAT, 0777);
    sprintf(pid, "%d", getpid());
    strcat(semStartName, pid);

    //Semáforo adicional utilizado para sincronizar el inicio de la partida cuando ambos estén listos.
    semStart = sem_open( semStartName , O_CREAT | O_EXCL , 0777 , 0 );
    pthread_mutex_init(&semSyncMessage, NULL);
    pthread_mutex_init(&mutexProyectiles, NULL);
    pthread_mutex_init(&mutexGaviotas, NULL);
    pthread_mutex_init(&mutexTorta, NULL);

}

void inicializarPosiciones()
{

    felix1.size.width= FELIX_WIDTH;
    felix1.size.height = FELIX_HEIGHT;
    felix2.size.width= FELIX_WIDTH;
    felix2.size.height = FELIX_HEIGHT;
    felix1.posicion.columna = 0;
    felix1.posicion.fila = 0;
    if(felix1.estado == ESTADO_INVULNERABLE)felix1.estado= ESTADO_VULNERABLE ;
    if(felix2.estado == ESTADO_INVULNERABLE)felix2.estado= ESTADO_VULNERABLE ;
    felix2.posicion.columna = VENTANA_PISO-1;
    felix2.posicion.fila = 0;
    felix1.posicionFisica = devolverPosicionFisica(felix1.posicion);
    felix2.posicionFisica = devolverPosicionFisica(felix2.posicion);
    ralph.posicion.columna=RALPH_POSICION_INICIAL;
    ralph.posicion.fila=-1; //Está arriba de las matriz del juego
    // david
    gaviotasList.clear();
    proyectilesList.clear();

}

void leerEstadoInicial()
{
    //Leo los datos que me dejó el servidor del torneo, y luego hago un post para permitir escribir a los demás procesos.
    partidaActual = *partidaMC;
    partidaActual.pid = getpid();
    sem_post(semEscritura);
}

void sincronizarClientes()
{
    char mensaje[20];

    //Envío la inicialización al primer jugador, y creo su listener.
    /*AGREGADO LUIS*/
    sprintf(mensaje, START_PMSG, partidaActual.jugador1.cantVidas, partidaActual.jugador2.nombre, 1);
    if((send(partidaActual.jugador1.socket, mensaje, TAMBUF, 0) ) < 0)
    {
        printf("Fallo en el send al jugador 1 de sincronizarClientes\n");
        fflush(stdout);
        partidaActual.jugador1.estado=DESCONECTADO;
    }
    //Envío la inicialización al segundo jugador, y creo su listener.
    sprintf(mensaje, START_PMSG, partidaActual.jugador2.cantVidas, partidaActual.jugador1.nombre, 2);
    if( ( send(partidaActual.jugador2.socket, mensaje, TAMBUF, 0) ) < 0 )
    {
        printf("Fallo en el send al jugador 2 de sincronizarClientes\n");
        fflush(stdout);
        partidaActual.jugador2.estado=DESCONECTADO;

    }
    if(partidaActual.jugador1.estado != DESCONECTADO && partidaActual.jugador2.estado != DESCONECTADO)
    {
        if (pthread_create(&player1Thread_tid, NULL, playerListener, (void*)&(jugador1))) printf("FALLO EL CREATE DEL thread 1\n");
        player1TSet=true;

        if (pthread_create(&player2Thread_tid, NULL, playerListener, (void*) &(jugador2))) printf("FALLO EL CREATE DEL thread 1\n");
        player2TSet=true;
        //Inicializo el mensaje que se utilizará para sincronizar a lo largo del juego.
        strcpy(syncMessage,"");

    }
    else
        finalizarPartida(1);
    /*AGREGADO LUIS*/

}

void inicializarNivel(int nroNivel)
{
    inicializarVentanas();
    inicializarPosiciones();
    if (!(nroNivel == 1 || nroNivel == 4 || nroNivel == 7 || nroNivel == 10)) return;
    apareceGaviota = nroNivel>1;
    if (nroNivel > 1) nroNivel /= 2;
    fVelocidadMovimientoGaviota = nroNivel * COEF_MOVIMIENTO_GAVIOTA;

    if(nroNivel > 3)
        fVelocidadMovimientoGaviota = 2 * COEF_MOVIMIENTO_GAVIOTA;

    fVelocidadTiroRalph = 1 * COEF_VELOCIDAD_TIRO_RALPH;

	 if(nroNivel < 3)
	 {
    	fFrecuenciaAparicionGaviota = COEF_APARICION_GAVIOTA/ nroNivel;
    	fFrecuenciaTiroRalph = COEF_FRECUENCIA_TIRO_RALPH / nroNivel;
	 }
	 else
	 {
		fFrecuenciaAparicionGaviota = COEF_APARICION_GAVIOTA/ 3; //nroNivel;
    	fFrecuenciaTiroRalph = COEF_FRECUENCIA_TIRO_RALPH / 3; // nroNivel;
	 }

    fFrecuenciaAparicionTorta = COEF_APARICION_TORTA * nroNivel;


}

void inicializarVentanas()
{
    ventanasList.clear();
    for (int i=0; i<PISOS; i++)
    {
        for (int x=0; x<VENTANA_PISO; x++)
        {
            int factorColumna = (VENTANA_PISO-1) - x;
            int factorFila = (PISOS-1) - i;
            Ventana v;
            v.posicion.fila = i;
            v.posicion.columna = x;
            v.size.width = WINDOW_LX;
            v.size.height = WINDOW_LY;
            v.posicionFisica.x = START_POS_X + x * MOVE_X;
            v.posicionFisica.y = START_POS_Y - i * MOVE_Y;
            v.estado = mapaDeJuego[i][x];
            ventanasList.push_back(v);
        }
    }
}

void juego()
{
    //Hasta que ambos no me mandaron la señal de start, no inicio.
    if (sem_wait(semStart)==-1)
    {
        printf("ERROR EN EL SEMAFORO :D");
        fflush(stdout);
    }
    if (sem_wait(semStart)==-1)
    {
        printf("ERROR EN EL SEMAFORO");
        fflush(stdout);
    }

    printf("Paso ambos semaforos de start\n");
    fflush(stdout);
    //Mando el mensaje de start a ambos.
    broadcastMessage((char *)"start");

    //Comenzamos a mandar cambios
    pthread_create(&sync_tid, NULL, sync, NULL);
    syncTSet=true;

    pthread_create(&randomShoot_tid, NULL, randomShoot, NULL);
    randomShootTSet=true;

    pthread_create(&randomTorta_tid, NULL, randomTorta, NULL);
    randomTortaTSet = true;

    pthread_create(&randomGaviota_tid, NULL, randomGaviota, NULL);
    randomGaviotaTSet=true;

    while (1)
    {
        sleep(3);
    }
}

void * playerListener(void * nPlayer)
{
    char mensaje[TAMBUF];
    char * command;
    int numPlayer = *((int *) nPlayer);
    Jugador * jugador = numPlayer == 1? &partidaActual.jugador1:&partidaActual.jugador2;

    bool finWhile = true;

    //Itero hasta que me mande el ready
    while (finWhile)
    {

        if ((recv(jugador->socket, mensaje, TAMBUF, 0) ) < 1 )
        {
            printf("Fallo el recv del jugador %d ANTES DEL READY\n", numPlayer);
            fflush(stdout);
            jugador->estado=DESCONECTADO;
            /*AGREGADO LUIS*/
            sem_post(semStart);
            /*AGREGADO LUIS*/
            sleep(10000);
        }

        if (!strcmp(mensaje,"ready"))
        {
            finWhile = false;
            fflush(stdout);
            sem_post(semStart);
        }
    }

    while (1)
    {
        if ((recv(jugador->socket, mensaje, TAMBUF, 0) ) < 1 )
        {
            printf("Fallo el recv del jugador %d EN JUEGO\n", numPlayer);
            fflush(stdout);
            jugador->estado=DESCONECTADO;
            controlarGanador();
            sleep(10000);
        }

        string msj = string(mensaje);
        vector<string> splittedMessage = split(msj, ' ');
        if (!splittedMessage.size())
        {
            printf("No hay tamanio de mensaje\n");
            fflush(stdout);
            continue;
        }
        command = strdup(splittedMessage[0].c_str());
        //Protocolo.

        if (!strcmp(command, MOVER_FELIX_RMSG))
            moverFelix(numPlayer, splittedMessage[1].at(0));
        else if (!strcmp(command, ARREGLAR_VENTANA_RMSG))
        {
            Posicion pVentana;
            pVentana.fila = atoi(splittedMessage[1].c_str());
            pVentana.columna = atoi(splittedMessage[2].c_str());
            arreglarVentana(numPlayer, pVentana);
        }
    }
}

void *sync(void *p)  	// hilo de sincronismo y mensajes a los clientes
{
    while (1)
    {
        usleep(80000);
        pthread_mutex_lock(&mutexProyectiles);
        moverProyectiles();
        pthread_mutex_unlock(&mutexProyectiles);
        pthread_mutex_lock(&mutexGaviotas);
        moverGaviotas();
        pthread_mutex_unlock(&mutexGaviotas);
        controlarInvulnerabilidad();
        if (!strcmp(syncMessage, "")) continue;
        //Enviamos el mensaje con todos los cambios a ambos jugadores.
        pthread_mutex_lock(&semSyncMessage);
        broadcastMessage(syncMessage);
        /*AGREGADO LUIS*/
        if(strstr("sn",syncMessage)!=0){
          tortasList.clear();
          proyectilesList.clear();
          gaviotasList.clear();
          cambiarEstadoFelix(1, ESTADO_VULNERABLE);
          cambiarEstadoFelix(2, ESTADO_VULNERABLE);
        }
        /*AGREGADO LUIS*/
        //Limpiamos el mensaje global, y continuamos.
        strcpy(syncMessage, "");
        pthread_mutex_unlock(&semSyncMessage);
    }
}

void moverProyectiles()
{
    vector<Proyectil>::iterator it = proyectilesList.begin();
    char buffer[50];
    int hImpacto=0;
    vector<int> toErase;
    int counter=0;

    while ( it != proyectilesList.end())
    {
        it->posicionFisica.y+= unidadesMovimientoProyectil*fVelocidadTiroRalph;
        sprintf(buffer, DIBUJAR_PROYECTIL_PMSG, it->posicion.columna, it->posicionFisica.y);
        concatenarSyncMessage(buffer);

        if (hImpacto=(partidaActual.jugador1.estado == ESTADO_VULNERABLE &&
                       partidaActual.jugador1.cantVidas &&
                       huboImpactoProyectil(*it, felix1)))
            impactoFelix(1);
        if (hImpacto=(partidaActual.jugador2.estado == ESTADO_VULNERABLE &&
                           partidaActual.jugador2.cantVidas &&
                            huboImpactoProyectil(*it, felix2)))
            impactoFelix(2);

        if (hImpacto || fueraDePantalla(it->posicionFisica))
        {
            toErase.push_back(counter);
        }
        counter++;
        it++;
    }

    for (int i=0; i<toErase.size(); i++)
        proyectilesList.erase(proyectilesList.begin()+toErase[i]);
}

void moverGaviotas()
{

    vector<Gaviota>::iterator it = gaviotasList.begin();
    char buffer[50];
    vector<int> toErase;
    int counter=0;
    int hImpacto=0;

    while ( it != gaviotasList.end() )
    {
        vector<Gaviota>::iterator current = it++;
        current->posicionFisica.x += unidadesMovimientoGaviota*fVelocidadMovimientoGaviota;
        sprintf(buffer, DIBUJAR_GAVIOTA_PMSG, current->posicion.fila, current->posicionFisica.x);

        concatenarSyncMessage(buffer);

        if (hImpacto=(partidaActual.jugador1.estado == ESTADO_VULNERABLE &&
                      partidaActual.jugador1.cantVidas &&
                      huboImpactoGaviota(*current, felix1)))
            impactoFelix(1);
        if (hImpacto=(partidaActual.jugador2.estado == ESTADO_VULNERABLE &&
                           partidaActual.jugador2.cantVidas &&
                           huboImpactoGaviota(*current, felix2)))
            impactoFelix(2);

        if (hImpacto || fueraDePantalla(current->posicionFisica))
        {
            toErase.push_back(counter);
        }
        counter++;
    }
    for (int i=0; i<toErase.size(); i++)
        gaviotasList.erase(gaviotasList.begin()+toErase[i]);
}

void impactoFelix(int nroJugador)
{

    Jugador * jugador = nroJugador == 1 ?&partidaActual.jugador1:&partidaActual.jugador2;

    if ( nroJugador == 1 && (felix1.estado == ESTADO_INVULNERABLE
    										|| felix1.estado == DESCONECTADO )) return;
    if ( nroJugador == 2 && (felix2.estado == ESTADO_INVULNERABLE
    										|| felix2.estado == DESCONECTADO )) return;


    if (jugador->cantVidas <= 0) return;

    jugador->cantVidas--;
    char buffer[100];
    sprintf(buffer, IMPACTO_FELIX_PMSG, nroJugador);
    concatenarSyncMessage(buffer);
    controlarGanador();
}

void controlarGanador()
{
    /*if ((partidaActual.jugador1.estado!=DESCONECTADO && partidaActual.jugador1.cantVidas > 0) ||
        (partidaActual.jugador2.estado!=DESCONECTADO && partidaActual.jugador2.cantVidas > 0)) return;*/
   
   if ((partidaActual.jugador1.cantVidas > 0) ||
        (partidaActual.jugador2.cantVidas > 0)) return;
    //Si todavía tiene vidas, continúo, sino marco ganador y termino el servidor.

    partidaActual.ganador = partidaActual.puntaje2 > partidaActual.puntaje1?
                            2: partidaActual.puntaje1 > partidaActual.puntaje2; //Pone un 1 si ganó el 1, y sino, pone 0 si empataron, y 2 si ganó el 2.
    printf("Partida finalizada, gano: %d\n", partidaActual.ganador);
    fflush(stdout);
    finalizarPartida(0);
}

void controlarInvulnerabilidad()
{
    if (felix1.estado == ESTADO_INVULNERABLE &&
        difftime(time(NULL),felix1.tInicioVulnerabilidad) >= partidaActual.tiempoInmunidad)
        cambiarEstadoFelix(1, ESTADO_VULNERABLE);

    if (felix2.estado == ESTADO_INVULNERABLE &&
        difftime(time(NULL), felix2.tInicioVulnerabilidad) >= partidaActual.tiempoInmunidad)
        cambiarEstadoFelix(2, ESTADO_VULNERABLE);
}

int fueraDePantalla(PosicionFisica p)
{
    return (p.x < 0 || p.x>SCREEN_WIDTH || p.y<0 || p.y > 350);
}

void *randomShoot(void*)
{
    int tiempo;
    srand(time(NULL));
    while (1)
    {
        usleep(fFrecuenciaTiroRalph*1000000);
        moverRalph();
        sleep(1);
        lanzarProyectil();
    }
}

void lanzarProyectil()
{
    Proyectil p;
    p.posicionFisica.y = 0;
    p.posicionFisica.x = ralph.posicion.columna;
    //Para que no pinche en la validacion de fuera de pantalla, va a ser siempre mayor a 0.
    p.posicion.columna = ralph.posicion.columna;
    p.size.width = PROYECTIL_WIDTH;
    p.size.height = PROYECTIL_HEIGHT;
    char buffer[255];
    strcpy(buffer, RALPH_ATACA_PMSG);
    concatenarSyncMessage(buffer);
    pthread_mutex_lock(&mutexProyectiles);
    proyectilesList.push_back(p);
    pthread_mutex_unlock(&mutexProyectiles);
}

void moverRalph()
{
    int aDonde;
    do
        aDonde = rand()%(VENTANA_PISO-1);
    while (aDonde == ralph.posicion.columna);

    ralph.posicionFisica.x += MOVE_X * (aDonde-ralph.posicion.columna); //Hacer un vector de posiciones.
    ralph.posicion.columna = aDonde;

    char buffer[50];
    sprintf(buffer, MOVER_RALPH_PMSG, ralph.posicion.columna);
    concatenarSyncMessage(buffer);
}

void *randomGaviota(void*)
{
    int tiempo;
    srand(time(NULL));
    while (1)
    {
        tiempo = rand() % 5000 / fFrecuenciaAparicionGaviota;
        usleep(fFrecuenciaAparicionGaviota*1000000);

        if (!gaviotasList.size()) spawnearGaviota();
    }
}

void spawnearGaviota()
{
    Gaviota gv;
    //Me copia todos los campos, inclusive la posición x,y de la ventana, solo vamos a usar el Y para inicializar la gaviota.
    gv.posicion.fila = rand() % (PISOS);
    gv.posicionFisica.x = 0;
    gv.posicionFisica.y = gv.posicion.fila;
    gv.size.width = GAVIOTA_WIDTH;
    gv.size.height = GAVIOTA_HEIGHT;
    pthread_mutex_lock(&mutexGaviotas);
    gaviotasList.push_back(gv);
    pthread_mutex_unlock(&mutexGaviotas);

}

void *randomTorta(void*)
{
    int tiempo;
    srand(time(NULL));
    while (1)
    {
        tiempo = rand() % 5000 / fFrecuenciaAparicionTorta;
        usleep(fFrecuenciaAparicionTorta*1000000);
        if (!tortasList.size()) spawnearTorta(); //Si no hay torta
    }
}

void spawnearTorta()
{
    Torta torta;
    torta.posicion.columna = rand() % (VENTANA_PISO-1);
    torta.posicion.fila = rand() % (PISOS-1);

    if (torta.posicion.columna == 2 && torta.posicion.fila == 0)
    {
        torta.posicion.columna = 1;
        torta.posicion.fila = 0 ;

    }

    char buffer[100];
    sprintf(buffer, DIBUJAR_TORTA_PMSG, torta.posicion.fila, torta.posicion.columna);
    concatenarSyncMessage(buffer);
    pthread_mutex_lock(&mutexTorta);
    tortasList.push_back(torta);
    pthread_mutex_unlock(&mutexTorta);
}

void moverFelix(int numPlayer, char direccion)
{
    Felix * felixActual;
    felixActual = (numPlayer == 1)? &felix1:&felix2;

    switch (direccion)
    {

    case MOVER_IZQUIERDA: //Izquierda
        if (felixActual->posicion.columna > 0)
            felixActual->posicion.columna--;
        break;
    case MOVER_DERECHA: //Derecha
        if (felixActual->posicion.columna < VENTANA_PISO -1)
            felixActual->posicion.columna++;
        break;
    case MOVER_ARRIBA: //Arriba
        if (felixActual->posicion.fila < PISOS -1)
            felixActual->posicion.fila++;
        break;
    case MOVER_ABAJO: //Abajo
        if (felixActual->posicion.fila > 0)
            felixActual->posicion.fila--;
        break;
    }
    felixActual->posicionFisica = devolverPosicionFisica(felixActual->posicion);
    char buffer[100];
    sprintf(buffer, MOVER_FELIX_PMSG, numPlayer, felixActual->posicion.fila, felixActual->posicion.columna);
    concatenarSyncMessage(buffer);

    pthread_mutex_lock(&mutexTorta);
    controlarBuff(numPlayer);
    pthread_mutex_unlock(&mutexTorta);

}

PosicionFisica devolverPosicionFisica(Posicion p)
{
    for (int i=0; i<ventanasList.size(); i++)
        if (ventanasList[i].posicion.fila == p.fila && ventanasList[i].posicion.columna == p.columna)
            return ventanasList[i].posicionFisica;
}

void controlarBuff(int numPlayer)
{
    Felix * felixActual;
    int tortaActual = -1;
    felixActual = (numPlayer == 1)? &felix1:&felix2;
    for (int i=0; i<tortasList.size(); i++)
        if (tortasList[i].posicion.columna == felixActual->posicion.columna
                && tortasList[i].posicion.fila == felixActual->posicion.fila)
        {

            tortaActual = i;
            break;
        }
    if (tortaActual == -1) return;

    felixActual->tInicioVulnerabilidad = time(NULL); //Seteo la hora actual como tiempo de invulnerabilidad
    cambiarEstadoFelix(numPlayer, ESTADO_INVULNERABLE);
    eliminarTorta(tortaActual);
}

void cambiarEstadoFelix(int numPlayer, int estado)
{
    Felix * felixActual;
    felixActual = (numPlayer == 1)? &felix1:&felix2;
    felixActual->estado = estado;
    char buffer[100];
    sprintf(buffer,CAMBIAR_ESTADO_FELIX_PMSG, numPlayer, estado);
    concatenarSyncMessage(buffer);
}

void eliminarTorta(int numTorta)
{
    char buffer[100];
    sprintf(buffer, ELIMINAR_TORTA_PMSG, tortasList[numTorta].posicion.fila, tortasList[numTorta].posicion.columna);
    concatenarSyncMessage(buffer);
    tortasList.erase(tortasList.begin()+numTorta);
}

void arreglarVentana(int numPlayer, Posicion posicionVentana)
{
    Jugador * jugadorActual;
    Ventana * ventanaActual;
    jugadorActual = (numPlayer == 1)? &partidaActual.jugador1:&partidaActual.jugador2;
    for (int i=0; i<ventanasList.size(); i++)
    {
        if (ventanasList[i].posicion.columna == posicionVentana.columna
                && ventanasList[i].posicion.fila == posicionVentana.fila)
        {
            ventanaActual = &ventanasList[i];
            break;
        }
    }
    ventanaActual->estado--;
    if (numPlayer==1) partidaActual.puntaje1+= PUNTO_X_VENTANA;
    else partidaActual.puntaje2+= PUNTO_X_VENTANA;

    char buffer[100];
    sprintf(buffer, ARREGLAR_VENTANA_PMSG, numPlayer, posicionVentana.fila, posicionVentana.columna);
    concatenarSyncMessage(buffer);

    if (ventanaActual->estado == 1) //Si la arregló completamente, controlo si queda alguna por arreglar.
        controlarVentanas();

}

void controlarVentanas()
{
    for (int i=0; i<ventanasList.size(); i++)
        if (ventanasList[i].estado==2 || ventanasList[i].estado==3) return;
    //Si están todas arregladas, subo el nivel
    subirNivel();

}

void subirNivel()
{
    inicializarNivel(++numeroNivel);
    char buffer[100];
    sprintf(buffer, SUBIR_NIVEL_PMSG, numeroNivel);
    concatenarSyncMessage(buffer);
}

void concatenarSyncMessage(char * buffer)
{
    pthread_mutex_lock(&semSyncMessage);

    // Si el mensaje es subir nivel limpio el buffer
    if(strstr(buffer, "sn") != 0)
    	strcpy(syncMessage, "");

    strcat(syncMessage, buffer);
    pthread_mutex_unlock(&semSyncMessage);
}

void broadcastMessage(char * mensaje)
{
    if (partidaActual.jugador1.estado != DESCONECTADO)
        if((send(partidaActual.jugador1.socket, mensaje, TAMBUF, 0) ) < 0 )
        {
            //printf("Fallo el send del jugador1\n");
            partidaActual.jugador1.estado = DESCONECTADO;
        }

    if (partidaActual.jugador2.estado != DESCONECTADO)
        if((send(partidaActual.jugador2.socket, mensaje, TAMBUF, 0) ) < 0 )
        {
            //printf("Fallo el send del jugador2\n");
            partidaActual.jugador2.estado = DESCONECTADO;
        }
    if (partidaActual.jugador1.estado == partidaActual.jugador2.estado && partidaActual.jugador1.estado == DESCONECTADO)
    {
        printf("Ambos desconectados\n");
        fflush(stdout);
        finalizarPartida(0);
    }
    else if (!jugador2Avisado &&partidaActual.jugador1.estado == DESCONECTADO)
    {
        if((send(partidaActual.jugador2.socket, "rd", TAMBUF, 0) ) < 0 )
        {
            //printf("Fallo el send del jugador2\n");
            partidaActual.jugador2.estado = DESCONECTADO;
        }
        jugador2Avisado++;
    }
    else if (!jugador1Avisado &&partidaActual.jugador2.estado == DESCONECTADO)
    {

        if((send(partidaActual.jugador1.socket, "rd", TAMBUF, 0) ) < 0 )
        {
            //printf("Fallo el send del jugador2\n");
            partidaActual.jugador1.estado = DESCONECTADO;
        }
        jugador1Avisado++;
    }

}

bool huboImpactoGaviota(Gaviota g, Felix f)
{
    return (g.posicion.fila == f.posicion.fila && (abs(f.posicionFisica.x - g.posicionFisica.x)  < 15));
}

bool huboImpactoProyectil(Proyectil p, Felix f)
{
    return (p.posicion.columna == f.posicion.columna && (abs(p.posicionFisica.y - f.posicionFisica.y) < 20));
}

static void finalizarPartida(int signo)
{
    //Cancela todos los hilos, envía el mensaje de finalización a cada cliente, y cierra el socket, espera a los hilos con pthread_exit.
    char buffer[255];
    sprintf(buffer, END_PARTIDA_PMSG, partidaActual.ganador);
    
	if (partidaActual.jugador1.estado != DESCONECTADO)
        if (send(partidaActual.jugador1.socket, buffer , TAMBUF, 0)<0) partidaActual.jugador1.estado = DESCONECTADO;
    
	if (partidaActual.jugador2.estado != DESCONECTADO)
        if (send(partidaActual.jugador2.socket, buffer , TAMBUF, 0)<0) partidaActual.jugador2.estado = DESCONECTADO;
    
	//Si cerró abruptamente, cierro los sockets luego de avisarles.
    if (partidaActual.ganador == -9)
    {
        close(partidaActual.jugador1.socket);
        close(partidaActual.jugador2.socket);
    }
    
	 printf("Cerrando recursos de partida\n");
    fflush(stdout);
    
	 if (partidaActual.ganador != -9)
    {
        sem_wait(semEscritura);
        *partidaMC = partidaActual;
    }

    printf("Cerrando semaforos\n");
    fflush(stdout);
    sem_close(semStart);
    sem_close(semEscritura);
    sem_close(semLectura);
    sem_close(semLive);
    sem_unlink(semLecturaName);
    sem_unlink(semEscrituraName);
    sem_unlink(semLiveName);
    sem_unlink(semStartName);
	
	 // Liberar memoria compartida
	 munmap(partidaMC, sizeof(Partida));
    shm_unlink(shmName);
    munmap(tiempoMC, sizeof(Partida));
    shm_unlink(shmNameLive);

    printf("Terminando de cerrar semaforos\n");
    fflush(stdout);
    exit(0);
}

void finalizarTorneo()
{
    printf("Finalizando torneo\n");
    fflush(stdout);
    /*munmap(partidaMC, sizeof(Partida));
    shm_unlink(shmName);
    munmap(tiempoMC, sizeof(Partida));
    shm_unlink(shmNameLive);*/
    /*sem_close(semLectura);
    sem_close(semEscritura);
    sem_close(semLive);
    sem_unlink(semLecturaName);
    sem_unlink(semEscrituraName);
    sem_unlink(semLiveName);*/
    partidaActual.ganador = -9;
    finalizarPartida(0);
}

void exitForzado()
{
    printf("Entro a exit\n");
    fflush(stdout);
    finalizarPartida(0);
}

vector<string> split(string s, char delim)
{
    vector<string> elems;
    elems = split(s, delim, elems);
    return elems;
}

vector<string> split(string s, char delim, vector<string> elems)
{
    stringstream ss(s);
    string item;
    while (getline(ss, item, delim))
    {
        elems.push_back(item);
    }
    return elems;
}


