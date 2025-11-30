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
    bool isVisible = true, isActive = true; //isActive indica si está dispuesto a comprobar colisiones
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
struct AnimMuerteJugador{
    float duracionMuerte, tempMuerte = 0, velocidadAnimacion;
};

struct Jugador{
    Rana ranaJugador;
    int puntuacion = 0, vidas = 0;
    AnimMuerteJugador animMuerte;
};

struct Vehiculo{
    TipoVehiculo tipoVehiculo;
    Direccion direccion;
	Sprite sprite;
    float velocidadMovimiento;
};

struct Troncodrilo{
    Direccion direccion;
	Sprite sprite;
    float velocidadMovimiento;
    // Si no es un tronco, será un cocodrilo
    bool isTronco = true;
    int longTronco = 3;
};

struct Tortuga{
    Direccion direccion;
	Sprite sprite;
    float velocidadMovimiento;
    // Indica si la tortuga tiene la posibilidad de hundirse o no
    bool isSecure = true;
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

//-- Prototipado
bool areCollidersVisible = false;

//-- UI
Pantalla pantallaActual = INICIO;
int nivelActual = 0;

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

SpriteSheet tortugaSpriteSheet;
int tortugaSpriteSheet_Coords[12];

SpriteSheet troncoSpriteSheet;
int troncoSpriteSheet_Coords[6];

//-- Sprites | Declaración de los handles cuyos sprites: 
//  -Siempre serán iguales (Sin animación ni acceso multiple como los vehiculos)
//  -No necesitan collider
esat::SpriteHandle arbustoSprite;


//-- Jugadores
const unsigned char maxJugadores = 2;
unsigned char jugadoresActuales = 1;
Jugador jugadores[maxJugadores];

//-- Obstáculos
const int filasCarretera = 5;
const int maxCamiones = 8, maxCochesBlancos = 4, maxCochesAmarillos = 4;
const int maxCochesRosas = 5, maxTractores = 4;
Vehiculo camiones[maxCamiones], cochesBlancos[maxCochesBlancos], cochesAmarillos[maxCochesAmarillos]; 
Vehiculo cochesRosas[maxCochesRosas], tractores[maxTractores];

const int filasRio = 5;
const int maxTortugas_1 = 12, maxTroncos_1 = 9, maxTroncos_2 = 12;
const int maxTortugas_2 = 10, maxTroncos_3 = 12;


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

// Se guardan en sus structs correspondientes para facilitar su uso y legibilidad durante el proyecto
// Los archivos de hojas de sprite en .png se ubican en ./Recursos/Imagenes/SpriteSheets/*.png
void InicializarSpriteSheet(char rutaSpriteSheetPNG[], SpriteSheet *spriteSheet, int spriteSheet_Coords[], int tiposAnim, int indicesAnim, bool ignoreCoords = false){
    (*spriteSheet).spriteSheet = esat::SpriteFromFile(rutaSpriteSheetPNG);
    (*spriteSheet).tiposAnim = tiposAnim;
    (*spriteSheet).indicesAnim = indicesAnim;
    (*spriteSheet).coordsAnim = (*spriteSheet).indicesAnim * 2;
    (*spriteSheet).totalCoordsAnim = (*spriteSheet).tiposAnim * (*spriteSheet).coordsAnim;
    (*spriteSheet).spriteWidth = (esat::SpriteWidth((*spriteSheet).spriteSheet) / (*spriteSheet).indicesAnim);
    (*spriteSheet).spriteHeight = (esat::SpriteHeight((*spriteSheet).spriteSheet) / (*spriteSheet).tiposAnim);
    if(!ignoreCoords){
        InicializarCoordsSpriteSheet(&(*spriteSheet), spriteSheet_Coords);
    }
}

// Inicializa todos los valores de todos los SpriteSheets para ser utilizables durante el resto de la ejecución del programa
void InicializarSpriteSheets(){
    //Inicializa el SpriteSheet de la animación de muerte del jugador
    InicializarSpriteSheet(
        "./Recursos/Imagenes/SpriteSheets/AnimMuerteSpriteSheet.png",
        &animMuerteSpriteSheet,
        animMuerteSpriteSheet_Coords,
        1,
        7
    );

    // Inicializa los SpriteSheets de las ranas en todos sus colores y el array de sus coordenadas
    InicializarSpriteSheet(
        "./Recursos/Imagenes/SpriteSheets/RanaBaseSpriteSheet.png",
        &ranaBaseSpriteSheet,
        ranasSpriteSheet_Coords,
        4,
        2
    );
    InicializarSpriteSheet(
        "./Recursos/Imagenes/SpriteSheets/RanaRosaSpriteSheet.png",
        &ranaRosaSpriteSheet,
        ranasSpriteSheet_Coords,
        4,
        2,
        true
    );
    InicializarSpriteSheet(
        "./Recursos/Imagenes/SpriteSheets/RanaRojaSpriteSheet.png",
        &ranaRojaSpriteSheet,
        ranasSpriteSheet_Coords,
        4,
        2,
        true
    );

    // Inicializa el SpriteSheet de los vehiculos y su array de coordenadas
    InicializarSpriteSheet(
        "./Recursos/Imagenes/SpriteSheets/VehiculosSpriteSheet.png",
        &vehiculosSpriteSheet,
        vehiculosSpriteSheet_Coords,
        1,
        6
    );

    // Inicializa el SpriteSheet de las tortugas y su array de coordenadas
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
    if (tipo == CAMION_BACK){
        printf("INICIALIZANDO CAMION_BACK\n");
        printf("IMAGEN %p\n",(*vehiculo).sprite.imagen);
        printf("tipoAnimacion %d\n",(*vehiculo).sprite.tipoAnimacion);
        printf("indiceAnimacion %d\n",(*vehiculo).sprite.indiceAnimacion);
    } 
}

//*** MANEJO DE INICIALIZACIÓN DE OBJETOS ***/
void InicializarRio(){
    //TO_DO
}

// Instancia los valores por defecto de los vehiculos al inicio de un nivel
void InicializarVehiculos(){
    // Se usa maxCamiones como límite ya que es el array de los obstaculos de carretera mas largo
    // Luego internamente se comprueba que el indice sea valido para cada columna (cantidad de vehiculos)
    for(int i = 0; i < maxCamiones; i++){
        // InicializarCochesAmarillos
        if(i < maxCochesAmarillos){
            InicializarVehiculo(
                &cochesAmarillos[i],
                COCHE_AMARILLO,
                IZQUIERDA,
                // Fila 3 empezando desde abajo
                3,
                // Si es el primero, aparece en el borde, si no detras del ultimo + el margen que se le asigne
                i == 0 ?  
                0: 
                cochesAmarillos[i-1].sprite.collider.P1.x+vehiculosSpriteSheet.spriteWidth, 
                //Velocidad
                1,
                // Si es el primero no se le asigna margen.
                i == 0 ? 0 : 2
            );
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
                tractores[i-1].sprite.collider.P1.x-vehiculosSpriteSheet.spriteWidth,
                1,
                i == 0 ? 0 : 2
            );
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
                cochesRosas[i-1].sprite.collider.P1.x+vehiculosSpriteSheet.spriteWidth, 
                1,
                i == 0 ? 0 : 2
            );
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
                cochesBlancos[i-1].sprite.collider.P1.x-vehiculosSpriteSheet.spriteWidth, 
                1,
                i == 0 ? 0 : 2
            );
        }

        // InicializarCamiones
        InicializarVehiculo(
            &camiones[i],
            i%2 == 0 ? CAMION_FRONT : CAMION_BACK,
            IZQUIERDA,
            7,
            i == 0 ?  
            0: 
            camiones[i-1].sprite.collider.P1.x+vehiculosSpriteSheet.spriteWidth, 
            1,
            // Si es el primero no se le asigna margen.
            // Si no es el primero, tampoco se le asigna margen si se quiere instanciar la parte trasera del camion
            i == 0 ? 0 : i%2==0 ? 2 : 0
        );
    }
}

