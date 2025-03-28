#include <Wire.h>
#include <LiquidCrystal_PCF8574.h>
#include <time.h>
#include <RTClib.h>
#include <ArduinoJson.h>
#include "Configuraciones.h"
#include "CMenuItem.h"

#define BOTON_MENU 13  // Pin donde está conectado el botón
#define BOTON_UP 12
#define BOTON_DOWN 14
#define BOTON_ENTER 27
#define BOTON_TOPE_UP 26
#define BOTON_TOPE_DOWN 25


//probando el cambio






#define LCD_LINEAS 2

#define ACCION_NUMERO "NUMERO"
#define TEMPERATURASUBIDA "TEMPERATURASUBIDA"
#define TEMPERATURABAJADA "TEMPERATURABAJADA"
#define FECHAACTUAL "FECHAACTUAL"
#define HORAACTUAL "HORAACTUAL"



#define LCD_ADDRESS 0x27 


//los pines comunes
#define LQ_SDA 21 //
#define LQ_SCL 22 //

#define RTC_SDA 35
#define RTC_SCL 32

LiquidCrystal_PCF8574  lcd(0x27);
RTC_DS3231 rtc;

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


CMenuItem* raiz;
CMenuItem* actual;
int en_accion;


int largo_menu=3;
String pantalla_inicio[]={"Roller UP","V1.0"};

int posicion_menu=-1;
String tipo_accion="";
String glosa_accion="";
int  temp_subida;
int temp_bajada;
int accion_numero;
String accion_fecha;
DateTime g_fecha_actual;


 DateTime obtener_fecha()
{
DateTime now_rtc = rtc.now();
  return now_rtc ;
}






void setup() {
  Serial.begin(115200);           // Inicia comunicación serie

   StaticJsonDocument<4096> doc;
  DeserializationError error = deserializeJson(doc, getJsonMenu());

  if (error) {
    Serial.println(F("Fallo al parsear JSON"));
    return;
  }

  JsonObject root = doc.as<JsonObject>();
  raiz = CMenuItem::parseMenu(root);
  


  
temp_subida=16;
temp_bajada=14;

  
tipo_accion="";
glosa_accion="";

  en_accion=0;
  
  //cargar_menu();
  
 
  String menu[]={"Configuracion","Manual","Salir"};

   

  
  


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
  else{
    Serial.println("encontre el RTC");  
  }

  if (rtc.lostPower()) {
    Serial.println("El RTC perdió la hora o es la primera vez");
    // Setea la fecha y hora actual (aquí puedes ajustar tu fecha/hora)
    rtc.adjust(DateTime(2025, 3, 25, 15, 30, 0));  // AAAA, MM, DD, HH, MM, SS
  }
  else{
    Serial.println("El RTC no es la primera vez");
  }






  
  lcd.begin(16, 2);   // Inicia LCD de 16x2
  lcd.setBacklight(255); // Activa la luz de fondo
  lcd.createChar(0, letra_N);

  limpiar_pantalla();
  mostrar_pantalla_inicio();

  delay(200);
    
}

void loop() {


  int estadoBoton_menu = digitalRead(BOTON_MENU); // Lee el estado del botón
  int estadoBoton_up = digitalRead(BOTON_UP); // Lee el estado del botón
  int estadoBoton_down = digitalRead(BOTON_DOWN); // Lee el estado del botón
  int estadoBoton_enter = digitalRead(BOTON_ENTER); // Lee el estado del botón
  int estadoBoton_tope_up = digitalRead(BOTON_TOPE_UP); // Lee el estado del botón
  int estadoBoton_tope_down = digitalRead(BOTON_TOPE_DOWN); // Lee el estado del botón

if (estadoBoton_tope_up == LOW) { 

  Serial.println("el tope_up est en LOW" );
}

if (estadoBoton_tope_down == LOW) { 

  Serial.println("el tope_down est en LOW" );
}
  

  //
if (actual==nullptr)
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
  sprintf(x_buffer,"%02d-%02d-%02d",now_rtc.day(), now_rtc.month(), now_rtc.year());  
  sprintf(x_buffer2,"%02d:%02d",now_rtc.hour(), now_rtc.minute());  

  x_f=String(x_buffer);
  x_h=String(x_buffer2);

//  Serial.println(x_f);
//  Serial.println(x_h);
  
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
    if (actual == nullptr){
       actual=raiz;
       actual->posicion=0;
       posicion_menu=0;
       mostrar_menu(actual,LCD_LINEAS);
      }    
    else{
       posicion_menu=-1;
       actual->posicion=-1;
       actual->activo=0;
       actual=nullptr;
       limpiar_pantalla();
       mostrar_pantalla_inicio();   
    }
  } else {
    //Serial.println("Botón en PIN 15 NO presionado");
  }
  
  if (estadoBoton_up == LOW) { // Activo en bajo por el pull-up
    Serial.println("Botón up en PRESIONADO");
     if (actual !=nullptr)
     {
      if (en_accion==0)
           {
            actual->anterior_menu();         
            mostrar_menu(actual,LCD_LINEAS);
           }
       else
           {
            limpiar_fila(1);
            if (tipo_accion==ACCION_NUMERO)
               {                 
                 lcd.setCursor(0, 1);
                 accion_numero++;
                 if (accion_numero>40)
                 {
                  accion_numero=40;
                 }
                 lcd.print(accion_numero);                
               }

            
           }
     }
  } 

  if (estadoBoton_down == LOW) { // Activo en bajo por el pull-up
    Serial.println("Botón down en PRESIONADO");
      if (actual !=nullptr)
     {
      if(en_accion==0)
         {
          actual->siguiente_menu();
          mostrar_menu(actual,LCD_LINEAS);
         }
      else
         {
            limpiar_fila(1);
            if (tipo_accion==ACCION_NUMERO)
               {                 
                 lcd.setCursor(0, 1);
                 accion_numero--;
                 if (accion_numero<-3)
                    {
                     accion_numero=-3;                      
                    }
                 lcd.print(accion_numero);                
               }

            
         }
     }
  } 

  if (estadoBoton_enter == LOW) { // Activo en bajo por el pull-up
    Serial.println("Botón enter en PRESIONADO");
      if (actual !=nullptr)
         {
          int x_pos=actual->posicion;
          if (en_accion==0)
          {
          if (actual->hijos[x_pos]->tipo=="MENU")
              {
                actual->posicion=-1;
                actual=actual->hijos[x_pos];
                actual->posicion=0;
                limpiar_pantalla();
                mostrar_menu(actual,LCD_LINEAS);
        
               }
           else if (actual->hijos[x_pos]->tipo=="ATRAS")
               {
                actual->posicion=-1;
                actual=actual->parent;               
                actual->posicion=0;
                limpiar_pantalla();
                mostrar_menu(actual,LCD_LINEAS);
               }
           else if (actual->hijos[x_pos]->tipo=="SALIR")
               {          
                actual->posicion=-1;
                actual->activo=0;
                actual=nullptr;
                limpiar_pantalla();
                mostrar_pantalla_inicio();   
                }
           else if (actual->hijos[x_pos]->tipo=="ACCION")
                {
                 ejecutar_accion(actual->hijos[x_pos]->accion);
                }      
          }
          else
          {
           if (tipo_accion==ACCION_NUMERO){
            if (actual->accion="TEMPERATURASUBIDA"){
              temp_subida=accion_numero;  
              tipo_accion="";
              glosa_accion="";
              en_accion=0;
              limpiar_pantalla();
              mostrar_menu(actual,LCD_LINEAS);
              
            }

           else if (actual->accion="TEMPERATURABAJADA"){
              temp_bajada=accion_numero;  
              tipo_accion="";
              glosa_accion="";
              en_accion=0;
              limpiar_pantalla();
              mostrar_menu(actual,LCD_LINEAS);
              
            }
            
           }
          }
         }
    
  }  
  

  delay(100); // Pequeña pausa para evitar rebotes
  
}

