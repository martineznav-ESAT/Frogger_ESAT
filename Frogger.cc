#include <esat/window.h>
#include <esat/input.h>
#include <esat/time.h>
#include <esat/sprite.h>
#include <esat/draw.h>

#include <stdlib.h>
#include <time.h>
#include <stdio.h>

/* Enums */

//Facilita la gestión de las pantallas, cambiar entre ellas y acciones posibles en las mismas
enum Pantalla {
    INTRO,
    PUNTUACIONES,
    INSERT,
    RANKING,
    START,
    JUEGO
};

// Principalmente utilizado para facilitar los movimientos de los elementos, detección de colisiones y cambios de sprites
enum Direccion {
    ARRIBA,
    DERECHA,
    ABAJO,
    IZQUIERDA
};

// Facilita el uso de una de las formas de uso de la función propia de dibujado de texto llamada DrawText. Indica la alineación del texto en horizontal
enum Align_H{
    LEFT,
    CENT,
    RIGHT,
};

// Facilita el uso de una de las formas de uso de la función propia de dibujado de texto llamada DrawText. Indica la alineación del texto en vertical
enum Align_V{
    TOP,
    MID,
    BOT
};

// Ayuda a determinar sobre qué fila del río se está trabajando para poder actuar en consecuencia. El propio valor del enum también se puede utilizar para indicar por ejemplo una coordenada en Y o en caso de FILA_RIO_T para indicar el total de filas de río
enum FilaRio{
    FILA_RIO_0,
    FILA_RIO_1,
    FILA_RIO_2,
    FILA_RIO_3,
    FILA_RIO_4,
    FILA_RIO_5,
    FILA_RIO_T
};

// Facilita la selección del valor indiceAnimacion de los vehículos para dibujar el vehículo. Diferenciación de vehículo.
enum TipoVehiculo {
    COCHE_AMARILLO,
    TRACTOR,
    COCHE_ROSA,
    COCHE_BLANCO,
    CAMION_FRONT,
    CAMION_BACK
};

// Indica el estado de la estructura MoscaCroc para gestionar su actualización de estado.
enum EstadoMoscaCroc{
    MOSCA,
    CROC,
    PUNTOS_MC //Puntos MoscaCroc
};

// Indica el estado de la estructura RanaBonus para gestionar su actualización de estado.
enum EstadoRanaBonus{
    LIBRE,
    ANCLADA,
    PUNTOS_RB //Puntos RanaBonus
};

// Indica el modo de juego seleccionado por el usuario. Sirve para la gestión de variaciones de eventos como por ejemplo la decisión de cuándo termina una partida
enum Gamemode{
    NORMAL_SINGLE,
    NORMAL_MULTI,
};

/* STRUCTS */

// Estructura que almacena 2 números flotantes pensados para indicar coordenadas ‘x’ e ‘y’ en un plano de 2 dimensiones como es la pantalla de juego.
struct PuntoCoord{
    float x = 0, y = 0;
};

// Estructura que almacena los valores en formato RGBa de un color. El valor transparencia  alpha “a” por defecto será 255 para que sea visible.
struct Color{
    unsigned char r, g, b, a = 255;
};

// Se encarga de almacenar 2 estructuras PuntoCoord con las que ser capaz de ubicar un rectángulo de coordenadas. Principalmente utilizado para la detección de colisiones aunque por su naturalidad también puede ser utilizado para determinar posiciones o dibujar rectángulos
struct Collider{
    // P1 Tambien sirve como ubicación
    PuntoCoord P1 = {0.0f,0.0f};
    PuntoCoord P2 = {48.0f,48.0f};
};

// Estructura que se compone de los valores necesarios para manejar la subdivisión de una imagen en formato SpriteSheet 
struct SpriteSheet{
    //SpriteSheet
    esat::SpriteHandle spriteSheet;
    //Filas y columnas del SpriteSheet
    unsigned char tiposAnim, indicesAnim;
    //Total Coordenadas X,Y de acceso al sprite deseado. Sin contar todos los tipos de animacion
    unsigned char coordsAnim;
    //Total Coordenadas X,Y de acceso al sprite deseado. Contando todos los tipos de animacion
    unsigned char totalCoordsAnim;
    //Tamaño de los sprites
    int spriteWidth,spriteHeight;
};

// Estructura que se compone de los valores necesarios para manejar un sprite junto a sus posibles animaciones, visibilidad y el estado de colisiones.
struct Sprite{
    esat::SpriteHandle imagen;
    Collider collider;
    // tipoAnimacion -> El set de indiceAnimacion a usar (Fila en spritesheet);
    // indiceAnimacion -> Secuencia de la animación a usar (Columna en spritesheet);
    unsigned char tipoAnimacion = 0, indiceAnimacion = 0;
    bool isVisible = true, isActive = true; //isActive indica si está dispuesto a comprobar colisiones
};

// Contiene información sobre el control de tiempo de una animación
// duracion     -> La duración total de la animacion antes de reiniciarse/finalizar
// velocidad    -> El tiempo que tarda en avanzar de frame. Se calcula dividiendo la duracion entre la cantidad de frames de la animacion. Este es el uso general, pero puede ser utilizado de forma creativa
// temporizador -> Utilizado por la funcion HacerCadaX/HacerDuranteX. Contiene la información del reloj de sistema necesaria para realizar correctamente el contador. Importante igualar a last_time una única vez antes de usar el contador
struct Animacion{
    // La duración total de la animacion antes de reiniciarse/finalizar
    float duracion; 
    // El tiempo que tarda en avanzar de frame. Se calcula dividiendo la duracion entre la cantidad de frames de la animacion 
    float velocidad; 
    // Pensado para el uso de la función HacerCadaX/HacerDuranteX. 
    // Contiene la información del reloj de sistema necesaria para realizar correctamente el contador. Importante igualar a global last_time una única vez antes de usar el contador
    float temporizador = 0;
};

// Contiene información sobre el contador que calcula los puntos 
// extra por tiempo y el limite del mismo que tienes para llegar al final con una rana
struct Cronometro{
    // Estructura de animación para gestión del tiempo
    Animacion animCronometro;
    // Almacena el valor actual del contador en décimas de segundo (ds)
    int contador = 60;
    // Almacena el valor en (ds) a mostrar cuando se alcanza una meta para que el contador pueda seguir funcionando sin sobrescribirse
    int tiempoRestante = 60;
    // Indica si la barra debe continuar avanzando o no, o lo que es lo mismo, si debe actualizar el estado del cronómetro. No para el tiempo.
    bool isBarraParada = false;
    // Indica si debe mostrarse el tiempo restante con el que se ha calculado la puntuación extra
    bool isTiempoVisible = false;
};

// Almacena los valores necesarios para el comportamiento de una rana. Independientemente del tipo de rana
struct Rana{
    // Indica la dirección a la que apunta
	Direccion direccion;
    // Estructura del sprite a mostrar
	Sprite sprite;
    // Indica si está saltando o no
    bool isJumping = false;
    // Se encarga de conservar la coordenada final en la que debe aterrizar al saltar para asegurar su correcto movimiento y comprobar la validez del salto.
    Collider finSalto;
    // Indica la distancia del salto en píxeles. Sirve para calcular la velocidad de su animación de salto en base a su duración.
    int distanciaSalto;
    // Estructura de animación que almacena la información necesaria para realizar el salto en los tiempos correspondientes
    Animacion animSalto;
};

// Almacena la información necesaria para gestionar un jugador, tanto para su movimiento como su puntuación y nivel actual.
struct Jugador{
    // Estructura de la rana que corresponde al jugador
    Rana ranaJugador;
    //Puntuaciones -> Actual y la que tiene en cuenta la posible vida extra
    int puntuacion = 0, puntuacion_VE = 0;
    //Vidas actuales del jugador
    int vidas = 0;
    // Número del nivel actual (max 255)
    unsigned char nivelActual = 1;
    // Dificultad actual del nivel (1-5)
    unsigned char dificultadActual = 1;
    // Ultima fila alcanzada en el nivel actual
    int filaPuntuacion = 0;
    // Valores de animación de la muerte del jugador
    Animacion animMuerte;
};

// Almacena la información necesaria para gestionar la rana bonus, tanto su movimiento como su puntuación y estado actual.
struct RanaBonus{
    // Estructura de la rana que corresponde a la rana bonus
    Rana rana;
    // Puntuación que ofrece al jugador al ser rescatada
    int puntuacion = 200;
    // Almacena el valor de dirección previo a empezar a rotar para saber hacia dónde continuar
    Direccion direccionRotacion;
    // Indica si debe dibujarse con la paleta de colores de bug o la normal
    bool isBug;
    // Indica el estado de actualización de la rana bonus
    EstadoRanaBonus estadoUpdate;
    // Almacena la información de cada cuanto tiempo debe de saltar por su cuenta o en su defecto, cada cuanto tiempo debe ejecutar una acción de rotación ya que comparten tiempo de ejecución
    Animacion accionarSalto;
    // Almacena la información de cuánto tiempo debe permanecer en pantalla mostrándose como la puntuación que ha ofrecido al jugador
    Animacion animPuntos;
    // Almacena el valor del índice del array de troncos del tronco sobre el que se encuentra la rana bonus para gestionar us comprobación de salto e invocación
    int indiceTroncoFin = 0;
};

// Almacena la información necesaria para la gestión e inicialización de un vehículo.
struct Vehiculo{
    // Indica que tipo de vehículo es para su gestión
    TipoVehiculo tipoVehiculo;
    // Indica la dirección de movimiento
    Direccion direccion;
    // Información del sprite a mostrar
	Sprite sprite;
    // Velocidad de movimiento del vehículo (px/frame)
    float velocidadMovimiento;
};

// Almacena la información necesaria para la gestión e inicialización de un tronco/cocodrilo.
struct Troncodrilo{
    // Indica la dirección de movimiento
    Direccion direccion;
    // Información del sprite a mostrar
	Sprite sprite;
    // Velocidad de movimiento del tronco/cocodrilo (px/frame)
    float velocidadMovimiento;
    // Indica si es o no un tronco. Si no es un tronco, será un cocodrilo
    bool isTronco = true;
    // Almacena cada cuanto tiempo debe abrir y cerrar la boca en caso de ser un cocodrilo
    Animacion animBocaCocodrilo;
};

// Almacena la información necesaria para la gestión e inicialización de una unidad de tortuga.
struct Tortuga{
    // Indica la dirección de movimiento
    Direccion direccion;
    // Información del sprite a mostrar
	Sprite sprite;
    // Velocidad de movimiento de la tortuga (px/frame)
    float velocidadMovimiento;
    // Indica si la tortuga tiene la habilidad de sumergirse o no
    bool isSumergible = true;
    // Indica si la tortuga se está sumergiendo o no para la gestión de la animación de la misma
    bool isSumergiendo = true;
    // Almacena la información de la animación de hundimiento
    Animacion animSumergir;
};

// Almacena la información necesaria para la gestión e inicialización de una serpiente
struct Serpiente{
    // Indica la dirección de movimiento
    Direccion direccion;
    // Información del sprite a mostrar
	Sprite sprite;
    // Almacena la información de la animación de movimiento de la serpiente junto a su velocidad de movimiento
    Animacion animMovimiento;
    // Indica la dirección de la animación de movimiento. 
    // True -> La animación avanza | False -> La animación retrocede
    bool direccionAnimacion;
};

// Almacena la información necesaria para la gestión e inicialización de la nutria.
struct Nutria{
    // Indica la dirección de movimiento
    Direccion direccion;
    // Información del sprite a mostrar
	Sprite sprite;
    // Almacena la información de la animación de ataque junto a su velocidad de movimiento
    Animacion animMovimiento;
    // Almacena la información de las coordenadas donde efectuará su ataque
    // No dibuja nada, pero su información de isActive es útil para determinar proximidad y zona de ataque activa o inactiva
    Sprite colisionAtaque;
};

// Almacena la información necesaria para la gestión e inicialización de la mosca bonus y la boca de cocodrilo. Es la misma siempre pues sólo puede aparecer una de las 2 en pantalla
struct MoscaCroc{
    // Información del sprite a mostrar y si es ofensivo o no
	Sprite sprite;
    // Almacena la información de cada cuanto tiempo debe de aparecer uno de los 2 y en caso de la boca del cocodrilo, el atributo de velocidad indica cuánto tarda en salir la cabeza por completo
    Animacion animMoscaCroc;
    // Indica, en caso de obtener los puntos extra de la mosca, cuánto tiempo debe permanecer en pantalla la puntuación adquirida
    Animacion animPuntos;
    // Puntos que ofrece la mosca al ser obtenida
    int puntuacion = 200;
    // Almacena el estado de actualización.
    EstadoMoscaCroc estado = MOSCA;
};

/* FIN STRUCTS */

/* GLOBALES */
//-- Tamaños ventana
const int VENTANA_X = 672, VENTANA_Y = 768;
const unsigned char SPRITE_SIZE = 48;
const int VENTANA_COLUMNAS = VENTANA_X/SPRITE_SIZE +1, VENTANA_FILAS = VENTANA_Y/SPRITE_SIZE;

//-- FPS
const unsigned char FPS=60;
double last_time;

//-- Prototipado
bool areCollidersVisible = false;

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

SpriteSheet cocodriloSpriteSheet;
int cocodriloSpriteSheet_Coords[8];

SpriteSheet zonaFinalSpriteSheet;
int zonaFinalSpriteSheet_Coords[8];

SpriteSheet ranaFinSpriteSheet;
int ranaFinSpriteSheet_Coords[4];

SpriteSheet tituloSpriteSheet;
int tituloSpriteSheet_Coords[10];

SpriteSheet ranaIntroSpriteSheet;
int ranaIntroSpriteSheet_Coords[12];

SpriteSheet puntosSpriteSheet;
int puntosSpriteSheet_Coords[4];

SpriteSheet moscaCrocSpriteSheet;
int moscaCrocSpriteSheet_Coords[8];

SpriteSheet serpienteSpriteSheet;
int serpienteSpriteSheet_Coords[6];

SpriteSheet nutriaSpriteSheet;
int nutriaSpriteSheet_Coords[4];

//-- Sprites | Declaración de los handles cuyos sprites: 
//  -Siempre serán iguales (Sin animación ni acceso múltiple como el indicador de vidas)
//  -No necesitan collider
esat::SpriteHandle arbustoSprite;
esat::SpriteHandle indicadorNivelSprite;
esat::SpriteHandle vidaSprite;
esat::SpriteHandle copySprite;

//-- UI
const unsigned char FONT_SIZE = 24;
const unsigned char maxScoreDigits = 5, maxCreditDigits = 2, maxTimeDigits = 2;
int credits = 0;

const unsigned char maxLetrasTitulo = 7;
int letraActual = 0;
Rana letrasTitulo[maxLetrasTitulo];

const unsigned char maxRankingScores = 5;
int rankingScores[maxRankingScores] = {2350,10,0,0,0};

Pantalla pantallaActual = INTRO;

//Animaciones

Animacion animIntro = {37000,1000,0};
//Booleana para detectar si la animacion vertical de la introducción ha empezado
bool isAnimIntroVIniciada = false;
Animacion animPuntuaciones = {10000,1000,0};
Animacion animRanking = {6000,1000,0};
Animacion animInsert = {6000,1000,0};

Animacion animAvanceNivel;
bool isAnimAvanceNivel = false;

Animacion animGameOver;
bool isAnimGameOver = false;
Collider barridoGameOver_Azul;
Collider barridoGameOver_Negro;

// Duracion, velocidad de avance y temporizador del 
// contador para que el jugador llegue a una zona final.
// Contiene información tambien para indicar cosas relacionadas con el mismo por pantalla
Cronometro cronometro;

//-- Jugadores
const unsigned char maxJugadores = 2;
unsigned char jugadorActual = 0;
Gamemode gamemode = NORMAL_SINGLE;
Jugador jugadores[maxJugadores];

//-- Obstáculos
const int filasCarretera = 5;
const int maxCamiones = 8, maxCochesBlancos = 4, maxCochesAmarillos = 5;
const int maxCochesRosas = 5, maxTractores = 4;
Vehiculo camiones[maxCamiones], cochesBlancos[maxCochesBlancos], cochesAmarillos[maxCochesAmarillos]; 
Vehiculo cochesRosas[maxCochesRosas], tractores[maxTractores];

Tortuga tortugas_1[VENTANA_COLUMNAS], tortugas_2[VENTANA_COLUMNAS];
Troncodrilo troncos_1[VENTANA_COLUMNAS], troncos_2[VENTANA_COLUMNAS], troncos_3[VENTANA_COLUMNAS];

const int maxZonasFinales = 28;
Sprite zonasFinales[maxZonasFinales];

const int maxRanasFinales = 5;
Sprite ranasFinales[maxRanasFinales];

const int tamanyoFilaArbustos = VENTANA_COLUMNAS-1;

// Animales
// Solo permite un moscaCroc activo a la vez (sea mosca o croc (cocodrilo trampa)), no dos a la vez en distintos hogares.
MoscaCroc moscaCroc;

const int maxSerpientes = 2;
Serpiente serpientes[maxSerpientes];

Nutria nutria;

RanaBonus ranaBonus;

/* FIN GLOBALES */

/* FUNCIONALIDADES */
void ControlFPS(){
    double current_time;
    do{
        current_time = esat::Time();
    }while((current_time-last_time)<=1000.0/FPS);
}