// Instancia los valores por defecto del jugador
// Pensado para utilizarse después de cada muerte para ubicar al jugador en su posicion inicial
void SpawnJugador(Jugador *jugador){
    (*jugador).ranaJugador.sprite.isActive = true;
    (*jugador).ranaJugador.direccion = ARRIBA;
    (*jugador).ranaJugador.sprite.tipoAnimacion = 0;
    (*jugador).ranaJugador.sprite.indiceAnimacion = 0;
    (*jugador).ranaJugador.sprite.collider.P1 = {
        VENTANA_X/2,(float)(VENTANA_Y-ranaBaseSpriteSheet.spriteHeight*2)
    };
    (*jugador).ranaJugador.sprite.collider.P2 = {
        (*jugador).ranaJugador.sprite.collider.P1.x + ranaBaseSpriteSheet.spriteWidth,
        (*jugador).ranaJugador.sprite.collider.P1.y + ranaBaseSpriteSheet.spriteHeight
    };
    (*jugador).ranaJugador.isJumping = false;
    (*jugador).ranaJugador.finSalto = (*jugador).ranaJugador.sprite.collider; 
    (*jugador).ranaJugador.distanciaSalto = ranaBaseSpriteSheet.spriteHeight;
    (*jugador).ranaJugador.duracionSalto = 100;
    (*jugador).ranaJugador.velocidadSalto = ((*jugador).ranaJugador.distanciaSalto/((*jugador).ranaJugador.duracionSalto/(1000.0f/FPS)));
    (*jugador).ranaJugador.tempSalto = 0;
    ActualizarSprite(&ranaBaseSpriteSheet, ranasSpriteSheet_Coords, &(*jugador).ranaJugador.sprite);
}

