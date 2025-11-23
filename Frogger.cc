#include <esat/window.h>
#include <esat/draw.h>
#include <esat/input.h>
#include <esat/time.h>
#include <esat/sprite.h>

#include <stdlib.h>
#include <time.h>
#include <stdio.h>

/* Enums */
enum Direccion {
  ARRIBA,
  DERECHA,
  ABAJO,
  IZQUIERDA
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
    //Array con las coordenadas
    int spriteSheetCoords[]; 
};

struct Sprite{
    esat::SpriteHandle imagen;
    Colision collider;
    // tipoAnimacion -> El set de indiceAnimes a usar (Fila en spritesheet);
    // indiceAnimacion -> Secuencia de la animación a usar (Columna en spritesheet);
    unsigned char tipoAnimacion = 0, indiceAnimacion = 0;
};


/* FIN STRUCTS */

/* GLOBALES */
// Constantes
//-- Tamaños ventanas
const int VENTANA_X = 1008, VENTANA_Y = 720;

//-- Fuentes
const unsigned char ALTURA_FUENTE = 19;

//-- FPS
const unsigned char FPS=60;

//-- UI

//-- Fin UI

// Fin Constantes


// Variables
//-- Tiempos para FPS
double current_time,last_time;


//-- UI


//-- Fin UI


//-- SpriteSheets - Se declara todo lo necesario por SpriteSheet para funcionar durante la ejecución
//Recursos/Imagenes/SpriteSheets

//SpriteSheet
SpriteSheet animMuerteSheet;

//-- Fin SpriteSheets

Sprite muerte {0,{0.0f,0.0f},0,0};

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
void InicializarCoordsSpriteSheet(SpriteSheet *spriteSheet){
    for(int i = 0; i < (*spriteSheet).tiposAnim; i++){
        for(int j = 0, k = 0; j < (*spriteSheet).coordsAnim; j+=2, k++){
            //Coordenada X
            (*spriteSheet).spriteSheetCoords[GetIndiceArray(i,(*spriteSheet).coordsAnim,j)] = (*spriteSheet).spriteWidth*k;
            //Coordenada Y
            (*spriteSheet).spriteSheetCoords[GetIndiceArray(i,(*spriteSheet).coordsAnim,j+1)] = (*spriteSheet).spriteHeight*i;
        }
    }
}

// Inicializa todos los valores de todos los SpriteSheets para ser utilizables durante el resto de la ejecución del programa
void InicializarSpriteSheets(){
    animMuerteSheet.spriteSheet = esat::SpriteFromFile("./Recursos/Imagenes/SpriteSheets/AnimMuerteSheet.png");
    animMuerteSheet.tiposAnim = 1;
    animMuerteSheet.indicesAnim = 7;
    animMuerteSheet.coordsAnim = animMuerteSheet.indicesAnim*2;
    animMuerteSheet.totalCoordsAnim = animMuerteSheet.tiposAnim * animMuerteSheet.coordsAnim;
    animMuerteSheet.spriteWidth = (esat::SpriteWidth(animMuerteSheet.spriteSheet)/animMuerteSheet.indicesAnim);
    animMuerteSheet.spriteHeight = (esat::SpriteHeight(animMuerteSheet.spriteSheet)/animMuerteSheet.tiposAnim);
    InicializarCoordsSpriteSheet(&animMuerteSheet);
}

// Recupera el sprite del tipoAnimacion(Fila) e indiceAnimacion(Columna) de una spriteSheet
esat::SpriteHandle GetSpriteFromSheet(SpriteSheet *spriteSheet, int tipoAnimacion, int indiceAnimacion){
    return (esat::SubSprite(
        (*spriteSheet).spriteSheet,
        (*spriteSheet).spriteSheetCoords[GetIndiceArray(tipoAnimacion,(*spriteSheet).coordsAnim,indiceAnimacion)],
        (*spriteSheet).spriteSheetCoords[GetIndiceArray(tipoAnimacion,(*spriteSheet).coordsAnim,indiceAnimacion+1)],
        (*spriteSheet).spriteWidth,
        (*spriteSheet).spriteHeight
    ));
}

// Dada una estructura SpriteSheet y una estructura Sprite, actualiza la imagen
// del Sprite en base al SpriteSheet y el tipoAnimacion(Fila) e indiceAnimacion(Columna) que tiene el Sprite.
//
// Mediante la variable local buffer, se asegura de liberar la imagen previa que tenia asignada el Sprite
// para prevernir leaks de memoria
void AsignarYDibujarNuevoSprite(SpriteSheet *spriteSheet, Sprite *sprite){
    esat::SpriteHandle buffer = (*sprite).imagen;
    (*sprite).imagen = GetSpriteFromSheet(&(*spriteSheet),(*sprite).tipoAnimacion,(*sprite).indiceAnimacion);
    esat::DrawSprite((*sprite).imagen,(*sprite).collider.P1.x,(*sprite).collider.P1.y);
    if(buffer != NULL){
        esat::SpriteRelease(buffer);
    }
}

// Dada una estructura SpriteSheet y una estructura Sprite, actualiza la imagen
// del Sprite en base al SpriteSheet y el tipoAnimacion(Fila) e indiceAnimacion(Columna) que tiene el Sprite.
//
// Después, avanza el indice de animación del sprite en +2 para que el siguiente Sprite que guarde sea el correspondiente
// al siguiente en su animación. Si el indice es mayor o igual al total de coordenadas, lo reinicia a 0 para volver a
// comenzar la animación
//
// Mediante la variable local buffer, se asegura de liberar la imagen previa que tenia asignada el Sprite
// para prevernir leaks de memoria
void AvanzarYDibujarSpriteAnimado(SpriteSheet *spriteSheet, Sprite *sprite){
    AsignarYDibujarNuevoSprite(&(*spriteSheet), &(*sprite));
    // Avanza
    if((*sprite).indiceAnimacion < (*spriteSheet).totalCoordsAnim){
        (*sprite).indiceAnimacion += 2;
    }else{
        (*sprite).indiceAnimacion = 0;
    }

    
}

/* FIN FUNCIONALIDADES*/
int esat::main(int argc, char **argv) {
    GenerarSemillaAleatoria();

    esat::WindowInit(VENTANA_X,VENTANA_Y);
    WindowSetMouseVisibility(true);

    InicializarSpriteSheets();

    while(esat::WindowIsOpened() && !esat::IsSpecialKeyDown(esat::kSpecialKey_Escape)) {
        last_time = esat::Time(); 

        //Inicio dibujado de pantalla
        esat::DrawBegin();
        esat::DrawClear(0,0,0);

        AvanzarYDibujarSpriteAnimado(&animMuerteSheet,&muerte);

        esat::DrawEnd();      
        esat::WindowFrame();
        //Fin dibujado de pantalla
        ControlFPS();
    }

    esat::WindowDestroy();
    return 0;  
}

