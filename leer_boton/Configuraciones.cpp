#include "Configuraciones.h"

const char* jsonString = R"rawliteral(
{
  "descripcion":"",
  "tipo":"MENU",
  "cantidad":4,
  "activo":-1,
  "nivel":0,
  "accion":"",
  "hijos":[
    {"descripcion":"Configuracion","tipo":"MENU","cantidad":4,"activo":-1,"nivel":1,"accion":"",
      "hijos":[
        {"descripcion":"Temp Subida","tipo":"ACCION","cantidad":0,"activo":0,"nivel":2,"accion":"TEMPERATURASUBIDA"},
        {"descripcion":"Temp Bajada","tipo":"ACCION","cantidad":0,"activo":0,"nivel":2,"accion":"TEMPERATURABAJADA"},
        {"descripcion":"Fecha actual","tipo":"ACCION","cantidad":0,"activo":0,"nivel":2,"accion":"FECHAACTUAL"},        
        {"descripcion":"Atras","tipo":"ATRAS","cantidad":0,"activo":0,"nivel":2,"accion":"ATRAS"}
      ]
    },
    {"descripcion":"Activo","tipo":"ACCION","cantidad":0,"activo":0,"nivel":1,"accion":"ACTIVADO"},
    {"descripcion":"Manual","tipo":"ACCION","cantidad":0,"activo":0,"nivel":1,"accion":"MANUAL"},
    {"descripcion":"Salir","tipo":"ACCION","cantidad":0,"activo":0,"nivel":1,"accion":"SALIR", "hijos":[]}
  ]
}
)rawliteral";


const char* getJsonMenu() {
  return jsonString;
}
