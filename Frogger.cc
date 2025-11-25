#include <esat/window.h>
#include <esat/draw.h>
#include <esat/input.h>
#include <esat/time.h>
#include <esat/sprite.h>

#include <stdlib.h>
#include <time.h>
#include <stdio.h>

/* Enums */
enum Pantalla {
    INICIO,
    MENU,
    JUEGO
};

enum Direccion {
    ARRIBA,
    DERECHA,
    ABAJO,
    IZQUIERDA
};

// Enumerador que indicará que tipo 
// de objeto es a ciertas funciones
enum TipoObjeto{
    RANAJUGADOR_O,
    RANABONUS_O,
    VEHICULO_O,
    TRONCO_O,
    COCODRILO_CUERPO_O,
    COCODRILO_BOCA_O,
    NUTRIA_O,
    SERPIENTE_O,
    MOSCA_O,
    CROC_O
};

/* STRUCTS */
struct PuntoCoord{
    float x = 0, y = 0;
};

struct Colision{
    // P1 Tambien sirve como ubicación
    PuntoCoord P1 = {0.0f,0.0f};
    PuntoCoord P2 = {10.0f,10.0f};
};

struct SpriteSheet{
    //SpriteSheet
    esat::SpriteHandle spriteSheet;
    //Filas y columnas del SpriteSheet
    unsigned char tiposAnim, indicesAnim;
    //Total Coordenadas X,Y de acceso al sprite deseado
    unsigned char coordsAnim;
    unsigned char totalCoordsAnim;
    //Tamaño de los sprites
    int spriteWidth,spriteHeight;
};

struct Sprite{
    esat::SpriteHandle imagen;
    Colision collider;
    // tipoAnimacion -> El set de indiceAnimacion a usar (Fila en spritesheet);
    // indiceAnimacion -> Secuencia de la animación a usar (Columna en spritesheet);
    unsigned char tipoAnimacion = 0, indiceAnimacion = 0;
};

struct Rana{
	TipoObjeto tipoObjeto;
	Direccion direccion;
	Sprite sprite;
    bool isMoving = false;
    int distanciaSalto;
    float duracionSalto;
};

/* FIN STRUCTS */

/* GLOBALES */
//-- Tamaños ventanas
const int VENTANA_X = 1008, VENTANA_Y = 720;

//-- Fuentes
const unsigned char ALTURA_FUENTE = 19;

//-- FPS
const unsigned char FPS=60;

//-- Tiempos para FPS
double current_time,last_time;


//-- UI
Pantalla pantallaActual = JUEGO;

//-- Fin UI


//-- SpriteSheets - Se declara todo lo necesario por SpriteSheet para funcionar durante la ejecución
// SpriteSheets y arrays de coordenadas del mismo
// Estos arrays no se incluyen en el propio spriteSheet para poder inicializar su tamaño
// evitando problemas de acceso a memoria al no tener la capacidad de usar gestión dinámica de memoria
SpriteSheet animMuerteSpriteSheet;
int animMuerteSpriteSheet_Coords[14]; 

SpriteSheet ranaBaseSpriteSheet;
SpriteSheet ranaRosaSpriteSheet;
SpriteSheet ranaRojaSpriteSheet;
int ranasSpriteSheet_Coords[16];

//-- Fin SpriteSheets

//-- Jugadores
const unsigned char maxJugadores = 2;
unsigned char jugadoresActuales = 2;
Rana jugadores[maxJugadores];

// Fin Variables
/* FIN GLOBALES */

/* FUNCIONALIDADES */
void ControlFPS(){
    do{
        current_time = esat::Time();
    }while((current_time-last_time)<=1000.0/FPS);
}

//-- Randoms
    // Genera nueva semilla en base al 
    // ms del reloj del sistema
void GenerarSemillaAleatoria(){
    srand(time(NULL)); 
}
//Genera un número del 0 al límite indicado
int GenerarNumeroAleatorio(int limite){
    return (rand()%limite);
}
//-- Fin Randoms

//-- Arrays
int GetIndiceArray(int fila, int columnasTotales, int columna){
    return((fila*columnasTotales)+columna);
}

int GetFilaIndice(int indice, int columnasTotales){
    return(indice/columnasTotales);
}

int GetColumnaIndice(int indice, int columnasTotales){
    return(indice%columnasTotales);
}
//-- Fin Arrays

