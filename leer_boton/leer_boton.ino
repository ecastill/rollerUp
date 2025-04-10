#include <Wire.h>
#include <LiquidCrystal_PCF8574.h>
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

#define TIPO_EVENTO_ATRAS "ATRAS"
#define TIPO_EVENTO_SALIR "SALIR"
#define TIPO_EVENTO_ACCION "ACCION"
#define TIPO_EVENTO_MENU "MENU"




#define ACCION_TEMPERATURASUBIDA "TEMPERATURASUBIDA"
#define ACCION_TEMPERATURABAJADA "TEMPERATURABAJADA"
#define ACCION_FECHAACTUAL "FECHAACTUAL"
#define ACCION_HORAACTUAL "HORAACTUAL"






#define LCD_ADDRESS 0x27


//los pines comunes
#define LQ_SDA 21 //
#define LQ_SCL 22 //


LiquidCrystal_PCF8574  lcd(0x27);
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
int en_accion;


int largo_menu = 3;
String pantalla_inicio[] = {"RUP", "V1.0"};

int posicion_menu = -1;
String tipo_accion = "";
String glosa_accion = "";

int accion_numero;
String accion_fecha;
DateTime g_fecha_actual;
String g_modifica_actual;

int g_year;
int g_month;
int g_day;

int en_movimiento;

float g_temperatura;
float g_humedad;


DateTime obtener_fecha()
{
  DateTime now_rtc = rtc.now();
  return now_rtc ;
}



CParametros g_parametros;



void setup() {
  en_movimiento=0;
  Serial.begin(115200);           // Inicia comunicación serie
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



  g_parametros = leerParametrosDesdeArchivo("/parametros.json");

  //inicializar sensor
  dht.begin();


  //leer el menu
  StaticJsonDocument<4096> doc;
  DeserializationError error = deserializeJson(doc, getJsonMenu());

  if (error) {
    Serial.println(F("Fallo al parsear JSON"));
    return;
  }

  JsonObject root = doc.as<JsonObject>();
  raiz = CMenuItem::parseMenu(root);







  tipo_accion = "";
  glosa_accion = "";

  en_accion = 0;

  //cargar_menu();


  String menu[] = {"Configuracion", "Manual", "Salir"};







  //setear fechay hora
  struct tm timeinfo;
  time_t now;

  // Configurar fecha y hora manualmente (Año, Mes, Día, Hora, Minuto, Segundo)
  struct tm t = {0};
  t.tm_year = 2025 - 1900; // Año - 1900
  t.tm_mon = 2;  // Mes (0 = Enero)
  t.tm_mday = 12; // Día del mes
  t.tm_hour = 12;
  t.tm_min = 30;
  t.tm_sec = 0;

  time_t t_of_day = mktime(&t);
  struct timeval tv = { t_of_day, 0 };
  settimeofday(&tv, NULL);


  //





  pinMode(BOTON_MENU, INPUT_PULLUP); // Configura el pin 13 como entrada con pull-up interno
  pinMode(BOTON_UP, INPUT_PULLUP); // Configura el pin 12 como entrada con pull-up interno
  pinMode(BOTON_DOWN, INPUT_PULLUP); // Configura el pin 14 como entrada con pull-up interno
  pinMode(BOTON_ENTER, INPUT_PULLUP); // Configura el pin 27 como entrada con pull-up interno
  pinMode(BOTON_TOPE_UP, INPUT_PULLUP); // Configura el pin 26 como entrada con pull-up interno
  pinMode(BOTON_TOPE_DOWN, INPUT_PULLUP); // Configura el pin 25 como entrada con pull-up interno


  Wire.begin(LQ_SDA, LQ_SCL); // Configura I2C en ESP32 (SDA = GPIO 21, SCL = GPIO 22)

  Serial.println("iniciando----->");
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







  lcd.begin(16, 2);   // Inicia LCD de 16x2
  lcd.setBacklight(255); // Activa la luz de fondo
  lcd.createChar(0, letra_N);
  lcd.createChar(1, letra_grado);


  limpiar_pantalla();
  mostrar_pantalla_inicio();

  delay(200);

}

