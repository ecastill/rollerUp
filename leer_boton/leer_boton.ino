#include <Wire.h>
#include <time.h>
#include <RTClib.h>
#include <ArduinoJson.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <SPI.h>
#include <Bounce2.h> //para evitar el rebote del boton
#include "SPIFFS.h" //para leer y escribir en la memoria flash

#include "Configuraciones.h"
#include "CMenuItem.h"
#include "CParametros.h"
#include "HelperFuncionesComunes.h"

#define BOTON_MENU 13  // Pin donde está conectado el botón
#define BOTON_UP 12
#define BOTON_DOWN 14
#define BOTON_ENTER 27
#define BOTON_TOPE_UP 26
#define BOTON_TOPE_DOWN 25

#define DHT_PIN_TEMPERATURA_DIGITAL 15
#define DHTTYPE DHT11  // Cambia a DHT22 si usas ese modelo


//IBT_2
#define RPWM 33
#define LPWM 32
#define R_EN 18
#define L_EN 19


//creando los botones bounce
Bounce boton_b_menu = Bounce();
Bounce boton_b_up = Bounce();
Bounce boton_b_down = Bounce();
Bounce boton_b_enter = Bounce();






#define LCD_LINEAS 2

#define TIPO_ACCION_NUMERO "NUMERO"
#define TIPO_ACCION_FECHA "FECHA"
#define TIPO_ACCION_MANUAL "MANUAL"
#define TIPO_ACCION_SINO "SINO"


#define TIPO_NODO_MENU "MENU"
#define TIPO_NODO_ACCION "ACCION"
#define TIPO_NODO_ATRAS "ATRAS"





//#define TIPO_EVENTO_ATRAS "ATRAS"
//#define TIPO_EVENTO_SALIR "SALIR"
//#define TIPO_EVENTO_ACCION "ACCION"
//#define TIPO_EVENTO_MENU "MENU"







#define ACCION_TEMPERATURASUBIDA "TEMPERATURASUBIDA"
#define ACCION_TEMPERATURABAJADA "TEMPERATURABAJADA"
#define ACCION_FECHAACTUAL "FECHAACTUAL"
#define ACCION_HORAACTUAL "HORAACTUAL"
#define ACCION_MANUAL "MANUAL"
#define ACCION_ATRAS "ATRAS"
#define ACCION_SALIR "SALIR"
#define ACCION_ACTIVADO "ACTIVADO"






//direccion de memoria del LCD
#define LCD_ADDRESS 0x27


//los pines comunes
#define LQ_SDA 21 //
#define LQ_SCL 22 //


LiquidCrystal_PCF8574  lcd(LCD_ADDRESS );
RTC_DS3231 rtc;


DHT dht(DHT_PIN_TEMPERATURA_DIGITAL , DHTTYPE);

byte letra_N[] = {
  0b01110,
  0b00000,
  0b11001,
  0b10101,
  0b10011,
  0b10001,
  0b10001,
  0b00000
};

byte letra_grado[] = {
  0b01110,
  0b01010,
  0b01110,
  0b00000,
  0b00000,
  0b00000,
  0b00000,
  0b00000
};


CMenuItem* raiz;
CMenuItem* actual;

String pantalla_inicio[] = {"RUP", "V1.0"};

int g_en_accion;
String g_tipo_accion = "";
String g_glosa_accion = "";

int g_accion_numero;
String g_accion_fecha;
DateTime g_fecha_actual;
DateTime g_fecha_reloj;

DateTime g_fecha_actual_ant = DateTime(2000, 1, 1, 0, 0, 0);
String g_modifica_actual; //se usa para cuando queremos saber que parte de la fecha estamos modificando
DateTime g_fecha_ingreso_a_menu;  //se va a usar para saber cuando ingresamos al menu, despues de un par de minutos de inactividad se sale de el


int g_en_movimiento; //indica si esta en movimiento el motor
int g_en_funcion;  //indica si esta activo el funcionamiento
float g_temperatura; //guarda la temperatura
float g_humedad; //guarda la humedad
int g_manual_up;  //indca si esta en manual y se apreto el up
int g_manual_down; //indca si esta en manual y se apreto el down
int g_subiendo = 0;
int g_bajando = 0;