// Instancia los valores por defecto de cada jugador
// Pensado para utilizarse al inicio de cada partida
void InicializarJugadores(){
    for(int i = 0; i < jugadoresActuales; i++){
        //Propiedades del jugador al inicio de la partida
        jugadores[i].ranaJugador.sprite.isActive = true;
        jugadores[i].puntuacion = false;
        jugadores[i].vidas = 3;

        jugadores[i].animMuerte.duracionMuerte = 2000;
        jugadores[i].animMuerte.tempMuerte = 0;
        // La velocidad de animación es cada cuanto debe de avanzar el frame de animacion para completarla en duracionMuerte
        jugadores[i].animMuerte.velocidadAnimacion = jugadores[i].animMuerte.duracionMuerte/animMuerteSpriteSheet.indicesAnim;

        //Spawn Rana en posición inicial del jugador
        SpawnJugador(&jugadores[i]);
    }
}

void InicializarNivel(){
    // Inicialización de cosas interactivas durante la ejecución del nivel
    InicializarRio();
    InicializarVehiculos();
    InicializarJugadores();
}

//*** INICIO DE ACCIONES DE LOS OBJETOS Y JUGADORES***//

// Dada una rana con posicion de finSalto, comprueba si puede realizar ese salto
// El comportamiento cambiará en función de si se indica que es una rana de jugador o bonus
bool IsSaltoRanaPosible(Rana rana, bool isPlayer = true){
    if(isPlayer){
        return (
            rana.finSalto.P1.x >=0 && rana.finSalto.P2.x <= VENTANA_X &&
            rana.finSalto.P1.y >= ranaBaseSpriteSheet.spriteHeight*2 && rana.finSalto.P2.y <= VENTANA_Y-ranaBaseSpriteSheet.spriteHeight
        );
    }else{
        // TO_DO
        return false;
    }
}
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

    // Si detecta que la posición final de salto, excede el borde de la pantalla de juego, 
    // no saltará
    if(IsSaltoRanaPosible(*rana)){
        (*rana).isJumping = true;
        (*rana).tempSalto = last_time;
        (*rana).sprite.tipoAnimacion = newDireccion;
        AvanzarSpriteAnimado(&ranaBaseSpriteSheet, ranasSpriteSheet_Coords, &(*rana).sprite);
    }else{
        (*rana).finSalto = (*rana).sprite.collider;
    }
}

void MatarJugador(Jugador *jugador){
    (*jugador).ranaJugador.sprite.isActive = false;
    (*jugador).ranaJugador.sprite.tipoAnimacion = 0;
    (*jugador).ranaJugador.sprite.indiceAnimacion = 0;
    (*jugador).animMuerte.tempMuerte = last_time;
    ActualizarSprite(&animMuerteSpriteSheet, animMuerteSpriteSheet_Coords, &(*jugador).ranaJugador.sprite);
}