void loop() {

  int subir=0;
  int bajar=0;

  
  int estadoBoton_menu = digitalRead(BOTON_MENU); // Lee el estado del botón
  int estadoBoton_up = digitalRead(BOTON_UP); // Lee el estado del botón
  int estadoBoton_down = digitalRead(BOTON_DOWN); // Lee el estado del botón
  int estadoBoton_enter = digitalRead(BOTON_ENTER); // Lee el estado del botón
  int estadoBoton_tope_up = digitalRead(BOTON_TOPE_UP); // Lee el estado del botón
  int estadoBoton_tope_down = digitalRead(BOTON_TOPE_DOWN); // Lee el estado del botón


  g_temperatura = dht.readTemperature(); // en °C
  g_humedad = dht.readHumidity();

  if (g_temperatura >= g_parametros.tempSubida)
  {
    subir=1;  
    bajar=0;
    en_movimiento=1;
  }
  else if (g_temperatura <= g_parametros.tempBajada)
  {
    subir=0;
    bajar=1;
    en_movimiento=1;
  }
  


  if (estadoBoton_tope_up == LOW) {
    subir=0;    
    Serial.println("el tope_up est en LOW" );
  }

  if (estadoBoton_tope_down == LOW) {
    bajar=0;
    Serial.println("el tope_down est en LOW" );
  }

  if (subir==1)
  {
   analogWrite(LPWM, 0);
   analogWrite(RPWM, 255); // Velocidad media (0-255)   
  }
  else if (bajar==1)
  {
   analogWrite(RPWM, 0); // Velocidad media (0-255)
   analogWrite(LPWM, 255);
  }
  else
  {
   analogWrite(RPWM, 0); // Velocidad media (0-255)
   analogWrite(LPWM, 0);
  }





  


  


  //
  if (actual == nullptr)
  {
    DateTime now_rtc = rtc.now();

    /*
      Serial.print(now.year()); Serial.print('/');
      Serial.print(now.month()); Serial.print('/');
      Serial.print(now.day()); Serial.print(" ");
      Serial.print(now.hour()); Serial.print(':');
      Serial.print(now.minute()); Serial.print(':');
      Serial.println(now.second());
    */



    /*
      struct tm timeinfo;
      time_t now = time(NULL);
      localtime_r(&now, &timeinfo);
    */


    String x_f;
    String x_h;

    String x_fecha;


    char x_buffer[50];
    char x_buffer2[50];
    sprintf(x_buffer, "%02d-%02d-%02d", now_rtc.day(), now_rtc.month(), now_rtc.year());
    sprintf(x_buffer2, "%02d:%02d", now_rtc.hour(), now_rtc.minute());

    x_f = String(x_buffer);
    x_h = String(x_buffer2);

    //  Serial.println(x_f);
    //  Serial.println(x_h);

    lcd.setCursor(9, 0);

    if (isnan(g_temperatura))
    {
      lcd.print("   --");
    }
    else {
      lcd.print(g_temperatura);

    }
    lcd.setCursor(14, 0);
    lcd.write(1);
    lcd.setCursor(15, 0);
    lcd.print("C");


    lcd.setCursor(0, 1);
    lcd.print(x_f);
    lcd.setCursor(11, 1);
    lcd.print(x_h);



    //lcd.setCursor(0, 1);
    //lcd.print(pantalla_inicio[1]);



    //Serial.printf("%02d-%02d-%04d %02d:%02d:%02d\n",
    //  timeinfo.tm_mday, timeinfo.tm_mon + 1, timeinfo.tm_year + 1900,
    //  timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
  }

  //




  if (estadoBoton_menu == LOW) { // Activo en bajo por el pull-up
    Serial.println("Botón menu en PRESIONADO");
    if (actual == nullptr) {
      actual = raiz;
      actual->posicion = 0;
      posicion_menu = 0;
      mostrar_menu(actual, LCD_LINEAS);
    }
    else {
      posicion_menu = -1;
      en_accion = 0;
      actual->posicion = -1;
      actual->activo = 0;
      actual = nullptr;
      limpiar_pantalla();
      mostrar_pantalla_inicio();
    }
  } else {
    //Serial.println("Botón en PIN 15 NO presionado");
  }

  if (estadoBoton_up == LOW) { // Activo en bajo por el pull-up
    Serial.println("Botón up en PRESIONADO");
    if (actual != nullptr)
    {
      if (en_accion == 0)
      {
        actual->anterior_menu();
        mostrar_menu(actual, LCD_LINEAS);
      }
      else
      {

        if (tipo_accion == TIPO_ACCION_NUMERO)
        {
          limpiar_fila(1);
          lcd.setCursor(0, 1);
          accion_numero++;
          if (accion_numero > 40)
          {
            accion_numero = 40;
          }
          lcd.print(accion_numero);
        }
        else if (tipo_accion == TIPO_ACCION_FECHA)
        {
          Serial.println("en enter tipo_accion_fecha");
          Serial.println(g_modifica_actual);
          Serial.println("++++++++++++++++++++++++");
          accion_numero++;
          if (g_modifica_actual == "YEAR")
          {
            lcd.setCursor(0, 1);
            if (accion_numero > 2040)
            {
              accion_numero = 2040;
            }
            g_year = accion_numero;
            lcd.print(accion_numero);
          }
          else if (g_modifica_actual == "MONTH")
          {
            lcd.setCursor(5, 1);
            if (accion_numero < 1)
            {
              accion_numero = 12;
            }
            else if (accion_numero > 12)
            {
              accion_numero = 1;
            }

            if (accion_numero < 10)
            {
              lcd.print("0");
            }
            lcd.print(accion_numero);

          }
          else if (g_modifica_actual == "DAY")
          {
            lcd.setCursor(8, 1);
            Serial.println("en subida");
            Serial.println(accion_numero);
            Serial.println(g_fecha_actual.month());



            if (accion_numero > 31) {
              Serial.println("mayor que 31");
              accion_numero = 1;
            }
            else if (accion_numero > 30)
            {
              Serial.println("mayor que 30");
              if ((g_fecha_actual.month() == 1) || (g_fecha_actual.month() == 3) || (g_fecha_actual.month() == 5) || (g_fecha_actual.month() == 7) || (g_fecha_actual.month() == 8) || (g_fecha_actual.month() == 10) || (g_fecha_actual.month() == 12))
              {
                Serial.println("mes de 31");
              }
              else {
                Serial.println("mes de 30");
                accion_numero = 1;

              }

            }
            else if (accion_numero > 29) {
              if (g_fecha_actual.month() == 2) {
                if ((g_fecha_actual.year() % 4) == 0 )
                {
                  accion_numero = 1;
                }
              }
            }
            else if (accion_numero > 28)
            {
              if (g_fecha_actual.month() == 2) {
                if ((g_fecha_actual.year() % 4) != 0 )
                {
                  Serial.println("febrero");
                  accion_numero = 1;
                }
              }


            }

            if (accion_numero < 10)
            {
              lcd.print("0");
            }
            lcd.print(accion_numero);
          }
          else if (g_modifica_actual == "HOUR"){
             lcd.setCursor(11, 1);            
            if (accion_numero>23) {
              accion_numero=0;
            }
            if (accion_numero < 10)
            {
              lcd.print("0");
            }
            lcd.print(accion_numero);
          }
          else if (g_modifica_actual == "MINUTE"){
            lcd.setCursor(14, 1);            
            if (accion_numero>59) {
              accion_numero=0;
            }
              if (accion_numero < 10)
            {
              lcd.print("0");
            }
            lcd.print(accion_numero);
          }


        }


      }
    }
  }

  if (estadoBoton_down == LOW) { // Activo en bajo por el pull-up
    Serial.println("Botón down en PRESIONADO");
    if (actual != nullptr)
    {
      Serial.println("xxx1---------------->");
      Serial.println("actual es no nulo");
      Serial.println(actual->descripcion);
      Serial.println(tipo_accion);
      Serial.println("xxx2---------------->");


      if (en_accion == 0)
      {
        Serial.println("en accion 0");
        actual->siguiente_menu();
        mostrar_menu(actual, LCD_LINEAS);
      }
      else
      {
        Serial.println("en accion 1");
        if (tipo_accion == TIPO_ACCION_NUMERO)
        {
          limpiar_fila(1);
          lcd.setCursor(0, 1);
          accion_numero--;
          if (accion_numero < -3)
          {
            accion_numero = -3;
          }
          lcd.print(accion_numero);
        }
        else if (tipo_accion == TIPO_ACCION_FECHA)
        {
          Serial.println("tipo accion fecha");

          accion_numero--;
          Serial.print("accion_numero="  );
          Serial.println(accion_numero );
          Serial.println(g_modifica_actual );

          if (g_modifica_actual == "YEAR")
          {
            lcd.setCursor(0, 1);
            if (accion_numero < 2020)
            {
              accion_numero = 2020;
            }
            g_year = accion_numero;
            lcd.print(accion_numero);
          }
          else if (g_modifica_actual == "MONTH")
          {

            lcd.setCursor(5, 1);
            if (accion_numero < 1)
            {
              accion_numero = 12;
            }
            else if (accion_numero > 12)
            {
              accion_numero = 1;
            }

            if (accion_numero < 10)
            {
              lcd.print("0");
            }
            lcd.print(accion_numero);



          }
          else if (g_modifica_actual == "DAY")
          {
            lcd.setCursor(8, 1);
            if (accion_numero < 1)
            {
              if ((g_fecha_actual.month() == 1) || (g_fecha_actual.month() == 3) || (g_fecha_actual.month() == 5) || (g_fecha_actual.month() == 7) || (g_fecha_actual.month() == 8) || (g_fecha_actual.month() == 10) || (g_fecha_actual.month() == 12))
              {
                accion_numero = 31;
              }
              else
              {
                if (g_fecha_actual.month() != 2 )
                {
                  accion_numero = 30;
                }
                else {
                  if ((g_fecha_actual.year() % 4) == 0 )
                  {
                    accion_numero = 29;
                  }
                  else {
                    accion_numero = 28;
                  }
                }
              }

            }



            if (accion_numero < 10)
            {
              lcd.print("0");
            }
            lcd.print(accion_numero);
          }
          else if (g_modifica_actual == "HOUR"){
             lcd.setCursor(11, 1);            
            if (accion_numero<0) {
              accion_numero=23;
            }
            if (accion_numero < 10)
            {
              lcd.print("0");
            }
            lcd.print(accion_numero);
          }
          else if (g_modifica_actual == "MINUTE"){
            lcd.setCursor(14, 1);            
            if (accion_numero<0) {
              accion_numero=59;
            }
              if (accion_numero < 10)
            {
              lcd.print("0");
            }
            lcd.print(accion_numero);
          }
        }

      }
    }
    else {
      Serial.println("actual es nulo");
    }
  }

  if (estadoBoton_enter == LOW) { // Activo en bajo por el pull-up
    Serial.println("Botón enter en PRESIONADO");

    if (actual != nullptr)
    {
      Serial.println("i***************");
      Serial.println(actual->descripcion);
      Serial.println("f***************");
      int x_pos = actual->posicion;
      if (en_accion == 0)
      {
        Serial.println("en accion 0");
        if (actual->hijos[x_pos]->tipo == TIPO_EVENTO_MENU)
        {
          actual->posicion = -1;
          actual = actual->hijos[x_pos];
          actual->posicion = 0;
          limpiar_pantalla();
          mostrar_menu(actual, LCD_LINEAS);

        }
        else if (actual->hijos[x_pos]->tipo == TIPO_EVENTO_ATRAS)
        {
          actual->posicion = -1;
          actual = actual->parent;
          actual->posicion = 0;
          limpiar_pantalla();
          mostrar_menu(actual, LCD_LINEAS);
        }
        else if (actual->hijos[x_pos]->tipo == TIPO_EVENTO_SALIR)
        {
          actual->posicion = -1;
          actual->activo = 0;
          actual = nullptr;
          limpiar_pantalla();
          mostrar_pantalla_inicio();
        }
        else if (actual->hijos[x_pos]->tipo == TIPO_EVENTO_ACCION)
        {
          actual = actual->hijos[x_pos];
          ejecutar_accion(actual->accion);
        }
      }
      else
      {

        if (tipo_accion == TIPO_ACCION_NUMERO) {

          if (actual->accion == ACCION_TEMPERATURASUBIDA) {

            g_parametros.tempSubida = accion_numero;

            grabar_en_archivo(g_parametros, "/parametros.json");
            tipo_accion = "";
            glosa_accion = "";
            en_accion = 0;

            limpiar_pantalla();

            actual = actual->parent;
            mostrar_menu(actual, LCD_LINEAS);


          }

          else if (actual->accion == ACCION_TEMPERATURABAJADA) {

            g_parametros.tempBajada = accion_numero;
            grabar_en_archivo(g_parametros, "/parametros.json");

            tipo_accion = "";
            glosa_accion = "";
            en_accion = 0;
            limpiar_pantalla();
            actual = actual->parent;
            mostrar_menu(actual, LCD_LINEAS);

          }
        }
        else if (tipo_accion == TIPO_ACCION_FECHA)
        {
          if (actual->accion = ACCION_FECHAACTUAL) {

            //tipo_accion = "";
            glosa_accion = "Fecha";
            en_accion = 1;
            limpiar_pantalla();

            //mostrar_menu(actual,LCD_LINEAS);
            Serial.println("en accion fecha---3");

            if (g_modifica_actual == "YEAR")
            {

              g_fecha_actual = DateTime(accion_numero, g_fecha_actual.month(), g_fecha_actual.day(), g_fecha_actual.hour(), g_fecha_actual.minute(), g_fecha_actual.second() );

              accion_numero = g_fecha_actual.month();
              Serial.println("en accion fecha---4");
              g_modifica_actual = "MONTH";

              String accion_fecha = escribir_fecha(g_fecha_actual);
              limpiar_pantalla();
              lcd.setCursor(0, 0);
              lcd.print(glosa_accion);
              lcd.setCursor(0, 1);
              lcd.print(accion_fecha);



              Serial.println("en accion fecha---5");
            }
            else if (g_modifica_actual == "MONTH")
            {
              g_fecha_actual = DateTime(g_fecha_actual.year(),accion_numero , g_fecha_actual.day(), g_fecha_actual.hour(), g_fecha_actual.minute(), g_fecha_actual.second() );
              accion_numero = g_fecha_actual.day();
              String accion_fecha = escribir_fecha(g_fecha_actual);
              limpiar_pantalla();
              lcd.setCursor(0, 0);
              lcd.print(glosa_accion);
              lcd.setCursor(0, 1);
              lcd.print(accion_fecha);
              g_modifica_actual = "DAY";

            }
            else if (g_modifica_actual == "DAY") {
              g_fecha_actual = DateTime(g_fecha_actual.year(),g_fecha_actual.month(), accion_numero , g_fecha_actual.hour(), g_fecha_actual.minute(), g_fecha_actual.second() );
              accion_numero = g_fecha_actual.hour();
              String accion_fecha = escribir_fecha(g_fecha_actual);

              limpiar_pantalla();
              lcd.setCursor(0, 0);
              lcd.print(glosa_accion);
              lcd.setCursor(0, 1);
              lcd.print(accion_fecha);
              g_modifica_actual = "HOUR";
              


            }
            else if (g_modifica_actual == "HOUR") {
              g_fecha_actual = DateTime(g_fecha_actual.year(),g_fecha_actual.month(), g_fecha_actual.day()  , accion_numero, g_fecha_actual.minute(), g_fecha_actual.second() );
              accion_numero = g_fecha_actual.minute();
              String accion_fecha = escribir_fecha(g_fecha_actual);

              limpiar_pantalla();
              lcd.setCursor(0, 0);
              lcd.print(glosa_accion);
              lcd.setCursor(0, 1);
              lcd.print(accion_fecha);
              g_modifica_actual = "MINUTE";


            }
            else if (g_modifica_actual == "MINUTE") {
               //RTC
              DateTime now_rtc = rtc.now();
              g_fecha_actual = DateTime(g_fecha_actual.year(), g_fecha_actual.month(), g_fecha_actual.day(), g_fecha_actual.hour(), accion_numero, now_rtc.second() );

             
             
               rtc.adjust(g_fecha_actual);  // AAAA, MM, DD, HH, MM, SS

              tipo_accion = "";
              glosa_accion = "";
              en_accion = 0;
              limpiar_pantalla();
              actual = actual->parent;
              mostrar_menu(actual, LCD_LINEAS);


            }




          }

        }
        Serial.println("aqui2.2");
      }

    }
  }

  delay(100); // Pequeña pausa para evitar rebotes

}

