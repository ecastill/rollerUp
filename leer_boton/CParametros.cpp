#include "CParametros.h"

const char* CParametros::NOMBRE_ARCHIVO = "/parametros.json"; 

CParametros::CParametros() {
  tempSubida = 20;
  tempBajada = 18;
  enFuncion = 0;
}


bool CParametros::leerParametros() {
  

  // Inicializar SPIFFS
  if (!SPIFFS.begin(true)) {
    Serial.println("Error al montar SPIFFS");
    return false;
  }

  // Verificar si el archivo existe
  if (!SPIFFS.exists(NOMBRE_ARCHIVO )) {
    Serial.println("archivo no existe");
    grabar_en_archivo();
  }

  // Abrir archivo para lectura
  File file = SPIFFS.open(NOMBRE_ARCHIVO , "r");
  if (!file) {
    Serial.println("Error al abrir el archivo: " + String(NOMBRE_ARCHIVO ));
    return false;
  }

  Serial.println("1-leer_parametros---------------");

  // Crear buffer para el documento JSON
  StaticJsonDocument<256> doc;

  // Deserializar el JSON
  DeserializationError error = deserializeJson(doc, file);
  file.close();

  serializeJsonPretty(doc, Serial);
  Serial.println(); // Para agregar una nueva línea al final

  Serial.println("2-leer_parametros---------------");

  // Comprobar si hay errores
  if (error) {
    Serial.print("Error deserializando JSON: ");
    Serial.println(error.c_str());
    return false;
  }

  // Extraer valores del JSON y asignarlos al objeto
  if (doc.containsKey("tempSubida")) {
    tempSubida = doc["tempSubida"];
  }

  if (doc.containsKey("tempBajada")) {
    tempBajada = doc["tempBajada"];
  }

  if (doc.containsKey("enFuncion")) {
    enFuncion = doc["enFuncion"];
  }
  

Serial.println("3-leer_parametros---------------");
  return true;
}


  bool CParametros::grabar_en_archivo( ) {
    // Verificar que SPIFFS esté montado
    if (!SPIFFS.begin(true)) {
      Serial.println("Error al montar SPIFFS");
      return false;
    }

    // Crear documento JSON
    StaticJsonDocument<256> doc;
    doc["tempSubida"] = tempSubida;
    doc["tempBajada"] = tempBajada;
    doc["enFuncion"]= enFuncion;
    

    // Abrir archivo para escritura
    File file = SPIFFS.open(NOMBRE_ARCHIVO , "w");
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

  