//falta el que indica si esta en el tope arriba y abajo




CParametros g_parametros;





void setup() {



  Serial.begin(115200);           // Inicia comunicación serie

  //inicializar variables

  //varriables motor
  g_manual_up = 0;
  g_manual_down = 0;
  g_en_movimiento = 0;
  g_tipo_accion = "";
  g_glosa_accion = "";
  g_en_accion = 0;
  g_en_funcion = 0;




  delay(2000);  //demorar en 2 segundos el inicio


  //inicio configuracion IBT_2
  pinMode(RPWM, OUTPUT);
  pinMode(LPWM, OUTPUT);
  pinMode(R_EN, OUTPUT);
  pinMode(L_EN, OUTPUT);

  // Habilitar el driver IBT-2
  digitalWrite(R_EN, HIGH);
  digitalWrite(L_EN, HIGH);

  //fin configuracion IBT_2


  bool x;
  x = g_parametros.leerParametros();

  //inicializar sensor temperatura
  dht.begin();


  //leer el menu
  StaticJsonDocument<4096> doc;
  DeserializationError error = deserializeJson(doc, getJsonMenu());

  if (error) {
    Serial.println(F("Fallo al parsear JSON"));
    return;
  }


  //cargar menu
  JsonObject root = doc.as<JsonObject>();
  raiz = CMenuItem::parseMenu(root);







  pinMode(BOTON_MENU, INPUT_PULLUP); // Configura el pin 13 como entrada con pull-up interno
  pinMode(BOTON_UP, INPUT_PULLUP); // Configura el pin 12 como entrada con pull-up interno
  pinMode(BOTON_DOWN, INPUT_PULLUP); // Configura el pin 14 como entrada con pull-up interno
  pinMode(BOTON_ENTER, INPUT_PULLUP); // Configura el pin 27 como entrada con pull-up interno
  pinMode(BOTON_TOPE_UP, INPUT_PULLUP); // Configura el pin 26 como entrada con pull-up interno
  pinMode(BOTON_TOPE_DOWN, INPUT_PULLUP); // Configura el pin 25 como entrada con pull-up interno


  boton_b_menu.attach(BOTON_MENU);
  boton_b_menu.interval(25);

  boton_b_up.attach(BOTON_UP);
  boton_b_up.interval(25);

  boton_b_down.attach(BOTON_DOWN);
  boton_b_down.interval(25);

  boton_b_enter.attach(BOTON_ENTER);
  boton_b_enter.interval(25);

  int estadoBoton_menu  = HIGH;
  int estadoBoton_up    = HIGH;
  int estadoBoton_down  = HIGH;
  int estadoBoton_enter = HIGH;

  if (boton_b_menu.fell()) {
    estadoBoton_menu = LOW;
  }

  if (boton_b_up.fell()) {
    estadoBoton_up = LOW;
  }

  if (boton_b_down.fell()) {
    estadoBoton_down = LOW;
  }

  if (boton_b_enter.fell()) {
    estadoBoton_enter = LOW;
  }



  //
  Wire.begin(LQ_SDA, LQ_SCL); // Configura I2C en ESP32 (SDA = GPIO 21, SCL = GPIO 22)

  if (!rtc.begin()) {
    Serial.println("No se encontró el RTC");
  }
  else {
    Serial.println("encontre el RTC");
  }

  if (rtc.lostPower()) {
    Serial.println("El RTC perdió la hora o es la primera vez");
    // Setea la fecha y hora actual (aquí puedes ajustar tu fecha/hora)
    rtc.adjust(DateTime(2025, 3, 25, 15, 30, 0));  // AAAA, MM, DD, HH, MM, SS
  }
  else {
    Serial.println("El RTC no es la primera vez");
  }

  g_fecha_actual = obtener_fecha(rtc);
  g_fecha_reloj = obtener_fecha(rtc);





  //setea el LCD
  lcd.begin(16, 2);   // Inicia LCD de 16x2
  lcd.setBacklight(255); // Activa la luz de fondo
  lcd.createChar(0, letra_N);
  lcd.createChar(1, letra_grado);
  //fin setea el LCD


  limpiar_pantalla(lcd);
  mostrar_pantalla_inicio(lcd, g_parametros.enFuncion);


  g_subiendo = 0;
  g_bajando = 0;


  delay(200);

}