void ejecutar_accion(String accion)
{
  if (accion == ACCION_TEMPERATURASUBIDA)
  {
    en_accion = 1;
    tipo_accion = "NUMERO";
    glosa_accion = "TEMP SUBIDA";
    accion_numero = g_parametros.tempSubida;
    limpiar_pantalla();
    lcd.setCursor(0, 0);
    lcd.print(glosa_accion);
    lcd.setCursor(0, 1);
    lcd.print(accion_numero);
  }
  else if (accion == ACCION_TEMPERATURABAJADA)
  {
    en_accion = 1;
    tipo_accion = "NUMERO";
    glosa_accion = "TEMP BAJADA";
    accion_numero = g_parametros.tempBajada;
    limpiar_pantalla();
    lcd.setCursor(0, 0);
    lcd.print(glosa_accion);
    lcd.setCursor(0, 1);
    lcd.print(accion_numero);

  }
  else if (accion == ACCION_FECHAACTUAL)
  {
    Serial.println("ejecutar_accion->ACCION_FECHAACTUAL");
    g_fecha_actual = obtener_fecha();


    en_accion = 1;
    g_modifica_actual = "YEAR";
    tipo_accion = TIPO_ACCION_FECHA;
    glosa_accion = "Fecha";
    accion_numero = g_fecha_actual.year();
    String accion_fecha = escribir_fecha(g_fecha_actual);
    limpiar_pantalla();
    lcd.setCursor(0, 0);
    lcd.print(glosa_accion);
    lcd.setCursor(0, 1);
    lcd.print(accion_fecha);

  }
  else if (accion == ACCION_HORAACTUAL)
  {
    en_accion = 1;
    tipo_accion = "NUMERO";
    glosa_accion = "TEMP BAJADA";
    accion_numero = g_parametros.tempSubida;
    limpiar_pantalla();
    lcd.setCursor(0, 0);
    lcd.print(glosa_accion);
    lcd.setCursor(0, 1);
    lcd.print(accion_numero);

  }

}


