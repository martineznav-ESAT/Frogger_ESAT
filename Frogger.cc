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

// Facilita la selección del indiceAnimacion 
// de los vehiculos para dibujar el vehiculo deaseado
enum TipoVehiculo {
    COCHE_AMARILLO,
    TRACTOR,
    COCHE_ROSA,
    COCHE_BLANCO,
    CAMION_FRONT,
    CAMION_BACK
};

/* STRUCTS */
struct PuntoCoord{
    float x = 0, y = 0;
};

struct Collider{
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
    Collider collider;
    // tipoAnimacion -> El set de indiceAnimacion a usar (Fila en spritesheet);
    // indiceAnimacion -> Secuencia de la animación a usar (Columna en spritesheet);
    unsigned char tipoAnimacion = 0, indiceAnimacion = 0;
    bool isVisible = true, isActive = true; 
};

struct Rana{
	Direccion direccion;
	Sprite sprite;
    bool isJumping = false;
    // finSalto se encarga de conservar la coordenada final en la
    // que debe aterrizar la rana asegurar su correcto movimiento
    Collider finSalto;
    int distanciaSalto;
    //Tiempos en milisegunos/ms
    float duracionSalto, tempSalto, velocidadSalto;
};

struct Vehiculo{
    TipoVehiculo tipoVehiculo;
    Direccion direccion;
	Sprite sprite;
    float velocidadMovimiento;
};

/* FIN STRUCTS */

/* GLOBALES */
//-- Tamaños ventanas
const int VENTANA_X = 672, VENTANA_Y = 768;

//-- Fuentes
const unsigned char ALTURA_FUENTE = 19;

//-- FPS
const unsigned char FPS=60;

//-- Tiempos para FPS
double current_time,last_time;


//-- UI
Pantalla pantallaActual = JUEGO;

//-- Fin UI


//-- SpriteSheets y arrays de coordenadas del mismo
// Estos arrays no se incluyen en el propio spriteSheet para poder inicializar su tamaño
// evitando problemas de acceso a memoria al no tener la capacidad de usar gestión dinámica de memoria
SpriteSheet animMuerteSpriteSheet;
int animMuerteSpriteSheet_Coords[14]; 

SpriteSheet ranaBaseSpriteSheet;
SpriteSheet ranaRosaSpriteSheet;
SpriteSheet ranaRojaSpriteSheet;
int ranasSpriteSheet_Coords[16];

SpriteSheet vehiculosSpriteSheet;
int vehiculosSpriteSheet_Coords[12];

//-- Sprites | Declaración de los handles cuyos sprites: 
//  -Siempre serán iguales (Sin animación ni acceso multiple como los vehiculos)
//  -No necesitan collider
esat::SpriteHandle arbustoSprite;


//-- Jugadores
const unsigned char maxJugadores = 2;
unsigned char jugadoresActuales = 1;
Rana jugadores[maxJugadores];

//-- Obstáculos
const int filasCarretera = 5;
const int maxCamiones = 8, maxCochesBlancos = 4, maxCochesAmarillos = 4;
const int maxCochesRosas = 5, maxTractores = 4;
Vehiculo camiones[maxCamiones], cochesBlancos[maxCochesBlancos], cochesAmarillos[maxCochesAmarillos]; 
Vehiculo cochesRosas[maxCochesRosas], tractores[maxTractores];

//-- Estructuras
const int tamanyoFilaArbustos = 14;

// Fin Variables
/* FIN GLOBALES */

/* FUNCIONALIDADES */
void ControlFPS(){
    do{
        current_time = esat::Time();
    }while((current_time-last_time)<=1000.0/FPS);
}

// Actua como "trigger" donde:
// Si el tiempo actual de ejecución menos el temporizador pasado 
// por parametro es mayor a 'x' entonces permitirá acceso 
// (Todo en milesimas de segundo)
bool HacerCadaX(float *temp, float x){
    bool isAccesible = false;
    // printf("LOG--- Valores funcion HacerCadaX()\n");
    // printf("LOG--- T.Actual    Temp    Contador    Limite\n");
    // printf("LOG--- %14.10f | %14.10f | %14.10f |%14.10f\n",last_time,(*temp), last_time - (*temp),x);
    isAccesible = last_time - (*temp) > x;
    if(isAccesible){
        *temp = last_time;
    }
    return isAccesible;
}