// Devuelve el valor del contador del temporizador proporcionado en milisegundos
//  temp = temporizador del que se quiere obtener el contador
//  unidad = unidades en las que se quiere recuperar el contador. Por defecto lo devuelve en milisegundos. Como referencia, si unidad es 1000 entonces lo devolverá en segundos
float GetContadorFromTemp(float temp, int unidad = 1){
    return (last_time - temp)/unidad;
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
    isAccesible = GetContadorFromTemp((*temp)) > x;
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

//Genera un número del 0 al límite indicado -1
int GenerarNumeroAleatorio(int limite){
    return (rand()%limite);
}

// Dado un número entero, cada vez que se ejecuta esta función genera true o false en base a esa probabilidad.
// Si probabilidad == 24, entonces generará true con un 24% de posibilidades 
bool BoolPorProbabilidad(int probabilidad){
    // Se suma 1 al numero aleatorio generado para que simule un resultado de 1 a 100 y no de 0 a 99.
    // Mediante la comprobación probabilidad <= (1 | 100)
    // De esta manera las probabilidades se podrían generar de esta manera:
    // Probabilidad 2 -> 2 de cada 100 posibilidades -> 2%
    // Probabilidad 30 -> 30 de cada 100 posibilidades -> 30%
    // Probabilidad 100 -> 100 de cada 100 posibilidades -> 100%
    // ...
    int random = GenerarNumeroAleatorio(100)+1;
    // printf("probabilidad %d\n",probabilidad);
    // printf("random %d\n",random);

    return random <= probabilidad;
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

// Dado un sprite, devuelve su número de fila en posición vertical contando desde abajo
float GetFilaPantallaSprite(Sprite s){
    return (VENTANA_Y - s.collider.P1.y)/SPRITE_SIZE;
}
// Dado un sprite, devuelve su número de columna en posición horizontal contando desde la izquierda
float GetColumnaPantallaSprite(Sprite s){
    return s.collider.P1.x/SPRITE_SIZE;
}

//Dada una coordenada (formato collider) actual y una esperada final, devuelve true si son iguales y false si son distintas
bool AreColliderEqual(Collider c1, Collider c2){
    return (
        c1.P1.x == c2.P1.x && c1.P2.x == c2.P2.x &&
        c1.P1.y == c2.P1.y && c1.P2.y == c2.P2.y
    );
}

bool ComprobarSalidaVentanaSprite(Sprite sprite, Direccion direccion){
    bool isOut = false;
    switch(direccion){
        case ARRIBA:
            isOut = sprite.collider.P2.y < 0;
        break;
        case DERECHA:
            isOut = sprite.collider.P1.x > VENTANA_X;
        break;
        case ABAJO:
            isOut = sprite.collider.P1.y > VENTANA_Y;
        break;
        case IZQUIERDA:
            isOut = sprite.collider.P2.x < 0;
        break;
    }

    return isOut;
}

bool ComprobarSalidaVentanaCollider(Collider *collider, Direccion direccion){
    bool isOut = false;
    switch(direccion){
        case ARRIBA:
            isOut = (*collider).P2.y < 0;
        break;
        case DERECHA:
            isOut = (*collider).P1.x > VENTANA_X;
        break;
        case ABAJO:
            isOut = (*collider).P1.y > VENTANA_Y;
        break;
        case IZQUIERDA:
            isOut = (*collider).P2.x < 0;
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
    InicializarSpriteSheet(
        "./Recursos/Imagenes/SpriteSheets/TortugaSpriteSheet.png",
        &tortugaSpriteSheet,
        tortugaSpriteSheet_Coords,
        1,
        6
    );

    // Inicializa el SpriteSheet de los troncos y su array de coordenadas
    InicializarSpriteSheet(
        "./Recursos/Imagenes/SpriteSheets/TroncoSpriteSheet.png",
        &troncoSpriteSheet,
        troncoSpriteSheet_Coords,
        1,
        3
    );

    // Inicializa el SpriteSheet de los cocodrilos y su array de coordenadas
    InicializarSpriteSheet(
        "./Recursos/Imagenes/SpriteSheets/CocodriloSpriteSheet.png",
        &cocodriloSpriteSheet,
        cocodriloSpriteSheet_Coords,
        1,
        4
    );

    // Inicializa el SpriteSheet de la zona final y su array de coordenadas
    InicializarSpriteSheet(
        "./Recursos/Imagenes/SpriteSheets/ZonaFinalSpriteSheet.png",
        &zonaFinalSpriteSheet,
        zonaFinalSpriteSheet_Coords,
        1,
        4
    );

    // Inicializa el SpriteSheet de la rana que aparece al llegar a una zona final y su array de coordenadas
    InicializarSpriteSheet(
        "./Recursos/Imagenes/SpriteSheets/RanaFinSpriteSheet.png",
        &ranaFinSpriteSheet,
        ranaFinSpriteSheet_Coords,
        1,
        2
    );

    // Inicializa el SpriteSheet del titulo del juego y su array de coordenadas
    InicializarSpriteSheet(
        "./Recursos/Imagenes/SpriteSheets/TituloSpriteSheet.png",
        &tituloSpriteSheet,
        tituloSpriteSheet_Coords,
        1,
        5
    );

    // Inicializa el SpriteSheet de las ranas que aparecen en la intro del juego
    InicializarSpriteSheet(
        "./Recursos/Imagenes/SpriteSheets/RanaIntroSpriteSheet.png",
        &ranaIntroSpriteSheet,
        ranaIntroSpriteSheet_Coords,
        2,
        3
    );

    // Inicializa el SpriteSheet de las puntuaciones de bonificacion
    InicializarSpriteSheet(
        "./Recursos/Imagenes/SpriteSheets/PuntosSpriteSheet.png",
        &puntosSpriteSheet,
        puntosSpriteSheet_Coords,
        1,
        2
    );

    // Inicializa el SpriteSheet del cocodrilo trampa
    InicializarSpriteSheet(
        "./Recursos/Imagenes/SpriteSheets/MoscaCrocSpriteSheet.png",
        &moscaCrocSpriteSheet,
        moscaCrocSpriteSheet_Coords,
        2,
        2
    );

    // Inicializa el SpriteSheet de la serpiente
    InicializarSpriteSheet(
        "./Recursos/Imagenes/SpriteSheets/SerpienteSpriteSheet.png",
        &serpienteSpriteSheet,
        serpienteSpriteSheet_Coords,
        1,
        3
    );

    // Inicializa el SpriteSheet de la nutria
    InicializarSpriteSheet(
        "./Recursos/Imagenes/SpriteSheets/NutriaSpriteSheet.png",
        &nutriaSpriteSheet,
        nutriaSpriteSheet_Coords,
        1,
        2
    );
}

void InicializarSprites(){
    arbustoSprite = esat::SpriteFromFile("./Recursos/Imagenes/Sprites/ArbustoSprite.png");
    indicadorNivelSprite = esat::SpriteFromFile("./Recursos/Imagenes/Sprites/IndicadorNivelSprite.png");
    vidaSprite = esat::SpriteFromFile("./Recursos/Imagenes/Sprites/VidaSprite.png");
    copySprite = esat::SpriteFromFile("./Recursos/Imagenes/Sprites/CopySprite.png");
}

//*** MANEJO DE SPRITES/SPRITESHEETS ***/
// Recupera el sprite del tipoAnimacion(Fila) e indiceAnimacion(Columna) de una spriteSheet
esat::SpriteHandle GetSpriteFromSheet(SpriteSheet spriteSheet, int spriteSheetCoords[], int tipoAnimacion, int indiceAnimacion){
    return (esat::SubSprite(
        spriteSheet.spriteSheet,
        spriteSheetCoords[GetIndiceArray(tipoAnimacion,spriteSheet.coordsAnim,indiceAnimacion)],
        spriteSheetCoords[GetIndiceArray(tipoAnimacion,spriteSheet.coordsAnim,indiceAnimacion+1)],
        spriteSheet.spriteWidth,
        spriteSheet.spriteHeight
    ));
}

// ** USAR SOLO CUANDO SE DESEA CAMBIAR DE SPRITE NO PARA DIBUJAR UNA IMAGEN DE FORMA REGULAR **
// Dada una estructura SpriteSheet y una estructura Sprite, actualiza la imagen
// del Sprite en base al SpriteSheet y el tipoAnimacion(Fila) e indiceAnimacion(Columna) que 
// tiene el Sprite
void ActualizarSprite(SpriteSheet spriteSheet, int spriteSheetCoords[], Sprite *sprite){
    // Mediante la variable local buffer, se asegura de liberar la imagen previa que tenia asignada el Sprite
    // para prevernir leaks de memoria
    esat::SpriteHandle buffer = (*sprite).imagen;
    (*sprite).imagen = GetSpriteFromSheet(spriteSheet, spriteSheetCoords, (*sprite).tipoAnimacion,(*sprite).indiceAnimacion);

    if(buffer != NULL){
        esat::SpriteRelease(buffer);
    }
}

// ** USAR SOLO CUANDO SE DESEA AVANZAR EL INDICE DE ANIMACIÓN NO PARA ACTUALIZAR EL SPRITE O DIBUJARLO DE FORMA REGULAR **
// Dada una estructura SpriteSheet y una estructura Sprite.
// Avanza el indice de animación del sprite en +2 para que el siguiente Sprite que guarde sea el correspondiente
// al siguiente en su animación. Si el indice es mayor del total de coordenadas, lo reinicia 0 para volver a comenzar la animación
void AvanzarSpriteAnimado(SpriteSheet spriteSheet, int spriteSheetCoords[], Sprite *sprite){
    // Avanza indice de animación. 
    (*sprite).indiceAnimacion += 2;
    // Si despues de sumar 2, el indice es mayor o igual a las columnas totales de coordenadas
    // de animacion, lo instancia a 0 para reiniciar la animacion
    if((*sprite).indiceAnimacion >= (spriteSheet).coordsAnim){
        (*sprite).indiceAnimacion = 0;
    }
}

// Dada una estructura SpriteSheet y una estructura Sprite.
// Retrocede el indice de animación del sprite en -2 para que el siguiente Sprite que guarde sea el correspondiente
// al anterior en su animación. Si el indice es menor de 0, lo reinicia al total de coordenadas para volver a comenzar la animación
void RetrocederSpriteAnimado(SpriteSheet spriteSheet, int spriteSheetCoords[], Sprite *sprite){
    // Avanza indice de animación. 
    (*sprite).indiceAnimacion -= 2;
    // Si despues de restar 2, el indice es menor de 0
    // lo instancia a la ultima coordenada de animacion -1 para reiniciar la animacion a la inversa
    if((*sprite).indiceAnimacion < 0){
        (*sprite).indiceAnimacion = (spriteSheet).coordsAnim-1;
    }
}

//Reubica un sprite dado un punto de coordenada
void RellocateSprite(Sprite *sprite, PuntoCoord nuevaUbicacion){
    (*sprite).collider.P1 = nuevaUbicacion;
    (*sprite).collider.P2 = {nuevaUbicacion.x + esat::SpriteWidth((*sprite).imagen), nuevaUbicacion.y + esat::SpriteHeight((*sprite).imagen)};
}

// Version de RellocateSprite con un spriteSheet para usar el ancho del mismo en caso de estar moviendo un sprite sin imagen 
// (Colision de ataque de la nutria por ejemplo)
void RellocateSprite(Sprite *sprite, PuntoCoord nuevaUbicacion, SpriteSheet spriteSheet){
    (*sprite).collider.P1 = nuevaUbicacion;
    (*sprite).collider.P2 = {nuevaUbicacion.x + spriteSheet.spriteWidth, nuevaUbicacion.y + spriteSheet.spriteHeight};
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
            nuevaUbicacion = {(*sprite).collider.P1.x,0.0f-esat::SpriteHeight((*sprite).imagen)};
        break;
        case IZQUIERDA:
            nuevaUbicacion = {VENTANA_X,(*sprite).collider.P1.y};
        break;
    }
    RellocateSprite(&(*sprite), nuevaUbicacion);
}

// Version de RellocateSprite con un spriteSheet para usar el ancho del mismo en caso de estar moviendo un sprite sin imagen 
// (Colision de ataque de la nutria por ejemplo)
void RellocateCollider(Collider *collider, PuntoCoord nuevaUbicacion){
    (*collider).P2 = {nuevaUbicacion.x + ((*collider).P2.x-(*collider).P1.x), nuevaUbicacion.y + ((*collider).P2.y-(*collider).P1.y)};
    (*collider).P1 = nuevaUbicacion;
}

// Ubica el collider en el borde opuesto de la dirección que se indica ya que se asume que se ha escapado
// por ese borde y se desea reubicar en el lugar contrario
//   - *collider        -> El collider a actualizar. El P1 de un collider indica también el punto de inicio de dibujado
//   - direccion        -> Indica en que dirección se moverá
void RellocateColliderOnBorderEscape(Collider *collider, Direccion direccion){
    PuntoCoord nuevaUbicacion;
    switch(direccion){
        case ARRIBA:
            nuevaUbicacion = {(*collider).P1.x,VENTANA_Y};
        break;
        case DERECHA:
            nuevaUbicacion = {0.0f-((*collider).P2.x-(*collider).P1.x),(*collider).P1.y};
        break;
        case ABAJO:
            nuevaUbicacion = {(*collider).P1.x,0.0f-((*collider).P2.y-(*collider).P1.y)};
        break;
        case IZQUIERDA:
            nuevaUbicacion = {VENTANA_X,(*collider).P1.y};
        break;
    }
    RellocateCollider(&(*collider), nuevaUbicacion);
}


// Version de RellocateSpriteOnBorderEscape con un spriteSheet para usar el ancho del mismo en caso de estar moviendo un sprite sin imagen 
// (Colision de ataque de la nutria por ejemplo)
void RellocateSpriteOnBorderEscape(Sprite *sprite, Direccion direccion, SpriteSheet spriteSheet){
    PuntoCoord nuevaUbicacion;
    switch(direccion){
        case ARRIBA:
            nuevaUbicacion = {(*sprite).collider.P1.x,VENTANA_Y};
        break;
        case DERECHA:
            nuevaUbicacion = {0.0f-spriteSheet.spriteWidth,(*sprite).collider.P1.y};
        break;
        case ABAJO:
            nuevaUbicacion = {(*sprite).collider.P1.x,0.0f-spriteSheet.spriteHeight};
        break;
        case IZQUIERDA:
            nuevaUbicacion = {VENTANA_X,(*sprite).collider.P1.y};
        break;
    }
    RellocateSprite(&(*sprite), nuevaUbicacion, spriteSheet);
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

//*** MANEJO DE INICIALIZACIÓN DE OBJETOS ***/
// Inicializa los sprites del rótulo del juego que aparecen en las pantallas de INTRO, PUNTUACION y RANKING asegurandose de que aparecen en la ubicacion correcta
// El parametro booleano indica si debe inicializarse en base a: 
//  -INTRO. Ubicando los sprites como ranas fuera de la pantalla
//  -PUNTUACION/RANKING Ubicando los sprites como el rótulo
void InicializarTitulo(bool isIntro = false){
    letraActual = 0;
    isAnimIntroVIniciada = false;

    for(int i = 0; i < maxLetrasTitulo; i++){
        letrasTitulo[i].sprite.isVisible = true;
        letrasTitulo[i].sprite.isActive = false;
        letrasTitulo[i].direccion = IZQUIERDA;
        letrasTitulo[i].animSalto.temporizador = last_time;
        // Para aprovechar el Struct de rana, se utiliza isJumping como indicador de 
        // Avance/Retroceso de frame de animación
        letrasTitulo[i].isJumping = true;

        // INICIALIZACIÓN DE SPRITES DE LETRAS EN FUNCIÓN DE SI 
        // ES LA PANTALLA INTRO O NO
        if(isIntro){
            letrasTitulo[i].sprite.tipoAnimacion = 1;
            letrasTitulo[i].sprite.indiceAnimacion = 0;
            letrasTitulo[i].sprite.collider = {
                {VENTANA_X,SPRITE_SIZE*9},
                {VENTANA_X+SPRITE_SIZE,SPRITE_SIZE*10}
            };

            if(i == 0){
                letrasTitulo[i].finSalto = {
                    {(i+2.0f)*SPRITE_SIZE,SPRITE_SIZE*9},
                    {(i+3.0f)*SPRITE_SIZE,SPRITE_SIZE*10}
                };
            }else{
                letrasTitulo[i].finSalto = {
                    {letrasTitulo[i-1].finSalto.P2.x+(SPRITE_SIZE/2),letrasTitulo[i-1].finSalto.P1.y},
                    {letrasTitulo[i-1].finSalto.P2.x+((SPRITE_SIZE/2)+SPRITE_SIZE),letrasTitulo[i-1].finSalto.P2.y},
                };
            }

            letrasTitulo[i].distanciaSalto = letrasTitulo[i].sprite.collider.P1.x-letrasTitulo[i].finSalto.P1.x;
            letrasTitulo[i].animSalto.velocidad = SPRITE_SIZE/2;

            if(i < 3){
                letrasTitulo[i].animSalto.duracion = 6000;
            }else{
                if(i < 5){
                    letrasTitulo[i].animSalto.duracion = 3000;
                }else{
                    letrasTitulo[i].animSalto.duracion = 2000;
                }
            }

            ActualizarSprite(ranaIntroSpriteSheet,ranaIntroSpriteSheet_Coords,&letrasTitulo[i].sprite);
        }else{
            letrasTitulo[i].sprite.tipoAnimacion = 0;

            if(i == 0){
                letrasTitulo[i].sprite.collider = {
                    {(i+2.0f)*SPRITE_SIZE,SPRITE_SIZE*3},
                    {(i+3.0f)*SPRITE_SIZE,SPRITE_SIZE*4}
                };
            }else{
                letrasTitulo[i].sprite.collider = {
                    {letrasTitulo[i-1].sprite.collider.P2.x+(SPRITE_SIZE/2),letrasTitulo[i-1].sprite.collider.P1.y},
                    {letrasTitulo[i-1].sprite.collider.P2.x+((SPRITE_SIZE/2)+SPRITE_SIZE),letrasTitulo[i-1].sprite.collider.P2.y},
                };
            }

            switch(i){
                case 0:
                    //AsignarLetra F
                    letrasTitulo[i].sprite.indiceAnimacion = 0;
                break;
                case 1:
                case 6:
                    //AsignarLetra R
                    letrasTitulo[i].sprite.indiceAnimacion = 2;    
                break;
                case 2:
                    //AsignarLetra O
                    letrasTitulo[i].sprite.indiceAnimacion = 4;
                break;
                case 3:
                case 4:
                    //AsignarLetra G
                    letrasTitulo[i].sprite.indiceAnimacion = 6;
                break;
                case 5:
                    //AsignarLetra E
                    letrasTitulo[i].sprite.indiceAnimacion = 8;
                break;
            }
            ActualizarSprite(tituloSpriteSheet,tituloSpriteSheet_Coords,&letrasTitulo[i].sprite);
        }
    }
}   

//Comprueba si el grupo de obstaculos puede generarse sin problemas en su fila correspondiente
bool CabeGrupoObstaculos(int posicionGrupo, int columna, int longitud){
    return posicionGrupo == 0 && columna + longitud < VENTANA_COLUMNAS || posicionGrupo != 0 && columna + (longitud - posicionGrupo) < VENTANA_COLUMNAS;
}

// Inicializar valores de las ranas que aparecen al llegar a una zona final donde:
// filaRio      -> Indica la fila del rio en la que se debe ubicar en Y
void InicializarRanasFinales(FilaRio filaRio){
    int posicionZona_X;
    for(int i = 0; i < maxRanasFinales; i++){
        ranasFinales[i].tipoAnimacion = 0;
        ranasFinales[i].indiceAnimacion = 0;
        ranasFinales[i].isVisible = false;
        ranasFinales[i].isActive = true;

        // Recupera la posición libre de la zona final
        posicionZona_X = (i*6+1)*zonaFinalSpriteSheet.spriteWidth;
        ranasFinales[i].collider.P1 = {((float)posicionZona_X),((VENTANA_Y-(SPRITE_SIZE*9))-(((float)filaRio)*SPRITE_SIZE))};
        ranasFinales[i].collider.P2 = {((float)posicionZona_X+ranaFinSpriteSheet.spriteWidth),((VENTANA_Y-(SPRITE_SIZE*8))-(((float)filaRio)*SPRITE_SIZE))};
        
        ActualizarSprite(ranaFinSpriteSheet, ranaFinSpriteSheet_Coords, &ranasFinales[i]);
    }
}

// Inicializar valores por defecto de las moscas bonus de las zonas finales / cocodrilo trampa:
void InicializarMoscaCroc(){
    // En el nivel uno, será una mosca siempre, por lo tanto lo inicializa a esto
    if(jugadores[jugadorActual].dificultadActual <= 1){
        moscaCroc.estado = MOSCA;
    }else{
        //En el resto de niveles, al inicializar BoolPorProbabilidad será true cuando sea un cocodrilo trampa
        moscaCroc.estado = BoolPorProbabilidad(jugadores[jugadorActual].dificultadActual*10) ? CROC : MOSCA;
    }
    //Puntos que ofrece
    moscaCroc.puntuacion = 200;
    //Valores de aparición de mosca y croc (La velocidad aquí se utilizará solo para determinar cuando cambia el croc de "aviso" a "letal")
    moscaCroc.animMoscaCroc = {4000,2000,0.0f};
    //En este caso la velocidad no se utiliza para la animacion ni para otro cometido
    moscaCroc.animPuntos = {3000,0,0.0f};

    //Inicializa imagen
    moscaCroc.sprite.indiceAnimacion = 0;
    moscaCroc.sprite.tipoAnimacion = 0;
    moscaCroc.sprite.isActive = false;
    moscaCroc.sprite.isVisible = false;
    // La ubicación inicial no importa que se incialice, con el valor por defecto del struct sirve
    // moscaCroc.sprite.collider = ;
    ActualizarSprite(moscaCrocSpriteSheet, moscaCrocSpriteSheet_Coords, &moscaCroc.sprite);
}

void AsignarVelocidadAleatoriaSerpiente(Serpiente *serpiente){
    int varianteVelocidad = GenerarNumeroAleatorio(3)+1;
    (*serpiente).animMovimiento.duracion = 150-(varianteVelocidad*20);
    (*serpiente).animMovimiento.velocidad = 0.25+(varianteVelocidad*0.25);
}

// Seleccionar ubicación de aparición y mover allí la serpiente
void SpawnSerpiente(Serpiente *serpiente){
    // printf("SPAWN SERPIENTE\n");
    AsignarVelocidadAleatoriaSerpiente(&(*serpiente));
    switch(GenerarNumeroAleatorio(jugadores[jugadorActual].dificultadActual >= 5 ? 8 : 3)){
        //Dificultad 5 -> (2/8)*100 -> 25% de posibilidades de salir en el arbusto -> 75% tronco 
        //Resto de dificultades (en principio 3 y 4) -> (2/3)*100 -> 66.6% de posibilidades de salir en el arbusto -> 33.3% tronco
        case 0:
            //Fila arbustos desde la derecha
            (*serpiente).direccion = IZQUIERDA;
            (*serpiente).sprite.collider.P1.x = VENTANA_X;
            (*serpiente).sprite.collider.P2.x = VENTANA_X+(serpienteSpriteSheet.spriteWidth);
            (*serpiente).sprite.collider.P1.y = VENTANA_Y-(SPRITE_SIZE*8);
            (*serpiente).sprite.collider.P2.y = VENTANA_Y-(SPRITE_SIZE*7);
        break;
        case 1:
            //Fila arbustos desde la izquierda
            (*serpiente).direccion = DERECHA;
            (*serpiente).sprite.collider.P1.x = -serpienteSpriteSheet.spriteWidth;
            (*serpiente).sprite.collider.P2.x = 0;
            (*serpiente).sprite.collider.P1.y = VENTANA_Y-(SPRITE_SIZE*8);
            (*serpiente).sprite.collider.P2.y = VENTANA_Y-(SPRITE_SIZE*7);
            
        break;
        default:
            //Fila troncos_2 desde la izquierda
            // printf("FILA TRONCOS\n");
            (*serpiente).direccion = DERECHA;
            (*serpiente).sprite.collider.P1.x = -serpienteSpriteSheet.spriteWidth;
            (*serpiente).sprite.collider.P2.x = 0;
            (*serpiente).sprite.collider.P1.y = VENTANA_Y-(SPRITE_SIZE*11);
            (*serpiente).sprite.collider.P2.y = VENTANA_Y-(SPRITE_SIZE*10);
        break;
    }
}

// Inicializar valores por defecto de las serpientes y las genera en caso de ser visibles
void InicializarSerpientes(){
    for(int i = 0; i < maxSerpientes; i++){
        serpientes[i].direccion = DERECHA;
        serpientes[i].direccionAnimacion = true;

        serpientes[i].animMovimiento.temporizador = last_time;

        //Inicializa sprite
        serpientes[i].sprite.indiceAnimacion = 0;
        serpientes[i].sprite.tipoAnimacion = 0;

        //En dificultades por debajo o igual a 2, no se activan
        if(jugadores[jugadorActual].dificultadActual <= 2){
            serpientes[i].sprite.isActive = false;
            serpientes[i].sprite.isVisible = false;
        }else{
            //En la máxima dificultad, se activan si o si
            if(jugadores[jugadorActual].dificultadActual >= 5){
                serpientes[i].sprite.isActive = true;
                serpientes[i].sprite.isVisible = true;
            }else{
                //En el resto de dificultades, solo aparece una, por lo tanto, si es indice 0 se activará
                serpientes[i].sprite.isActive = (i == 0);
                serpientes[i].sprite.isVisible = (i == 0);
            }
        }

        if(serpientes[i].sprite.isVisible){
            SpawnSerpiente(&serpientes[i]);
        }

        ActualizarSprite(serpienteSpriteSheet, serpienteSpriteSheet_Coords, &serpientes[i].sprite);
    }
}

// Asigna la velocidad de movimiento y animación de la nutria 
// en funcion de la fila del rio donde haya aparecido
void AsignarVelocidadNutria(FilaRio filaRioSpawn){
    switch (filaRioSpawn){
        case FILA_RIO_4:
            nutria.animMovimiento.velocidad = troncos_3[0].velocidadMovimiento+0.5;
        break;
        case FILA_RIO_3:
            nutria.animMovimiento.velocidad = tortugas_2[0].velocidadMovimiento+0.5;
        break;
        case FILA_RIO_2:
            nutria.animMovimiento.velocidad = troncos_2[0].velocidadMovimiento+0.5;
        break;
        case FILA_RIO_1:
            nutria.animMovimiento.velocidad = troncos_1[0].velocidadMovimiento+0.5;
        break;
        case FILA_RIO_0:
            nutria.animMovimiento.velocidad = tortugas_1[0].velocidadMovimiento+0.5;
        break;
    }
    nutria.animMovimiento.duracion = 1500 - nutria.animMovimiento.velocidad;
}

// Seleccionar ubicación de aparición y mover allí la nutria
void SpawnNutria(){
    // printf("SPAWN NUTRIA\n");
    FilaRio filaRioSpawn;
    int random = GenerarNumeroAleatorio(jugadores[jugadorActual].dificultadActual >= 5 ? FILA_RIO_T-1 : FILA_RIO_T-3);

    //Dificultad 5 -> Todas las filas del rio
    //Resto de dificultades (en principio 3 y 4) -> Solo en filas de troncos
    // printf("Random %d\n",random);
    switch(random){
        case 0:
        case 1:
        case 2:
            //Fila de troncos (Empieza desde borde izquierdo)
            // printf("Nutria Fila Troncos\n");
            nutria.direccion = DERECHA;
            nutria.sprite.collider.P1.x = -nutriaSpriteSheet.spriteWidth;
            nutria.sprite.collider.P2.x = 0;
        break;
        case 3:
        case 4:
            //Fila de tortugas (Empieza desde borde derecho)
            // printf("Nutria Fila Tortugas\n");
            nutria.direccion = IZQUIERDA;
            nutria.sprite.collider.P1.x = VENTANA_X;
            nutria.sprite.collider.P2.x = VENTANA_X+nutriaSpriteSheet.spriteWidth;
        break;
    }
    
    // Esto se hace a modo de traduccion para facilitar la generación del random dependiendo del nivel.
    // Ya que el resto de números coinciden con sus respectivas filas (la 2 por ejemplo es una fila de troncos)
    // No hace falta "traducirlas" para calcular la altura de la ubicación
    if(random == 0 || random == 4){
        filaRioSpawn = (FilaRio) (random == 0 ? 4 : 0);
    }else{
        filaRioSpawn = (FilaRio) random;
    }
    nutria.sprite.collider.P1.y = VENTANA_Y-(SPRITE_SIZE*(9+filaRioSpawn));
    nutria.sprite.collider.P2.y = VENTANA_Y-(SPRITE_SIZE*(8+filaRioSpawn));

    //Ubicación y estado colisión de ataque
    if(nutria.direccion == DERECHA){
        nutria.colisionAtaque.collider.P1 = {nutria.sprite.collider.P2.x, nutria.sprite.collider.P1.y};
        nutria.colisionAtaque.collider.P2 = {nutria.sprite.collider.P2.x + nutriaSpriteSheet.spriteWidth, nutria.sprite.collider.P2.y};
    }else{
        nutria.colisionAtaque.collider.P1 = {nutria.sprite.collider.P1.x - nutriaSpriteSheet.spriteWidth, nutria.sprite.collider.P1.y};
        nutria.colisionAtaque.collider.P2 = {nutria.sprite.collider.P1.x, nutria.sprite.collider.P2.y};
    }
    nutria.colisionAtaque.isActive = false;

    // Estado Animación y velocidad
    nutria.sprite.indiceAnimacion = 0;
    nutria.sprite.tipoAnimacion = 0;
    AsignarVelocidadNutria(filaRioSpawn);

    ActualizarSprite(nutriaSpriteSheet, nutriaSpriteSheet_Coords, &nutria.sprite);
}

// Inicializar valores por defecto de la nutria y la genera en caso de ser visible
void InicializarNutria(){
    //Inicializa sprite
    nutria.colisionAtaque.isActive = false;
    nutria.colisionAtaque.isVisible = false;

    //En dificultades por debajo de 2, no se activa
    if(jugadores[jugadorActual].dificultadActual <= 2){
        nutria.sprite.isActive = false;
        nutria.sprite.isVisible = false;
    }else{
        //En el resto de dificultades si
        nutria.sprite.isActive = true;
        nutria.sprite.isVisible = true;
    }

    SpawnNutria();
}

// Seleccionar ubicación de aparición y mover allí la rana bonus con los valores correspondientes
void SpawnRanaBonus(){
    // Valores accion de salto
    ranaBonus.accionarSalto.duracion =  1000;
    ranaBonus.accionarSalto.temporizador =  last_time;
    ranaBonus.accionarSalto.velocidad =  ranaBonus.accionarSalto.duracion;
    
    // Ubicacion
    // Siempre que se invoque, aparecerá en la primera posicion del primer tronco de la primera fila.
    ranaBonus.rana.sprite.collider = troncos_1[0].sprite.collider;
    ranaBonus.rana.finSalto = ranaBonus.rana.sprite.collider;

    // Visibilidad
    ranaBonus.rana.sprite.isActive = true;
    ranaBonus.rana.sprite.isVisible = false;
    ranaBonus.isBug = true;
    ranaBonus.estadoUpdate = LIBRE;

    // Sprite y estados
    ranaBonus.rana.isJumping = false;
    ranaBonus.direccionRotacion = DERECHA;
    ranaBonus.rana.direccion = ranaBonus.direccionRotacion;
    ranaBonus.rana.sprite.tipoAnimacion = (int) ranaBonus.direccionRotacion;
    ranaBonus.rana.sprite.indiceAnimacion = (int) ranaBonus.rana.isJumping;
}

void InicializarRanaBonus(){
    ranaBonus.puntuacion = 200;

    // Valores animacion puntos extra
    ranaBonus.animPuntos.duracion =  3000;
    ranaBonus.animPuntos.temporizador =  last_time;
    ranaBonus.animPuntos.velocidad =  ranaBonus.accionarSalto.duracion;

    // Valores Salto estáticos
    ranaBonus.rana.distanciaSalto = SPRITE_SIZE;
    ranaBonus.rana.animSalto.temporizador = last_time;
    ranaBonus.rana.animSalto.duracion = 100;
    ranaBonus.rana.animSalto.velocidad = (ranaBonus.rana.distanciaSalto/(ranaBonus.rana.animSalto.duracion/(1000.0f/FPS)));

    SpawnRanaBonus();
}

// Inicializar valores de las zonas finales donde:
// filaRio   -> Indica la fila del rio en la que está para ubicarse en Y
void InicializarZonasFinales(FilaRio filaRio){
    int contador = 0;
    for(int i = 0; i < maxZonasFinales; i++){
        zonasFinales[i].collider.P1 = {((float)i*zonaFinalSpriteSheet.spriteWidth),((VENTANA_Y-(SPRITE_SIZE*9))-(((float)filaRio+0.5f)*SPRITE_SIZE))}; // se ajusta 0.5f en la fila para coincidir adecuadamente al no medir el sprite 48px de alto
        zonasFinales[i].collider.P2 = {((float)(i+1)*zonaFinalSpriteSheet.spriteWidth),((VENTANA_Y-(SPRITE_SIZE*8))-(((float)filaRio)*SPRITE_SIZE))};
        zonasFinales[i].tipoAnimacion = 0;
        zonasFinales[i].isVisible = true;
        switch (contador){
        case 0:
            zonasFinales[i].indiceAnimacion = 0;
            zonasFinales[i].isActive = true;
            break;
        case 1:
        case 2:
            zonasFinales[i].indiceAnimacion = 2;
            zonasFinales[i].isActive = false;
            break;
        case 3:
            zonasFinales[i].indiceAnimacion = 4;
            zonasFinales[i].isActive = true;
            break;
        case 4:
        case 5:
            zonasFinales[i].indiceAnimacion = 6;
            zonasFinales[i].isActive = true;
            break;
        }
        ActualizarSprite(zonaFinalSpriteSheet, zonaFinalSpriteSheet_Coords, &zonasFinales[i]);

        ++contador %= 6;
    }
}

// Inicializar valores de una fila de tortugas donde:
// longitud  -> Indica el tamaño de cada grupo de tortugas
// margen    -> Indica la cantidad de espaciado entre grupos de la fila (Tortugas Inactivas)
// velocidad -> Indica la velocidad a la que se moveran las tortugas
// filaRio   -> Indica la fila del rio en la que está para ubicarse en Y
void InicializarFilaTortugas(Tortuga tortugas[], int longitud, int margen, float velocidad, FilaRio filaRio){
    // alternator indica cuando debe inicializar en base a la longitud o al margen (por defecto empieza por la longitud)
    bool alternator = true;
    int contador = 0;
    for(int i = 0; i < VENTANA_COLUMNAS; i++){
        tortugas[i].direccion = IZQUIERDA;
        tortugas[i].velocidadMovimiento = velocidad;
        tortugas[i].sprite.collider.P1 = {((float)i*tortugaSpriteSheet.spriteWidth),((VENTANA_Y-(SPRITE_SIZE*9))-((float)filaRio*tortugaSpriteSheet.spriteHeight))};
        tortugas[i].sprite.collider.P2 = {((float)(i+1)*tortugaSpriteSheet.spriteWidth),((VENTANA_Y-(SPRITE_SIZE*8))-((float)filaRio*tortugaSpriteSheet.spriteHeight))};
        tortugas[i].sprite.tipoAnimacion = 0;
        tortugas[i].sprite.indiceAnimacion = 0;
        tortugas[i].isSumergiendo = true;

        tortugas[i].animSumergir.duracion = 1500;
        tortugas[i].animSumergir.temporizador = 0;
        // Dividimos la velocidad de la animacion/velocidad del parametro para adaptar la velocidad de sumersion a la velocidad de movimiento
        // Al dividirlo de esta manera, hacemos que contra mas lenta sea la tortuga, mas lento se sumergirá y contra mas rápida, mas rápido se sumergirá
        tortugas[i].animSumergir.velocidad = (tortugas[i].animSumergir.duracion/tortugaSpriteSheet.indicesAnim)/velocidad;

        if(alternator && longitud > 0 && CabeGrupoObstaculos(contador,i,longitud)){
            //Inicializar grupo
            tortugas[i].sprite.isVisible = true;
            tortugas[i].sprite.isActive = true;

            // Hace que el primer grupo de tortugas sea sumergible
            if(i < longitud){
                tortugas[i].isSumergible = true;
            }else{
                tortugas[i].isSumergible = false;
            }
            ++contador %= longitud;
            if(contador == 0){
                alternator = !alternator;
            }
        }else{
            //Inicializar "margen"
            tortugas[i].sprite.isVisible = false;
            tortugas[i].sprite.isActive = false;

            ++contador %= margen;
            if(contador == 0){
                alternator = !alternator;
            }
        }
        ActualizarSprite(tortugaSpriteSheet,tortugaSpriteSheet_Coords,&tortugas[i].sprite);
    }
}

// Asigna al troncodrilo proporcionado, el valor de indiceAnimacion para mostrar el sprite del tronco correcto.
// El indice debe ser ubicacion relativa. Es decir, 0 indica que es el principio del tronco, y longitud-1 indica el final del tronco
void AsignarValoresTronco(Troncodrilo *troncodrilo, int indice, int longitud){
    if(indice == 0){
        (*troncodrilo).sprite.indiceAnimacion = 0;
    }else{
        if(indice == longitud-1){
            (*troncodrilo).sprite.indiceAnimacion = troncoSpriteSheet.coordsAnim-2;
        }else{
            (*troncodrilo).sprite.indiceAnimacion = 2;
        }
    }
}

// Inicializar valores de una fila de troncodrilos donde:
// longitud  -> Indica el tamaño del tronco
// margen    -> Indica la cantidad de espaciado entre troncos
// velocidad -> Indica la velocidad a la que se moveran los troncodrilos
// filaRio   -> Indica la fila del rio en la que está para ubicarse en Y
void InicializarFilaTroncodrilos(Troncodrilo troncodrilos[], int longitud, int margen, float velocidad, FilaRio filaRio){
    // alternator indica cuando debe inicializar en base a la longitud o al margen (por defecto empieza por la longitud)
    bool alternator = true;
    int contador = 0;
    
    for(int i = 0; i < VENTANA_COLUMNAS; i++){
        troncodrilos[i].direccion = DERECHA;
        troncodrilos[i].velocidadMovimiento = velocidad;
        troncodrilos[i].sprite.collider.P1 = {((float)i*troncoSpriteSheet.spriteWidth),((VENTANA_Y-(SPRITE_SIZE*9))-((float)filaRio*troncoSpriteSheet.spriteHeight))};
        troncodrilos[i].sprite.collider.P2 = {((float)(i+1)*troncoSpriteSheet.spriteWidth),((VENTANA_Y-(SPRITE_SIZE*8))-((float)filaRio*troncoSpriteSheet.spriteHeight))};
        troncodrilos[i].sprite.tipoAnimacion = 0;

        if(alternator && longitud > 0 && CabeGrupoObstaculos(contador,i,longitud)){
            //Inicializar grupo
            //Siempre empiezan habiendo troncos en pantalla
            troncodrilos[i].isTronco = true;
            troncodrilos[i].sprite.isVisible = true;
            troncodrilos[i].sprite.isActive = true;

            // Construye el tronco
            AsignarValoresTronco(&troncodrilos[i],contador,longitud);
            
            ++contador %= longitud;
            if(contador == 0){
                alternator = !alternator;
            }
        }else{
            //Inicializar "margen"
            troncodrilos[i].sprite.isVisible = false;
            troncodrilos[i].sprite.isActive = false;

            ++contador %= margen;
            if(contador == 0){
                alternator = !alternator;
            }
        }

        troncodrilos[i].animBocaCocodrilo.duracion = 1000.0f;
        troncodrilos[i].animBocaCocodrilo.temporizador = last_time;
        troncodrilos[i].animBocaCocodrilo.velocidad = 1000.0f;
        ActualizarSprite(troncoSpriteSheet,troncoSpriteSheet_Coords,&troncodrilos[i].sprite);
    }
}

// Instancia los valores de la fila correspondiente
void InicializarFilaRio(FilaRio filaRio){
    // En el nivel base, se mantiene predefinido para asegurar su funcionamiento a modo de tutorial
    // A partir del nivel 2, se adapta la cantidad de elementos (aumentando el margen) y la velocidad al dificultadActual 

    // Hay calculos hechos a mano obtenidos a base de probar para comprobar si era jugable con estos parametros

    switch (filaRio){
        case FILA_RIO_5:
            InicializarZonasFinales(filaRio);
            InicializarRanasFinales(filaRio);
        break;
        case FILA_RIO_4:
            if(jugadores[jugadorActual].dificultadActual <= 1){
                InicializarFilaTroncodrilos(troncos_3, 4, 1, 1, filaRio);
            }else{
                InicializarFilaTroncodrilos(troncos_3, 4, jugadores[jugadorActual].dificultadActual*1.5, jugadores[jugadorActual].dificultadActual/2.0f, filaRio);
            }
        break;
        case FILA_RIO_3:
            if(jugadores[jugadorActual].dificultadActual <= 1){
                InicializarFilaTortugas(tortugas_2, 2, 2, 1.25, filaRio);
            }else{
                InicializarFilaTortugas(tortugas_2, 2, 2+(jugadores[jugadorActual].dificultadActual), 1.25*(jugadores[jugadorActual].dificultadActual/3.0f), filaRio);
            }
        break;
        case FILA_RIO_2:
            if(jugadores[jugadorActual].dificultadActual <= 1){
                InicializarFilaTroncodrilos(troncos_2, 5, 2, 0.5, filaRio);
            }else{
                InicializarFilaTroncodrilos(troncos_2, 5, 2+(jugadores[jugadorActual].dificultadActual), 0.5*(jugadores[jugadorActual].dificultadActual/2.0f), filaRio);
            }
        break;
        case FILA_RIO_1:
            if(areCollidersVisible){
                InicializarFilaTroncodrilos(troncos_1, 3, 2, 0, filaRio);
            }else{
                if(jugadores[jugadorActual].dificultadActual <= 1){
                    InicializarFilaTroncodrilos(troncos_1, 3, 2, 1.5, filaRio);
                }else{
                    InicializarFilaTroncodrilos(troncos_1, 3, 2+(jugadores[jugadorActual].dificultadActual), 1.5*(jugadores[jugadorActual].dificultadActual/2.5f), filaRio);
                }
            }

        break;
        case FILA_RIO_0:
            if(jugadores[jugadorActual].dificultadActual <= 1){
                InicializarFilaTortugas(tortugas_1, 3, 1, 0.75, filaRio);
            }else{
                InicializarFilaTortugas(tortugas_1, 3, jugadores[jugadorActual].dificultadActual, 0.75*(jugadores[jugadorActual].dificultadActual/2.5f), filaRio);
            }
        break;
    }
}

// Instancia los valores por defecto de los elementos de cada fila del rio al inicio de un nivel
void InicializarRio(){
    for(int i = 0 ; i < ((int) FILA_RIO_T); i++){
        InicializarFilaRio((FilaRio) i);
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

    ActualizarSprite(vehiculosSpriteSheet,vehiculosSpriteSheet_Coords,&(*vehiculo).sprite);  
}

// Instancia los valores por defecto de los vehiculos al inicio de un nivel
void InicializarVehiculos(){
    // Se usa maxCamiones como límite ya que es el array de los obstaculos de carretera mas largo
    // Luego internamente se comprueba que el indice sea valido para cada columna (cantidad de vehiculos)
    for(int i = 0; i < maxCamiones; i++){

        // InicializarCocheAmarillo
        if(i < maxCochesAmarillos){
            //Por cada nivel, el margen disminuye en 0'5. 
            //Empezando en 4 en el nivel 1 y terminando en 2 en el nivel 5 (estos numeros multiplicados por el ancho del sprite)
            // printf("%f\n",(4-(1+(0.5*(jugadores[jugadorActual].dificultadActual-1)))));
            InicializarVehiculo(
                &cochesAmarillos[i], COCHE_AMARILLO, IZQUIERDA,
                // Fila 3 empezando desde abajo
                3,
                // Si es el primero, aparece en el borde, si no detras del ultimo + el margen que se le asigne.
                i == 0 ?  
                0 : cochesAmarillos[i-1].sprite.collider.P1.x + vehiculosSpriteSheet.spriteWidth, 
                //Velocidad
                (0.5+(jugadores[jugadorActual].dificultadActual/10.0f)),
                // Si es el primero no se le asigna margen.
                i == 0 ? 
                // El margen se calcula por nivel de dificultad
                // En caso de los coches amarillos, el margen será de un máximo de 4 veces en el nivel 1 y un mínimo de 2 en el nivel 5,
                // Encogiendo 0.5 veces el tamaño del margen (margen por defecto = SPRITE_SIZE) por nivel 
                0 : (4-((0.5*(jugadores[jugadorActual].dificultadActual-1))))
            );

            // Dependiendo de la dificultad actual, se activaran una cantidad de coches amarillos
            if(jugadores[jugadorActual].dificultadActual <= 1 ){
                // Dificultad 1
                // 3 coches activos
                if(i >= 3){
                    cochesAmarillos[i].sprite.isActive = false;
                    cochesAmarillos[i].sprite.isVisible = false;
                }else{
                    cochesAmarillos[i].sprite.isActive = true;
                    cochesAmarillos[i].sprite.isVisible = true;
                }
            }else{
                // Dificultad 2-4
                if(jugadores[jugadorActual].dificultadActual <= 4 ){
                    // 4 coches activos
                    if(i >= 4){
                        cochesAmarillos[i].sprite.isActive = false;
                        cochesAmarillos[i].sprite.isVisible = false;
                    }else{
                        cochesAmarillos[i].sprite.isActive = true;
                        cochesAmarillos[i].sprite.isVisible = true;
                    }
                }else{
                    // Dificultad 5
                    // Todos Activos
                    cochesAmarillos[i].sprite.isActive = true;
                    cochesAmarillos[i].sprite.isVisible = true;
                }
            }
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
                tractores[i-1].sprite.collider.P1.x - vehiculosSpriteSheet.spriteWidth,
                // Mantienen la velocidad en todos los niveles
                1,
                // El margen siempre será el mismo excepto para el tercer tractor que está un poco mas separado
                i == 0 ? 0 : (i == 2 ? 2.5 : 2)
            );

            // Dependiendo de la dificultad actual, se activaran una cantidad de tractores
            //Dificultad 1
            if(jugadores[jugadorActual].dificultadActual <= 1 ){
                // 3 tractores activos
                if(i >= 3){
                    tractores[i].sprite.isActive = false;
                    tractores[i].sprite.isVisible = false;
                }else{
                    tractores[i].sprite.isActive = true;
                    tractores[i].sprite.isVisible = true;
                }
            }else{
                // Resto de dificultades
                // Todos Activos
                tractores[i].sprite.isActive = true;
                tractores[i].sprite.isVisible = true;
            }
        }

        // InicializarCochesRosas
        if(i < maxCochesRosas){
            InicializarVehiculo(
                &cochesRosas[i],
                COCHE_ROSA,
                IZQUIERDA,
                5,
                // A diferencia del resto, inicia con un espacio de separacion en lugar de pegado al borde para que
                // en el nivel de dificultad 5 no vaya exactamente igual que la fila de coches amarillos, si no que aparezca una posicion por detrás
                // al ir a la misma velocidad
                i == 0 ?  
                vehiculosSpriteSheet.spriteWidth: 
                cochesRosas[i-1].sprite.collider.P1.x + vehiculosSpriteSheet.spriteWidth, 
                // Velocidad
                // A diferencia del resto, se va reduciendo hasta acabar teniendo en dificultad 5 la misma velocidad que el coche amarillo
                (1-(jugadores[jugadorActual].dificultadActual)/10),
                // Separacion entre coches de 3 en dificultades 1, 2 y 4. De 2 en dificultades 3 y 5
                i == 0 ? 0 : (jugadores[jugadorActual].dificultadActual <= 2 || jugadores[jugadorActual].dificultadActual == 4 ? 3 : 2)
            );

            // Dependiendo de la dificultad actual, se activaran una cantidad de coches rosas (En este caso igual que los coches amarillos)
            if(jugadores[jugadorActual].dificultadActual <= 1 ){
                // Dificultad 1
                // 3 coches activos
                if(i >= 3){
                    cochesRosas[i].sprite.isActive = false;
                    cochesRosas[i].sprite.isVisible = false;
                }else{
                    cochesRosas[i].sprite.isActive = true;
                    cochesRosas[i].sprite.isVisible = true;
                }
            }else{
                // Dificultad 2-4
                if(jugadores[jugadorActual].dificultadActual <= 4 ){
                    // 4 coches activos
                    if(i >= 4){
                        cochesRosas[i].sprite.isActive = false;
                        cochesRosas[i].sprite.isVisible = false;
                    }else{
                        cochesRosas[i].sprite.isActive = true;
                        cochesRosas[i].sprite.isVisible = true;
                    }
                }else{
                    // Dificultad 5
                    // Todos Activos
                    cochesRosas[i].sprite.isActive = true;
                    cochesRosas[i].sprite.isVisible = true;
                }
            }
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
                //Velocidad aumenta en 1 por cada dificultad
                0.5 + jugadores[jugadorActual].dificultadActual,
                i == 0 ?
                // El margen se calcula por nivel de dificultad
                // En caso de los coches blancos, el margen será de un máximo de 4'5 veces en el nivel 1 y un mínimo de 2'5 en el nivel 5,
                // Encogiendo 0.5 veces el tamaño del margen (margen por defecto = SPRITE_SIZE) por nivel 
                0 : (4.5-(0.5*(jugadores[jugadorActual].dificultadActual-1)))
            );

            // Dependiendo de la dificultad actual, se activaran una cantidad de coches blancos
            if(jugadores[jugadorActual].dificultadActual <= 2 ){
                // Dificultad 1-2
                // 1 coche activo
                if(i >= 1){
                    cochesBlancos[i].sprite.isActive = false;
                    cochesBlancos[i].sprite.isVisible = false;
                }else{
                    cochesBlancos[i].sprite.isActive = true;
                    cochesBlancos[i].sprite.isVisible = true;
                }
            }else{
                // Dificultad 3-5
                if(i >= jugadores[jugadorActual].dificultadActual-1){
                    // Coches activos == jugadores[jugadorActual].dificultadActual-1 (dificultad 3, 2 activos por ejemplo)
                    cochesBlancos[i].sprite.isActive = false;
                    cochesBlancos[i].sprite.isVisible = false;
                }else{
                    cochesBlancos[i].sprite.isActive = true;
                    cochesBlancos[i].sprite.isVisible = true;
                }
            }
        }

        // InicializarCamiones
        InicializarVehiculo(
            &camiones[i],
            i % 2 == 0 ? CAMION_FRONT : CAMION_BACK,
            IZQUIERDA,
            7,
            i == 0 ?  
            0: 
            camiones[i-1].sprite.collider.P1.x+vehiculosSpriteSheet.spriteWidth, 
            // A partir de la dificultad 4, reducen un poco la velocidad
            jugadores[jugadorActual].dificultadActual <= 3 ? 1 : 0.75,
            // Si es el primero no se le asigna margen.
            // Si no es el primero, tampoco se le asigna margen si se quiere instanciar la parte trasera del camion (Posicion impar)
            // El margen entre camiones cambiará en funcion de la dificultad. Será de:
            // Dificultad 1-2 -> 3.5
            // Dificultad 3-4 -> 2.5
            // Dificultad 5   -> 2
            i == 0 ? 0 : i%2 != 0 ? 0 : 
            jugadores[jugadorActual].dificultadActual <= 2 ? 3.5 : jugadores[jugadorActual].dificultadActual <= 4 ? 2.5 : 2
        );

        // Dependiendo de la dificultad actual, se activaran una cantidad de camiones
        if(jugadores[jugadorActual].dificultadActual <= 1 ){
            // Dificultad 1
            // 2 camiones activos (Se multiplica por 2 al ocupar 2 espacios cada camion)
            if(i >= 4){
                camiones[i].sprite.isActive = false;
                camiones[i].sprite.isVisible = false;
            }else{
                camiones[i].sprite.isActive = true;
                camiones[i].sprite.isVisible = true;
            }
        }else{
            // Dificultad 2-4
            if(jugadores[jugadorActual].dificultadActual <= 4 ){
                // 3 camiones activos
                if(i >= 6){
                    camiones[i].sprite.isActive = false;
                    camiones[i].sprite.isVisible = false;
                }else{
                    camiones[i].sprite.isActive = true;
                    camiones[i].sprite.isVisible = true;
                }
            }else{
                // Dificultad 5
                // Todos Activos
                camiones[i].sprite.isActive = true;
                camiones[i].sprite.isVisible = true;
            }
        }
    }
}

// Establece los valores por defecto del cronometro
void InicializarCronometro(){
    cronometro.animCronometro.duracion = 30000;
    cronometro.animCronometro.temporizador = last_time;
    cronometro.animCronometro.velocidad = 500;

    cronometro.contador = 60;
    cronometro.isBarraParada = false;
}

// Instancia los valores por defecto del jugador
// Pensado para utilizarse después de cada muerte para ubicar al jugador en su posicion inicial o despúes de inicializar un nivel
void SpawnJugador(){
    
    jugadores[jugadorActual].ranaJugador.sprite.isVisible = true;
    jugadores[jugadorActual].ranaJugador.sprite.isActive = true;
    jugadores[jugadorActual].ranaJugador.direccion = ARRIBA;
    jugadores[jugadorActual].ranaJugador.sprite.tipoAnimacion = 0;
    jugadores[jugadorActual].ranaJugador.sprite.indiceAnimacion = 0;
    jugadores[jugadorActual].ranaJugador.sprite.collider.P1 = {
        VENTANA_X/2,
        (float)(VENTANA_Y-ranaBaseSpriteSheet.spriteHeight*2)
    };
    jugadores[jugadorActual].ranaJugador.sprite.collider.P2 = {
        jugadores[jugadorActual].ranaJugador.sprite.collider.P1.x + ranaBaseSpriteSheet.spriteWidth,
        jugadores[jugadorActual].ranaJugador.sprite.collider.P1.y + ranaBaseSpriteSheet.spriteHeight
    };
    jugadores[jugadorActual].ranaJugador.isJumping = false;
    jugadores[jugadorActual].ranaJugador.finSalto = jugadores[jugadorActual].ranaJugador.sprite.collider; 
    jugadores[jugadorActual].ranaJugador.distanciaSalto = SPRITE_SIZE;
    jugadores[jugadorActual].ranaJugador.animSalto.duracion = 100;
    jugadores[jugadorActual].ranaJugador.animSalto.velocidad = (jugadores[jugadorActual].ranaJugador.distanciaSalto/(jugadores[jugadorActual].ranaJugador.animSalto.duracion/(1000.0f/FPS)));
    jugadores[jugadorActual].ranaJugador.animSalto.temporizador = last_time;
    jugadores[jugadorActual].filaPuntuacion = 2;

    jugadores[jugadorActual].animMuerte.temporizador = last_time;

    InicializarCronometro();
    ActualizarSprite(ranaBaseSpriteSheet, ranasSpriteSheet_Coords, &jugadores[jugadorActual].ranaJugador.sprite);
}

// Instancia los valores por defecto del jugador
// Pensado para utilizarse al inicio de cada partida
void InicializarJugadores(){
    int jugadoresPartida;
    switch (gamemode){
        case NORMAL_SINGLE:
            jugadoresPartida = 1;
        break;

        case NORMAL_MULTI:
            jugadoresPartida = 2;
        break;
    }
    
    for(int i = 0; i < jugadoresPartida; i++){
        //Propiedades del jugador al inicio de la partida
        jugadores[i].ranaJugador.sprite.isActive = true;
        jugadores[i].puntuacion = 0;
        jugadores[i].vidas = 3;
        jugadores[i].dificultadActual = 1;
        jugadores[i].nivelActual = 1;

        jugadores[i].animMuerte.duracion = 2000;
        jugadores[i].animMuerte.velocidad = jugadores[i].animMuerte.duracion/animMuerteSpriteSheet.indicesAnim;
    }

    jugadorActual = 0;
}

void InicializarNivel(){
    // Inicialización de cosas interactivas durante la ejecución del nivel
    InicializarRio();
    InicializarVehiculos();

    InicializarCronometro();

    InicializarMoscaCroc();
    InicializarSerpientes();
    InicializarNutria();
    InicializarRanaBonus();

    SpawnJugador();
}

//*** INICIO DE ACCIONES DE LOS OBJETOS Y JUGADORES***//

// Dada una rana con posicion de finSalto, comprueba si puede realizar ese salto
// El comportamiento cambiará en función de si se indica que es una rana de jugador o bonus
bool IsSaltoRanaJugadorPosible(Rana rana){
    return (
        rana.finSalto.P1.x >=0 && rana.finSalto.P2.x <= VENTANA_X &&
        rana.finSalto.P1.y >= ranaBaseSpriteSheet.spriteHeight*2 && rana.finSalto.P2.y <= VENTANA_Y-ranaBaseSpriteSheet.spriteHeight
    );
}

// Actualiza la posición de la zona de fina de salto según la direccion indicada.
// Además, si es el jugador se encarga de comprobar si el salto es posible
void IniciarSaltoRana(Rana *rana, Direccion newDireccion, bool isPlayer = true){
    (*rana).direccion = newDireccion;
    // CALCULA LA POSICION FINAL DONDE DEBE ATERRIZAR
    // printf("Salto -> %d\n",(*rana).distanciaSalto);
    switch((*rana).direccion){
        case ARRIBA:
            (*rana).finSalto.P1.y = (*rana).sprite.collider.P1.y-(*rana).distanciaSalto;
            (*rana).finSalto.P2.y = (*rana).sprite.collider.P2.y-(*rana).distanciaSalto;
            (*rana).finSalto.P1.x = (*rana).sprite.collider.P1.x;
            (*rana).finSalto.P2.x = (*rana).sprite.collider.P2.x;
        break;
        case DERECHA:
            (*rana).finSalto.P1.x = (*rana).sprite.collider.P1.x+(*rana).distanciaSalto;
            (*rana).finSalto.P2.x = (*rana).sprite.collider.P2.x+(*rana).distanciaSalto;
            (*rana).finSalto.P1.y = (*rana).sprite.collider.P1.y;
            (*rana).finSalto.P2.y = (*rana).sprite.collider.P2.y;
        break;
        case ABAJO:
            (*rana).finSalto.P1.y = (*rana).sprite.collider.P1.y+(*rana).distanciaSalto;
            (*rana).finSalto.P2.y = (*rana).sprite.collider.P2.y+(*rana).distanciaSalto;
            (*rana).finSalto.P1.x = (*rana).sprite.collider.P1.x;
            (*rana).finSalto.P2.x = (*rana).sprite.collider.P2.x;
        break;
        case IZQUIERDA:
            (*rana).finSalto.P1.x = (*rana).sprite.collider.P1.x-(*rana).distanciaSalto;
            (*rana).finSalto.P2.x = (*rana).sprite.collider.P2.x-(*rana).distanciaSalto;
            (*rana).finSalto.P1.y = (*rana).sprite.collider.P1.y;
            (*rana).finSalto.P2.y = (*rana).sprite.collider.P2.y;
        break;
    }

    if(isPlayer){
        // Si detecta que la posición final de salto, excede el borde de la pantalla de juego, 
        // no saltará
        if(IsSaltoRanaJugadorPosible(*rana)){
            (*rana).isJumping = true;
            (*rana).animSalto.temporizador = last_time;
            (*rana).sprite.tipoAnimacion = newDireccion;
            AvanzarSpriteAnimado(ranaBaseSpriteSheet, ranasSpriteSheet_Coords, &(*rana).sprite);
            ActualizarSprite(ranaBaseSpriteSheet, ranasSpriteSheet_Coords, &(*rana).sprite);
        }else{
            (*rana).finSalto = (*rana).sprite.collider;
        }
    }else{
        (*rana).isJumping = true;
        (*rana).animSalto.temporizador = last_time;
        (*rana).sprite.tipoAnimacion = newDireccion;
        AvanzarSpriteAnimado(ranaBaseSpriteSheet, ranasSpriteSheet_Coords, &(*rana).sprite);
    }
}

void MatarJugador(){
    jugadores[jugadorActual].ranaJugador.sprite.isActive = false;
    jugadores[jugadorActual].ranaJugador.sprite.tipoAnimacion = 0;
    jugadores[jugadorActual].ranaJugador.sprite.indiceAnimacion = 0;
    jugadores[jugadorActual].animMuerte.temporizador = last_time;
    cronometro.isTiempoVisible = false;
    cronometro.tiempoRestante = cronometro.contador;
    cronometro.isBarraParada = true;
    ActualizarSprite(animMuerteSpriteSheet, animMuerteSpriteSheet_Coords, &jugadores[jugadorActual].ranaJugador.sprite);
}

// Comprueba en que posicion va la nueva puntuacion, la asigna y desplaza las puntuaciones por debajo en caso de que
// la puntuacion sea mayor a alguna de las actuales en el ranking
void GuardarPuntuacion(){
    int puntosUltimaPartida = jugadores[jugadorActual].puntuacion;
    int i = 0, aux = -1;
    do{
        // aux se utiliza para manejar la puntuacion anterior para poder moverla sin eliminar el valor. 
        // Si es -1, es el valor por defecto y por lo tanto que la nueva puntuacion no tiene posición asignada 
        // todavia. Si recorre todo el array de puntuaciones y sigue siendo -1 significa que no es mayor a 
        // otro de los valores del listado y por lo tanto que no se debe almacenar.
        if(aux == -1){
            if(jugadores[jugadorActual].puntuacion > rankingScores[i]){
                aux = rankingScores[i];
                rankingScores[i] = jugadores[jugadorActual].puntuacion;
                jugadores[jugadorActual].puntuacion = aux;
            }else{
                // Si el valor a almacenar es exactamente igual que uno que ya existe, avanza el índice para terminar 
                // la búsqueda ya que no debe actualizar el ranking
                if(jugadores[jugadorActual].puntuacion == rankingScores[i]){
                    i = maxRankingScores;
                }
            }
        }else{
            // El doble if anterior se usa de esa manera para asegurar que este apartado
            // solo se ejecuta si ha encontrado una posicion válida para la nueva puntuacion.
            
            // A partir de aquí, jugadores[jugadorActual].puntuacion es el valor anterior del ranking que estamos desplazando hacia abajo
            // rankingScores[i] es el actual que queremos almacenar para la siguiente iteración.
            // aux sirve alternar los valores para que así en la siguiente iteración funcione igual
            
            aux = rankingScores[i];
            rankingScores[i] = jugadores[jugadorActual].puntuacion;
            jugadores[jugadorActual].puntuacion = aux;
        }

        i++;
    //Termina cuando recorra todo o cuando el valor a mover y el actual sean iguales
    }while(i < maxRankingScores && jugadores[jugadorActual].puntuacion != rankingScores[i]);

    // printf("aux %d\n",aux);
    // printf("jugadores[jugadorActual].puntuacion %d\n",jugadores[jugadorActual].puntuacion);
    jugadores[jugadorActual].puntuacion = puntosUltimaPartida;
}

// En función de la pantalla y creditos actuales, se comprueba si 
// debería cambiar de pantalla y ejecuta el cambio si es necesario  
void ComprobarCreditos(){
    switch (pantallaActual){
        case INTRO:
        case PUNTUACIONES:
        case INSERT:
            if(credits >= 1){
                gamemode = NORMAL_SINGLE;
                pantallaActual = START;
            }
        break;
        case RANKING:
            if(credits >= 1){
                gamemode = NORMAL_SINGLE;
                pantallaActual = START;
            }else{
                animInsert.temporizador = last_time;
                pantallaActual = INSERT;
            }
        break;
        case JUEGO:
            InicializarTitulo();
            animRanking.temporizador=last_time;
            pantallaActual = RANKING;
        break;
    }
}

//ManejoVidasJugador
void InicializarGameOver(){
    cronometro.tiempoRestante = cronometro.contador;
    cronometro.isBarraParada = true;

    animGameOver.duracion = 2000;
    animGameOver.velocidad = VENTANA_X/((animGameOver.duracion-1000)/(1000.0f/FPS));
    animGameOver.temporizador = last_time;

    barridoGameOver_Azul = {
        {VENTANA_X, 0},
        {VENTANA_X, (VENTANA_Y)/2}
    };

    barridoGameOver_Negro = {
        {VENTANA_X, (VENTANA_Y)/2},
        {VENTANA_X, VENTANA_Y}
    };

    isAnimGameOver = true;
}

void SumarVidasJugador(int vidas){
    jugadores[jugadorActual].vidas += vidas;
    if(jugadores[jugadorActual].vidas >= 10){
        jugadores[jugadorActual].vidas = 10;
    }
}

void RestarVidasJugador(int vidas){
    jugadores[jugadorActual].vidas -= vidas;
    if(jugadores[jugadorActual].vidas <= 0){
        jugadores[jugadorActual].vidas = 0;
    }

    switch (gamemode){
        case NORMAL_SINGLE:
            if(jugadores[jugadorActual].vidas <= 0){
                InicializarGameOver();
            }else{
                SpawnJugador();
            }
        break;

        case NORMAL_MULTI:
            if(jugadores[0].vidas <= 0 && jugadores[1].vidas <= 0){
                InicializarGameOver();
            }else{
                jugadorActual = ++jugadorActual % maxJugadores;
                if(jugadores[jugadorActual].vidas <= 0){
                    jugadorActual = ++jugadorActual % maxJugadores;
                    SpawnJugador();
                }else{
                    InicializarNivel();
                }
            }
        break;
    }
}

//Manejo puntuacion jugador
void SumarPuntosJugador(int puntos){
    jugadores[jugadorActual].puntuacion += puntos;
    jugadores[jugadorActual].puntuacion_VE += puntos;

    if(jugadores[jugadorActual].puntuacion >= 99999){
        jugadores[jugadorActual].puntuacion = 99999;
    }

    if(jugadores[jugadorActual].puntuacion_VE >= 20000){
        jugadores[jugadorActual].puntuacion_VE -= 20000;
        SumarVidasJugador(1);
    }
}

//Manejo Creditos
void SumarCreditos(int creditos){
    credits += creditos;
    if(credits >= 99){
        credits = 99;
    }
}

void RestarCreditos(int creditos){
    credits -= creditos;
    if(credits <= 0){
        credits = 0;
    }
}

// Si cumple las condiciones, suma la puntuación correspondiente por avanzar 
// verticalmente en la pantalla al jugador actual
void SumarPuntosAvanceVertical(){
    int filaActual = GetFilaPantallaSprite(jugadores[jugadorActual].ranaJugador.sprite);
    if(filaActual != 8 && filaActual > jugadores[jugadorActual].filaPuntuacion && jugadores[jugadorActual].ranaJugador.direccion == ARRIBA){
        jugadores[jugadorActual].filaPuntuacion = filaActual;
        SumarPuntosJugador(10);
    }
}

void RestarPuntosJugador(int puntos){
    jugadores[jugadorActual].puntuacion -= puntos;
    jugadores[jugadorActual].puntuacion_VE -= puntos;

    if(jugadores[jugadorActual].puntuacion <= 0){
        jugadores[jugadorActual].puntuacion = 0;
    }

    if(jugadores[jugadorActual].puntuacion_VE <= 0){
        jugadores[jugadorActual].puntuacion_VE = 0;
    }
}

void InicializarAvanceNivel(){
    jugadores[jugadorActual].ranaJugador.sprite.isVisible = false;
    jugadores[jugadorActual].ranaJugador.sprite.isActive = false;
    cronometro.isBarraParada = true;

    animAvanceNivel.duracion = 4000;
    animAvanceNivel.velocidad = animAvanceNivel.duracion/maxRanasFinales;
    animAvanceNivel.temporizador = last_time;
    isAnimAvanceNivel = true;
}

void AvanzarNivel(){
    // Suma puntos, pero si al sumar se pasa del maximo de puntos, entonces se asegura de asignar el valor máximo
    SumarPuntosJugador(1000);

    // Avanza el nivel de dificultar, cada 5 niveles, se resetea a la dificultad del nivel 2
    if(jugadores[jugadorActual].dificultadActual >= 5){
        jugadores[jugadorActual].dificultadActual = 2;
    }else{
        jugadores[jugadorActual].dificultadActual += 1;
    }

    // En la UI mostrará hasta un máximo de 10 niveles aunque el valor siga aumentando hasta un valor máximo de 255
    if(jugadores[jugadorActual].nivelActual < 255){
        jugadores[jugadorActual].nivelActual++;
    }

    cronometro.isTiempoVisible = false;
    InicializarNivel();
}

//*** DETECCIÓN INPUT DEL JUGADOR ***//
void DetectarControles(){
    switch (pantallaActual){
        case INTRO:
        case PUNTUACIONES:
        case INSERT:
        case RANKING:
            if(esat::IsSpecialKeyDown(esat::kSpecialKey_F1)){
                SumarCreditos(1);
                ComprobarCreditos();
            }
        break;

        case START:
            if(esat::IsSpecialKeyDown(esat::kSpecialKey_F1)){
                SumarCreditos(1);
                if(credits >= 2){
                    gamemode = NORMAL_MULTI;
                }
            }

            if((esat::IsSpecialKeyDown(esat::kSpecialKey_Right) || esat::IsSpecialKeyDown(esat::kSpecialKey_Left)) && credits >= 2){
                gamemode = (gamemode == NORMAL_SINGLE ? NORMAL_MULTI : NORMAL_SINGLE);
            }

            if(esat::IsSpecialKeyDown(esat::kSpecialKey_Enter)){
                RestarCreditos(gamemode == NORMAL_SINGLE ? 1:2);
                pantallaActual = JUEGO;
                InicializarJugadores();
                InicializarNivel();
            }
        break;
        case JUEGO:
            // CONTROLES RANA_JUGADOR
            // El jugador podrá realizar acciones siempre que:
            //   - No esté saltando
            if(!jugadores[jugadorActual].ranaJugador.isJumping && jugadores[jugadorActual].ranaJugador.sprite.isActive){
                if(esat::IsSpecialKeyDown(esat::kSpecialKey_Up)){
                    IniciarSaltoRana(&jugadores[jugadorActual].ranaJugador, ARRIBA);
                }
                if(esat::IsSpecialKeyDown(esat::kSpecialKey_Right)){
                    IniciarSaltoRana(&jugadores[jugadorActual].ranaJugador, DERECHA);
                }
                if(esat::IsSpecialKeyDown(esat::kSpecialKey_Down)){
                    IniciarSaltoRana(&jugadores[jugadorActual].ranaJugador, ABAJO);
                }
                if(esat::IsSpecialKeyDown(esat::kSpecialKey_Left)){
                    IniciarSaltoRana(&jugadores[jugadorActual].ranaJugador, IZQUIERDA);
                }
            }

            //PROTOTIPADO
            //Sumar 10 puntos
            if(esat::IsKeyPressed('P')){
                SumarPuntosJugador(10);
            }
            //Sumar 1 vida
            if(esat::IsKeyDown('V')){
                SumarVidasJugador(2);
            }

            //Matar al jugador
            if(esat::IsSpecialKeyDown(esat::kSpecialKey_Backspace)){
                MatarJugador();
            }

            //Avanzar nivel
            if(esat::IsSpecialKeyDown(esat::kSpecialKey_Keypad_8)){
                AvanzarNivel();
            }

            //Game Over
            if(esat::IsSpecialKeyDown(esat::kSpecialKey_Keypad_2)){
                InicializarGameOver();
            }

        break;
    }

    //PROTOTIPADO
    if(esat::IsKeyDown('1')){
        // printf("INTRO\n");
        InicializarTitulo(true);
        animIntro.temporizador=last_time;
        pantallaActual = INTRO;
    }
    if(esat::IsKeyDown('2')){
        // printf("PUNTUACIONES\n");
        InicializarTitulo();
        animPuntuaciones.temporizador=last_time;
        pantallaActual = PUNTUACIONES;
    }
    if(esat::IsKeyDown('3')){
        // printf("RANKING\n");
        InicializarTitulo();
        animRanking.temporizador=last_time;
        pantallaActual = RANKING;
    }
    if(esat::IsKeyDown('4')){
        // printf("INSERT\n");
        animInsert.temporizador=last_time;
        pantallaActual = INSERT;
    }
    if(esat::IsKeyDown('5')){
        // printf("START\n");
        pantallaActual = START;
    }
    if(esat::IsKeyDown('6')){
        // printf("JUEGO\n");
        pantallaActual = JUEGO;
        InicializarJugadores();
        InicializarNivel();
    }
    
    //Alterna la visualización de los colliders
    if(esat::IsSpecialKeyDown(esat::kSpecialKey_Alt)){
        areCollidersVisible = !areCollidersVisible;
        if(areCollidersVisible){
            // printf("SHOWING COLLIDERS\n");
        }else{
            // printf("HIDDEN COLLIDERS\n");
        }
    }
}

/*** FUNCIONES DE ACTUALIZACIÓN DE ESTADO DEL JUEGO ***/

/** COLISIONES **/
// Dados 2 collider, comprueba si hay colisión entre ellos
bool DetectarColision(Collider C1, Collider C2) {
  return (C1.P2.x >= C2.P1.x) &&
         (C1.P1.x <= C2.P2.x) &&
         (C1.P2.y >= C2.P1.y) &&
         (C1.P1.y <= C2.P2.y);
}

// Ajusta el radio de colision del jugador
// y comprueba si hay colisión entre el jugador y el
// collider proporcionado como parámetro
bool DetectarColisionJugador(Collider object_C) {
    if(jugadores[jugadorActual].ranaJugador.sprite.isActive){
        return 
        (jugadores[jugadorActual].ranaJugador.sprite.collider.P2.x-10 > object_C.P1.x) &&
        (jugadores[jugadorActual].ranaJugador.sprite.collider.P1.x+10 < object_C.P2.x) &&
        (jugadores[jugadorActual].ranaJugador.sprite.collider.P2.y-10 > object_C.P1.y) &&
        (jugadores[jugadorActual].ranaJugador.sprite.collider.P1.y+10 < object_C.P2.y);
    }else{
        return false;
    }
    
}

// Si está saltando, comprueba si se choca con la pared.
// Si consigue aterrizar, se asegura que haga contacto una zona desocupada
//      Zona Ocupada -> Mismo efecto que chocar con la pared
//      Zona Desocupada -> Suma de puntuacion y comprobar si se ha superado el nivel, ejecutando unos ventos u otros
void DetectarColisionJugadorZonaFinal(){
    // Almacena el total de zonas ocupadas. Si todas están ocupadas, entonces se avanza de nivel
    int victoria = 0;
    if(jugadores[jugadorActual].ranaJugador.isJumping){
        for(int i = 0; i < maxZonasFinales; i++){
            if(zonasFinales[i].isActive && DetectarColisionJugador(zonasFinales[i].collider)){
                MatarJugador();
            }
        }
    }else{
        // Colision con MoscaCroc para puntos extra o muerte en caso de que esté activo
        if(DetectarColisionJugador(moscaCroc.sprite.collider) && moscaCroc.sprite.isActive){
            //Solo si está activo, comprueba si es una trampa o no y actua en consecuencia
            if(moscaCroc.estado == CROC){
                MatarJugador();
            }else{
                //Suma la puntuación oportuna, cambia el sprite al de 200 puntos y reinicia el temporizador de la animación de puntos
                SumarPuntosJugador(moscaCroc.puntuacion);
                moscaCroc.sprite.isActive = false;
                moscaCroc.sprite.tipoAnimacion = 0;
                moscaCroc.sprite.indiceAnimacion = 2;
                moscaCroc.estado = PUNTOS_MC;
                moscaCroc.animPuntos.temporizador = last_time;

                ActualizarSprite(puntosSpriteSheet, puntosSpriteSheet_Coords, &moscaCroc.sprite);
            }
        }

        // Si después de comprobar el moscaCroc, sigue vivo, entonces hace las comprobaciones de llegar a una meta
        if(jugadores[jugadorActual].ranaJugador.sprite.isActive){
            for(int i = 0; i < maxRanasFinales; i++){
                if(DetectarColisionJugador(ranasFinales[i].collider)){
                    if(ranasFinales[i].isVisible){
                        //Si la meta está ocupada, mata al jugador
                        MatarJugador();
                    }else{
                        ranasFinales[i].isVisible = true;
                        if(moscaCroc.estado == CROC){
                            moscaCroc.sprite.isVisible = false;
                        }

                        //Puntos por llegar al final
                        SumarPuntosJugador(50);
                        // 10 Puntos extra por cada segundo sobrante 
                        // Se divide primero entre 2 para sascar los segundos reales ya que el contador funciona con medios segundos y luego se multiplica por 10;
                        SumarPuntosJugador((cronometro.contador/2)*10);

                        cronometro.tiempoRestante = cronometro.contador;
                        cronometro.isTiempoVisible=true;
                        SpawnJugador();
                    }
                }

                if(ranasFinales[i].isVisible){
                    victoria++;
                }
            }

            if(victoria == maxRanasFinales){
                // printf("VICTORIA | AVANZANDO NIVEL");
                
                InicializarAvanceNivel();
            }
        }
    }
}

// Comprueba si puede posarse sobre un troncodrilo
void DetectarColisionJugadorFilaTroncodrilos(Troncodrilo troncodrilos[]){
    // Al saltar puede quedarse en medio de 2 colisiones. 
    // Los indices de estas se almacenen aquí, siendo la colision izquierda la primera y la derecha la segunda.
    int colisionesDetectadas_T[2] = {-1,-1};
    int colisionesDetectadas_I = 0, indiceActual = 0;

    //Almacenar colisiones actuales
    do{
        if(DetectarColisionJugador(troncodrilos[indiceActual].sprite.collider)){
            colisionesDetectadas_T[colisionesDetectadas_I] = indiceActual;
            colisionesDetectadas_I++;
        }
        indiceActual++;
    } while (colisionesDetectadas_I < 2 && indiceActual < VENTANA_COLUMNAS);
       
    // LOG
    // for(int i = 0; i < 2; i++){
    //     printf("INDICE %d\n",colisionesDetectadas_T[i]);
    // }

    if(jugadores[jugadorActual].ranaJugador.isJumping){
        //Comprobar "aplastamiento"
        // SALTO LATERAL NO PERMITIDO SI CAMBIAS DE PLATAFORMA
        // Básicamente detecta si en algun punto en el aire, no detecta colisión con ninguna tortuga, indicando que
        // estas cambiando de grupo
        switch(jugadores[jugadorActual].ranaJugador.direccion){
            case DERECHA:
            case IZQUIERDA:
                // Si solo está en contacto con una colisión Y esa colisión no está activa...
                if(colisionesDetectadas_T[1] == -1 && !troncodrilos[colisionesDetectadas_T[0]].sprite.isActive){
                    MatarJugador();
                }
            break;
        }
    }else{
        // LOG
        // for(int i = 0; i < 2; i++){
        //     printf("INDICE %d\n",colisionesDetectadas_T[i]);
        // }
        // Si está en contacto con 2 
        if(colisionesDetectadas_T[0] != -1 && colisionesDetectadas_T[1] != -1){
            // Con que una de las 2 esté activa, se mueve a la vez q la fila en general
            if(troncodrilos[colisionesDetectadas_T[0]].sprite.isActive || troncodrilos[colisionesDetectadas_T[1]].sprite.isActive){
                MoveCollider(
                    &jugadores[jugadorActual].ranaJugador.sprite.collider, 
                    (Direccion) troncodrilos[colisionesDetectadas_T[0]].direccion, 
                    troncodrilos[colisionesDetectadas_T[0]].velocidadMovimiento
                );
            }else{
            // Si ninguno está activo el jugador muere
                MatarJugador();
            }
        }else{
            // Si solo está en contacto con uno, será el primero, luego hace lo mismo...
            if(colisionesDetectadas_T[0] != -1){
                //Comprueba que está activo
                if(troncodrilos[colisionesDetectadas_T[0]].sprite.isActive){
                    MoveCollider(
                        &jugadores[jugadorActual].ranaJugador.sprite.collider, 
                        (Direccion) troncodrilos[colisionesDetectadas_T[0]].direccion, 
                        troncodrilos[colisionesDetectadas_T[0]].velocidadMovimiento
                    );
                }else{
                    MatarJugador();
                }
            }
            //Siempre debería estar en contacto con alguno si se está comprobando la colisión de la fila al aterrizar
        }
    }
}

// Comprueba si puede posarse sobre una tortuga
void DetectarColisionJugadorFilaTortugas(Tortuga tortugas[]){
    // Al saltar puede quedarse en medio de 2 colisiones. 
    // Los indices de estas se almacenen aquí, siendo la colision izquierda la primera y la derecha la seggunda.
    int colisionesDetectadas_T[2] = {-1,-1};
    int colisionesDetectadas_I = 0, indiceActual = 0;

    //Almacenar colisiones actuales
    do{
        if(DetectarColisionJugador(tortugas[indiceActual].sprite.collider)){
            colisionesDetectadas_T[colisionesDetectadas_I] = indiceActual;
            colisionesDetectadas_I++;
        }
        indiceActual++;
    } while (colisionesDetectadas_I < 2 && indiceActual < VENTANA_COLUMNAS);
       
    // LOG
    // for(int i = 0; i < 2; i++){
    //     printf("INDICE %d\n",colisionesDetectadas_T[i]);
    // }

    if(jugadores[jugadorActual].ranaJugador.isJumping){
        //Comprobar "aplastamiento"
        // SALTO LATERAL NO PERMITIDO SI CAMBIAS DE PLATAFORMA
        // Básicamente detecta si en algun punto en el aire, no detecta colisión con ninguna tortuga, indicando que
        // estas cambiando de grupo
        switch(jugadores[jugadorActual].ranaJugador.direccion){
            case DERECHA:
            case IZQUIERDA:
                // Si solo está en contacto con una colisión Y esa colisión no está activa...
                if(colisionesDetectadas_T[1] == -1 && !tortugas[colisionesDetectadas_T[0]].sprite.isActive){
                    MatarJugador();
                }
            break;
        }
    }else{
        // LOG
        // for(int i = 0; i < 2; i++){
        //     printf("INDICE %d\n",colisionesDetectadas_T[i]);
        // }
        // Si está en contacto con 2 
        if(colisionesDetectadas_T[0] != -1 && colisionesDetectadas_T[1] != -1){
            // Con que una de las 2 esté activa, se mueve a la vez q la fila en general
            if(tortugas[colisionesDetectadas_T[0]].sprite.isActive || tortugas[colisionesDetectadas_T[1]].sprite.isActive){
                MoveCollider(
                    &jugadores[jugadorActual].ranaJugador.sprite.collider, 
                    (Direccion) tortugas[colisionesDetectadas_T[0]].direccion, 
                    tortugas[colisionesDetectadas_T[0]].velocidadMovimiento
                );
            }else{
            // Si ninguno está activo el jugador muere
                MatarJugador();
            }
        }else{
            // Si solo está en contacto con uno, será el primero, luego hace lo mismo...
            if(colisionesDetectadas_T[0] != -1){
                //Comprueba que está activo
                if(tortugas[colisionesDetectadas_T[0]].sprite.isActive){
                    MoveCollider(
                        &jugadores[jugadorActual].ranaJugador.sprite.collider, 
                        (Direccion) tortugas[colisionesDetectadas_T[0]].direccion, 
                        tortugas[colisionesDetectadas_T[0]].velocidadMovimiento
                    );
                }else{
                    MatarJugador();
                }
            }
            //Siempre debería estar en contacto con alguno si se está comprobando la colisión de la fila al aterrizar
        }
    }
}

// Todos los vehiculos actuan de la misma forma contra la rana:
// La rana es atropellada y muere en el acto, activando todos los 
// eventos que deban ocurrir cuando esta fallece
void DetectarColisionJugadorFilaVehiculos(Vehiculo vehiculos[], int totalVehiculos){
    for(int i = 0; i < totalVehiculos; i++){
        if(DetectarColisionJugador(vehiculos[i].sprite.collider) && vehiculos[i].sprite.isActive){
            MatarJugador();
        }
    }
}

// Dado un sprite y una fila, devuelve verdadero si debería comprobar la fila pasada por parametro y sus adyacentes al estar el sprite ubicado en una de ellas
bool ComprobarFilaActualYAdyacentes(Sprite sprite, int fila){
    // return(GetFilaPantallaSprite(sprite) == fila);
    return(GetFilaPantallaSprite(sprite) >= fila-0.9 && GetFilaPantallaSprite(sprite) <= fila+0.9);
}

// Comprueba la colisión de la rana bonus con una rana final visible. Si hay colisión, suma los puntos correspondientes y actualiza los datos de la rana bonus para mostrarlo por pantalla
void DetectarColisionRanaBonusRanaFin(){
    int indice = 0;
    bool isColision = false;

    //Si está en colisión con la rana del jugador, pero esta esta muriendo, la comprobación de la colisión con las ranas finales no se hará
    if(!(DetectarColision(jugadores[jugadorActual].ranaJugador.sprite.collider,ranaBonus.rana.sprite.collider) && !jugadores[jugadorActual].ranaJugador.sprite.isActive)){
        do{
            isColision = (
                
                ranasFinales[indice].isVisible && 
                DetectarColision(ranaBonus.rana.sprite.collider, ranasFinales[indice].collider)
            );
        } while (++indice < maxRanasFinales && !isColision);
    }
    
    if(isColision){
        indice--;
        // printf("Rana Bonus Salvada con éxito\n");
        SumarPuntosJugador(ranaBonus.puntuacion);
        ranaBonus.rana.sprite.tipoAnimacion = 0;
        ranaBonus.rana.sprite.indiceAnimacion = 2;
        RellocateCollider(
            &ranaBonus.rana.sprite.collider, 
            {
                ranasFinales[indice].collider.P1.x,
                ranasFinales[indice].collider.P1.y - SPRITE_SIZE
            }
        );
        ActualizarSprite(puntosSpriteSheet, puntosSpriteSheet_Coords, &ranaBonus.rana.sprite);
        ranaBonus.animPuntos.temporizador = last_time;
        ranaBonus.estadoUpdate = PUNTOS_RB;
    }else{
        ranaBonus.estadoUpdate = LIBRE;
        SpawnRanaBonus();
    }
}

// En función de donde se ubica la rana del jugador, comprueba las colisiones de la fila de la pantalla donde se encuentra y
// de las adyacentes debido a sus posibles movimientos futuros.
// Esto permite no tener que comprobar las colisiones de absolutamente todos los obstaculos
void ComprobarColisionesJugador(){
    //Recorre las filas con posibles interacciones con el jugador si está activo
    if(jugadores[jugadorActual].ranaJugador.sprite.isActive){
        //Colision Jugador con bordes
        if(jugadores[jugadorActual].ranaJugador.sprite.collider.P1.x < 0 || jugadores[jugadorActual].ranaJugador.sprite.collider.P2.x > VENTANA_X){
            MatarJugador();
        }

        //Recorre todas las filas de la pantalla y...
        for(int filaComprobar = 3; filaComprobar < VENTANA_Y/ranaBaseSpriteSheet.spriteHeight - 1; filaComprobar++){
            if(ComprobarFilaActualYAdyacentes(jugadores[jugadorActual].ranaJugador.sprite,filaComprobar)){
                // Si es una fila que en la que se encuentra el jugador o es adyacente, comprueba las colisiones 
                // relacionadas con el jugador correspondientes
                switch(filaComprobar){
                    //FINAL
                    case 14:
                        // printf("FINAL\n");
                        DetectarColisionJugadorZonaFinal();
                    break;
                    // RIO
                    case 13:
                        // printf("FILA_RIO_4\n");
                        DetectarColisionJugadorFilaTroncodrilos(troncos_3);
                    break;
                    case 12:
                        // printf("FILA_RIO_3\n");
                        DetectarColisionJugadorFilaTortugas(tortugas_2);
                    break;
                    case 11:
                        // printf("FILA_RIO_2\n");
                        DetectarColisionJugadorFilaTroncodrilos(troncos_2);
                    break;
                    case 10:
                        // printf("FILA_RIO_1\n");
                        DetectarColisionJugadorFilaTroncodrilos(troncos_1);
                    break;
                    case 9:
                        // printf("FILA_RIO_0\n");
                        DetectarColisionJugadorFilaTortugas(tortugas_1);
                    break;

                    //CARRETERA
                    case 7:
                        // printf("CAMION\n");
                        DetectarColisionJugadorFilaVehiculos(camiones, maxCamiones);
                    break;
                    case 6:
                        // printf("COCHE BLANCO\n");
                        DetectarColisionJugadorFilaVehiculos(cochesBlancos, maxCochesBlancos);
                    break;
                    case 5:
                        // printf("COCHE ROSA\n");
                        DetectarColisionJugadorFilaVehiculos(cochesRosas, maxCochesRosas);
                    break;
                    case 4:
                        // printf("TRACTOR\n");
                        DetectarColisionJugadorFilaVehiculos(tractores, maxTractores);
                    break;
                    case 3:
                        // printf("COCHE AMARILLO\n");
                        DetectarColisionJugadorFilaVehiculos(cochesAmarillos, maxCochesAmarillos);
                    break;
                }
            }
        }
        // printf("\n");
    }

    //Colision rana bonus
    if(DetectarColisionJugador(ranaBonus.rana.sprite.collider)){
        //Solo detecta los cambios correspondientes y los asigna. 
        //De lo demás se encarga la actualización de estado de la rana bonus
        ranaBonus.rana.sprite.indiceAnimacion = 0;
        ranaBonus.estadoUpdate = ANCLADA;
        ranaBonus.rana.sprite.isVisible = true;

    }else{
        //Si estaba enganchada al jugador y pierde la colisión, la desengancha.
        //Si está en contacto con una ranaFin visible, hace el sumado de puntos y actualiza
        //los valores de la ranaBonus para ello
        //Reinvocar a la rana bonus
        if(ranaBonus.estadoUpdate == ANCLADA){
            DetectarColisionRanaBonusRanaFin();
        }
    }

}

/** ESTADOS **/

// Cambia los valores de posicion de un obstaculo en función de los parametros indicados
// Pensado para el movimiento de obstaculos en movimiento como los vehiculos o los troncos
void ActualizarMovimientoObstaculo(Sprite *sprite, Direccion direccion, float velocidad){
    if (ComprobarSalidaVentanaSprite((*sprite), direccion)){
        RellocateSpriteOnBorderEscape(&(*sprite), direccion);
    }else{
        MoveCollider(&(*sprite).collider, direccion, velocidad);
    }
}

// Version de ActualizarMovimientoObstaculo con un spriteSheet para usar el ancho del mismo en caso de estar moviendo un sprite sin imagen 
// (Colision de ataque de la nutria por ejemplo)
void ActualizarMovimientoObstaculo(Sprite *sprite, Direccion direccion, float velocidad, SpriteSheet spriteSheet){
    if (ComprobarSalidaVentanaSprite((*sprite), direccion)){
        RellocateSpriteOnBorderEscape(&(*sprite), direccion, spriteSheet);
    }else{
        MoveCollider(&(*sprite).collider, direccion, velocidad);
    }
}

// Cambia los valores de posicion de un collider en función de los parametros indicados
// Pensado para el movimiento de collider en movimiento
void ActualizarMovimientoCollider(Collider *collider, Direccion direccion, float velocidad){
    if (ComprobarSalidaVentanaCollider(&(*collider), direccion)){
        RellocateColliderOnBorderEscape(&(*collider), direccion);
    }else{
        MoveCollider(&(*collider), direccion, velocidad);
    }
}

//Comprueba si la tortuga se está sumergiendo o no (Direccion derecha o inversa de la animacion y deteccion de colision) 
void ComprobarSumersionTortuga(Tortuga *tortuga){
    int preSumersionCoord = (tortugaSpriteSheet.coordsAnim/2)-2;
    int penUltimaCoord = tortugaSpriteSheet.coordsAnim-(2*2);
    int ultimaCoord = tortugaSpriteSheet.coordsAnim-2;

    // Si está en indice 0, cambia el sentido de la animacion y no comprueba nada mas
    if((*tortuga).sprite.indiceAnimacion == 0){
        (*tortuga).isSumergiendo = true;
    }else {
        // Si está en el frame previo a hundirse...
        if((*tortuga).sprite.indiceAnimacion == preSumersionCoord){
            // Si la tortuga no se puede hundir, entonces simplemente cambia el sentido de  la animacion
            if(!(*tortuga).isSumergible){
                (*tortuga).isSumergiendo = false;
            }
        }else {
            //Si está en el frame justo antes de terminar de hundirse y se está sumergiendo, le quitamos la tangibilidad 
            // para que en el proximo frame (completamente hundida) no puedas usarla de plataforma
            if((*tortuga).sprite.indiceAnimacion == penUltimaCoord && (*tortuga).isSumergiendo){
                (*tortuga).sprite.isActive = false;
            }else{
                //Por último, si está en el último indice de animación. 
                //Cambia el sentido de la animacion para que vuelva a emerger y activa su tangibilidad para el proximo frame
                if((*tortuga).sprite.indiceAnimacion == ultimaCoord){
                    // Si se está sumergiendo, le quitamos tangibilidad
                    (*tortuga).isSumergiendo = false;
                    (*tortuga).sprite.isActive = true;
                }
            }
        }
    }
}

// Actualiza el estado de una tortuga 
void ActualizarEstadoTortuga(Tortuga *tortuga){
    ActualizarMovimientoObstaculo(&(*tortuga).sprite,(*tortuga).direccion,(*tortuga).velocidadMovimiento);
    
    //Animacion de la tortuga
    if(HacerCadaX(&(*tortuga).animSumergir.temporizador,(*tortuga).animSumergir.velocidad) && (*tortuga).sprite.isVisible){
        ComprobarSumersionTortuga(&(*tortuga));
        //En funcion de si la tortuga se está sumergiendo o no, entonces avanza o retrocede el frame de animacion
        if((*tortuga).isSumergiendo){
            AvanzarSpriteAnimado(tortugaSpriteSheet,tortugaSpriteSheet_Coords,&(*tortuga).sprite);
        }else{
            RetrocederSpriteAnimado(tortugaSpriteSheet,tortugaSpriteSheet_Coords,&(*tortuga).sprite);
        }
    }
    
    ActualizarSprite(tortugaSpriteSheet,tortugaSpriteSheet_Coords,&(*tortuga).sprite);

}

//Recorre todas las tortugas la fila de un rio y actualiza sus estados
// array[] -> Fila de tortugas
void ActualizarEstadoFilaTortugas(Tortuga array[]){
    for(int i = 0; i < VENTANA_COLUMNAS; i++){
        ActualizarEstadoTortuga(&array[i]);
    }
}

// Actualiza el estado de movimiento y sprite de un troncodrilo
void ActualizarEstadoTroncodrilo(Troncodrilo *troncodrilo){
    ActualizarMovimientoObstaculo(&(*troncodrilo).sprite,(*troncodrilo).direccion,(*troncodrilo).velocidadMovimiento);
    if((*troncodrilo).isTronco){
        ActualizarSprite(troncoSpriteSheet,troncoSpriteSheet_Coords,&(*troncodrilo).sprite);
    }else{
        ActualizarSprite(cocodriloSpriteSheet,cocodriloSpriteSheet_Coords,&(*troncodrilo).sprite);
    }
}

// Actualiza el estado de un troncodrilo dado su indice para comprobar conversion de cocodrilo
// DEBE USARSE CON INDICES QUE SE SABE QUE FORMAN PARTE DE UN TRONCODRILO VISIBLE
void ActualizarEstadoTroncodrilo(Troncodrilo filaTroncos[], int indice){
    int troncoProb;
    switch(jugadores[jugadorActual].dificultadActual){
        case 1:
            troncoProb = 100;
        break;
        case 2:
            troncoProb = 50;
        break;
        case 3:
        case 4:
            troncoProb = 25;
        break;
        default:
            troncoProb = 5;
        break;
    }

    if (ComprobarSalidaVentanaSprite(filaTroncos[indice].sprite, filaTroncos[indice].direccion)){
        // printf("INDICE DE TRONCO SALIDA -> %d\n", indice);
        if(indice == 3){
            filaTroncos[indice].isTronco = BoolPorProbabilidad(troncoProb);

            if(filaTroncos[indice].isTronco){
                AsignarValoresTronco(&filaTroncos[indice], indice, 4);
            }else{
                //Asignar valores cocodrilo con boca cerrada
                filaTroncos[indice].sprite.indiceAnimacion = 4;
                filaTroncos[indice].sprite.isActive = true;
            }
        }else{
            filaTroncos[indice].isTronco = filaTroncos[3].isTronco;

            if(filaTroncos[indice].isTronco){
                AsignarValoresTronco(&filaTroncos[indice], indice, 4);
                filaTroncos[indice].sprite.isVisible = true;
                filaTroncos[indice].sprite.isActive = true;
            }else{
                // printf("OCULTAR COLA -> %d\n", indice);
                // El cocodrilo mide 3, por lo tanto aseguramos que el resto que sobra del grupo
                // se oculte si es un coocodrilo
                if(indice <= 0){
                    filaTroncos[indice].sprite.isVisible = false;
                    filaTroncos[indice].sprite.isActive = false;
                }else{
                    filaTroncos[indice].sprite.indiceAnimacion = (indice-1)*2;
                    filaTroncos[indice].sprite.isActive = true;
                }
            }
        }

    }else{
        if(indice == 3 && !filaTroncos[indice].isTronco && HacerCadaX(&filaTroncos[indice].animBocaCocodrilo.temporizador, filaTroncos[indice].animBocaCocodrilo.duracion)){
            if(filaTroncos[indice].sprite.indiceAnimacion == 4){
                AvanzarSpriteAnimado(cocodriloSpriteSheet, cocodriloSpriteSheet_Coords, &filaTroncos[indice].sprite);
                filaTroncos[indice].sprite.isActive = false;
            }else{
                RetrocederSpriteAnimado(cocodriloSpriteSheet, cocodriloSpriteSheet_Coords, &filaTroncos[indice].sprite);
                filaTroncos[indice].sprite.isActive = true;
            }
        }
    }

    ActualizarEstadoTroncodrilo(&filaTroncos[indice]);
}

//Recorre todos los troncodrilos de la fila de un rio y actualiza sus estados
// array[] -> Fila de troncodrilos
// posibilidadCocodrilos -> Indica si el primero tronco, puede convertirse en determinadas circunstancias en un cocodrilo (Por defecto no puede)
void ActualizarEstadoFilaTroncodrilos(Troncodrilo array[], bool posibilidadCocodrilos = false){
    for(int i = 0; i < VENTANA_COLUMNAS; i++){
        if(posibilidadCocodrilos && i < 4){
            // TO_DO, DEBERÍA TENER UN STRUCT DE TIPO FilaTroncodrilo o similar con la información necesaria

            // 4 es el tamaño de grupo en la fila en la que pueden aparecer cocodrilos. 
            // Por lo tanto, dado que técnicamente solo el primero del array puede convertirse en cocodrilo
            // si el indice es mayor al número del grupo (Sprites del primero troncodrilo 0-3), 
            // se actualizará de forma normal
            ActualizarEstadoTroncodrilo(array, i);
        }else{
            ActualizarEstadoTroncodrilo(&array[i]);
        }
    }
}

void ActualizarEstadoRio(){
    for(int i = 0 ; i < ((int) FILA_RIO_T); i++){
        switch ((FilaRio) i){
            case FILA_RIO_0:
                ActualizarEstadoFilaTortugas(tortugas_1);
            break;
            case FILA_RIO_3:
                ActualizarEstadoFilaTortugas(tortugas_2);
            break;
            case FILA_RIO_1:
                ActualizarEstadoFilaTroncodrilos(troncos_1);
            break;
            case FILA_RIO_2:
                ActualizarEstadoFilaTroncodrilos(troncos_2);
            break;
            case FILA_RIO_4:
                ActualizarEstadoFilaTroncodrilos(troncos_3,true);
            break;
        }
    }
}

void ActualizarEstadoVehiculo(Vehiculo *vehiculo){
    ActualizarMovimientoObstaculo(&(*vehiculo).sprite,(*vehiculo).direccion,(*vehiculo).velocidadMovimiento);
    ActualizarSprite(vehiculosSpriteSheet,vehiculosSpriteSheet_Coords,&(*vehiculo).sprite);
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
        ActualizarSprite(vehiculosSpriteSheet,vehiculosSpriteSheet_Coords,&camiones[i].sprite);
    }
}

// Actualiza el estado de la Mosca bonus / cocodrilo trampa
void ActualizarEstadoMoscaCroc(){
    int randomPosition;

    if(moscaCroc.estado == PUNTOS_MC){
        // Comprobar que cuando se cambie a estado PUNTOS, ya tenga actualizado el sprite.
        // Esto solo sirve para reiniciar el contador de la aparicion de la mosca y la trampa cocodrilo y su nuevo esado
        if(HacerCadaX(&moscaCroc.animPuntos.temporizador, moscaCroc.animPuntos.duracion)){
            moscaCroc.sprite.isVisible = false;
            moscaCroc.sprite.isActive = false;
            moscaCroc.animMoscaCroc.temporizador = last_time;
            // En el nivel uno, será una mosca siempre, por lo tanto lo inicializa a esto
            if(jugadores[jugadorActual].dificultadActual <= 1){
                moscaCroc.estado = MOSCA;
            }else{
                //En el resto de niveles, al inicializar BoolPorProbabilidad será true cuando sea un cocodrilo trampa
                moscaCroc.estado = BoolPorProbabilidad(jugadores[jugadorActual].dificultadActual*10) ? CROC : MOSCA;
            }
        }
    }else{
        // Cada moscaCroc.animMoscaCroc.duracion, comprueba la aparición y desaparición del Moscacroc
        if(HacerCadaX(&moscaCroc.animMoscaCroc.temporizador,moscaCroc.animMoscaCroc.duracion)){
            if(moscaCroc.sprite.isVisible){
                moscaCroc.sprite.isVisible = false;
                moscaCroc.sprite.isActive = false;

                // A partir del nivel de dificultad 2, mediante un condicional que aumenta las posibilidades a mayor nivel de dificultad, 
                // comprueba de forma aleatoria si el siguiente moscaCroc que aparezca debe ser o no un cocodrilo trampa ((2*10) = 20 -> 20% posibilidades de trampa a dificultad 2 )
                if(jugadores[jugadorActual].dificultadActual >= 2){
                    //En este caso, BoolPorProbabilidad será true cuando sea un cocodrilo trampa
                    moscaCroc.estado = BoolPorProbabilidad(jugadores[jugadorActual].dificultadActual*10) ? CROC : MOSCA;
                }

            }else{
                //Genera una posicion aleatoria
                randomPosition = GenerarNumeroAleatorio(maxRanasFinales);

                //Si NO está ocupada por una ranaFin, entonces hace aparecer la mosca/croc
                if(!ranasFinales[randomPosition].isVisible){
                    moscaCroc.sprite.collider = ranasFinales[randomPosition].collider;

                    // Si es croc, en su primera instancia solo mostrará visible el hocico y no habilitará la colisión
                    if(moscaCroc.estado){
                        moscaCroc.sprite.tipoAnimacion = 1;
                        moscaCroc.sprite.indiceAnimacion = 0;
                    }else{
                        moscaCroc.sprite.isActive = true;
                        moscaCroc.sprite.tipoAnimacion = 0;
                        moscaCroc.sprite.indiceAnimacion = 0;
                    }

                    ActualizarSprite(moscaCrocSpriteSheet, moscaCrocSpriteSheet_Coords, &moscaCroc.sprite);
                    moscaCroc.sprite.isVisible = true;
                }
            }
        }else{
            // Si es un cocodrilo trampa visible, han pasado "animMoscaCroc.velocidad" ms y está con el hocico fuera...
            if(
                moscaCroc.estado == CROC && 
                moscaCroc.sprite.isVisible && 
                GetContadorFromTemp(moscaCroc.animMoscaCroc.temporizador) >= moscaCroc.animMoscaCroc.velocidad && 
                moscaCroc.sprite.tipoAnimacion == 1 &&
                moscaCroc.sprite.indiceAnimacion == 0
            ){
                //Activa su colisión y avanza de sprite para mostrar la cabeza por completo
                moscaCroc.sprite.isActive = true;
                AvanzarSpriteAnimado(moscaCrocSpriteSheet, moscaCrocSpriteSheet_Coords, &moscaCroc.sprite);
                ActualizarSprite(moscaCrocSpriteSheet, moscaCrocSpriteSheet_Coords, &moscaCroc.sprite);
            }
        }
    }
}

// Actualiza la animación de la serpiente
void ActualizarEstadoAnimacionSerpiente(Serpiente *serpiente){
    if(HacerCadaX(&(*serpiente).animMovimiento.temporizador, (*serpiente).animMovimiento.duracion)){

        // Comprobar direccion de la animación
        if((*serpiente).sprite.indiceAnimacion <= 0){
            (*serpiente).direccionAnimacion = true;
        }else{
            if((*serpiente).sprite.indiceAnimacion >= serpienteSpriteSheet.coordsAnim-2){
                (*serpiente).direccionAnimacion = false;
            }
        }

        //Avanzar/Retroceder Sprite
        if((*serpiente).direccionAnimacion){
            AvanzarSpriteAnimado(serpienteSpriteSheet, serpienteSpriteSheet_Coords, &(*serpiente).sprite);
        }else{
            RetrocederSpriteAnimado(serpienteSpriteSheet, serpienteSpriteSheet_Coords, &(*serpiente).sprite);
        }

        ActualizarSprite(serpienteSpriteSheet, serpienteSpriteSheet_Coords, &(*serpiente).sprite);
    }
}

// Comprueba si la serpiente está totalmente encima de un tronco en base a las colisiones detectadas en la actualización de estado de la misma
bool IsSerpienteSobreTronco(Serpiente *serpiente, int colisionesDetectadas[3]){
    bool isSobreTronco = false;
    int indice = 0, contador = 0;

    //Está sobre 3 sprites de tronco
    do{
        if(colisionesDetectadas[indice] != -1 && troncos_2[colisionesDetectadas[indice]].sprite.isVisible){
            contador++;
            isSobreTronco = contador >= 3;
        }
        indice++;
    } while (indice < 3 && !isSobreTronco);
    
    // printf("contador %d\n",contador);
    if(!isSobreTronco){
        indice = 0;
        contador = 0;
        //Está sobrepasando el borde izquierdo?
        if((*serpiente).sprite.collider.P1.x <= 0){
            //Está sobre 2 sprites de tronco?
            // La comprobación anterior se hace debido a que, fuera de los bordes solo puede haber un tronco, por lo tanto al estar la serpiente
            // en el borde esperando, justo cuando un segundo trozo del tronco completo se reubica está en contacto con 2 y además tiene un pequeño
            // margen de espacio para poder moverse antes de llegar al borde derecho del tronco
            do{
                if(colisionesDetectadas[indice] != -1 && troncos_2[colisionesDetectadas[indice]].sprite.isVisible){
                    contador++;
                    isSobreTronco = contador >= 2;
                }
                indice++;
            } while (indice < 3 && !isSobreTronco);
        }else{
            //Está sobrepasando el borde derecho?
            if((*serpiente).sprite.collider.P2.x >= VENTANA_X){
                //Fuerza el movimiento a la derecha para evitar "limbo"
                (*serpiente).direccion = DERECHA;
                isSobreTronco = true;
            }
        }

        
        
        // printf("WAITING FOR LOG\n");
        // for(int i = 0; i < 3; i++){
        //     if(colisionesDetectadas[i] != -1 && troncos_2[colisionesDetectadas[i]].sprite.isVisible){
        //         printf("Contact %d | %d \n",i,colisionesDetectadas[i]);
        //     }
        // }
        // printf("\n\n");
        // isSobreTronco = (*serpiente).sprite.collider.P2.x <= 0 && troncos_2[colisionesDetectadas[0]].sprite.isVisible;
        // if(!isSobreTronco){
        //     printf("WAITING FOR LOG\n");
        //     for(int i = 0; i < 3; i++){
        //         if(colisionesDetectadas[i] != -1 && troncos_2[colisionesDetectadas[i]].sprite.isVisible){
        //             printf("Contact %d | %d \n",i,colisionesDetectadas[i]);
        //         }
        //     }
        //     printf("\n\n");
        //     isSobreTronco = (*serpiente).sprite.collider.P2.x <= 0 && troncos_2[colisionesDetectadas[0]].sprite.isVisible && troncos_2[colisionesDetectadas[0]].sprite.isVisible;
        // }
    }

    return isSobreTronco;
}

// Actualiza los estados de una serpiente
// -Movimiento
// -Salida de pantalla
// -Respawn
void ActualizarEstadoSerpiente(Serpiente *serpiente){
    // Al reaparecer puede quedarse en medio de 3 colisiones. 
    // Los indices de estas se almacenen aquí, siendo la colision izquierda la primera.
    int colisionesDetectadas_T[3] = {-1,-1,-1};
    int colisionesDetectadas_I = 0, indiceActual = 0;
    bool isPlataforma = false;

    if((*serpiente).sprite.isVisible){
        //Lo primero de todo la colision con el jugador
        if(DetectarColisionJugador((*serpiente).sprite.collider) && !jugadores[jugadorActual].ranaJugador.isJumping){
            MatarJugador();
        }

        //Si sale por completo de la pantalla
        if(ComprobarSalidaVentanaSprite((*serpiente).sprite, (*serpiente).direccion)){
            //Respawnea la serpiente
            SpawnSerpiente(&(*serpiente));
        }

        // printf("Serpiente %d %d\n",i,(int)GetFilaPantallaSprite((*serpiente).sprite));
        if(GetFilaPantallaSprite((*serpiente).sprite) == 11){
            //Almacenar colisiones actuales si está en la segunda fila de troncos
            do{
                if(DetectarColision((*serpiente).sprite.collider, troncos_2[indiceActual].sprite.collider)){
                    colisionesDetectadas_T[colisionesDetectadas_I] = indiceActual;
                    colisionesDetectadas_I++;
                }
                indiceActual++;
            } while (colisionesDetectadas_I < 3 && indiceActual < VENTANA_COLUMNAS);

            //LOG
            // for(int i = 0 ; i < 3; i++){
            //     printf("%d |",colisionesDetectadas_T[i]);
            // }
            // printf("\n\n");

            //Si la última posición de las colisiones detectadas es distinta de -1, significa que está en conctacto con 3 plataformas activas
            if(IsSerpienteSobreTronco(&(*serpiente), colisionesDetectadas_T)){
                //Movimiento de tronco sobre la serpiente
                MoveCollider(&(*serpiente).sprite.collider, troncos_2[0].direccion, troncos_2[0].velocidadMovimiento);

                //Movimiento de la serpiente
                ActualizarEstadoAnimacionSerpiente(&(*serpiente));
                MoveCollider(&(*serpiente).sprite.collider, (*serpiente).direccion, (*serpiente).animMovimiento.velocidad);
            }else{
                //Accion colision con borde de tronco
                if((*serpiente).sprite.collider.P2.x > 0){
                    (*serpiente).direccion = (*serpiente).direccion == DERECHA ? IZQUIERDA : DERECHA;
                    if((*serpiente).direccion == troncos_2[0].direccion){
                        MoveCollider(&(*serpiente).sprite.collider, (*serpiente).direccion, (*serpiente).animMovimiento.velocidad+5);
                    }else{
                        MoveCollider(&(*serpiente).sprite.collider, (*serpiente).direccion, (*serpiente).animMovimiento.velocidad);
                    }
                }
            }
        }else{
            //Movimiento de la serpiente
            ActualizarEstadoAnimacionSerpiente(&(*serpiente));
            MoveCollider(&(*serpiente).sprite.collider, (*serpiente).direccion, (*serpiente).animMovimiento.velocidad);
        }
    }
}

// Actualiza los estados de todas las serpientes
void ActualizarEstadoSerpientes(){
    for (int i = 0; i < maxSerpientes; i++){
        ActualizarEstadoSerpiente(&serpientes[i]);
    }
}

// Devuelve el bool correspondiente después de determinar si el ataque de la nutria está en contacto con el sprite pasado por parámetro
// Util para indicar si se está acercando al rango de ataque
bool ComprobarColisionAtaqueNutria(Sprite sprite){
    return (DetectarColision(nutria.colisionAtaque.collider , sprite.collider) && sprite.isVisible);
}

// Devuelve el bool correspondiente después de determinar si la nutria está en contacto con el sprite pasado por parámetro
// Util para indicar si se la nutria debe ejecutar su ataque
bool ComprobarColisionNutria(Sprite sprite){
    return (DetectarColision(nutria.sprite.collider , sprite.collider) && sprite.isVisible);
}

// Comprueba si debe ejecutar el ataque de la nutria, matando así al jugador
void EjecutarAtaqueNutria(){
    //Si la nutria asoma por lo menos la mitad de su sprite por pantalla y el el jugador esta a rango del ataque de la nutria
    if(DetectarColisionJugador(nutria.colisionAtaque.collider) && nutria.sprite.collider.P2.x >= nutriaSpriteSheet.spriteWidth/2 && nutria.sprite.collider.P1.x <= VENTANA_X-nutriaSpriteSheet.spriteWidth/2){
        MatarJugador();
    }
}

// Actualiza los estados de la nutria
void ActualizarEstadoNutria(){
    int indiceColisionAtaque = 0;
    bool isColisionAtaque = false;

    if(nutria.sprite.isVisible){
        // Se resta 9 para identificar el numero de fila en base al rio (La primera fila del rio FILA_RIO_0 es la 9, pero su valor de enum es 0)
        FilaRio filanutria = (FilaRio)(GetFilaPantallaSprite(nutria.sprite)-9);
        // Detectar colisiones en la fila de la nutria
        // Empieza con la detección de colisión de la zona de ataque de la nutria. El indice que se almacene servirá 
        // para calcular el resto de colisiones y evitar repetir bucles
        do{
            switch (filanutria){
                case FILA_RIO_4:
                    isColisionAtaque = ComprobarColisionAtaqueNutria(troncos_3[indiceColisionAtaque].sprite);
                break;
                case FILA_RIO_3:
                    isColisionAtaque = ComprobarColisionAtaqueNutria(tortugas_2[indiceColisionAtaque].sprite);
                break;
                case FILA_RIO_2:
                    isColisionAtaque = ComprobarColisionAtaqueNutria(troncos_2[indiceColisionAtaque].sprite);
                break;
                case FILA_RIO_1:
                    isColisionAtaque = ComprobarColisionAtaqueNutria(troncos_1[indiceColisionAtaque].sprite);
                break;
                case FILA_RIO_0:
                    isColisionAtaque = ComprobarColisionAtaqueNutria(tortugas_1[indiceColisionAtaque].sprite);
                break;
            }
            indiceColisionAtaque++;
        } while (indiceColisionAtaque < VENTANA_COLUMNAS && !isColisionAtaque);
        
        // Si el ataque está en contacto
        if(isColisionAtaque){
            //Animacion nutria cerca de morder
            if(HacerCadaX(&nutria.animMovimiento.temporizador, nutria.animMovimiento.duracion)){
                AvanzarSpriteAnimado(nutriaSpriteSheet, nutriaSpriteSheet_Coords, &nutria.sprite);
                ActualizarSprite(nutriaSpriteSheet, nutriaSpriteSheet_Coords, &nutria.sprite);
            }

            //Colision del cuerpo de la nutria con obstaculo
            // printf("indiceColisionAtaque %d\n", indiceColisionAtaque);
            switch (filanutria){
                case FILA_RIO_4:
                    if(ComprobarColisionNutria(troncos_3[indiceColisionAtaque - (nutria.direccion == DERECHA ? 1 : 0)].sprite)){
                        nutria.colisionAtaque.isActive = true;
                    }
                break;
                case FILA_RIO_3:
                    if(ComprobarColisionNutria(tortugas_2[indiceColisionAtaque - (nutria.direccion == DERECHA ? 1 : 0)].sprite)){
                        nutria.colisionAtaque.isActive = true;
                    }
                break;
                case FILA_RIO_2:
                    if(ComprobarColisionNutria(troncos_2[indiceColisionAtaque - (nutria.direccion == DERECHA ? 1 : 0)].sprite)){
                        nutria.colisionAtaque.isActive = true;
                    }
                break;
                case FILA_RIO_1:
                    if(ComprobarColisionNutria(troncos_1[indiceColisionAtaque - (nutria.direccion == DERECHA ? 1 : 0)].sprite)){
                        nutria.colisionAtaque.isActive = true;
                    }
                break;
                case FILA_RIO_0:
                    if(ComprobarColisionNutria(tortugas_1[indiceColisionAtaque - (nutria.direccion == DERECHA ? 1 : 0)].sprite)){
                        nutria.colisionAtaque.isActive = true;
                    }
                break;
            }

            if(nutria.colisionAtaque.isActive){
                EjecutarAtaqueNutria();
                SpawnNutria();
            }
        }
        
        //Movimiento de la nutria
        ActualizarMovimientoObstaculo(&nutria.sprite, nutria.direccion, nutria.animMovimiento.velocidad);
        ActualizarMovimientoObstaculo(&nutria.colisionAtaque, nutria.direccion, nutria.animMovimiento.velocidad, nutriaSpriteSheet);
    }
}

void ActualizarSaltoRana(Rana *rana, bool isPlayer = true){
    // Durante la duración del salto, irá avanzando su velocidad en cada iteración distanciaSalto/duracionSalto
    if(HacerDuranteX(&(*rana).animSalto.temporizador,(*rana).animSalto.duracion)){
        MoveCollider(&(*rana).sprite.collider, (*rana).direccion, (*rana).animSalto.velocidad);
    }else{
        (*rana).isJumping = false;
        (*rana).sprite.collider = (*rana).finSalto;
        (*rana).sprite.indiceAnimacion = 0;

        //Si es el jugador el que salta, hace estas acciones extra
        if(isPlayer){
            ActualizarSprite(ranaBaseSpriteSheet, ranasSpriteSheet_Coords, &(*rana).sprite);
            SumarPuntosAvanceVertical();
        }
    }
}

// La rana rota en base al sentido indicado
// true -> Horario
// false -> Antihorario
void RotarRanaBonus(bool sentidoHorario = true){
    // printf("RotarRanaBonus\n");
    if(sentidoHorario){
        if(ranaBonus.rana.direccion == IZQUIERDA){
            ranaBonus.rana.direccion = ARRIBA;
        }else{
            ranaBonus.rana.direccion = (Direccion)(ranaBonus.rana.direccion + 1);
        }
    }else{
        if(ranaBonus.rana.direccion == ARRIBA){
            ranaBonus.rana.direccion = IZQUIERDA;
        }else{
            ranaBonus.rana.direccion = (Direccion)(ranaBonus.rana.direccion - 1);
        }
    }
    ranaBonus.rana.sprite.tipoAnimacion = ranaBonus.rana.direccion;
}

// Comprueba si la rana bonus NO está ubicada en el borde de un tronco mientras quiere saltar
// en la direccion de ese mismo borde
bool ComprobarRanaBonusBordeTronco(int colisionesDetectadas[]){
    return(
        (ranaBonus.rana.direccion == DERECHA && troncos_1[colisionesDetectadas[2]].sprite.isVisible) ||
        (ranaBonus.rana.direccion == IZQUIERDA && troncos_1[colisionesDetectadas[0]].sprite.isVisible)
    );
}

// Hace que el último valor sea el primero y los otros 2 avancen una posicion
// Pensado para la deteccion de colisiones de movimiento de la rana bonus
void ReordenarColisionesRanaBonus(int colisiones[]){
    int aux;
    aux = colisiones[0];
    colisiones[0] = (colisiones[2] == -1 ? 14 : colisiones[2]);
    colisiones[2] = colisiones[1];
    colisiones[1] = aux;
}

void ActualizarMovimientoRanaBonus(){
    // Los indices de las colisiones de la rana bonus se almacenen aquí, 
    // siendo la colision izquierda la primera, la del medio donde debería estar ubicada la rana y la derecha la ultima
    // con respecto al tronco en el que se ubica
    int colisionesDetectadas_T[3] = {-1,-1,-1};
    int colisionesDetectadas_I = 0, indiceActual = 0;
    
    // Movimiento a causa del rio
    ActualizarMovimientoObstaculo(&ranaBonus.rana.sprite, troncos_1[0].direccion, troncos_1[0].velocidadMovimiento);
    if(ranaBonus.rana.sprite.collider.P2.x <= 0){
        ranaBonus.isBug = false;
        ranaBonus.rana.sprite.isVisible = true;
    }
    //Se reubica el fin de salto a la ubicacion almacenada sobre la que la rana bonus quiere saltar 
    RellocateCollider(&ranaBonus.rana.finSalto, troncos_1[ranaBonus.indiceTroncoFin].sprite.collider.P1);

    if(HacerCadaX(&ranaBonus.accionarSalto.temporizador, ranaBonus.accionarSalto.duracion)){
        //Almacenar colisiones actuales
        do{
            // Con su tamaño +10 en x por cada lado para asegurar, entra en colisión con 3 zonas de tronco de la fila
            if(DetectarColision(
                {
                    {ranaBonus.rana.sprite.collider.P1.x-15,ranaBonus.rana.sprite.collider.P1.y},
                    {ranaBonus.rana.sprite.collider.P2.x+15,ranaBonus.rana.sprite.collider.P2.y}
                }, 
                troncos_1[indiceActual].sprite.collider
            )){
                colisionesDetectadas_T[colisionesDetectadas_I] = indiceActual;
                colisionesDetectadas_I++;
            }
            indiceActual++;
        } while (colisionesDetectadas_I < 3 && indiceActual < VENTANA_COLUMNAS);

        // Ya que el valor VENTANA_COLUMNAS-1 es un valor con el que solo podría tener contacto por el lado izquierdo
        // en caso de que el valor que corresponde al lado derecho sea VENTANA_COLUMNAS-1 o -1, reordena los valores para que
        // esté al lado izquierdo
        if(colisionesDetectadas_T[2] == VENTANA_COLUMNAS-1 || colisionesDetectadas_T[2] == -1){
            ReordenarColisionesRanaBonus(colisionesDetectadas_T);
        }

        //LOG
        // printf("Colisiones plataforma RanaBonus\n");
        // for(int i = 0; i < 3; i++){
        //     printf("%d ", colisionesDetectadas_T[i]);
        // }
        // printf("\n\n");

        // Se comprueba si está en el borde
        if(ComprobarRanaBonusBordeTronco(colisionesDetectadas_T)){
            ranaBonus.direccionRotacion = ranaBonus.rana.direccion;
            //Almacena el indice del tronco sobre el que se quiere terminar el salto para evitar descuadrarse
            if(ranaBonus.rana.direccion == DERECHA){
                ranaBonus.indiceTroncoFin = colisionesDetectadas_T[2];
            }else{
                ranaBonus.indiceTroncoFin = colisionesDetectadas_T[0];
            }
            // printf("indiceTroncoFin %d",ranaBonus.indiceTroncoFin);
            IniciarSaltoRana(&ranaBonus.rana, ranaBonus.rana.direccion, false);
        }else{
            // printf("RotarRanaBonus %d\n", ranaBonus.direccionRotacion);
            RotarRanaBonus(ranaBonus.direccionRotacion == IZQUIERDA);
        }
    }else{
        if(ranaBonus.rana.isJumping){
            ActualizarSaltoRana(&ranaBonus.rana);
        }
    }
}

void ActualizarEstadoRanaBonus(){
    switch (ranaBonus.estadoUpdate){
        case LIBRE:
            ActualizarMovimientoRanaBonus();
            //Preparar sprite para dibujar después de los cambios de estado
            if(ranaBonus.isBug){
                ActualizarSprite(ranaRojaSpriteSheet, ranasSpriteSheet_Coords, &ranaBonus.rana.sprite);
            }else{
                ActualizarSprite(ranaRosaSpriteSheet, ranasSpriteSheet_Coords, &ranaBonus.rana.sprite);
            }
        break;
        case ANCLADA:
            ranaBonus.rana.sprite.collider = jugadores[jugadorActual].ranaJugador.sprite.collider;
            ranaBonus.rana.sprite.tipoAnimacion = jugadores[jugadorActual].ranaJugador.sprite.tipoAnimacion;

            //Preparar sprite para dibujar después de los cambios de estado
            if(ranaBonus.isBug){
                ActualizarSprite(ranaRojaSpriteSheet, ranasSpriteSheet_Coords, &ranaBonus.rana.sprite);
            }else{
                ActualizarSprite(ranaRosaSpriteSheet, ranasSpriteSheet_Coords, &ranaBonus.rana.sprite);
            }
        break;
        case PUNTOS_RB:
            if(HacerCadaX(&ranaBonus.animPuntos.temporizador, ranaBonus.animPuntos.duracion)){
                ranaBonus.estadoUpdate = LIBRE;
                SpawnRanaBonus();
            }
        break;
    }
}

//Actualización del estado de la animacion de muerte del jugador y sus consecuencias
void ActualizarMuerteJugador(){
    if(HacerCadaX(&jugadores[jugadorActual].animMuerte.temporizador, jugadores[jugadorActual].animMuerte.velocidad)){
        //Fin animacion muerte
        if(jugadores[jugadorActual].ranaJugador.sprite.indiceAnimacion >= animMuerteSpriteSheet.totalCoordsAnim-2){
            RestarVidasJugador(1);
        }else{
            AvanzarSpriteAnimado(animMuerteSpriteSheet, animMuerteSpriteSheet_Coords, &jugadores[jugadorActual].ranaJugador.sprite);
            ActualizarSprite(animMuerteSpriteSheet, animMuerteSpriteSheet_Coords, &jugadores[jugadorActual].ranaJugador.sprite);
        }
    }
}

// Se encarga de efectuar correctamente la animación de avance de nivel
void ActualizarEstadoAvanceNivel(){
    int indice = 0;
    if(HacerCadaX(&animAvanceNivel.temporizador, animAvanceNivel.velocidad)){
        //Comprueba cual es el siguiente que falta por cambiar su sprite
        while(indice < maxRanasFinales && ranasFinales[indice].indiceAnimacion == 2){
            indice++;
        }

        //Si están todos cambiados, avanza de nivel
        if(indice >= maxRanasFinales){
            isAnimAvanceNivel = false;
            AvanzarNivel();
        }else{
            //Si no, actualiza el que ha detectado que falta por cambiar
            ranasFinales[indice].indiceAnimacion = 2;
            ActualizarSprite(ranaFinSpriteSheet, ranaFinSpriteSheet_Coords, &ranasFinales[indice]);
        }
    }
    
}

void ActualizarEstadoJugador(){
    ComprobarColisionesJugador();

    if(!jugadores[jugadorActual].ranaJugador.sprite.isActive){
        //Si la rana del jugador está muerta...
        ActualizarMuerteJugador();
    }else{
        //En caso de que no...
        //Si la rana está saltando actualizará el estado del salto
        if(jugadores[jugadorActual].ranaJugador.isJumping){
            ActualizarSaltoRana(&jugadores[jugadorActual].ranaJugador);
        }
    }
}

// Calcula cada cuanto tiempo debe ejecutarse algo dada una duracion, una distancia y una velocidad
// El resultado está pensado para utilizarse en una funcion HacerCadaX como duración
float CalcularTiempoPorAvance(float duracion, float distanciaTotal, float velocidad){
    // printf("duracion distanciaTotal velocidad | %.2f %.2f %.2f\n", duracion, distanciaTotal, velocidad);
    // printf("Movimientos necesarios: %.2f\n", distanciaTotal/velocidad);
    // printf("Movimiento cada %d ms\n", (int)(duracion / (distanciaTotal/velocidad)));
    return (duracion / (distanciaTotal/velocidad));
}

void ComprobarDireccionAnimacionRanaIntro(Rana *rana){
    // Si está en indice 0, cambia el sentido de la animacion y no comprueba nada mas
    if((*rana).sprite.indiceAnimacion == 0){
        (*rana).isJumping = true;
    }else {
        if((*rana).sprite.indiceAnimacion == ranaIntroSpriteSheet.coordsAnim-2){
            (*rana).isJumping = false;
        }
    }
}

//Lógica del avance en horizontal de la introducción
void AvanceHorizontalIntro(int contador){
    // tiempoCorregido se usa como contador en esta funcion para poder determinar mas facilmente la duración de cada animacion de cada letra.
    // comprobando el valor con el contador para asegurar que se ejecuta en el tiempo que toca.
    int tiempoCorregido = 0;
    for(int i = 0; i < letraActual; i++){
        //Calcula todo el tiempo ocupado por las letras anteriores actual y lo almacena
        tiempoCorregido += letrasTitulo[i].animSalto.duracion;
    }

    //Luego le resta este tiempo calculado al contador y lo devuelve
    tiempoCorregido = contador-(tiempoCorregido/1000);
    
    //Se restan 10 centesimas a CalcularTiempoPorAvance para dar margen a la animación y funcione correctamente
    if (tiempoCorregido <= letrasTitulo[letraActual].animSalto.duracion/1000 && HacerCadaX(&letrasTitulo[letraActual].animSalto.temporizador, CalcularTiempoPorAvance(letrasTitulo[letraActual].animSalto.duracion, letrasTitulo[letraActual].distanciaSalto, letrasTitulo[letraActual].animSalto.velocidad)-10)){
        MoveCollider(&letrasTitulo[letraActual].sprite.collider,letrasTitulo[letraActual].direccion, letrasTitulo[letraActual].animSalto.velocidad);

        if(tiempoCorregido >= letrasTitulo[letraActual].animSalto.duracion/1000){
            //Se asegura de reubicar la rana en la posicion correcta y actualiza el sprite al de la rana mirando hacia arriba
            RellocateSprite(&letrasTitulo[letraActual].sprite, letrasTitulo[letraActual].finSalto.P1);
            letrasTitulo[letraActual].sprite.tipoAnimacion = 0;
            letrasTitulo[letraActual].sprite.indiceAnimacion = 0;
            ActualizarSprite(ranaIntroSpriteSheet,ranaIntroSpriteSheet_Coords,&letrasTitulo[letraActual].sprite);

            // Avanza la letra actual (variable global para poder vigilarla comodamente) antes de reiniciar el temporizador
            // Reiniciando así el temporizador de la siguiente "letra" a animar
            letrasTitulo[++letraActual].animSalto.temporizador=last_time;
        }else{
            // Para aprovechar el Struct de rana, se utiliza isJumping como indicador de 
            // Avance/Retroceso de frame de animación
            ComprobarDireccionAnimacionRanaIntro(&letrasTitulo[letraActual]);
            if(letrasTitulo[letraActual].isJumping){
                AvanzarSpriteAnimado(ranaIntroSpriteSheet,ranaIntroSpriteSheet_Coords, &letrasTitulo[letraActual].sprite);
            }else{
                RetrocederSpriteAnimado(ranaIntroSpriteSheet,ranaIntroSpriteSheet_Coords, &letrasTitulo[letraActual].sprite);
            }

            ActualizarSprite(ranaIntroSpriteSheet,ranaIntroSpriteSheet_Coords,&letrasTitulo[letraActual].sprite);
        }

    }

}

//Lógica del avance en vertical de la introducción de todas las letras a la vez
void AvanceVerticalIntro(int contador){
    for(int i = 0; i < maxLetrasTitulo; i++){
        //Se restan 10 centesimas a CalcularTiempoPorAvance para dar margen a la animación y funcione correctamente
        if (contador <= 34 && HacerCadaX(&letrasTitulo[i].animSalto.temporizador, CalcularTiempoPorAvance(letrasTitulo[i].animSalto.duracion, letrasTitulo[i].distanciaSalto, letrasTitulo[i].animSalto.velocidad)-10)){
            MoveCollider(&letrasTitulo[i].sprite.collider,letrasTitulo[i].direccion, letrasTitulo[i].animSalto.velocidad);

            if(contador >= 34){
                //Se asegura de reubicar la rana en la posicion y frame correctos
                RellocateSprite(&letrasTitulo[i].sprite, letrasTitulo[i].finSalto.P1);
                letrasTitulo[i].sprite.indiceAnimacion = 0;

                // Reiniciar el temporizador
                letrasTitulo[i].animSalto.temporizador=last_time;
            }else{
                // Para aprovechar el Struct de rana, se utiliza isJumping como indicador de 
                // Avance/Retroceso de frame de animación
                ComprobarDireccionAnimacionRanaIntro(&letrasTitulo[i]);
                if(letrasTitulo[i].isJumping){
                    AvanzarSpriteAnimado(ranaIntroSpriteSheet,ranaIntroSpriteSheet_Coords, &letrasTitulo[i].sprite);
                }else{
                    RetrocederSpriteAnimado(ranaIntroSpriteSheet,ranaIntroSpriteSheet_Coords, &letrasTitulo[i].sprite);
                }
            }

            ActualizarSprite(ranaIntroSpriteSheet,ranaIntroSpriteSheet_Coords,&letrasTitulo[i].sprite);
        }
    }

    // Como todas se mueven a la vez, si la primera llega al final, todas deberían haber llegado al final
    // Terminando la animacion iniciada y asegurando que la animacion final empieza por la primera letra
    if(AreColliderEqual(letrasTitulo[0].sprite.collider,letrasTitulo[0].finSalto)){
        letraActual = 0;
        isAnimIntroVIniciada = false;
    }
}

//Actualiza los estados de las ranas en la pantalla de Intro para generar la animación inicial
void ActualizarEstadoIntro(){
    int contador = GetContadorFromTemp(animIntro.temporizador,1000);

    if(contador < 30){
        AvanceHorizontalIntro(contador);
    }else{
        if(!isAnimIntroVIniciada && contador <= 30){
            // Si la animacion vertical no esta iniciada
            // Se asegura de reubicar la nueva posicion final a todas
            // las ranas de la animacion y cambiar sus valores de animación 
            // para coincidir con la animacion vertical
            for(int i = 0; i < maxLetrasTitulo; i++){
                if(i == 0){
                    letrasTitulo[i].finSalto = {
                        {(i+2.0f)*SPRITE_SIZE,SPRITE_SIZE*3},
                        {(i+3.0f)*SPRITE_SIZE,SPRITE_SIZE*4}
                    };
                }else{
                    letrasTitulo[i].finSalto = {
                        {letrasTitulo[i-1].finSalto.P2.x+(SPRITE_SIZE/2),letrasTitulo[i-1].finSalto.P1.y},
                        {letrasTitulo[i-1].finSalto.P2.x+((SPRITE_SIZE/2)+SPRITE_SIZE),letrasTitulo[i-1].finSalto.P2.y},
                    };
                }
                letrasTitulo[i].distanciaSalto = letrasTitulo[i].sprite.collider.P1.y-letrasTitulo[i].finSalto.P1.y;
                letrasTitulo[i].animSalto.temporizador = last_time;
                letrasTitulo[i].animSalto.duracion = 4000;
                letrasTitulo[i].direccion = ARRIBA;
            }
            // Indica que la animacion vertical
            // está iniciada para que actualice su estado
            isAnimIntroVIniciada = true;
        }

        if(contador <= 34 && isAnimIntroVIniciada){
            AvanceVerticalIntro(contador);
        }else{
            // printf("CONTADOR %d\n",contador);
            //Usamos el contador de la primera letra ya que no hay diferencias de tiempo entre aparición de letras
            if(letraActual < maxLetrasTitulo && HacerCadaX(&letrasTitulo[0].animSalto.temporizador, 3000/maxLetrasTitulo)){
                letrasTitulo[letraActual].sprite.tipoAnimacion = 0;

                switch(letraActual){
                    case 0:
                        //AsignarLetra F
                        letrasTitulo[letraActual].sprite.indiceAnimacion = 0;
                    break;
                    case 1:
                    case 6:
                        //AsignarLetra R
                        letrasTitulo[letraActual].sprite.indiceAnimacion = 2;    
                    break;
                    case 2:
                        //AsignarLetra O
                        letrasTitulo[letraActual].sprite.indiceAnimacion = 4;
                    break;
                    case 3:
                    case 4:
                        //AsignarLetra G
                        letrasTitulo[letraActual].sprite.indiceAnimacion = 6;
                    break;
                    case 5:
                        //AsignarLetra E
                        letrasTitulo[letraActual].sprite.indiceAnimacion = 8;
                    break;
                }

                ActualizarSprite(tituloSpriteSheet,tituloSpriteSheet_Coords, &letrasTitulo[letraActual].sprite);
                letraActual++;
            }
        }
    }
}

void ActualizarEstadoGameOver(){
    if(HacerDuranteX(&animGameOver.temporizador, animGameOver.duracion)){
        if(GetContadorFromTemp(animGameOver.temporizador, 1000) >= 1){
            barridoGameOver_Azul.P1.x -= animGameOver.velocidad;
            barridoGameOver_Negro.P1.x -= animGameOver.velocidad;
        }
    }else{
        isAnimGameOver = false;
        switch (gamemode){
            case NORMAL_SINGLE:
                GuardarPuntuacion();
                ComprobarCreditos();
            break;

            case NORMAL_MULTI:
                for(int i = 0; i < maxJugadores; i++){
                    jugadorActual = i;
                    GuardarPuntuacion();
                }
                ComprobarCreditos();
            break;
        }
    }
}

void ActualizarEstadoJuego(){
    if(isAnimGameOver){
        ActualizarEstadoGameOver();
    }else{
        ActualizarEstadoRio();
        ActualizarEstadoVehiculos();

        ActualizarEstadoMoscaCroc();
        ActualizarEstadoSerpientes();
        ActualizarEstadoNutria();
        ActualizarEstadoRanaBonus();

        if(isAnimAvanceNivel){
            ActualizarEstadoAvanceNivel();
        }else{
            ActualizarEstadoJugador();
        }
    }
    
}

void ActualizarEstados(){
    switch(pantallaActual){
        case INTRO:
            ActualizarEstadoIntro();
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
    esat::DrawPath(coords,5);
}

//** Dibujados de texto y sobrecargas en función de los parametros  **/

// Muestra el texto indicado por pantalla con las características proporcionadas.
//  -texto -> Texto a mostrar
//  -ubicacion -> Indica un punto de coordenadas específico desde el que empezar a dibujar
//  -color  -> Indica de que color se mostrará el texto con el struct Color en formato RGBa. Por defecto 200,200,200,255
void DrawText(char texto[], PuntoCoord ubicacion, Color color = {200,200,200,255}){
    esat::DrawSetFillColor(color.r,color.g,color.b,color.a);
    esat::DrawText(ubicacion.x, ubicacion.y,texto);
}

// Muestra el texto indicado por pantalla con las características proporcionadas.
//  -texto -> Texto a mostrar
//  -longitud -> Longitud del texto para calcular correctamente las alineaciones
//  -ubi_y -> Indica una ubicacion específica en coordenada Y
//  -alineacionH -> Alineacion en vertical
//  -margen_X -> Desplazamiento en horizontal adicional a partir de alineacionH (indicar positivo o negativo)
//  -color  -> Indica de que color se mostrará el texto con el struct Color en formato RGBa. Por defecto 200,200,200,255
void DrawText(char texto[], int longitud, float ubi_y, Align_H alineacion, float margen_x, Color color = {200,200,200,255}){
    PuntoCoord ubicacion = {0.0,ubi_y};
    switch(alineacion){
        case LEFT:
            ubicacion.x = VENTANA_X-(FONT_SIZE*longitud);
            break;
        case CENT:
            ubicacion.x = VENTANA_X/2-(FONT_SIZE*(longitud/2-(longitud%2 == 0 ? 1 : 0)));
            break;
        case RIGHT:
            ubicacion.x = 0;
            break;
    }
    ubicacion.x += margen_x;

    esat::DrawSetFillColor(color.r,color.g,color.b,color.a);
    esat::DrawText(ubicacion.x, ubicacion.y,texto);
}

// Muestra el texto indicado por pantalla con las características proporcionadas.
//  -texto -> Texto a mostrar
//  -longitud -> Longitud del texto para calcular correctamente las alineaciones
//  -ubi_x -> Indica una ubicacion específica en coordenada X
//  -alineacionV -> Alineacion en vertical
//  -margen_y -> Desplazamiento en vertical adicional a partir de alineacionV (indicar positivo o negativo)
//  -color  -> Indica de que color se mostrará el texto con el struct Color en formato RGBa. Por defecto 200,200,200,255
void DrawText(char texto[], int longitud, float ubi_x, Align_V alineacion, float margen_y, Color color = {200,200,200,255}){
    PuntoCoord ubicacion = {ubi_x,0.0};
    switch(alineacion){
        case TOP:
            ubicacion.y = FONT_SIZE;
            break;
        case MID:
            ubicacion.y = VENTANA_Y/2-(FONT_SIZE*((longitud/2)+1));
            break;
        case BOT:
            ubicacion.y = VENTANA_Y;
            break;
    }

    ubicacion.y += margen_y;

    esat::DrawSetFillColor(color.r,color.g,color.b,color.a);
    esat::DrawText(ubicacion.x, ubicacion.y,texto);
}

void DrawRect(Collider collider, Color color){
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
    esat::DrawSetFillColor(color.r,color.g,color.b,color.a);
    esat::DrawSetStrokeColor(0,0,0,0);
    esat::DrawSolidPath(coords,5);
}

// Muestra el texto indicado por pantalla con las características proporcionadas.
//  -texto -> Texto a mostrar
//  -longitud -> Longitud del texto para calcular correctamente las alineaciones
//  -alineacionV -> Alineacion en vertical
//  -alineacionH -> Alineacion en horizontal
//  -margen_y -> Desplazamiento en vertical adicional a partir de alineacionV (indicar positivo o negativo)
//  -margen_x -> Desplazamiento en horizontal adicional a partir de alineacionH (indicar positivo o negativo)
//  -color  -> Indica de que color se mostrará el texto con el struct Color en formato RGBa. Por defecto 200,200,200,255
void DrawText(char texto[], int longitud, Align_V alineacionV, Align_H alineacionH, float margen_y, float margen_x, Color color = {200,200,200,255}, Color background = {0,0,0,0}){
    PuntoCoord ubicacion = {0.0,0.0};
    switch(alineacionH){
        case LEFT:
            ubicacion.x = 0;
            break;
        case CENT:
            //Le restara 1 a la longitud a desplazarse si es par para corregir el centro segun el juego original (Operación ternaria)
            ubicacion.x = VENTANA_X/2-(FONT_SIZE*(longitud/2-(longitud%2 == 0 ? 1 : 0)));
            break;
        case RIGHT:
            ubicacion.x = VENTANA_X-(FONT_SIZE*longitud);
            break;
    }
    switch(alineacionV){
        case TOP:
            ubicacion.y = FONT_SIZE;
            break;
        case MID:
            ubicacion.y = VENTANA_Y/2;
            break;
        case BOT:
            ubicacion.y = VENTANA_Y;
            break;
    }

    ubicacion.x += margen_x;
    ubicacion.y += margen_y;

    DrawRect({ubicacion, {ubicacion.x+(FONT_SIZE*longitud),ubicacion.y-FONT_SIZE}},background);
    esat::DrawSetFillColor(color.r,color.g,color.b,color.a);
    esat::DrawText(ubicacion.x, ubicacion.y,texto);
}

// Dibuja el sprite si es visible y el borde de la colision si se activa el modo "debug"
void DrawSprite(Sprite sprite, bool isPlayer = false){
    if(areCollidersVisible){
        if(sprite.isActive){
            esat::DrawSetStrokeColor(0,255,0);
        }else{
            esat::DrawSetStrokeColor(255,0,0);
        }

        if(isPlayer){
            DrawCollider({
                    {sprite.collider.P1.x+10, sprite.collider.P1.y+10},
                    {sprite.collider.P2.x-10, sprite.collider.P2.y-10}
                }
            );
        }else{
            DrawCollider(sprite.collider);
        }
    }

    if(sprite.isVisible){
        esat::DrawSprite(sprite.imagen, sprite.collider.P1.x, sprite.collider.P1.y);
    }
}

// Dibuja el sprite si es visible y el borde de la colision si se activa el modo "debug"
// Incluye posibilidad de transformación
void DrawSprite(Sprite sprite, esat::SpriteTransform transform, bool isPlayer = false){
    if(areCollidersVisible){
        if(sprite.isActive){
            esat::DrawSetStrokeColor(0,255,0);
        }else{
            esat::DrawSetStrokeColor(255,0,0);
        }

        if(isPlayer){
            DrawCollider({
                    {sprite.collider.P1.x+10, sprite.collider.P1.y+10},
                    {sprite.collider.P2.x-10, sprite.collider.P2.y-10}
                }
            );
        }else{
            DrawCollider(sprite.collider);
        }
    }

    if(sprite.isVisible){
        esat::DrawSprite(sprite.imagen, transform);
    }
}

void DibujarFondoRio(){
    DrawRect({{0,0},{VENTANA_X,(VENTANA_Y)/2}},{0,0,70});
}

//-- DIBUJADO PANTALLA DE INTRO --//
void DibujarTitulo(){
    for(int i = 0; i < maxLetrasTitulo; i++){
        DrawSprite(letrasTitulo[i].sprite);
        DrawCollider(letrasTitulo[i].finSalto);
    }
}

void DibujarCopyrightKonami(){
    esat::DrawSetTextSize(FONT_SIZE);
    DrawText("KONAMI",0,MID,LEFT,FONT_SIZE*13,FONT_SIZE*6);
    esat::DrawSprite(copySprite,VENTANA_X/2,(VENTANA_Y/2)+(FONT_SIZE*12));
    DrawText("1981",4,MID,RIGHT,FONT_SIZE*13,FONT_SIZE*-7);
    esat::DrawSetTextSize(FONT_SIZE);
}

void DibujarIntro(){
    if(HacerDuranteX(&animIntro.temporizador,animIntro.duracion)){
        // La los movimientos de la animacion se ejecutan en la actualización de estados.
        // solo si está en la pantalla de INTRO
        DibujarTitulo();
    }else{
        InicializarTitulo();
        animPuntuaciones.temporizador = last_time;
        pantallaActual = PUNTUACIONES;
    }
}

//-- DIBUJADO PANTALLA DE PUNTUACIONES --//
void DibujarPuntuaciones(){
    // int limiteAparicion sirve para calcular cada cuanto tiempo deberá de aparecer cada texto comparandolo con el resultado de GetContadorFromTemp
    // Por cada limiteAparicion += 2 en este caso, tarda 2 segundos mas en aparecer que el if anterior;
    int limiteAparicion = 1, contador;
    if(HacerDuranteX(&animPuntuaciones.temporizador,animPuntuaciones.duracion)){
        DibujarTitulo();
        DrawText("-POINT TABLE-",13,MID,CENT,FONT_SIZE*-5,0);

        contador = GetContadorFromTemp(animPuntuaciones.temporizador);

        if(contador >= (int)(animPuntuaciones.velocidad)*limiteAparicion){
            //Los que están alineados a la izquierda, no importa su longitud de texto
            DrawText("10 PTS FOR EACH STEP",0,MID,LEFT,FONT_SIZE*-2,FONT_SIZE*2,{200,200,0});
        }
        limiteAparicion+=2;
        if(contador >= (int)(animPuntuaciones.velocidad)*limiteAparicion){
            DrawText("50 PTS FOR EVERY FROG",0,MID,LEFT,FONT_SIZE,FONT_SIZE*2,{200,200,0});
            DrawText("ARRIVED HOME SAFELY",0,MID,LEFT,FONT_SIZE*2,FONT_SIZE*2,{200,0,0});
        }
        limiteAparicion+=2;
        if(contador >= (int)(animPuntuaciones.velocidad)*limiteAparicion){
            DrawText("1000 PTS BY SAVING FROGS",0,MID,LEFT,FONT_SIZE*4,FONT_SIZE*2,{200,200,0});
            DrawText("INTO FIVE HOMES",0,MID,LEFT,FONT_SIZE*5,FONT_SIZE*2,{200,0,0});
        }
        limiteAparicion+=2;
        if(contador >= (int)(animPuntuaciones.velocidad)*limiteAparicion){
            DrawText("PLUS BONUS",0,MID,LEFT,FONT_SIZE*7,FONT_SIZE*2,{200,200,0});
            DrawText("10 PTS X REMAINING SECOND",0,MID,LEFT,FONT_SIZE*8,FONT_SIZE*2,{200,0,0});
        }

        DibujarCopyrightKonami();
    }else{
        InicializarTitulo();
        animRanking.temporizador = last_time;
        pantallaActual = RANKING;
    }

}

//-- DIBUJADO PANTALLA DE RANKING --//
void DibujarRanking(){
    char scoreDigits[maxScoreDigits];
    if(HacerDuranteX(&animRanking.temporizador,animRanking.duracion)){
        DibujarTitulo();

        DrawText("SCORE RANKING",13,MID,CENT,FONT_SIZE*-3,0,{200,200,0});

        for(int i = 0, k = 0; i < maxRankingScores; i++, k++){
            switch (i){
            case 0:
                if(jugadores[jugadorActual].puntuacion != 0 && jugadores[jugadorActual].puntuacion == rankingScores[i]){
                    DrawText("1 ST",4,MID,CENT,FONT_SIZE*(i+k),FONT_SIZE*-6,{220,70,220});
                }else{
                    DrawText("1 ST",4,MID,CENT,FONT_SIZE*(i+k),FONT_SIZE*-6);
                }
                break;
            case 1:
                if(jugadores[jugadorActual].puntuacion != 0 && jugadores[jugadorActual].puntuacion == rankingScores[i]){
                    DrawText("2 ND",4,MID,CENT,FONT_SIZE*(i+k),FONT_SIZE*-6,{220,70,220});
                }else{
                    DrawText("2 ND",4,MID,CENT,FONT_SIZE*(i+k),FONT_SIZE*-6);
                }
                break;
            case 2:
                if(jugadores[jugadorActual].puntuacion != 0 && jugadores[jugadorActual].puntuacion == rankingScores[i]){
                    DrawText("3 RD",4,MID,CENT,FONT_SIZE*(i+k),FONT_SIZE*-6,{220,70,220});
                }else{
                    DrawText("3 RD",4,MID,CENT,FONT_SIZE*(i+k),FONT_SIZE*-6);
                }
                break;
            case 3:
                if(jugadores[jugadorActual].puntuacion != 0 && jugadores[jugadorActual].puntuacion == rankingScores[i]){
                    DrawText("4 TH",4,MID,CENT,FONT_SIZE*(i+k),FONT_SIZE*-6,{220,70,220});
                }else{
                    DrawText("4 TH",4,MID,CENT,FONT_SIZE*(i+k),FONT_SIZE*-6);
                }
                break;
            case 4:
                if(jugadores[jugadorActual].puntuacion != 0 && jugadores[jugadorActual].puntuacion == rankingScores[i]){
                    DrawText("5 TH",4,MID,CENT,FONT_SIZE*(i+k),FONT_SIZE*-6,{220,70,220});
                }else{
                    DrawText("5 TH",4,MID,CENT,FONT_SIZE*(i+k),FONT_SIZE*-6);
                }
                break;
            }
            itoa(rankingScores[i]+100000,scoreDigits,10);
            if(jugadores[jugadorActual].puntuacion != 0 && jugadores[jugadorActual].puntuacion == rankingScores[i]){
                DrawText(scoreDigits+1,maxScoreDigits,MID,CENT,FONT_SIZE*(i+k),FONT_SIZE*2,{220,70,220});
                DrawText("PTS",3,MID,CENT,FONT_SIZE*(i+k),FONT_SIZE*7,{220,70,220});
            }else{
                DrawText(scoreDigits+1,maxScoreDigits,MID,CENT,FONT_SIZE*(i+k),FONT_SIZE*2);
                DrawText("PTS",3,MID,CENT,FONT_SIZE*(i+k),FONT_SIZE*7);
            }
            
        }

        DibujarCopyrightKonami();
    }else{
        ComprobarCreditos();
    }
}

//-- DIBUJADO PANTALLA DE INSERT COIN --//
void DibujarInsert(){
    if(HacerDuranteX(&animInsert.temporizador,animInsert.duracion)){
        DrawText("INSERT COIN",12,MID,CENT,FONT_SIZE*-2,0,{0,200,0});
        DrawText("3 FROGS  PER PLAYER",19,MID,CENT,FONT_SIZE*6,0,{200,200,0});
    }else{
        InicializarTitulo(true);
        animIntro.temporizador = last_time;
        pantallaActual = INTRO;
    }
}

//-- DIBUJADO PANTALLA DE START --//
void DibujarStart(){
    DrawText("PUSH",4,MID,CENT,FONT_SIZE*-7,0);
    DrawText("START BUTTON",12,MID,CENT,FONT_SIZE*-2,0,{220,70,220});
    switch (gamemode){
        case NORMAL_SINGLE:
            DrawText("ONE PLAYER ONLY",15,MID,CENT,FONT_SIZE*2,0);
        break;

        case NORMAL_MULTI:
            DrawText("TWO PLAYERS - ALTERNATED",24,MID,CENT,FONT_SIZE*2,0);
        break;
    }
    DrawText("ONE EXTRA FROG 20000 PTS",24,MID,CENT,FONT_SIZE*5,0,{200,0,0});
}

//-- DIBUJADO PANTALLA DE JUEGO --//
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

// Dibujar la fila del rio pasada por parametro
void DibujarFilaRio(Tortuga array[]){
    for(int i = 0; i < VENTANA_COLUMNAS; i++){
        DrawSprite(array[i].sprite);
    }
}

// Dibujar la fila del rio pasada por parametro
void DibujarFilaRio(Troncodrilo array[]){
    for(int i = 0; i < VENTANA_COLUMNAS; i++){
        DrawSprite(array[i].sprite);
    }
}

// Dibujar las zonas finales donde deben acabar las ranas del jugador para puntuar
void DibujarZonasFinales(){
    for(int i = 0; i < maxZonasFinales; i++){
        DrawSprite(zonasFinales[i]);
    }
}

void DibujarRanasFinales(){
    for(int i = 0; i < maxRanasFinales; i++){
        DrawSprite(ranasFinales[i]);
    }
}

void DibujarRio(){
    for(int i = 0 ; i < ((int) FILA_RIO_T); i++){
        switch ((FilaRio) i){
            case FILA_RIO_0:
                DibujarFilaRio(tortugas_1);
            break;
            case FILA_RIO_3:
                DibujarFilaRio(tortugas_2);
            break;
            case FILA_RIO_1:
                DibujarFilaRio(troncos_1);
            break;
            case FILA_RIO_2:
                DibujarFilaRio(troncos_2);
            break;
            case FILA_RIO_4:
                DibujarFilaRio(troncos_3);
            break;
            case FILA_RIO_5:
                DibujarZonasFinales();
                DibujarRanasFinales();
            break;
        }
    }
}

void DibujarVehiculos(){
    for(int i = 0; i < maxCamiones; i++){
        if(i < maxCochesAmarillos){
            DrawSprite(cochesAmarillos[i].sprite);
        }

        if(i < maxTractores){
            DrawSprite(tractores[i].sprite);
        }

        if(i < maxCochesRosas){
            DrawSprite(cochesRosas[i].sprite);
        }
        
        if(i < maxCochesBlancos){
            DrawSprite(cochesBlancos[i].sprite);
        }

        DrawSprite(camiones[i].sprite);
    }
}

void DibujarTiempoRestante(){
    char timeDigits[maxTimeDigits];

    if(cronometro.isTiempoVisible){
        //Dibuja el fondo que ocupa el espacio de los 2 textos. Se dibuja primero para no taparlo
        DrawText(" ",7,MID,CENT,FONT_SIZE*2,0,{0,0,0},{0,0,0});

        DrawText("TIME",4,MID,CENT,FONT_SIZE*2,FONT_SIZE*-2,{200,0,0});
        //Se suma 100 para asegurar que se muestran 2 digitos y +1 para evitar el 0 por el cronometro entero
        itoa(cronometro.tiempoRestante+101,timeDigits,10);
        DrawText(timeDigits+1,maxTimeDigits,MID,CENT,FONT_SIZE*2,FONT_SIZE*2,{200,0,0});

        if(cronometro.contador <= 50){
            cronometro.isTiempoVisible = false;
        }
    }
}

void DibujarMoscaCroc(){
    DrawSprite(moscaCroc.sprite);
}

void DibujarSerpientes(){
    esat::SpriteTransform st;
    for(int i = 0; i < maxSerpientes; i++){
        if(serpientes[i].direccion == DERECHA){
            esat::SpriteTransformInit(&st);
            // Invertir la escala hace que se dibuje a la inversa
            st.scale_x = -1;
            // Por lo tanto la x se debe de ajustar en el caso de que se invierta
            st.x = serpientes[i].sprite.collider.P1.x + serpienteSpriteSheet.spriteWidth;
            st.y = serpientes[i].sprite.collider.P1.y;
            DrawSprite(serpientes[i].sprite, st);
        }else{
            DrawSprite(serpientes[i].sprite);
        }
    }
}

void DibujarNutria(){
    esat::SpriteTransform st;
    if(nutria.direccion == IZQUIERDA){
        esat::SpriteTransformInit(&st);
        // Invertir la escala hace que se dibuje a la inversa
        st.scale_x = -1;
        // Por lo tanto la x se debe de ajustar en el caso de que se invierta
        st.x = nutria.sprite.collider.P1.x + nutriaSpriteSheet.spriteWidth;
        st.y = nutria.sprite.collider.P1.y;
        DrawSprite(nutria.sprite, st);
    }else{
        DrawSprite(nutria.sprite);
    }
    DrawSprite(nutria.colisionAtaque);
    // printf("ALTURA NUTRIA ATAQUE\n");
    // printf("P1 %.2f | P2 %.2f\n",nutria.colisionAtaque.collider.P1.y,nutria.colisionAtaque.collider.P2.y);
}

void DibujarJugador(){
    DrawSprite(jugadores[jugadorActual].ranaJugador.sprite, true);
    DrawCollider(jugadores[jugadorActual].ranaJugador.finSalto);
}

void DibujarRanaBonus(){
    DrawSprite(ranaBonus.rana.sprite);
    DrawCollider(ranaBonus.rana.finSalto);
}

void DibujarGameOver(){
    if(isAnimGameOver){
        //Dibuja el fondo que ocupa el espacio de los 2 textos. Se dibuja primero para no taparlo
        DrawText(" ",9,MID,CENT,FONT_SIZE,0,{0,0,0},{0,0,0});
        DrawText("GAME OVER",9,MID,CENT,FONT_SIZE,0,{200,0,0});

        DrawRect(barridoGameOver_Azul,{0,0,70,255});
        DrawRect(barridoGameOver_Negro,{0,0,0,255});
    }
}

void DibujarJuego(){
    //Entorno
    DibujarArbustos();
    DibujarRio();
    DibujarVehiculos();
    DibujarTiempoRestante();

    //Animales
    DibujarMoscaCroc();
    DibujarSerpientes();
    DibujarNutria();
    //Jugador
    DibujarJugador();
    DibujarRanaBonus();

    DibujarGameOver();
}

// Se encarga de dibujar la cabecera de la UI por completo
void DibujarCabecera(){
    char scoreDigits[maxScoreDigits];

    //Puntuacion del jugador
    if(jugadorActual == 0){
        DrawText("1-UP",4+16,TOP,CENT,0,0);
    }else{
        DrawText("2-UP",4+16,TOP,CENT,0,0);
    }
    itoa(jugadores[jugadorActual].puntuacion+100000,scoreDigits,10);
    DrawText(scoreDigits+1,maxScoreDigits+16,TOP,CENT,FONT_SIZE,0,{200,0,0});

    //Puntuacion mas alta durante esta ejecución
    DrawText("HI-SCORE",8,TOP,CENT,0,0);
    itoa(rankingScores[0]+100000,scoreDigits,10);
    DrawText(scoreDigits+1,maxScoreDigits,TOP,CENT,FONT_SIZE,0,{200,0,0});
}

//Dibuja la cantidad de vidas del jugador actual
void DibujarVidas(){
    //El limite del bucle será vidas o en su defecto 10 si vidas es mayor o igual a 10
    for(int i = 1; i < jugadores[jugadorActual].vidas; i++){
        esat::DrawSprite(vidaSprite,i*esat::SpriteWidth(vidaSprite),VENTANA_Y-SPRITE_SIZE);
    }
}

//Dibuja la cantidad de niveles superados + el actual
void DibujarNiveles(){
    //El limite del bucle será nivelActual o en su defecto 10 si nivelActual es mayor o igual a 10
    for(int i = 1; i <= (jugadores[jugadorActual].nivelActual >= 10 ? 10 : jugadores[jugadorActual].nivelActual); i++){
        esat::DrawSprite(indicadorNivelSprite,VENTANA_X-(i*esat::SpriteWidth(indicadorNivelSprite)),VENTANA_Y-SPRITE_SIZE);
    }
}

//Dibuja la barra de tiempo del cronometro en la parte inferior derecha de la pantalla
void DibujarBarraCronometro(){
    if(cronometro.isBarraParada){
        // A partir de los 5 segundos, se empiza a pintar la barra en rojo
        if(cronometro.tiempoRestante > 0){
            if(cronometro.tiempoRestante < 10){
                DrawRect({
                        {(float)((VENTANA_X-(FONT_SIZE*4))-((cronometro.tiempoRestante)*6)),VENTANA_Y-FONT_SIZE},
                        {VENTANA_X-(FONT_SIZE*4),VENTANA_Y-2}
                    },
                    {200,0,0}
                );
            }else{
                DrawRect({
                        {(float)((VENTANA_X-(FONT_SIZE*4))-((cronometro.tiempoRestante)*6)),VENTANA_Y-FONT_SIZE},
                        // El segundo punto es el tamaño de la ventana menos lo que ocupa el texto "TIME" en pantalla
                        {VENTANA_X-(FONT_SIZE*4),VENTANA_Y-2}
                    },
                    {0,200,0}
                );
            }
        }
        
    }else{
        if(HacerDuranteX(&cronometro.animCronometro.temporizador, cronometro.animCronometro.duracion)){
            // El tiempo limite del contador es de 30 segundos, pero para obtener la puntuación se 
            // almacena directamente en el doble (60 medios de segundo)
            cronometro.contador = ((int)(cronometro.animCronometro.duracion-GetContadorFromTemp(cronometro.animCronometro.temporizador)))/500;

            // printf("ContadorCrono medios segundo = %d \n",cronometro.contador);

            // A partir de los 5 segundos, se empiza a pintar la barra en rojo
            if(cronometro.contador < 10){
                DrawRect({
                        {(float)((VENTANA_X-(FONT_SIZE*4))-((cronometro.contador)*6)),VENTANA_Y-FONT_SIZE},
                        {VENTANA_X-(FONT_SIZE*4),VENTANA_Y-2}
                    },
                    {200,0,0}
                );
            }else{
                DrawRect({
                        // El primer punto de la barra se calcula restandole lo siguiente a VENTANA_X: 
                        //  Por un lado:
                        //  Lo que ocupa el texto "TIME"
                        //
                        //  Por otro lado:
                        //  El valor del contadorCronometro del cronometro para que solo se actualice cada medio segundo al ser un entero.
                        //  *6 para que cada avance de la barra sea de 6px. 61 avances (por el +1 anterior) multiplicado de 6 hacen una cantidad razonable máxima de 366px
                        {(float)((VENTANA_X-(FONT_SIZE*4))-((cronometro.contador)*6)),VENTANA_Y-FONT_SIZE},
                        // El segundo punto es el tamaño de la ventana menos lo que ocupa el texto "TIME" en pantalla
                        {VENTANA_X-(FONT_SIZE*4),VENTANA_Y-2}
                    },
                    {0,200,0}
                );
            }
        }else{
            MatarJugador();
        }
    }
}

//Dibuja el texto y la barra de tiempo en la parte inferior derecha de la pantalla
void DibujarTiempo(){
    DibujarBarraCronometro();
    if(cronometro.contador < 10){
        DrawText("TIME",4,BOT,RIGHT,-2,0);
    }else{
        DrawText("TIME",4,BOT,RIGHT,-2,0,{255,255,0});
    }
}

// Se encarga de dibujar el pie de la UI por completo
void DibujarPie(){
    char creditDigits[maxCreditDigits];
    
    switch (pantallaActual){
        case JUEGO:
            DibujarVidas();
            DibujarNiveles();
            DibujarTiempo();
        break;
        default:
            //Dibuja los creditos
            itoa(credits+100,creditDigits,10);
            DrawText(creditDigits+1,maxCreditDigits,BOT,RIGHT,-2,0,{0,200,255});

            DrawText("CREDIT",6,BOT,RIGHT,-2,FONT_SIZE*-3,{0,200,255});
        break;
    }
}

// Es la única función que se debe llamar en el main. 
// Se ocupa del dibujado de todos los elementos en pantalla mediante subrutinas
void DibujarEntorno(){
    // Se dibuja primero el fondo del rio para que siempre aparezca por debajo de todos los
    // demás dibujados
    DibujarFondoRio();

    DibujarCabecera();
    DibujarPie();
    // Dibuja todo lo que aparezca en la zona central de la ventana
    // En función de la pantalla seleccionada
    switch(pantallaActual){
        case INTRO:
            DibujarIntro();
            break;
        case PUNTUACIONES:
            DibujarPuntuaciones();
            break;
        case RANKING:
            DibujarRanking();
            break;
        case INSERT:
            DibujarInsert();
            break;
        case START:
            DibujarStart();
            break;
        case JUEGO:
            DibujarJuego();
            break;
    }

} 

//*** FUNCIONES DE LIBERADO DE MEMORIA AL TERMINAL EL PROCESO ***///

void LiberarSpriteSheets(){
    esat::SpriteRelease(animMuerteSpriteSheet.spriteSheet);
    esat::SpriteRelease(ranaBaseSpriteSheet.spriteSheet);
    esat::SpriteRelease(ranaRosaSpriteSheet.spriteSheet);
    esat::SpriteRelease(ranaRojaSpriteSheet.spriteSheet);
    esat::SpriteRelease(vehiculosSpriteSheet.spriteSheet);
    esat::SpriteRelease(tortugaSpriteSheet.spriteSheet);
    esat::SpriteRelease(troncoSpriteSheet.spriteSheet);
    esat::SpriteRelease(cocodriloSpriteSheet.spriteSheet);
    esat::SpriteRelease(zonaFinalSpriteSheet.spriteSheet);
    esat::SpriteRelease(ranaFinSpriteSheet.spriteSheet);
    esat::SpriteRelease(tituloSpriteSheet.spriteSheet);
    esat::SpriteRelease(ranaIntroSpriteSheet.spriteSheet);
    esat::SpriteRelease(puntosSpriteSheet.spriteSheet);
    esat::SpriteRelease(moscaCrocSpriteSheet.spriteSheet);
    esat::SpriteRelease(serpienteSpriteSheet.spriteSheet);
    esat::SpriteRelease(nutriaSpriteSheet.spriteSheet);
}

void LiberarSpritesBase(){
    esat::SpriteRelease(arbustoSprite);
    esat::SpriteRelease(indicadorNivelSprite);
    esat::SpriteRelease(vidaSprite);
    esat::SpriteRelease(copySprite);
}

void LiberarSpritesLetrasTitulo(){
    //Libera los sprites del muro final
    for(int i = 0; i < maxLetrasTitulo; i++){
        esat::SpriteRelease(letrasTitulo[i].sprite.imagen);
    }
}

void LiberarSpritesZonaFinal(){
    //Libera los sprites del muro final
    for(int i = 0; i < maxZonasFinales; i++){
        esat::SpriteRelease(zonasFinales[i].imagen);
    }

    //Libera los sprites de las ranas finales
    for(int i = 0; i < maxRanasFinales; i++){
        esat::SpriteRelease(ranasFinales[i].imagen);
    }

    //Libera los sprites de las moscas bonus / cocodrilo trampa
    esat::SpriteRelease(moscaCroc.sprite.imagen);

}

void LiberarSpritesRio(){
    for(int i = 0; i < VENTANA_COLUMNAS; i++){
        if(tortugas_1[i].sprite.imagen != NULL){
            esat::SpriteRelease(tortugas_1[i].sprite.imagen);
        }
        if(tortugas_2[i].sprite.imagen != NULL){
            esat::SpriteRelease(tortugas_2[i].sprite.imagen);
        }
        if(troncos_1[i].sprite.imagen != NULL){
            esat::SpriteRelease(troncos_1[i].sprite.imagen);
        }
        if(troncos_2[i].sprite.imagen != NULL){
            esat::SpriteRelease(troncos_2[i].sprite.imagen);
        }
        if(troncos_3[i].sprite.imagen != NULL){
            esat::SpriteRelease(troncos_3[i].sprite.imagen);
        }
    }
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

void LiberarSpritesMoscaCroc(){
    if(moscaCroc.sprite.imagen != NULL){
        esat::SpriteRelease(moscaCroc.sprite.imagen);
    }
}

void LiberarSpritesSerpientes(){
    for(int i = 0; i < maxSerpientes; i++){
        if(serpientes[i].sprite.imagen != NULL){
            esat::SpriteRelease(jugadores[i].ranaJugador.sprite.imagen);
        }
    }
}

void LiberarSpritesNutria(){
    if(nutria.sprite.imagen != NULL){
        esat::SpriteRelease(nutria.sprite.imagen);
    }
    if(nutria.colisionAtaque.imagen != NULL){
        esat::SpriteRelease(nutria.colisionAtaque.imagen);
    }
}

void LiberarSpritesRanaBonus(){
    if(moscaCroc.sprite.imagen != NULL){
        esat::SpriteRelease(moscaCroc.sprite.imagen);
    }
}

//Libera de memoria todos los spriteHandle inicializados
void LiberarSprites(){
    LiberarSpritesZonaFinal();
    LiberarSpritesRio();
    LiberarSpritesVehiculos();
    LiberarSpritesMoscaCroc();
    LiberarSpritesSerpientes();
    LiberarSpritesNutria();
    LiberarSpritesJugadores();
    LiberarSpritesRanaBonus();
    LiberarSpritesBase();
    LiberarSpriteSheets();
}
/* FIN FUNCIONALIDADES*/

/* INICIO MAIN */
int esat::main(int argc, char **argv) {
    GenerarSemillaAleatoria();

    esat::WindowInit(VENTANA_X,VENTANA_Y);
    WindowSetMouseVisibility(true);

    esat::DrawSetTextFont("./Recursos/Fuentes/arcade-legacy.ttf");
    esat::DrawSetTextSize(FONT_SIZE);

    // Inicialización de todos aquellos recursos que necesitan ser precargados para
    // que otros los puedan usar
    InicializarSpriteSheets();
    InicializarSprites();
    InicializarTitulo(true);

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