void limpiar_pantalla()
{

  lcd.setCursor(0, 0);
  lcd.print("                ");
  lcd.setCursor(0, 1);
  lcd.print("                ");
}

void limpiar_fila(int fila)
{
  lcd.setCursor(0, fila);
  lcd.print("                ");
}

void mostrar_pantalla_inicio() {
  lcd.setCursor(0, 0);
  lcd.print(pantalla_inicio[0] + " " + pantalla_inicio[1] );
  lcd.setCursor(0, 1);
}

void mostrar_menu(CMenuItem* nodo,  int lineas)
{
  int cantidad = 0;

  limpiar_pantalla();

  lcd.setCursor(0, 0);
  String texto = nodo->hijos[nodo->posicion]->descripcion;

  String* partes = splitStringConDelimitador(texto, 'ñ', cantidad);
  for (int i = 0; i < cantidad; i++) {
    if (i == 0) {
      lcd.print(">");
    }
    Serial.println(partes[i]);
    if (partes[i] == "ñ")
    {
      Serial.println("entre-->" + partes[i]);
      lcd.write(0);
    }
    else {
      Serial.println("otro-->" + partes[i]);
      lcd.print(partes[i]);
    }
  }

  cantidad = 0;
  texto = nodo->hijos[(nodo->posicion + 1) % (nodo->cantidad)]->descripcion;
  lcd.setCursor(0, 1);
  partes = splitStringConDelimitador(texto, 'ñ', cantidad);
  for (int i = 0; i < cantidad; i++) {
    if (partes[i] == "ñ")
    {
      lcd.write(0);
    }
    else {
      lcd.print(partes[i]);
    }
  }
  //lcd.print(nodo->hijos[(nodo->posicion+1)%(nodo->cantidad)]->descripcion);

}




