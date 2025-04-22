#ifndef CPARAMETROS_H
#define CPARAMETROS_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include "SPIFFS.h" //para leer y escribir en la memoria flash

class CParametros {
  static const char* NOMBRE_ARCHIVO; // String fijo
  
  public:
    int tempSubida;
    int tempBajada;   
    int enFuncion;
    CParametros();    
    bool leerParametros();
    bool grabar_en_archivo();
};



#endif
