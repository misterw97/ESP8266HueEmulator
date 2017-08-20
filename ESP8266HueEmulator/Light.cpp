#include "Light.h"

Light::Light(char* n, LightType t) {
  this->name = n;
  this->type = t;
}

char* Light::getName() {
  return this->name;
}

HueLightInfo Light::getInfo() {
  return this->info;
}

char* Light::getTypeName() {
  switch(this->type) {
    case EXTENDED_COLOR_LIGHT:
      return "Extended Color Light";
    case DIMMABLE_LIGHT:
      return "Dimmable Light";
    case COLOR_LIGHT:
      return "Color Light";
    default:
      return "Generic HUE";
  }
}

char* Light::getTypeModelid() {
  switch(this->type) {
    case EXTENDED_COLOR_LIGHT:
      return "LCT001";
    case DIMMABLE_LIGHT:
      return "LWB004";
    case COLOR_LIGHT:
      return "LST001";
    default:
      return "LCT001";
  }
}