String* splitStringConDelimitador(String texto, char delimitador, int &cantidad) {
  String* partes = nullptr;
  int capacidad = 2;  // Capacidad inicial
  cantidad = 0;       // Número de elementos en el array

  partes = new String[capacidad];  // Reservar memoria inicial

  while (texto.length() > 0) {
    int pos = texto.indexOf(delimitador);

    // Capturar la parte antes de "Ñ"
    if (pos == -1) {
      partes[cantidad++] = texto; // Última parte sin el delimitador
      break;
    } else {
      if (pos > 0) {  // Evita agregar cadenas vacías si "Ñ" está al inicio
        Serial.println(pos);
        partes[cantidad++] = texto.substring(0, pos - 1);
        Serial.println("en pos->" + pos);
        Serial.println("en ->" + texto.substring(0, pos - 1));
      }

      // Añadir la "Ñ" como un elemento separado
      partes[cantidad++] = String(delimitador);

      texto = texto.substring(pos + 1);  // Cortar desde después del delimitador
    }

    // Redimensionar si es necesario
    if (cantidad >= capacidad) {
      capacidad *= 2;  // Duplicar tamaño
      String* nuevoArray = new String[capacidad];

      for (int i = 0; i < cantidad; i++) {
        nuevoArray[i] = partes[i];  // Copiar datos
      }

      delete[] partes;  // Liberar memoria anterior
      partes = nuevoArray;  // Apuntar al nuevo array
    }
  }

  return partes;  // Retorna el array dinámico
}


