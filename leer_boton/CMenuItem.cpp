#include "CMenuItem.h"

CMenuItem::CMenuItem() {
    descripcion = "";
    nivel = 0;
    cantidad = 0;
    activo = 0;
    posicion = -1;
    for (int i = 0; i < 10; i++) {
        hijos[i] = nullptr;
    }
    tipo = "";
    accion = "";
    parent = nullptr;
}

void CMenuItem::siguiente_menu() {
    posicion = (posicion + 1) % cantidad;
}

void CMenuItem::anterior_menu() {
    posicion = (posicion - 1 + cantidad) % cantidad;
}

CMenuItem* CMenuItem::parseMenu(JsonObject obj, CMenuItem* parent) {
    CMenuItem* item = new CMenuItem();
    item->descripcion = obj["descripcion"] | "";
    item->tipo = obj["tipo"] | "";
    item->cantidad = obj["cantidad"] | 0;
    item->activo = obj["activo"] | 0;
    item->nivel = obj["nivel"] | 0;
    item->accion = obj["accion"] | "";
    item->parent = parent;

    if (obj.containsKey("hijos")) {
        JsonArray hijosArray = obj["hijos"].as<JsonArray>();
        int hijosCount = min(10, (int)hijosArray.size());
        for (int i = 0; i < hijosCount; i++) {
            item->hijos[i] = CMenuItem::parseMenu(hijosArray[i], item);
        }
    }
    return item;
}