//-- Sprites
// Necesita un SpriteSheet con todos los parametros completados a excepción de spriteSheetCoords. 
// Calcula e inicializa en spriteSheetCoords las coordenadas X e Y de cada Sprite 
// dentro del SpriteSheet para uso futuro durante ejecución
void InicializarCoordsSpriteSheet(SpriteSheet *spriteSheet, int spriteSheetCoords[]){
    for(int i = 0; i < (*spriteSheet).tiposAnim; i++){
        for(int j = 0, k = 0; j < (*spriteSheet).coordsAnim; j+=2, k++){
            //Coordenada X
            spriteSheetCoords[GetIndiceArray(i,(*spriteSheet).coordsAnim,j)] = (*spriteSheet).spriteWidth*k;
            //Coordenada Y
            spriteSheetCoords[GetIndiceArray(i,(*spriteSheet).coordsAnim,j+1)] = (*spriteSheet).spriteHeight*i;
        }
    }
}

// Inicializa todos los valores de todos los SpriteSheets para ser utilizables durante el resto de la ejecución del programa
//
// Se guardan en sus structs correspondientes para facilitar su uso y legibilidad durante el proyecto
// Los archivos de hojas de sprite en .png se ubican en ./Recursos/Imagenes/SpriteSheets/*.png

void InicializarSpriteSheets(){
    //Inicializa el SpriteSheet de la animación de muerte del jugador
    animMuerteSpriteSheet.spriteSheet = esat::SpriteFromFile("./Recursos/Imagenes/SpriteSheets/AnimMuerteSpriteSheet.png");
    animMuerteSpriteSheet.tiposAnim = 1;
    animMuerteSpriteSheet.indicesAnim = 7;
    animMuerteSpriteSheet.coordsAnim = animMuerteSpriteSheet.indicesAnim*2;
    animMuerteSpriteSheet.totalCoordsAnim = animMuerteSpriteSheet.tiposAnim * animMuerteSpriteSheet.coordsAnim;
    animMuerteSpriteSheet.spriteWidth = (esat::SpriteWidth(animMuerteSpriteSheet.spriteSheet)/animMuerteSpriteSheet.indicesAnim);
    animMuerteSpriteSheet.spriteHeight = (esat::SpriteHeight(animMuerteSpriteSheet.spriteSheet)/animMuerteSpriteSheet.tiposAnim);
    InicializarCoordsSpriteSheet(&animMuerteSpriteSheet, animMuerteSpriteSheet_Coords);

    // Inicializa los SpriteSheets de las ranas en todos sus colores y el array de sus coordenadas
    ranaBaseSpriteSheet.spriteSheet = esat::SpriteFromFile("./Recursos/Imagenes/SpriteSheets/RanaBaseSpriteSheet.png");
    ranaBaseSpriteSheet.tiposAnim = 4;
    ranaBaseSpriteSheet.indicesAnim = 2;
    ranaBaseSpriteSheet.coordsAnim = ranaBaseSpriteSheet.indicesAnim*2;
    ranaBaseSpriteSheet.totalCoordsAnim = ranaBaseSpriteSheet.tiposAnim * ranaBaseSpriteSheet.coordsAnim;
    ranaBaseSpriteSheet.spriteWidth = (esat::SpriteWidth(ranaBaseSpriteSheet.spriteSheet)/ranaBaseSpriteSheet.indicesAnim);
    ranaBaseSpriteSheet.spriteHeight = (esat::SpriteHeight(ranaBaseSpriteSheet.spriteSheet)/ranaBaseSpriteSheet.tiposAnim);

    ranaRosaSpriteSheet.spriteSheet = esat::SpriteFromFile("./Recursos/Imagenes/SpriteSheets/RanaRosaSpriteSheet.png");
    ranaRosaSpriteSheet.tiposAnim = 4;
    ranaRosaSpriteSheet.indicesAnim = 2;
    ranaRosaSpriteSheet.coordsAnim = ranaRosaSpriteSheet.indicesAnim*2;
    ranaRosaSpriteSheet.totalCoordsAnim = ranaRosaSpriteSheet.tiposAnim * ranaRosaSpriteSheet.coordsAnim;
    ranaRosaSpriteSheet.spriteWidth = (esat::SpriteWidth(ranaRosaSpriteSheet.spriteSheet)/ranaRosaSpriteSheet.indicesAnim);
    ranaRosaSpriteSheet.spriteHeight = (esat::SpriteHeight(ranaRosaSpriteSheet.spriteSheet)/ranaRosaSpriteSheet.tiposAnim);

    ranaRojaSpriteSheet.spriteSheet = esat::SpriteFromFile("./Recursos/Imagenes/SpriteSheets/RanaRojaSpriteSheet.png");
    ranaRojaSpriteSheet.tiposAnim = 4;
    ranaRojaSpriteSheet.indicesAnim = 2;
    ranaRojaSpriteSheet.coordsAnim = ranaRojaSpriteSheet.indicesAnim*2;
    ranaRojaSpriteSheet.totalCoordsAnim = ranaRojaSpriteSheet.tiposAnim * ranaRojaSpriteSheet.coordsAnim;
    ranaRojaSpriteSheet.spriteWidth = (esat::SpriteWidth(ranaRojaSpriteSheet.spriteSheet)/ranaRojaSpriteSheet.indicesAnim);
    ranaRojaSpriteSheet.spriteHeight = (esat::SpriteHeight(ranaRojaSpriteSheet.spriteSheet)/ranaRojaSpriteSheet.tiposAnim);
    InicializarCoordsSpriteSheet(&ranaBaseSpriteSheet, ranasSpriteSheet_Coords);
    
}