CParametros leer_archivo_configuracion()
{

  const char* nombreArchivo = "/datos.txt";

  if (archivoExiste(nombreArchivo)) {

  } else {
    escribirArchivo(nombreArchivo, "{\"tempSubida\":18,\"tempBajada\":16}");
  }

  leerArchivo(nombreArchivo);



}


void escribirArchivo(const char * path, const char * message) {
  Serial.printf("Escribiendo en archivo: %s\n", path);

  File file = SPIFFS.open(path, FILE_WRITE);
  if (!file) {
    Serial.println("Error al abrir el archivo para escritura");
    return;
  }

  if (file.print(message)) {
    Serial.println("Archivo escrito");
  } else {
    Serial.println("Error al escribir");
  }

  file.close();
}

// Función para leer un archivo
void leerArchivo(const char * path) {
  Serial.printf("Leyendo archivo: %s\n", path);

  File file = SPIFFS.open(path);
  if (!file) {
    Serial.println("Error al abrir el archivo para lectura");
    return;
  }

  Serial.println("Contenido del archivo:");
  while (file.available()) {
    Serial.write(file.read());
  }

  file.close();
}

bool archivoExiste(const char * path) {
  return SPIFFS.exists(path);
}





CParametros leerParametrosDesdeArchivo(const char* path) {
  CParametros parametros;

  // Inicializar SPIFFS
  if (!SPIFFS.begin(true)) {
    Serial.println("Error al montar SPIFFS");
    return parametros;
  }

  // Verificar si el archivo existe
  if (!SPIFFS.exists(path)) {
    Serial.println("archivo no existe");
    escribirArchivo(path, "{\"tempSubida\":25,\"tempBajada\":15}");
  }

  // Abrir archivo para lectura
  File file = SPIFFS.open(path, "r");
  if (!file) {
    Serial.println("Error al abrir el archivo: " + String(path));
    return parametros;
  }

  // Crear buffer para el documento JSON
  StaticJsonDocument<256> doc;

  // Deserializar el JSON
  DeserializationError error = deserializeJson(doc, file);
  file.close();

  serializeJsonPretty(doc, Serial);
  Serial.println(); // Para agregar una nueva línea al final

  // Comprobar si hay errores
  if (error) {
    Serial.print("Error deserializando JSON: ");
    Serial.println(error.c_str());
    return parametros;
  }

  // Extraer valores del JSON y asignarlos al objeto
  if (doc.containsKey("tempSubida")) {
    parametros.tempSubida = doc["tempSubida"];
  }

  if (doc.containsKey("tempBajada")) {
    parametros.tempBajada = doc["tempBajada"];
  }

  return parametros;
}


