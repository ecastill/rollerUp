#include <Wire.h>
#include <LiquidCrystal_PCF8574.h>
#include <time.h>
#include <RTClib.h>
#include <ArduinoJson.h>
#include "Configuraciones.h"

#define BOTON_MENU 13  // Pin donde está conectado el botón
#define BOTON_UP 12
#define BOTON_DOWN 14
#define BOTON_ENTER 27



#define LCD_LINEAS 2

#define ACCION_NUMERO "NUMERO"
#define TEMPERATURASUBIDA "TEMPERATURASUBIDA"
#define TEMPERATURABAJADA "TEMPERATURABAJADA"


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


class CMenuItem{  
  public:
  String descripcion;  
  int nivel;
  int cantidad;
  int activo;
  int posicion;
  String tipo;
  String accion;
  CMenuItem* hijos[10];
  CMenuItem* parent;

  CMenuItem(){
        descripcion = "";
        nivel = 0;
        cantidad = 0;
        activo=0;
        posicion=-1;
        for (int i = 0; i < 6; i++) {
            hijos[i] = nullptr; // Inicializar hijos en NULL
        }
        tipo="";
        accion="";
        parent=nullptr;
  }

  void siguiente_menu()
  {
   posicion=(posicion+1)%cantidad;
  }

  void anterior_menu()
  {
   posicion=(posicion-1 + cantidad)%cantidad;
  }
  
};



CMenuItem* raiz;
CMenuItem* actual;
int en_accion;

//String menu[]={"Configuracion","Manual","Salir","OTRO"};
//String submenu_1[]={"Temperatura Subir","Temperatura Bajar","Atras"};
//String submenu_2[]={"Atras"};

int largo_menu=3;
String pantalla_inicio[]={"Roller UP","V1.0"};

int posicion_menu=-1;
String tipo_accion="";
String glosa_accion="";
int  temp_subida;
int temp_bajada;
int accion_numero;

/*
const char* jsonString = R"rawliteral(
{
  "descripcion":"",
  "tipo":"MENU",
  "cantidad":3,
  "activo":-1,
  "nivel":0,
  "accion":"",
  "hijos":[
    {"descripcion":"Configuracion","tipo":"MENU","cantidad":3,"activo":-1,"nivel":1,"accion":"",
      "hijos":[
        {"descripcion":"Temp Subida","tipo":"ACCION","cantidad":0,"activo":0,"nivel":2,"accion":"TEMPERATURASUBIDA"},
        {"descripcion":"Temp Bajada","tipo":"ACCION","cantidad":0,"activo":0,"nivel":2,"accion":"TEMPERATURABAJADA"},
        {"descripcion":"Fecha","tipo":"MENU","cantidad":6,"activo":0,"nivel":2,"accion":"",
          "hijos":[
            {"descripcion":"Ano","tipo":"ACCION","cantidad":0,"activo":0,"nivel":3,"accion":"YEAR"},
            {"descripcion":"Mes","tipo":"ACCION","cantidad":0,"activo":0,"nivel":3,"accion":"MES"},
            {"descripcion":"Dia","tipo":"ACCION","cantidad":0,"activo":0,"nivel":3,"accion":"DIA"},
            {"descripcion":"Hora","tipo":"ACCION","cantidad":0,"activo":0,"nivel":3,"accion":"HORA"},
            {"descripcion":"Minuto","tipo":"ACCION","cantidad":0,"activo":0,"nivel":3,"accion":"MINUTO"},
            {"descripcion":"Atras","tipo":"ATRAS","cantidad":0,"activo":0,"nivel":3,"accion":""}
          ]
        },
        {"descripcion":"Atras","tipo":"ATRAS","cantidad":0,"activo":0,"nivel":2,"accion":""}
      ]
    },
    {"descripcion":"Manual","tipo":"MENU","cantidad":0,"activo":-1,"nivel":1,"accion":"","hijos":[]},
    {"descripcion":"Salir","tipo":"SALIR","cantidad":0,"activo":0,"nivel":1,"accion":"", "hijos":[]}
  ]
}
)rawliteral";
*/