void ejecutar_accion(String accion)
{
  if (accion==TEMPERATURASUBIDA)
  {
    en_accion=1;
    tipo_accion="NUMERO";
    glosa_accion="TEMP SUBIDA";
    accion_numero=temp_subida;
    limpiar_pantalla();
    lcd.setCursor(0, 0);
    lcd.print(glosa_accion);  
    lcd.setCursor(0, 1);
    lcd.print(accion_numero);
  }
  else if (accion==TEMPERATURABAJADA)
  {
    en_accion=1;
    tipo_accion="NUMERO";
    glosa_accion="TEMP BAJADA";
    accion_numero=temp_subida;
    limpiar_pantalla();
    lcd.setCursor(0, 0);
    lcd.print(glosa_accion);  
    lcd.setCursor(0, 1);
    lcd.print(accion_numero);
  
  }
  else if (accion==FECHAACTUAL)
  {
    g_fecha_actual=obtener_fecha();
    
    
    en_accion=1;
    tipo_accion="FECHA";
    glosa_accion="Fecha";      
    String accion_fecha = String(g_fecha_actual.year()) + "-" +   (g_fecha_actual.month() < 10 ? "0" : "") + String(g_fecha_actual.month()) + "-" +     (g_fecha_actual.day() < 10 ? "0" : "") + String(g_fecha_actual.day());
    limpiar_pantalla();
    lcd.setCursor(0, 0);
    lcd.print(glosa_accion);  
    lcd.setCursor(0, 1);
    lcd.print(accion_fecha);
  
  }
  else if (accion==HORAACTUAL)
  {
    en_accion=1;
    tipo_accion="NUMERO";
    glosa_accion="TEMP BAJADA";
    accion_numero=temp_subida;
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

void mostrar_pantalla_inicio(){
    lcd.setCursor(0, 0);
  lcd.print(pantalla_inicio[0] + " " + pantalla_inicio[1] );    
  lcd.setCursor(0, 1);  
}

void mostrar_menu(CMenuItem* nodo,  int lineas)
{
  int cantidad = 0;
  
  limpiar_pantalla();

  lcd.setCursor(0, 0);
  String texto=nodo->hijos[nodo->posicion]->descripcion;

   String* partes = splitStringConDelimitador(texto, 'ñ', cantidad);
   for (int i = 0; i < cantidad; i++) {
    if (i==0){
      lcd.print(">");
    }
    Serial.println(partes[i]);
    if (partes[i]=="ñ")
    {
      Serial.println("entre-->" + partes[i]);
      lcd.write(0);
    }
    else{
      Serial.println("otro-->" + partes[i]);
      lcd.print(partes[i]);
    }    
  }
   
  cantidad=0;
  texto=nodo->hijos[(nodo->posicion+1)%(nodo->cantidad)]->descripcion;
  lcd.setCursor(0, 1);  
  partes = splitStringConDelimitador(texto, 'ñ', cantidad);
   for (int i = 0; i < cantidad; i++) {
    if (partes[i]=="ñ")
    {
      lcd.write(0);
    }
    else{
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
        partes[cantidad++] = texto.substring(0, pos-1);
        Serial.println("en pos->"+pos);
        Serial.println("en ->"+texto.substring(0, pos-1));
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
