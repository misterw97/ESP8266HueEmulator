#include "Light.h"

aJsonObject *Light::getJson() {
  aJsonObject *object = aJson.createObject();
  // type of lamp
  aJson.addStringToObject(object, "type", this->getTypeName());
  // the name as set through the web UI or app
  aJson.addStringToObject(object, "name", this->getName());
  String uniqueid = "AA:BB:CC:DD:EE:FF:00:11-";
  uniqueid += this->getId();
  aJson.addStringToObject(object, "uniqueid",  uniqueid.c_str());
  // the model number
  aJson.addStringToObject(object, "modelid", this->getTypeModelid());
  aJson.addItemToObject(object, "state", this->getStateJson());
  return object;
}

aJsonObject *Light::getStateJson() {
  return this->info.getJson();
}

Light::Light(char* n, LightType t) {
  strcpy(this->name,n);
  this->type = t;
}

char* Light::getName() {
  return this->name;
}

void Light::setId(int id) {
  this->id = id;
}

int Light::getId() {
  return this->id;
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