//*** MANEJO DE SPRITES/SPRITESHEETS ***/
// Recupera el sprite del tipoAnimacion(Fila) e indiceAnimacion(Columna) de una spriteSheet
esat::SpriteHandle GetSpriteFromSheet(SpriteSheet *spriteSheet, int spriteSheetCoords[], int tipoAnimacion, int indiceAnimacion){
    return (esat::SubSprite(
        (*spriteSheet).spriteSheet,
        spriteSheetCoords[GetIndiceArray(tipoAnimacion,(*spriteSheet).coordsAnim,indiceAnimacion)],
        spriteSheetCoords[GetIndiceArray(tipoAnimacion,(*spriteSheet).coordsAnim,indiceAnimacion+1)],
        (*spriteSheet).spriteWidth,
        (*spriteSheet).spriteHeight
    ));
}

// ** USAR SOLO CUANDO SE DESEA CAMBIAR DE SPRITE NO PARA DIBUJAR UNA IMAGEN DE FORMA REGULAR **
// Dada una estructura SpriteSheet y una estructura Sprite, actualiza la imagen
// del Sprite en base al SpriteSheet y el tipoAnimacion(Fila) e indiceAnimacion(Columna) que 
// tiene el Sprite y lo dibuja
//
// Mediante la variable local buffer, se asegura de liberar la imagen previa que tenia asignada el Sprite
// para prevernir leaks de memoria
void ActualizarSprite(SpriteSheet *spriteSheet, int spriteSheetCoords[], Sprite *sprite){
    esat::SpriteHandle buffer = (*sprite).imagen;
    (*sprite).imagen = GetSpriteFromSheet(&(*spriteSheet), spriteSheetCoords, (*sprite).tipoAnimacion,(*sprite).indiceAnimacion);
    if(buffer != NULL){
        esat::SpriteRelease(buffer);
    }
}

// ** USAR SOLO CUANDO SE DESEA AVANZAR EL INDICE DE ANIMACIÓN Y ACTUALIZAR EL SPRITE NO PARA DIBUJAR UNA IMAGEN DE FORMA REGULAR **
// Dada una estructura SpriteSheet y una estructura Sprite, actualiza la imagen
// del Sprite en base al SpriteSheet y el tipoAnimacion(Fila) e indiceAnimacion(Columna) que 
// tiene el Sprite, lo dibuja y libera la memoria de la imagen anterior.
//
// Después, avanza el indice de animación del sprite en +2 para que el siguiente Sprite que guarde sea el correspondiente
// al siguiente en su animación. Si el indice es mayor o igual al total de coordenadas, lo reinicia a 0 para volver a
// comenzar la animación
void AvanzarSpriteAnimado(SpriteSheet *spriteSheet, int spriteSheetCoords[], Sprite *sprite){
    ActualizarSprite(&(*spriteSheet), spriteSheetCoords, &(*sprite));
    // Avanza indice de animación. 
    (*sprite).indiceAnimacion += 2;
    // Si despues de sumar 2, el indice es mayor o igual a las columnas totales de coordenadas
    // de animacion, lo instancia a 0 para reiniciar la animacion
    if((*sprite).indiceAnimacion >= (*spriteSheet).coordsAnim){
        (*sprite).indiceAnimacion = 0;
    }
}