//*** DETECCIÓN INPUT DEL JUGADOR ***//
void DetectarControles(){
    if(pantallaActual == JUEGO){
        // CONTROLES RANA_J1
        // El jugador podrá realizar acciones siempre que:
        //   - No esté saltando
        if(!jugadores[0].ranaJugador.isJumping){
            if(esat::IsSpecialKeyDown(esat::kSpecialKey_Up)){
                IniciarSaltoRana(&jugadores[0].ranaJugador, ARRIBA);
            }
            if(esat::IsSpecialKeyDown(esat::kSpecialKey_Right)){
                IniciarSaltoRana(&jugadores[0].ranaJugador, DERECHA);
            }
            if(esat::IsSpecialKeyDown(esat::kSpecialKey_Down)){
                IniciarSaltoRana(&jugadores[0].ranaJugador, ABAJO);
            }
            if(esat::IsSpecialKeyDown(esat::kSpecialKey_Left)){
                IniciarSaltoRana(&jugadores[0].ranaJugador, IZQUIERDA);
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
        nivelActual = 0;
        InicializarNivel();
    }
    //Alterna la visualización de los colliders
    if(esat::IsSpecialKeyDown(esat::kSpecialKey_Alt)){
        areCollidersVisible = !areCollidersVisible;
        if(areCollidersVisible){
            printf("SHOWING COLLIDERS\n");
        }else{
            printf("HIDDEN COLLIDERS\n");
        }
    }
}

/*** FUNCIONES DE ACTUALIZACIÓN DE ESTADO DEL JUEGO ***/

// Dados 2 collider, comprueba si hay colisión entre ellos
bool DetectarColision(Collider C1, Collider C2) {
  return (C1.P2.x >= C2.P1.x) &&
         (C1.P1.x <= C2.P2.x) &&
         (C1.P2.y >= C2.P1.y) &&
         (C1.P1.y <= C2.P2.y);
}


float GetFilaPantallaSprite(Sprite s){
    return (VENTANA_Y-s.collider.P1.y)/ranaBaseSpriteSheet.spriteHeight;
}
float GetColumnaPantallaSprite(Sprite s){
    return s.collider.P1.x/ranaBaseSpriteSheet.spriteWidth;
}

// Dados 2 collider, siendo el primero de un jugador, ajusta el radio de colision 
// y comprueba si hay colisión entre ellos
bool DetectarColisionJugador(Collider player_C, Collider object_C) {
  return (player_C.P2.x-6 > object_C.P1.x) &&
         (player_C.P1.x+6 < object_C.P2.x) &&
         (player_C.P2.y-6 > object_C.P1.y) &&
         (player_C.P1.y+6 < object_C.P2.y);
}

// Todos los vehiculos actuan de la misma forma contra la rana:
// La rana es atropellada y muere en el acto, activando todos los 
// eventos que deban ocurrir cuando esta fallece
void DetectarColisionJugadorFilaVehiculos(Jugador *jugador, Vehiculo vehiculos[], int totalVehiculos){
    for(int i = 0; i < totalVehiculos; i++){
        if(DetectarColisionJugador((*jugador).ranaJugador.sprite.collider,vehiculos[i].sprite.collider)){
            MatarJugador(&(*jugador));
        }
    }
}

// Dado un sprite y una fila, devuelve verdadero si debería comprobar la fila pasada por parametro y sus adyacentes al estar el sprite ubicado en una de ellas
bool ComprobarFilaActualYAdyacentes(Sprite sprite, int fila){
    return(GetFilaPantallaSprite(sprite) >= fila-1 && GetFilaPantallaSprite(sprite) <= fila+1);
}

// En función de donde se ubica la rana del jugador, comprueba las colisiones de la fila de la pantalla donde se encuentra y
// de las adyacentes debido a sus posibles movimientos futuros.
// Esto permite no tener que comprobar las colisiones de absolutamente todos los obstaculos
void ComprobarColisionesJugador(Jugador *jugador){
    //Recorre las filas con posibles interacciones con el jugador si está activo
    if((*jugador).ranaJugador.sprite.isActive){
        for(int filaComprobar = 3; filaComprobar < VENTANA_Y/ranaBaseSpriteSheet.spriteHeight - 1; filaComprobar++){
            if(ComprobarFilaActualYAdyacentes((*jugador).ranaJugador.sprite,filaComprobar)){
                // Si es una fila que en la que se encuentra el jugador o es adyacente, comprueba las colisiones 
                // relacionadas con el jugador correspondientes
                switch(filaComprobar){
                    //FINAL
                    case 14:
                        // printf("FINAL\n");
                    break;
                    // RIO
                    case 13:
                        // printf("FILA_RIO_5\n");
                    break;
                    case 12:
                        // printf("FILA_RIO_4\n");
                    break;
                    case 11:
                        // printf("FILA_RIO_3\n");
                    break;
                    case 10:
                        // printf("FILA_RIO_2\n");
                    break;
                    case 9:
                        // printf("FILA_RIO_1\n");
                    break;

                    //CARRETERA
                    case 7:
                        // printf("CAMION\n");
                        DetectarColisionJugadorFilaVehiculos(&(*jugador),camiones, maxCamiones);
                    break;
                    case 6:
                        // printf("COCHE BLANCO\n");
                        DetectarColisionJugadorFilaVehiculos(&(*jugador),cochesBlancos, maxCochesBlancos);
                    break;
                    case 5:
                        // printf("COCHE ROSA\n");
                        DetectarColisionJugadorFilaVehiculos(&(*jugador),cochesRosas, maxCochesRosas);
                    break;
                    case 4:
                        // printf("TRACTOR\n");
                        DetectarColisionJugadorFilaVehiculos(&(*jugador),tractores, maxTractores);
                    break;
                    case 3:
                        // printf("COCHE AMARILLO\n");
                        DetectarColisionJugadorFilaVehiculos(&(*jugador),cochesAmarillos, maxCochesAmarillos);
                    break;
                }
            }
        }
        // printf("\n");
    }

}

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

//Actualización del estado de la animacion de muerte del jugador 
void ActualizarMuerteRanaJugador(Jugador *jugador){
    if(HacerCadaX(&(*jugador).animMuerte.tempMuerte, (*jugador).animMuerte.velocidadAnimacion)){
        if((*jugador).ranaJugador.sprite.indiceAnimacion >= animMuerteSpriteSheet.totalCoordsAnim-2){
            SpawnJugador(&(*jugador));
        }else{
            AvanzarSpriteAnimado(&animMuerteSpriteSheet, animMuerteSpriteSheet_Coords, &(*jugador).ranaJugador.sprite);
        }
    }
}

void ActualizarSaltoRana(Rana *rana){
    // Durante la duración del salto, irá avanzando su velocidad en cada iteración distanciaSalto/duracionSalto
    if(HacerDuranteX(&(*rana).tempSalto,(*rana).duracionSalto) && !ComprobarPosicionFinal((*rana).sprite.collider, (*rana).finSalto)){
        MoveCollider(&(*rana).sprite.collider, (*rana).direccion, (*rana).velocidadSalto);
    }else{
        (*rana).isJumping = false;
        (*rana).sprite.indiceAnimacion = 0;
        ActualizarSprite(&ranaBaseSpriteSheet, ranasSpriteSheet_Coords, &(*rana).sprite);
    }
}

void ActualizarEstadoJugadores(){
    for(int i = 0; i < jugadoresActuales; i++){
        ComprobarColisionesJugador(&jugadores[i]);

        if(!jugadores[i].ranaJugador.sprite.isActive){
            //Si la rana del jugador está muerta...
            ActualizarMuerteRanaJugador(&jugadores[i]);
        }else{
            //En caso de que no...

            //Si la rana está saltando actualizará el estado del salto
            if(jugadores[i].ranaJugador.isJumping){
                ActualizarSaltoRana(&jugadores[i].ranaJugador);
            }
        }
    }
}

void ActualizarEstadoJuego(){
    //TO_DO
    ActualizarEstadoVehiculos();
    ActualizarEstadoJugadores();
}

void ActualizarEstados(){
    switch(pantallaActual){
        case INICIO:
            break;
        case MENU:
            break;
        case JUEGO:
            ActualizarEstadoJuego();
            break;
    }
}


//*** FUNCIONES DE DIBUJADO DE ELEMENTOS EN PANTALLA ***///
void DrawCollider(Collider collider){
    float coords[10] = {
        collider.P1.x,
        collider.P1.y,

        collider.P1.x,
        collider.P2.y,
        
        collider.P2.x,
        collider.P2.y,
        
        collider.P2.x,
        collider.P1.y,

        collider.P1.x,
        collider.P1.y,
    };
    esat::DrawSetStrokeColor(0,255,0);
    esat::DrawPath(coords,5);
}

void DrawSprite(Sprite sprite, float x, float y, bool isPlayer = false){
    if(areCollidersVisible && sprite.isActive){
        if(isPlayer){
            DrawCollider({
                    {sprite.collider.P1.x+6, sprite.collider.P1.y+6},
                    {sprite.collider.P2.x-6, sprite.collider.P2.y-6}
                }
            );
        }else{
            DrawCollider(sprite.collider);
        }
    }

    if(sprite.isVisible){
        esat::DrawSprite(sprite.imagen, x, y);
    }
}

void DibujarFondoRio(){
    float coords[10] = {0,0,0,VENTANA_Y/2,VENTANA_X,VENTANA_Y/2,VENTANA_X,0,0,0};
    esat::DrawSetFillColor(0,0,70);
    esat::DrawSolidPath(coords,5);
}

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
            DrawSprite(cochesAmarillos[i].sprite, cochesAmarillos[i].sprite.collider.P1.x, cochesAmarillos[i].sprite.collider.P1.y);
        }

        if(i < maxTractores){
            DrawSprite(tractores[i].sprite, tractores[i].sprite.collider.P1.x, tractores[i].sprite.collider.P1.y);
        }

        if(i < maxCochesRosas){
            DrawSprite(cochesRosas[i].sprite, cochesRosas[i].sprite.collider.P1.x, cochesRosas[i].sprite.collider.P1.y);
        }
        
        if(i < maxCochesBlancos){
            DrawSprite(cochesBlancos[i].sprite, cochesBlancos[i].sprite.collider.P1.x, cochesBlancos[i].sprite.collider.P1.y);
        }

        DrawSprite(camiones[i].sprite, camiones[i].sprite.collider.P1.x, camiones[i].sprite.collider.P1.y);
    }
}