void loop() {
  Serial.println("entre al loop");

  g_fecha_reloj = obtener_fecha(rtc);


  int subir = 0;
  int bajar = 0;

  boton_b_menu.update();
  boton_b_up.update();
  boton_b_down.update();
  boton_b_enter.update();

  int estadoBoton_menu  = HIGH;
  int estadoBoton_up    = HIGH;
  int estadoBoton_down  = HIGH;
  int estadoBoton_enter = HIGH;



  if (boton_b_menu.fell()) {
    estadoBoton_menu = LOW;
  }

  if (boton_b_up.fell()) {
    estadoBoton_up = LOW;
  }

  if (boton_b_down.fell()) {
    estadoBoton_down = LOW;
  }

  if (boton_b_enter.fell()) {
    estadoBoton_enter = LOW;
  }

  int estadoBoton_tope_up = digitalRead(BOTON_TOPE_UP); // Lee el estado del botón
  int estadoBoton_tope_down = digitalRead(BOTON_TOPE_DOWN); // Lee el estado del botón


  //g_temperatura = dht.readTemperature(); // en °C
  if (g_fecha_reloj.second() < 30 )
  {
    g_temperatura = g_fecha_reloj.second();
  }
  else
  {
    g_temperatura = 60 - g_fecha_reloj.second();
  }

  g_humedad = dht.readHumidity();







  if (actual == nullptr)
  {
    if (diferencia_fecha(g_fecha_reloj, g_fecha_actual_ant) > 10)
    {
      escribir_fecha_pantalla_inicio(lcd, g_fecha_reloj);
      escribir_temperatura_pantalla_inicio(lcd, g_temperatura);
      g_fecha_actual_ant = g_fecha_reloj;
    }


    if (g_parametros.enFuncion == 1)
    {
      //si la temepratura es mayor que la de subida entonces subir siempre y cuando el tope no este activo
      if ((estadoBoton_tope_up == HIGH)  && (g_temperatura >= g_parametros.tempSubida))
      {
        //subir cortina
        if (g_subiendo == 0)
        {
          g_bajando = 0;
          g_subiendo = 1;
        }
      }
      else if ((estadoBoton_tope_down == HIGH)  && (g_temperatura <= g_parametros.tempBajada))
      {
        //subir cortina
        if (g_bajando == 0)
        {
          g_subiendo = 0;
          g_bajando = 1;
        }
      }
      else {
        g_bajando = 0;
        g_subiendo = 0;
      }

    }
    else {
      g_bajando = 0;
      g_subiendo = 0;
    }


   



    if (estadoBoton_menu == LOW) { // Activo en bajo por el pull-up
      Serial.println("Botón menu en PRESIONADO");

      actual = raiz;
      actual->posicion = 0;
      g_en_accion = 0;
      mostrar_menu(lcd, actual, LCD_LINEAS);
    }

    //ver si tiene que mover el motor, el motor se va a move siempre y cuando no este en el menu


  }
  else
  {


    if (estadoBoton_menu == LOW) { // Activo en bajo por el pull-up
      Serial.println("Botón menu en PRESIONADO");


      actual->posicion = 0;
      g_en_accion = 0;
      actual = actual->parent;

      if (actual == nullptr)
      {
        //mostrar_pantalla_inicio(lcd);
        limpiar_pantalla(lcd);
        mostrar_pantalla_inicio(lcd, g_parametros.enFuncion);
        escribir_fecha_pantalla_inicio(lcd, g_fecha_actual);
        escribir_temperatura_pantalla_inicio(lcd, g_temperatura);
      }
      else {
        limpiar_pantalla(lcd);
        mostrar_menu(lcd, actual, LCD_LINEAS);
      }






    }


    //=====================================================================

    if (estadoBoton_up == LOW) { // Activo en bajo por el pull-up
      Serial.println("Botón up en PRESIONADO");

      if (actual->tipo == TIPO_NODO_MENU)
      {
        Serial.println("Botón up en PRESIONADO----1");
        actual->posicion--;
        Serial.println("Botón up en PRESIONADO----2");
        if (actual->posicion < 0 )
        {

          actual->posicion = actual->cantidad - 1;
        }
        mostrar_menu(lcd, actual, LCD_LINEAS);
      }
      else if (actual->tipo == TIPO_NODO_ACCION) {
        if (actual->accion == ACCION_TEMPERATURASUBIDA)
        {
          g_accion_numero++;
          if (g_accion_numero > 40)
          {
            g_accion_numero = 40;
          }
          limpiar_pantalla(lcd);
          lcd.setCursor(0, 0);
          lcd.print(g_glosa_accion);
          lcd.setCursor(0, 1);
          lcd.print(g_accion_numero);
        }
        else if (actual->accion == ACCION_TEMPERATURABAJADA)
        {
          g_accion_numero++;
          if (g_accion_numero > 40)
          {
            g_accion_numero = 40;
          }
          limpiar_pantalla(lcd);
          lcd.setCursor(0, 0);
          lcd.print(g_glosa_accion);
          lcd.setCursor(0, 1);
          lcd.print(g_accion_numero);
        }
        else if (actual->accion == ACCION_ACTIVADO)
        {
          g_accion_numero++;
          if (g_accion_numero > 1)
          {
            g_accion_numero = 0;
          }
          limpiar_pantalla(lcd);
          lcd.setCursor(0, 0);
          lcd.print(g_glosa_accion);
          lcd.setCursor(0, 1);
          lcd.print(texto_onoff(g_accion_numero));
        }
        else if (actual->accion == ACCION_FECHAACTUAL )
        {
          if (g_modifica_actual == "YEAR")
          {
            g_accion_numero++;
            if (g_accion_numero > 2040) {
              g_accion_numero = 2040;
            }

            lcd.setCursor(0, 1);
            lcd.print(g_accion_numero);
          }
          else if (g_modifica_actual == "MONTH")
          {
            String texto = "";
            g_accion_numero++;
            if (g_accion_numero > 12) {
              g_accion_numero = 1;
            }
            lcd.setCursor(5, 1);
            texto = g_accion_numero;
            if (g_accion_numero < 10)
            {
              texto = "0" + texto;
            }


            lcd.print(texto);
          }
          else if (g_modifica_actual == "DAY")
          {

            Serial.println("el mes- es:");
            Serial.println(g_fecha_actual.month());


            String texto = "";
            g_accion_numero++;

            if (mes_con_30(g_fecha_actual.month()))
            {
              if (g_accion_numero > 30)
              {
                g_accion_numero = 1;
              }
            }
            else if (g_fecha_actual.month() == 2) {
              if (es_bisiesto(g_fecha_actual.year()) )
              {
                if (g_accion_numero > 29)
                {
                  g_accion_numero = 1;
                }
              }
              else {
                if (g_accion_numero > 28)
                {
                  g_accion_numero = 1;
                }
              }

            }
            else {
              if (g_accion_numero > 31)
              {
                g_accion_numero = 1;
              }
            }



            lcd.setCursor(8, 1);
            texto = g_accion_numero;
            if (g_accion_numero < 10)
            {
              texto = "0" + texto;
            }


            lcd.print(texto);
          }
          else if (g_modifica_actual == "HOUR")
          {
            String texto = "";
            g_accion_numero++;
            if (g_accion_numero > 23) {
              g_accion_numero = 0 ;
            }
            texto = g_accion_numero;
            if (g_accion_numero < 10)
            {
              texto = "0" + texto;
            }
            lcd.setCursor(11, 1);
            lcd.print(texto);
          }
          else if (g_modifica_actual == "MINUTE")
          {
            String texto = "";
            g_accion_numero++;
            if (g_accion_numero > 59) {
              g_accion_numero = 0 ;
            }
            texto = g_accion_numero;
            if (g_accion_numero < 10)
            {
              texto = "0" + texto;
            }
            lcd.setCursor(14, 1);
            lcd.print(texto);


          }
        }

      }



    }

    //=====================================================================

    if (estadoBoton_down == LOW) { // Activo en bajo por el pull-up
      Serial.println("Botón down en PRESIONADO");
      Serial.println("jjjjjjjj");
      Serial.println(g_modifica_actual);
      Serial.println("jjjjjjjjj");
      if (actual->tipo == TIPO_NODO_MENU)
      {
        actual->posicion++;
        if (actual->posicion >= actual->cantidad )
        {
          actual->posicion = 0;
        }
        mostrar_menu(lcd, actual, LCD_LINEAS);
      }
      else if (actual->tipo == TIPO_NODO_ACCION) {
        if (actual->accion == ACCION_TEMPERATURASUBIDA)
        {
          g_accion_numero--;
          if (g_accion_numero < -10)
          {
            g_accion_numero = -10;
          }
          limpiar_pantalla(lcd);
          lcd.setCursor(0, 0);
          lcd.print(g_glosa_accion);
          lcd.setCursor(0, 1);
          lcd.print(g_accion_numero);
        }
        else if (actual->accion == ACCION_TEMPERATURABAJADA)
        {
          g_accion_numero--;
          if (g_accion_numero < -10)
          {
            g_accion_numero = -10;
          }
          limpiar_pantalla(lcd);
          lcd.setCursor(0, 0);
          lcd.print(g_glosa_accion);
          lcd.setCursor(0, 1);
          lcd.print(g_accion_numero);
        }
        else if (actual->accion == ACCION_ACTIVADO)
        {
          g_accion_numero--;
          if (g_accion_numero < 0)
          {
            g_accion_numero = 1;
          }
          limpiar_pantalla(lcd);
          lcd.setCursor(0, 0);
          lcd.print(g_glosa_accion);
          lcd.setCursor(0, 1);
          lcd.print(texto_onoff(g_accion_numero));
        }
        else if (actual->accion == ACCION_FECHAACTUAL )
        {

          if (g_modifica_actual == "YEAR")
          {
            g_accion_numero--;
            if (g_accion_numero < 2000) {
              g_accion_numero = 2000;
            }
            lcd.setCursor(0, 1);
            lcd.print(g_accion_numero);
          }
          else if (g_modifica_actual == "MONTH")
          {
            String texto = "";
            g_accion_numero--;
            if (g_accion_numero < 1) {
              g_accion_numero = 12;
            }
            lcd.setCursor(5, 1);
            texto = g_accion_numero;
            if (g_accion_numero < 10)
            {
              texto = "0" + texto;
            }


            lcd.print(texto);
          }
          else if (g_modifica_actual == "DAY")
          {

            String texto = "";
            g_accion_numero--;

            Serial.println("el mes- es:");
            Serial.println(escribir_fecha(g_fecha_actual));
            Serial.println(g_fecha_actual.month());


            if (mes_con_30(g_fecha_actual.month()))
            {
              if (g_accion_numero < 1)
              {
                g_accion_numero = 30;
              }
            }
            else if (g_fecha_actual.month() == 2) {
              if (es_bisiesto(g_fecha_actual.year()) )
              {
                if (g_accion_numero < 1)
                {
                  g_accion_numero = 29;
                }
              }
              else {
                if (g_accion_numero < 1)
                {
                  g_accion_numero = 28;
                }
              }

            }
            else {
              if (g_accion_numero < 1)
              {
                g_accion_numero = 31;
              }
            }



            lcd.setCursor(8, 1);
            texto = g_accion_numero;
            if (g_accion_numero < 10)
            {
              texto = "0" + texto;
            }


            lcd.print(texto);
          }
          else if (g_modifica_actual == "HOUR")
          {
            String texto = "";
            g_accion_numero--;
            if (g_accion_numero < 0) {
              g_accion_numero = 23 ;
            }
            texto = g_accion_numero;
            if (g_accion_numero < 10)
            {
              texto = "0" + texto;
            }
            lcd.setCursor(11, 1);
            lcd.print(texto);
          }
          else if (g_modifica_actual == "MINUTE")
          {
            Serial.println("xxxxxxxxxxxxxxx" );
            Serial.println(g_modifica_actual );
            Serial.println("xxxxxxxxxxxxxxx" );

            String texto = "";
            g_accion_numero--;
            if (g_accion_numero < 0) {
              g_accion_numero = 59 ;
            }
            texto = g_accion_numero;
            if (g_accion_numero < 10)
            {
              texto = "0" + texto;
            }
            lcd.setCursor(14, 1);
            lcd.print(texto);

          }
        }




      }
    }

    //=========================================================

    if (estadoBoton_enter == LOW) { // Activo en bajo por el pull-up
      Serial.println("Botón enter en PRESIONADO");
      if (actual->tipo == TIPO_NODO_MENU)
      {
        if (actual->hijos[actual->posicion]->tipo == TIPO_NODO_ATRAS)
        {
          actual = actual->parent;
          actual->posicion = 0;
          limpiar_pantalla(lcd);
          mostrar_menu(lcd, actual, LCD_LINEAS);
        }
        else {
          Serial.println("Botón enter en PRESIONADO--1");

          actual = actual->hijos[actual->posicion];
          actual->posicion = 0;
          Serial.println("Botón enter en PRESIONADO--2");
          if (actual->tipo == TIPO_NODO_MENU) {
            Serial.println("Botón enter en PRESIONADO--3");
            Serial.println(actual->descripcion);
            Serial.println("Botón enter en PRESIONADO--3.1");
            mostrar_menu(lcd, actual, LCD_LINEAS);
            Serial.println("Botón enter en PRESIONADO--4");
          }
          else {
            Serial.println("Botón enter en PRESIONADO--5");


            ejecutar_accion(actual->accion);
            Serial.println("Botón enter en PRESIONADO--6");
          }
        }

      }
      else if (actual->tipo == TIPO_NODO_ACCION) {
        {
          if (actual->accion == ACCION_TEMPERATURABAJADA) {
            g_parametros.tempBajada = g_accion_numero;
            g_parametros.grabar_en_archivo();

            g_glosa_accion = "";
            g_en_accion = 0;
            limpiar_pantalla(lcd);
            actual = actual->parent;
            mostrar_menu(lcd, actual, LCD_LINEAS);
          }
          else if (actual->accion == ACCION_TEMPERATURASUBIDA) {
            g_parametros.tempSubida = g_accion_numero;
            g_parametros.grabar_en_archivo();

            g_glosa_accion = "";
            g_en_accion = 0;
            limpiar_pantalla(lcd);
            actual = actual->parent;
            mostrar_menu(lcd, actual, LCD_LINEAS);
          }
          else if (actual->accion == ACCION_ACTIVADO) {

            g_parametros.enFuncion = g_accion_numero;
            g_parametros.grabar_en_archivo();

            g_glosa_accion = "";
            g_en_accion = 0;
            limpiar_pantalla(lcd);
            actual = actual->parent;
            mostrar_menu(lcd, actual, LCD_LINEAS);
          }
          else if (actual->accion == ACCION_FECHAACTUAL) {
            if (g_modifica_actual == "YEAR")
            {
              g_fecha_actual = DateTime(g_accion_numero, g_fecha_actual.month(), g_fecha_actual.day(), g_fecha_actual.hour(), g_fecha_actual.minute(), 0);
              g_en_accion = 1;
              g_accion_numero = g_fecha_actual.month();
              g_modifica_actual = "MONTH";
              String texto = escribir_fecha(g_fecha_actual);

              lcd.setCursor(0, 1);
              lcd.print(texto);

            }
            else if (g_modifica_actual == "MONTH")
            {
              int x_dia;
              x_dia = g_fecha_actual.day();
              Serial.println("en MONTH");
              if (mes_con_30(g_accion_numero) && (g_fecha_actual.day() > 30) )
              {
                x_dia = 30;
                Serial.println("1-en MONTH");
                Serial.println(x_dia);

              }
              else if ((g_accion_numero == 2)  )
              {
                Serial.println("2-en MONTH");
                Serial.println(x_dia);
                if (es_bisiesto(g_fecha_actual.year())) {
                  Serial.println("3-en MONTH");
                  Serial.println(x_dia);
                  if (g_fecha_actual.day() > 29) {
                    Serial.println("4-en MONTH");

                    x_dia = 29;
                    Serial.println(x_dia);
                  }
                }
                else {
                  Serial.println("5-en MONTH");
                  Serial.println(x_dia);
                  if (g_fecha_actual.day() > 28) {
                    Serial.println("6-en MONTH");

                    x_dia = 28;
                    Serial.println(x_dia);
                  }
                }
              }
              else {
                Serial.println("7-en MONTH");
                x_dia = g_fecha_actual.day();
                Serial.println(x_dia);
              }


              Serial.println("8-en MONTH");
              Serial.println(escribir_fecha(g_fecha_actual));

              g_fecha_actual = DateTime(g_fecha_actual.year(), g_accion_numero,  x_dia, g_fecha_actual.hour(), g_fecha_actual.minute(), 0);
              Serial.println("9-en MONTH");
              Serial.println(escribir_fecha(g_fecha_actual));

              Serial.println("en enter month------");
              Serial.println(g_accion_numero);
              Serial.println(escribir_fecha(g_fecha_actual));
              Serial.println("out enter month------");



              g_en_accion = 1;



              g_accion_numero = g_fecha_actual.day();
              g_modifica_actual = "DAY";
              String texto = escribir_fecha(g_fecha_actual);

              lcd.setCursor(0, 1);
              lcd.print(texto);


            }
            else if (g_modifica_actual == "DAY")
            {
              Serial.println("apretando enter en DAY");

              g_fecha_actual = DateTime(g_fecha_actual.year(), g_fecha_actual.month(), g_accion_numero,  g_fecha_actual.hour(), g_fecha_actual.minute(), 0);
              g_en_accion = 1;

              g_accion_numero = g_fecha_actual.hour();
              g_modifica_actual = "HOUR";

              String texto = escribir_fecha(g_fecha_actual);
              lcd.setCursor(0, 1);
              lcd.print(texto);

              Serial.println("kkkkkkkkkkkkkkk");
              Serial.println(g_modifica_actual);
              Serial.println("kkkkkkkkkkkkkkk");



            }
            else if (g_modifica_actual == "HOUR")
            {
              Serial.println("apretando enter en HOUR");
              g_fecha_actual = DateTime(g_fecha_actual.year(), g_fecha_actual.month(), g_fecha_actual.day(), g_accion_numero,  g_fecha_actual.minute(), 0);
              g_en_accion = 1;

              g_accion_numero = g_fecha_actual.minute();
              g_modifica_actual = "MINUTE";

              String texto = escribir_fecha(g_fecha_actual);
              lcd.setCursor(0, 1);
              lcd.print(texto);



            }
            else if (g_modifica_actual == "MINUTE")
            {

              g_fecha_actual = DateTime(g_fecha_actual.year(), g_fecha_actual.month(), g_fecha_actual.day(), g_fecha_actual.hour(), g_accion_numero, 0);
              rtc.adjust(g_fecha_actual);
              actual = actual->parent;
              g_glosa_accion = "";
              g_en_accion = 0;
              limpiar_pantalla(lcd);
              actual = actual->parent;
              mostrar_menu(lcd, actual, LCD_LINEAS);
            }
          }
        }


      }


    }
  }



   Serial.println("antes de");
    if ((g_parametros.enFuncion == 1) && (actual == nullptr))
    {
      Serial.println("en funcion");

      if (g_subiendo == 1)
      {
        analogWrite(RPWM, 0); // Velocidad media (0-255)
        analogWrite(LPWM, 0);

        analogWrite(LPWM, 0);
        analogWrite(RPWM, 255); // Velocidad media (0-255)
      }
      else if (g_bajando == 1)
      {
        analogWrite(RPWM, 0); // Velocidad media (0-255)
        analogWrite(LPWM, 0);
        analogWrite(LPWM, 255);
        analogWrite(RPWM, 0); // Velocidad media (0-255)
      }
      else
      {
        analogWrite(RPWM, 0); // Velocidad media (0-255)
        analogWrite(LPWM, 0);
      }

    }
    else
    {
      Serial.println("no en funcion");
      analogWrite(RPWM, 0); // Velocidad media (0-255)
      analogWrite(LPWM, 0);
    }
    Serial.println("despues de");









  delay(100); // Pequeña pausa para evitar rebotes

}

