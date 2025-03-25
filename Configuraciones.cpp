#include "Configuraciones.h"

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


const char* getJsonMenu() {
  return jsonString;
}
