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
  aJsonObject *object = aJson.createObject();
  aJson.addBooleanToObject(object, "on", this->info.on);
  aJson.addNumberToObject(object, "bri", this->info.brightness);
  aJson.addNumberToObject(object, "hue", this->info.hue);
  aJson.addNumberToObject(object, "sat", this->info.saturation);
  aJsonObject *xyArray = aJson.createArray();
  // TODO actually store this (xy mode: CIE 1931 color co-ordinates)
  double numbers[2] = {0.0, 0.0};
  aJson.addItemToObject(object, "xy", aJson.createFloatArray(numbers, 2));
  // TODO actually store this (ct mode: color temp (expressed in mireds range 154-500))
  aJson.addNumberToObject(object, "ct", 500);
  aJson.addStringToObject(object, "alert", this->info.alert == ALERT_SELECT ? "select" : ( this->info.alert == ALERT_LSELECT ? "lselect" : "none" ));
  aJson.addStringToObject(object, "effect", this->info.effect == EFFECT_COLORLOOP ? "colorloop" : "none");
  // TODO the current color mode 
  aJson.addStringToObject(object, "colormode", "hs");
  // lamp can be seen by the hub
  aJson.addBooleanToObject(object, "reachable", true);
  return object;  
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