CMenuItem* parseMenu(JsonObject obj, CMenuItem* parent = nullptr) {
  CMenuItem* item = new CMenuItem();
  item->descripcion = obj["descripcion"].as<String>();
  item->tipo = obj["tipo"].as<String>();
  item->cantidad = obj["cantidad"].as<int>();
  item->activo = obj["activo"].as<int>();
  item->nivel = obj["nivel"].as<int>();
  item->accion = obj["accion"].as<String>();
  item->parent = parent;

  if (obj.containsKey("hijos")) {
    JsonArray hijosArray = obj["hijos"].as<JsonArray>();
    int hijosCount = min(10, (int)hijosArray.size());  // Máximo 10 hijos por tu estructura
    for (int i = 0; i < hijosCount; i++) {
      item->hijos[i] = parseMenu(hijosArray[i], item); // Recursión
    }
  }
  return item;
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
  raiz = parseMenu(root);
  


  
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

void cargar_menu()
{
  raiz= new CMenuItem();
  raiz->cantidad=3;
  raiz->descripcion="";
  raiz->activo=-1;
  raiz->nivel=0;
  raiz->tipo="MENU";
  raiz->accion="";
  raiz->parent=nullptr;

  CMenuItem* subhijo = new CMenuItem();

  CMenuItem* hijo = new CMenuItem();
  hijo->descripcion="Configuracion";
  hijo->cantidad=3;
  hijo->nivel=1;
  hijo->activo=0;
  hijo->tipo="MENU";
  hijo->accion="";
  hijo->parent=raiz;

  hijo->hijos[0]=new CMenuItem();
  hijo->hijos[0]->descripcion="Temp subida";
  hijo->hijos[0]->cantidad=0;
  hijo->hijos[0]->nivel=2;
  hijo->hijos[0]->tipo="ACCION";
  hijo->hijos[0]->accion="TEMPERATURASUBIDA";
  hijo->hijos[0]->parent=hijo;
  

  hijo->hijos[1]=new CMenuItem();
  hijo->hijos[1]->descripcion="Temp bajada";
  hijo->hijos[1]->cantidad=0;
  hijo->hijos[1]->nivel=2;
  hijo->hijos[1]->tipo="ACCION";
  hijo->hijos[1]->accion="TEMPERATURABAJADA";
  hijo->hijos[1]->parent=hijo;

  hijo->hijos[2]=new CMenuItem();
  hijo->hijos[2]->descripcion="Fecha";
  hijo->hijos[2]->cantidad=6;
  hijo->hijos[2]->nivel=2;
  hijo->hijos[2]->tipo="MENU";
  hijo->hijos[2]->accion="";
  hijo->hijos[2]->parent=hijo;


//year
  subhijo = new CMenuItem();
  subhijo->descripcion="Año";
  subhijo->cantidad=0;
  subhijo->nivel=3;
  subhijo->tipo="ACCION";
  subhijo->accion="YEAR";  
  subhijo->parent=hijo->hijos[2];
  hijo->hijos[2]->hijos[0]=subhijo;
  


//mes
  subhijo = new CMenuItem();
  subhijo->descripcion="Mes";
  subhijo->cantidad=0;
  subhijo->nivel=3;
  subhijo->tipo="ACCION";
  subhijo->accion="MES";  
  subhijo->parent=hijo->hijos[2];
  hijo->hijos[2]->hijos[1]=subhijo;

//dia
  subhijo = new CMenuItem();
  subhijo->descripcion="Dia";
  subhijo->cantidad=0;
  subhijo->nivel=3;
  subhijo->tipo="ACCION";
  subhijo->accion="DIA";  
  subhijo->parent=hijo->hijos[2];
  hijo->hijos[2]->hijos[2]=subhijo;


//hora
  subhijo = new CMenuItem();
  subhijo->descripcion="Hora";
  subhijo->cantidad=0;
  subhijo->nivel=3;
  subhijo->tipo="ACCION";
  subhijo->accion="HORA";  
  subhijo->parent=hijo->hijos[2];
  hijo->hijos[2]->hijos[3]=subhijo;
  
//minut0
  subhijo = new CMenuItem();
  subhijo->descripcion="Minuto";
  subhijo->cantidad=0;
  subhijo->nivel=3;
  subhijo->tipo="ACCION";
  subhijo->accion="HORA";  
  subhijo->parent=hijo->hijos[2];
  hijo->hijos[2]->hijos[4]=subhijo;
//atras
subhijo = new CMenuItem();
  subhijo->descripcion="Atras";
  subhijo->cantidad=0;
  subhijo->nivel=3;
  subhijo->tipo="ATRAS";
  subhijo->accion="";  
  subhijo->parent=hijo->hijos[2];
  hijo->hijos[2]->hijos[5]=subhijo;




  


  hijo->hijos[3]=new CMenuItem();
  hijo->hijos[3]->descripcion="Atras";
  hijo->hijos[3]->cantidad=0;
  hijo->hijos[3]->nivel=2;
  hijo->hijos[3]->tipo="ATRAS";
  hijo->hijos[3]->accion="";
  hijo->hijos[3]->parent=hijo;



   
  raiz->hijos[0]=hijo;
  raiz->cantidad=1;
  



  

  hijo = new CMenuItem();
  hijo->descripcion="Manual";
  hijo->cantidad=0;
  hijo->nivel=1;
  hijo->activo=0;  
  hijo->tipo="MENU";
  hijo->accion="";
  hijo->parent=raiz;
  raiz->hijos[1]=hijo;
  raiz->cantidad=2;
  

  hijo = new CMenuItem();
  hijo->descripcion="Salir";
  hijo->cantidad=0;
  hijo->nivel=1;
  hijo->activo=0; 
  hijo->tipo="SALIR";
  hijo->accion=""; 
  raiz->hijos[2]=hijo;
  hijo->parent=raiz;
  raiz->cantidad=3;

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