void ejecutar_accion(String accion)
{
  if (accion == ACCION_SALIR) {
    g_en_accion = 0;
    g_tipo_accion = "";
    g_glosa_accion = "";
    g_accion_numero = 0;
    actual->posicion = 0;
    actual = nullptr;
    limpiar_pantalla(lcd);
    mostrar_pantalla_inicio(lcd,  g_parametros.enFuncion);
  }
  else if (accion == ACCION_ATRAS) {
    Serial.println("en accion atras");
    Serial.println(actual->descripcion);

    g_en_accion = 0;
    g_tipo_accion = "";
    g_glosa_accion = "";
    g_accion_numero = 0;
    actual = actual->parent;
    actual->posicion = 0;
    Serial.println(actual->descripcion);
    limpiar_pantalla(lcd);
    mostrar_menu(lcd, actual, LCD_LINEAS);
  }
  else if (accion == ACCION_TEMPERATURASUBIDA)
  {
    g_en_accion = 1;
    g_tipo_accion = "NUMERO";
    g_glosa_accion = "TEMP SUBIDA";
    g_accion_numero = g_parametros.tempSubida;
    limpiar_pantalla(lcd);
    lcd.setCursor(0, 0);
    lcd.print(g_glosa_accion);
    lcd.setCursor(0, 1);
    lcd.print(g_accion_numero);
  }
  else if (accion == ACCION_TEMPERATURABAJADA)
  {
    g_en_accion = 1;
    g_tipo_accion = "NUMERO";
    g_glosa_accion = "TEMP BAJADA";
    g_accion_numero = g_parametros.tempBajada;
    limpiar_pantalla(lcd);
    lcd.setCursor(0, 0);
    lcd.print(g_glosa_accion);
    lcd.setCursor(0, 1);
    lcd.print(g_accion_numero);

  }
  else if (accion == ACCION_FECHAACTUAL)
  {
    Serial.println("ejecutar_accion->ACCION_FECHAACTUAL");
    g_fecha_actual = obtener_fecha(rtc);


    g_en_accion = 1;
    g_modifica_actual = "YEAR";
    g_tipo_accion = TIPO_ACCION_FECHA;
    g_glosa_accion = "Fecha";
    g_accion_numero = g_fecha_actual.year();
    g_accion_fecha = escribir_fecha(g_fecha_actual);
    limpiar_pantalla(lcd);
    lcd.setCursor(0, 0);
    lcd.print(g_glosa_accion);
    lcd.setCursor(0, 1);
    lcd.print(g_accion_fecha);

  }
  else if (accion == ACCION_MANUAL)
  {
    g_en_accion = 1;
    g_tipo_accion = TIPO_ACCION_MANUAL;
    g_glosa_accion = "Manual";

    limpiar_pantalla(lcd);
    lcd.setCursor(0, 0);
    lcd.print(g_glosa_accion);
    //lcd.setCursor(0, 1);
    //lcd.print(accion_numero);

  }
  else if (accion == ACCION_ACTIVADO)
  {
    g_en_accion = 1;
    g_tipo_accion = TIPO_ACCION_SINO;
    g_glosa_accion = actual->descripcion;
    g_accion_numero = g_parametros.enFuncion;

    String texto = "";
    if (g_accion_numero == 0)
    {
      texto = "Off";
    }
    else
    {
      texto = "On";

    }
    limpiar_pantalla(lcd);
    lcd.setCursor(0, 0);
    lcd.print(g_glosa_accion);
    lcd.setCursor(0, 1);
    lcd.print(texto);


  }


}
