#ifndef HELPER_FUNCIONESCOMUNES_H
#define HELPER_FUNCIONESCOMUNES_H

#include <Arduino.h>
#include <RTClib.h>
#include <LiquidCrystal_PCF8574.h>
#include "SPIFFS.h" //para leer y escribir en la memoria flash

#include "CMenuItem.h"

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

unsigned long diferencia_fecha(DateTime i_fecha_ini, DateTime i_fecha_fin )
{
  uint32_t segundos_actual = i_fecha_fin.unixtime();
  uint32_t segundos_anterior = i_fecha_ini.unixtime();

  uint32_t diferencia_segundos = segundos_actual - segundos_anterior;

  return diferencia_segundos;
}


DateTime obtener_fecha(RTC_DS3231 rtc)
{
  DateTime now_rtc = rtc.now();
  return now_rtc ;
}

String escribir_fecha(DateTime i_fecha_actual)
{
  String x_d = String(i_fecha_actual.year()) + "-"
               +   (i_fecha_actual.month() < 10 ? "0" : "")  + String(i_fecha_actual.month()) + "-"
               +   (i_fecha_actual.day() < 10 ? "0" : "")    + String(i_fecha_actual.day()) + " "
               +   (i_fecha_actual.hour() < 10 ? "0" : "")   + String(i_fecha_actual.hour()) + ":"
               +   (i_fecha_actual.minute() < 10 ? "0" : "") + String(i_fecha_actual.minute());
  return x_d;
}





void escribir_fecha_pantalla_inicio(LiquidCrystal_PCF8574  lcd, DateTime i_fecha_actual)
{
  String x_fecha = (i_fecha_actual.day() < 10 ? "0" : "")    + String(i_fecha_actual.day())  + "-"
                   +   (i_fecha_actual.month() < 10 ? "0" : "")  + String(i_fecha_actual.month()) + "-"
                   +   String( i_fecha_actual.year()) + " "
                   +   (i_fecha_actual.hour() < 10 ? "0" : "")   + String(i_fecha_actual.hour()) + ":"
                   +   (i_fecha_actual.minute() < 10 ? "0" : "") + String(i_fecha_actual.minute());

  lcd.setCursor(0, 1);
  lcd.print(x_fecha);

}


void escribir_temperatura_pantalla_inicio(LiquidCrystal_PCF8574  lcd, float i_temperatura) {
  lcd.setCursor(9, 0);
  lcd.print(i_temperatura);
  lcd.write(1);

}


bool archivoExiste(const char * path) {
  return SPIFFS.exists(path);
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



void limpiar_pantalla(LiquidCrystal_PCF8574  lcd)
{

  lcd.setCursor(0, 0);
  lcd.print("                ");
  lcd.setCursor(0, 1);
  lcd.print("                ");
}

void limpiar_fila(LiquidCrystal_PCF8574  lcd, int fila)
{
  lcd.setCursor(0, fila);
  lcd.print("                ");
}

void mostrar_pantalla_inicio(LiquidCrystal_PCF8574  lcd, int i_en_funcion) {
  lcd.setCursor(0, 0);
  if (i_en_funcion == 0 )
  {
    lcd.print("E:Off" );
  }
  else {
    lcd.print("E:On" );
  }
  //lcd.print(pantalla_inicio[0] + " " + pantalla_inicio[1] );
  lcd.setCursor(0, 1);
}

void mostrar_menu(LiquidCrystal_PCF8574  lcd, CMenuItem* nodo,  int lineas)
{
  int cantidad = 0;


  limpiar_pantalla(lcd);

  lcd.setCursor(0, 0);
  String texto = nodo->hijos[nodo->posicion]->descripcion;

  lcd.print(">");
  lcd.print(texto);


  texto = nodo->hijos[(nodo->posicion + 1) % (nodo->cantidad)]->descripcion;
  lcd.setCursor(0, 1);
  lcd.print(texto);


}

boolean mes_con_30(  int mes)
{
  if ((mes == 4) || (mes == 6) || (mes == 9) || (mes == 11))
  {
   return true;
  }
  else
  {
    return false;
  }
}

boolean es_bisiesto(  int i_year)
{
  if (i_year % 4 == 0)
  {
   return true;
  }
  else
  {
    return false;
  }
}

String texto_onoff(  int i_estado)
{
  if (i_estado == 0)
  {
   return "Off";
  }
  else
  {
    return "On";
  }
}









#endif