void InicializarJugadores(){
    for(int i = 0; i < jugadoresActuales; i++){
        jugadores[i].tipoObjeto = RANAJUGADOR_O;
        jugadores[i].direccion = ARRIBA;
        jugadores[i].sprite.tipoAnimacion = 0;
        jugadores[i].sprite.indiceAnimacion = 0;
        jugadores[i].sprite.collider.P1 = {
            i*50.0F,0.0F
        };
        jugadores[i].sprite.collider.P2 = {
            jugadores[i].sprite.collider.P1.x + ranaBaseSpriteSheet.spriteWidth,
            jugadores[i].sprite.collider.P1.y + ranaBaseSpriteSheet.spriteHeight
        };
        jugadores[i].isMoving = false;
        ActualizarSprite(&ranaBaseSpriteSheet, ranasSpriteSheet_Coords, &jugadores[i].sprite);
    }
}


void DetectarControles(){
    if(pantallaActual == JUEGO){
        //CONTROLES RANA_J1
        if(esat::IsSpecialKeyDown(esat::kSpecialKey_Up)){
            jugadores[0].direccion = ARRIBA;
            jugadores[0].isMoving = true;
        }
        if(esat::IsSpecialKeyDown(esat::kSpecialKey_Right)){
            jugadores[0].direccion = DERECHA;
            jugadores[0].isMoving = true;
        }
        if(esat::IsSpecialKeyDown(esat::kSpecialKey_Down)){
            jugadores[0].direccion = ABAJO;
            jugadores[0].isMoving = true;
        }
        if(esat::IsSpecialKeyDown(esat::kSpecialKey_Left)){
            jugadores[0].direccion = IZQUIERDA;
            jugadores[0].isMoving = true;
        }
    }

    //PROTOTIPADO
    if(esat::IsKeyDown('1')){
        printf("INICIO\n");
        pantallaActual = INICIO;
    }
    if(esat::IsKeyDown('2')){
        printf("MENU\n");
        pantallaActual = MENU;
    }
    if(esat::IsKeyDown('3')){
        printf("JUEGO\n");
        pantallaActual = JUEGO;
    }
}

//*** FUNCIONES DE DIBUJADO DE ELEMENTOS EN PANTALLA ***///
void DibujarJugadores(){
    for(int i = 0; i < jugadoresActuales; i++){
        if(jugadores[i].isMoving){
            //PROTOTIPADO
            jugadores[i].sprite.tipoAnimacion = jugadores[i].direccion;
            ActualizarSprite(&ranaBaseSpriteSheet, ranasSpriteSheet_Coords, &jugadores[i].sprite);
            jugadores[i].isMoving = false;
        }
        esat::DrawSprite(jugadores[i].sprite.imagen, jugadores[i].sprite.collider.P1.x, jugadores[i].sprite.collider.P1.y);
    }
}

void DibujarJuego(){
    DibujarJugadores();
}

void DibujarEntorno(){
    // DibujarCabecera()
    switch(pantallaActual){
        case INICIO:
            break;
        case MENU:
            break;
        case JUEGO:
            DibujarJuego();
            break;
    }
    // DibujarPie();
}

//*** FUNCIONES DE LIBERADO DE MEMORIA AL TERMINAL EL PROCESO ***///
void LiberarSpriteSheets(){
    // Los delete[] liberan la memoria de los arrays de coordenadas de los spriteSheets 
    esat::SpriteRelease(animMuerteSpriteSheet.spriteSheet);
    esat::SpriteRelease(ranaBaseSpriteSheet.spriteSheet);
    esat::SpriteRelease(ranaRosaSpriteSheet.spriteSheet);
    esat::SpriteRelease(ranaRojaSpriteSheet.spriteSheet);
}
void LiberarSpritesJugadores(){
    for(int i = 0; i < maxJugadores; i++){
        if(jugadores[i].sprite.imagen != NULL){
            esat::SpriteRelease(jugadores[i].sprite.imagen);
        }
    }
}
//Libera de memoria todos los spriteHandle inicializados
void LiberarSprites(){
    LiberarSpriteSheets();
    LiberarSpritesJugadores();
}
/* FIN FUNCIONALIDADES*/

/* INICIO MAIN */
int esat::main(int argc, char **argv) {
    GenerarSemillaAleatoria();

    esat::WindowInit(VENTANA_X,VENTANA_Y);
    WindowSetMouseVisibility(true);

    InicializarSpriteSheets();
    InicializarJugadores();

    while(esat::WindowIsOpened() && !esat::IsSpecialKeyDown(esat::kSpecialKey_Escape)) {
        last_time = esat::Time(); 

        //Inicio dibujado de pantalla
        esat::DrawBegin();
        esat::DrawClear(0,0,0);

        DetectarControles();
        DibujarEntorno();

        esat::DrawEnd();      
        esat::WindowFrame();
        //Fin dibujado de pantalla
        ControlFPS();
    }

    LiberarSprites();
    esat::WindowDestroy();
    return 0;  
}

