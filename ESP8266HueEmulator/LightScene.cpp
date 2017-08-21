#include "LightScene.h"

void LightScene::begin(int id) {
  strcpy(name,"");
  strcpy(owner,"");
  this->id = id;
  this->removeLights();
  active = true;
}

void LightScene::removeLights() {
  for (int i = 0 ; i < MAX_LIGHTS; ++i)
    lights[i] = nullptr;
  numLights = 0;
}

aJsonObject *LightScene::getJson() {
  aJsonObject *object = aJson.createObject();
  aJson.addStringToObject(object, "name", this->name);
  aJson.addStringToObject(object, "owner", this->owner);
  aJson.addStringToObject(object, "picture", "");
  aJson.addStringToObject(object, "lastupdated", "");
  aJson.addBooleanToObject(object, "recycle", false);
  aJson.addBooleanToObject(object, "locked", false);
  aJson.addNumberToObject(object, "version", 2);
  aJsonObject *lightsArray = aJson.createArray();
  aJson.addItemToObject(object, "lights", lightsArray);
  String lightId;
  for (int i = 0; i < MAX_LIGHTS; ++i) {
    if (this->lights[i]) {
      lightId = "";
      lightId += this->lights[i]->getId();
      aJson.addItemToArray(lightsArray, aJson.createItem(lightId));
    }
  }
  // TODO add lightstates
  return object;
}

void LightScene::addLight(Light *light) {
  
}

void LightScene::setId(int id) {
  this->id = id;
}

int LightScene::getId() {
  return this->id;
}

void LightScene::setName(char* name) {
  strcpy(this->name,name);
}

void LightScene::setOwner(const char* owner) {
  strcpy(this->owner,owner);
}