bool HacerDuranteX(float *temp, float x){
    return(!HacerCadaX(&(*temp),x));
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


//Dada una coordenada actual y una esperada final, devuelve false si es difetente
bool ComprobarPosicionFinal(Collider actual, Collider final){
    return (
        actual.P1.x == final.P1.x && actual.P2.x == final.P2.x &&
        actual.P1.y == final.P1.y && actual.P2.y == final.P2.y
    );
}

bool ComprobarSalidaVentanaSprite(Sprite *sprite, Direccion direccion){
    bool isOut = false;
    switch(direccion){
        case ARRIBA:
            isOut = (*sprite).collider.P2.y < 0;
        break;
        case DERECHA:
            isOut = (*sprite).collider.P1.x > VENTANA_X;
        break;
        case ABAJO:
            isOut = (*sprite).collider.P1.y > VENTANA_Y;
        break;
        case IZQUIERDA:
            isOut = (*sprite).collider.P2.x < 0;
        break;
    }

    return isOut;
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

    // Inicializa el SpriteSheet de los vehiculos y su array de coordenadas
    vehiculosSpriteSheet.spriteSheet = esat::SpriteFromFile("./Recursos/Imagenes/SpriteSheets/VehiculosSpriteSheet.png");
    vehiculosSpriteSheet.tiposAnim = 1;
    vehiculosSpriteSheet.indicesAnim = 6;
    vehiculosSpriteSheet.coordsAnim = vehiculosSpriteSheet.indicesAnim*2;
    vehiculosSpriteSheet.totalCoordsAnim = vehiculosSpriteSheet.tiposAnim * vehiculosSpriteSheet.coordsAnim;
    vehiculosSpriteSheet.spriteWidth = (esat::SpriteWidth(vehiculosSpriteSheet.spriteSheet)/vehiculosSpriteSheet.indicesAnim);
    vehiculosSpriteSheet.spriteHeight = (esat::SpriteHeight(vehiculosSpriteSheet.spriteSheet)/vehiculosSpriteSheet.tiposAnim);
    InicializarCoordsSpriteSheet(&vehiculosSpriteSheet, vehiculosSpriteSheet_Coords);
}

void InicializarSprites(){
    arbustoSprite = esat::SpriteFromFile("./Recursos/Imagenes/Sprites/ArbustoSprite.png");
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
    // Avanza indice de animación. 
    (*sprite).indiceAnimacion += 2;
    // Si despues de sumar 2, el indice es mayor o igual a las columnas totales de coordenadas
    // de animacion, lo instancia a 0 para reiniciar la animacion
    if((*sprite).indiceAnimacion >= (*spriteSheet).coordsAnim){
        (*sprite).indiceAnimacion = 0;
    }
    ActualizarSprite(&(*spriteSheet), spriteSheetCoords, &(*sprite));
}

void RellocateSprite(Sprite *sprite, PuntoCoord nuevaUbicacion){
    (*sprite).collider.P1 = nuevaUbicacion;
    (*sprite).collider.P2 = {nuevaUbicacion.x + esat::SpriteWidth((*sprite).imagen), nuevaUbicacion.y + esat::SpriteHeight((*sprite).imagen)};
}

// Ubica el Sprite en el borde opuesto de la dirección que se indica ya que se asume que se ha escapado
// por ese borde y se desea reubicar en el lugar contrario
//   - *collider        -> El collider a actualizar. El P1 de un collider indica también el punto de inicio de dibujado
//   - direccion        -> Indica en que dirección se moverá
void RellocateSpriteOnBorderEscape(Sprite *sprite, Direccion direccion){
    PuntoCoord nuevaUbicacion;
    switch(direccion){
        case ARRIBA:
            nuevaUbicacion = {(*sprite).collider.P1.x,VENTANA_Y};
        break;
        case DERECHA:
            nuevaUbicacion = {0.0f-esat::SpriteWidth((*sprite).imagen),(*sprite).collider.P1.y};
        break;
        case ABAJO:
            nuevaUbicacion = {(*sprite).collider.P1.x,0.0f-esat::SpriteWidth((*sprite).imagen)};
        break;
        case IZQUIERDA:
            nuevaUbicacion = {VENTANA_X,(*sprite).collider.P1.y};
        break;
    }
    RellocateSprite(&(*sprite), nuevaUbicacion);
}

// Actualiza las coordenadas de un Collider
//   - *collider -> El collider a actualizar. El P1 de un collider indica también el punto de inicio de dibujado
//   - direccion -> Indica en que dirección se moverá
//   - velocidad -> Indica la cantidad de pixeles que se moverá el collider en la direccion indicada
void MoveCollider(Collider *collider, Direccion direccion, float velocidad){
    switch(direccion){
        case ARRIBA:
            (*collider).P1.y -= velocidad;
            (*collider).P2.y -= velocidad;
        break;
        case DERECHA:
            (*collider).P1.x += velocidad;
            (*collider).P2.x += velocidad;
        break;
        case ABAJO:
            (*collider).P1.y += velocidad;
            (*collider).P2.y += velocidad;
        break;
        case IZQUIERDA:
            (*collider).P1.x -= velocidad;
            (*collider).P2.x -= velocidad;
        break;
    }
}

// Instancia los valores de un vehiculo
// *vehiculo            -> Vehiculo a instanciar
// tipo                 -> Indica que tipo de vehiculo
// direccion            -> Establece la dirección en la que se está moviendo
// fila                 -> Fila de la pantalla (contando desde abajo) en la que se va a dibujar (coord -> Y)
// posicionX            -> Posición de X del punto 1 del sprite del vehiculo
// velocidadMovimiento  -> Velocidad a la que se moverá el vehiculo
// margen               -> Indica el espacio adicional en X en funcion de la dirección
void InicializarVehiculo(
    Vehiculo *vehiculo, TipoVehiculo tipo, Direccion direccion,
    int fila, float posicionX, 
    float velocidadMovimiento, 
    float margen
){
    margen = vehiculosSpriteSheet.spriteWidth * margen;

    (*vehiculo).direccion = direccion;
    (*vehiculo).sprite.tipoAnimacion = 0;
    (*vehiculo).sprite.isVisible = true;
    (*vehiculo).sprite.isActive = true;
    (*vehiculo).velocidadMovimiento = velocidadMovimiento;
    
    (*vehiculo).sprite.collider.P1 = {posicionX, ((float)(VENTANA_Y-(vehiculosSpriteSheet.spriteHeight*fila)))};
    (*vehiculo).sprite.collider.P2 = {posicionX + vehiculosSpriteSheet.spriteWidth, ((float) VENTANA_Y-(vehiculosSpriteSheet.spriteHeight*(fila-1)))};
    
    switch(direccion){
        case DERECHA:
            (*vehiculo).sprite.collider.P1.x -= margen;
            (*vehiculo).sprite.collider.P2.x -= margen;
        break;
        case IZQUIERDA:
            (*vehiculo).sprite.collider.P1.x += margen;
            (*vehiculo).sprite.collider.P2.x += margen;
        break;
    }
    
    (*vehiculo).sprite.indiceAnimacion = tipo*2;

    ActualizarSprite(&vehiculosSpriteSheet,vehiculosSpriteSheet_Coords,&(*vehiculo).sprite);   
}

//*** MANEJO DE INICIALIZACIÓN DE OBJETOS ***/
// Instancia los valores por defecto de los vehiculos al inicio de un nivel
void InicializarCarretera(){
    float ultimaPX_Amarillo, ultimaPX_Tractor, ultimaPX_Rosa, ultimaPX_Blanco, ultimaPX_Camion;
    // Se usa maxCamiones como límite ya que es el array de los obstaculos de carretera mas largo
    // Luego internamente se comprueba que el indice sea valido para cada columna (cantidad de vehiculos)
    for(int i = 0; i < maxCamiones; i++){
        // InicializarCochesAmarillos
        if(i < maxCochesAmarillos){
            InicializarVehiculo(
                &cochesAmarillos[i],
                COCHE_AMARILLO,
                IZQUIERDA,
                3,
                i == 0 ?  
                0: 
                ultimaPX_Amarillo+vehiculosSpriteSheet.spriteWidth, 
                1,
                i == 0 ? 0 : 2
            );
            ultimaPX_Amarillo = cochesAmarillos[i].sprite.collider.P1.x;
        }

        // InicializarTractores
        if(i < maxTractores){
            InicializarVehiculo(
                &tractores[i],
                TRACTOR,
                DERECHA,
                4,
                i == 0 ? 
                VENTANA_X-vehiculosSpriteSheet.spriteWidth : 
                ultimaPX_Tractor-vehiculosSpriteSheet.spriteWidth, 
                1,
                i == 0 ? 0 : 2
            );

            ultimaPX_Tractor = tractores[i].sprite.collider.P1.x;
        }

        // InicializarCochesRosas
        if(i < maxCochesRosas){
            InicializarVehiculo(
                &cochesRosas[i],
                COCHE_ROSA,
                IZQUIERDA,
                5,
                i == 0 ?  
                0: 
                ultimaPX_Amarillo+vehiculosSpriteSheet.spriteWidth, 
                1,
                i == 0 ? 0 : 2
            );
            ultimaPX_Rosa = cochesAmarillos[i].sprite.collider.P1.x;
        }
        
        // InicializarCochesBlancos
        if(i < maxCochesBlancos){
            InicializarVehiculo(
                &cochesBlancos[i],
                COCHE_BLANCO,
                DERECHA,
                6,
                i == 0 ? 
                VENTANA_X-vehiculosSpriteSheet.spriteWidth : 
                ultimaPX_Tractor-vehiculosSpriteSheet.spriteWidth, 
                1,
                i == 0 ? 0 : 2
            );

            ultimaPX_Tractor = tractores[i].sprite.collider.P1.x;
        }

        // InicializarCamiones
        InicializarVehiculo(
            &camiones[i],
            i%2 == 0 ? CAMION_FRONT : CAMION_BACK,
            IZQUIERDA,
            7,
            i == 0 ?  
            0: 
            ultimaPX_Camion+vehiculosSpriteSheet.spriteWidth, 
            1,
            i == 0 ? 0 : i%2==0 ? 2 : 0
        );
        ultimaPX_Camion = camiones[i].sprite.collider.P1.x;
    }
}

// Instancia los valores por defecto de cada jugador
// Pensado para utilizarse al inicio de cada partida
void InicializarJugadores(){
    for(int i = 0; i < jugadoresActuales; i++){
        jugadores[i].direccion = ARRIBA;
        jugadores[i].sprite.tipoAnimacion = 0;
        jugadores[i].sprite.indiceAnimacion = 0;
        jugadores[i].sprite.collider.P1 = {
            VENTANA_X/2,VENTANA_Y/2
        };
        jugadores[i].sprite.collider.P2 = {
            jugadores[i].sprite.collider.P1.x + ranaBaseSpriteSheet.spriteWidth,
            jugadores[i].sprite.collider.P1.y + ranaBaseSpriteSheet.spriteHeight
        };
        jugadores[i].isJumping = false;
        jugadores[i].finSalto = jugadores[i].sprite.collider; 
        jugadores[i].distanciaSalto = ranaBaseSpriteSheet.spriteHeight;
        jugadores[i].duracionSalto = 100;
        jugadores[i].velocidadSalto = (jugadores[i].distanciaSalto/(jugadores[i].duracionSalto/(1000.0f/FPS)));
        jugadores[i].tempSalto = 0;
        ActualizarSprite(&ranaBaseSpriteSheet, ranasSpriteSheet_Coords, &jugadores[i].sprite);
    }
}

//*** INICIO DE ACCIONES DE LOS OBJETOS ***//
void IniciarSaltoRana(Rana *rana, Direccion newDireccion){
    (*rana).direccion = newDireccion;
    // CALCULA LA POSICION FINAL DONDE DEBE ATERRIZAR
    switch((*rana).direccion){
        case ARRIBA:
            (*rana).finSalto.P1.y -= (*rana).distanciaSalto;
            (*rana).finSalto.P2.y -= (*rana).distanciaSalto;
        break;
        case DERECHA:
            (*rana).finSalto.P1.x += (*rana).distanciaSalto;
            (*rana).finSalto.P2.x += (*rana).distanciaSalto;
        break;
        case ABAJO:
            (*rana).finSalto.P1.y += (*rana).distanciaSalto;
            (*rana).finSalto.P2.y += (*rana).distanciaSalto;
        break;
        case IZQUIERDA:
            (*rana).finSalto.P1.x -= (*rana).distanciaSalto;
            (*rana).finSalto.P2.x -= (*rana).distanciaSalto;
        break;
    }
    (*rana).isJumping = true;
    (*rana).tempSalto = last_time;
    (*rana).sprite.tipoAnimacion = newDireccion;
    AvanzarSpriteAnimado(&ranaBaseSpriteSheet, ranasSpriteSheet_Coords, &(*rana).sprite);
}

//*** DETECCIÓN INPUT DEL JUGADOR ***//
void DetectarControles(){
    if(pantallaActual == JUEGO){
        // CONTROLES RANA_J1
        // El jugador podrá realizar acciones siempre que:
        //   - No esté saltando
        if(!jugadores[0].isJumping){
            if(esat::IsSpecialKeyDown(esat::kSpecialKey_Up)){
                IniciarSaltoRana(&jugadores[0], ARRIBA);
            }
            if(esat::IsSpecialKeyDown(esat::kSpecialKey_Right)){
                IniciarSaltoRana(&jugadores[0], DERECHA);
            }
            if(esat::IsSpecialKeyDown(esat::kSpecialKey_Down)){
                IniciarSaltoRana(&jugadores[0], ABAJO);
            }
            if(esat::IsSpecialKeyDown(esat::kSpecialKey_Left)){
                IniciarSaltoRana(&jugadores[0], IZQUIERDA);
            }
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

/*** FUNCIONES DE ACTUALIZACIÓN DE ESTADO DEL JUEGO ***/
void ActualizarEstadoVehiculo(Vehiculo *vehiculo){
    if(ComprobarSalidaVentanaSprite(&(*vehiculo).sprite,(*vehiculo).direccion)){
        RellocateSpriteOnBorderEscape(&(*vehiculo).sprite, (*vehiculo).direccion);
    }else{
        MoveCollider(&(*vehiculo).sprite.collider,(*vehiculo).direccion,(*vehiculo).velocidadMovimiento);
    }
    ActualizarSprite(&vehiculosSpriteSheet,vehiculosSpriteSheet_Coords,&(*vehiculo).sprite);
}

void ActualizarEstadoVehiculos(){
    for(int i = 0; i < maxCamiones; i++){
        if(i < maxCochesAmarillos){
            ActualizarEstadoVehiculo(&cochesAmarillos[i]);
        }

        if(i < maxTractores){
            ActualizarEstadoVehiculo(&tractores[i]);
        }

        if(i < maxCochesRosas){
            ActualizarEstadoVehiculo(&cochesRosas[i]);
        }
        
        if(i < maxCochesBlancos){
            ActualizarEstadoVehiculo(&cochesBlancos[i]);
        }

        // Actualiza el estado de los camiones
        if(i%2 != 0 && camiones[i].sprite.collider.P2.x < 0){
            camiones[i-1].sprite.collider.P1.x = VENTANA_X;
            camiones[i-1].sprite.collider.P2.x = VENTANA_X+vehiculosSpriteSheet.spriteWidth;
            camiones[i].sprite.collider.P1.x = camiones[i-1].sprite.collider.P2.x;
            camiones[i].sprite.collider.P2.x = camiones[i-1].sprite.collider.P2.x+vehiculosSpriteSheet.spriteWidth;
        }else{
            MoveCollider(&camiones[i].sprite.collider,camiones[i].direccion,camiones[i].velocidadMovimiento);
        }
        ActualizarSprite(&vehiculosSpriteSheet,vehiculosSpriteSheet_Coords,&camiones[i].sprite);
    }
}

void ActualizarEstadoJugadores(){
    for(int i = 0; i < jugadoresActuales; i++){
        //Si la rana del jugador está saltando...
        if(jugadores[i].isJumping){
            // Durante la duración del salto, irá avanzando su velocidad en cada iteración distanciaSalto/duracionSalto
            if(HacerDuranteX(&jugadores[i].tempSalto,jugadores[i].duracionSalto) && !ComprobarPosicionFinal(jugadores[i].sprite.collider, jugadores[i].finSalto)){
                MoveCollider(&jugadores[i].sprite.collider, jugadores[i].direccion, jugadores[i].velocidadSalto);
            }else{
                jugadores[i].isJumping = false;
                jugadores[i].sprite.indiceAnimacion = 0;
                ActualizarSprite(&ranaBaseSpriteSheet, ranasSpriteSheet_Coords, &jugadores[i].sprite);
            }
        }
    }
}

void ActualizarEstadoJuego(){
    //TO_DO
    // DetectarColisiones();
    ActualizarEstadoVehiculos();
    ActualizarEstadoJugadores();
}


//*** FUNCIONES DE DIBUJADO DE ELEMENTOS EN PANTALLA ***///
void DibujarArbustos(){
    int posInicial_X = 0;
    int posActual_X;
    int posFila1_Y = VENTANA_Y-(esat::SpriteHeight(arbustoSprite)*2);
    int posFila2_Y = VENTANA_Y-(esat::SpriteHeight(arbustoSprite)*8);
    for(int i = 0; i < 2; i++){
        posActual_X = posInicial_X;
        for(int arbusto = 0; arbusto < tamanyoFilaArbustos; arbusto++){
            if(i == 0){
                esat::DrawSprite(arbustoSprite, posActual_X, posFila1_Y);
            }else{
                esat::DrawSprite(arbustoSprite, posActual_X, posFila2_Y);
            }
            posActual_X += esat::SpriteWidth(arbustoSprite);
        }
    }
}

void DibujarVehiculos(){
    for(int i = 0; i < maxCamiones; i++){
        if(i < maxCochesAmarillos){
            esat::DrawSprite(cochesAmarillos[i].sprite.imagen, cochesAmarillos[i].sprite.collider.P1.x, cochesAmarillos[i].sprite.collider.P1.y);
        }

        if(i < maxTractores){
            esat::DrawSprite(tractores[i].sprite.imagen, tractores[i].sprite.collider.P1.x, tractores[i].sprite.collider.P1.y);
        }

        if(i < maxCochesRosas){
            esat::DrawSprite(cochesRosas[i].sprite.imagen, cochesRosas[i].sprite.collider.P1.x, cochesRosas[i].sprite.collider.P1.y);
        }
        
        if(i < maxCochesBlancos){
            esat::DrawSprite(cochesBlancos[i].sprite.imagen, cochesBlancos[i].sprite.collider.P1.x, cochesBlancos[i].sprite.collider.P1.y);
        }

        esat::DrawSprite(camiones[i].sprite.imagen, camiones[i].sprite.collider.P1.x, camiones[i].sprite.collider.P1.y);
    }
}

void DibujarJugadores(){
    for(int i = 0; i < jugadoresActuales; i++){
        esat::DrawSprite(jugadores[i].sprite.imagen, jugadores[i].sprite.collider.P1.x, jugadores[i].sprite.collider.P1.y);
    }
}

void DibujarJuego(){
    DibujarArbustos();
    DibujarVehiculos();
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
    esat::SpriteRelease(animMuerteSpriteSheet.spriteSheet);
    esat::SpriteRelease(ranaBaseSpriteSheet.spriteSheet);
    esat::SpriteRelease(ranaRosaSpriteSheet.spriteSheet);
    esat::SpriteRelease(ranaRojaSpriteSheet.spriteSheet);
    esat::SpriteRelease(vehiculosSpriteSheet.spriteSheet);
}

void LiberarSpritesBase(){
    esat::SpriteRelease(arbustoSprite);
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
    LiberarSpritesJugadores();
    LiberarSpritesBase();
    LiberarSpriteSheets();
}
/* FIN FUNCIONALIDADES*/

/* INICIO MAIN */
int esat::main(int argc, char **argv) {
    GenerarSemillaAleatoria();

    esat::WindowInit(VENTANA_X,VENTANA_Y);
    WindowSetMouseVisibility(true);

    // Inicialización de todos aquellos recursos que necesitan ser precargados para
    // que otros los puedan usar
    InicializarSpriteSheets();
    InicializarSprites();

    // Inicialización de cosas interactivas durante la ejecución de JUEGO
    InicializarCarretera();
    InicializarJugadores();

    while(esat::WindowIsOpened() && !esat::IsSpecialKeyDown(esat::kSpecialKey_Escape)) {
        last_time = esat::Time(); 

        //Inicio dibujado de pantalla
        esat::DrawBegin();
        esat::DrawClear(0,0,0);

        //INPUT
        DetectarControles();
        //UPDATE
        ActualizarEstadoJuego();
        //DRAW
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