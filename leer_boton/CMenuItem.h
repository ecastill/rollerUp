#ifndef CMENUITEM_H
#define CMENUITEM_H

#include <Arduino.h>
#include <ArduinoJson.h>

class CMenuItem {
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

    CMenuItem();

    void siguiente_menu();
    void anterior_menu();
    static CMenuItem* parseMenu(JsonObject obj, CMenuItem* parent = nullptr);
};

#endif