void DibujarJugadores(){
    for(int i = 0; i < jugadoresActuales; i++){
        DrawSprite(jugadores[i].ranaJugador.sprite, jugadores[i].ranaJugador.sprite.collider.P1.x, jugadores[i].ranaJugador.sprite.collider.P1.y, true);
    }
}

void DibujarJuego(){
    DibujarArbustos();
    DibujarVehiculos();
    DibujarJugadores();
}

// Es la única función que se debe llamar en el main. 
// Se ocupa del dibujado de todos los elementos en pantalla mediante subrutinas
void DibujarEntorno(){
    // Se dibuja primero el fondo del rio para que siempre aparezca por debajo de todos los
    // demás dibujados
    DibujarFondoRio();

    // Dibuja todo lo que aparezca en la zona central de la ventana
    // En función de la pantalla seleccionada
    switch(pantallaActual){
        case INICIO:
            break;
        case MENU:
            break;
        case JUEGO:
            DibujarJuego();
            break;
    }

    // Se dibuja lo ultimo para asegurarnos de que se muestra por encima 
    // de todo como la UI que es
    //TO_DO
    // DibujarCabecera()
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

void LiberarSpritesVehiculos(){
    for(int i = 0; i < maxCochesAmarillos; i++){
        if(cochesAmarillos[i].sprite.imagen != NULL){
            esat::SpriteRelease(cochesAmarillos[i].sprite.imagen);
        }
    }
    for(int i = 0; i < maxTractores; i++){
        if(tractores[i].sprite.imagen != NULL){
            esat::SpriteRelease(tractores[i].sprite.imagen);
        }
    }
    for(int i = 0; i < maxCochesRosas; i++){
        if(cochesRosas[i].sprite.imagen != NULL){
            esat::SpriteRelease(cochesRosas[i].sprite.imagen);
        }
    }
    for(int i = 0; i < maxCochesBlancos; i++){
        if(cochesBlancos[i].sprite.imagen != NULL){
            esat::SpriteRelease(cochesBlancos[i].sprite.imagen);
        }
    }
    for(int i = 0; i < maxCamiones; i++){
        if(camiones[i].sprite.imagen != NULL){
            esat::SpriteRelease(camiones[i].sprite.imagen);
        }
    }
}

void LiberarSpritesJugadores(){
    for(int i = 0; i < maxJugadores; i++){
        if(jugadores[i].ranaJugador.sprite.imagen != NULL){
            esat::SpriteRelease(jugadores[i].ranaJugador.sprite.imagen);
        }
    }
}

//Libera de memoria todos los spriteHandle inicializados
void LiberarSprites(){
    LiberarSpritesVehiculos();
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


    while(esat::WindowIsOpened() && !esat::IsSpecialKeyDown(esat::kSpecialKey_Escape)) {
        last_time = esat::Time(); 

        //Inicio dibujado de pantalla
        esat::DrawBegin();
        esat::DrawClear(0,0,0);

        //INPUT
        DetectarControles();
        //UPDATE
        ActualizarEstados();
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