bool grabar_en_archivo(CParametros g_p , const char* path) {
  // Verificar que SPIFFS esté montado
  if (!SPIFFS.begin(true)) {
    Serial.println("Error al montar SPIFFS");
    return false;
  }

  // Crear documento JSON
  StaticJsonDocument<256> doc;
  doc["tempSubida"] = g_p.tempSubida;
  doc["tempBajada"] = g_p.tempBajada;

  // Abrir archivo para escritura
  File file = SPIFFS.open(path, "w");
  if (!file) {
    Serial.println("Error al abrir el archivo para escritura");
    return false;
  }

  // Serializar JSON al archivo
  if (serializeJson(doc, file) == 0) {
    Serial.println("Error al escribir en el archivo");
    file.close();
    return false;
  }

  file.close();
  Serial.println("Parámetros guardados exitosamente");
  return true;
}


String escribir_fecha(DateTime i_fecha_actual)
{
String x_d=String(i_fecha_actual.year()) + "-" 
                             +   (i_fecha_actual.month() < 10 ? "0" : "")  + String(i_fecha_actual.month()) + "-" 
                             +   (i_fecha_actual.day() < 10 ? "0" : "")    + String(i_fecha_actual.day()) + " "
                             +   (i_fecha_actual.hour() < 10 ? "0" : "")   + String(i_fecha_actual.hour()) + ":" 
                             +   (i_fecha_actual.minute() < 10 ? "0" : "") + String(i_fecha_actual.minute());   
           return x_d;              
}
