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
  aJson.addNumberToObject(object, "version", 2);
  aJson.addStringToObject(object, "lastupdated", "");
  aJson.addBooleanToObject(object, "recycle", true);
  aJson.addBooleanToObject(object, "locked", false); 
  aJsonObject *lightsArray = aJson.createArray();
  aJson.addItemToObject(object, "lights", lightsArray);
  char lightId[5];
  for (int i = 0; i < numLights; ++i) {
    if (this->lights[i]) {
      sprintf(lightId, "%d", this->lights[i]->getId());
      aJson.addItemToArray(lightsArray, aJson.createItem(lightId));
    }
  }
  // lightstates
  aJsonObject *lightstates = aJson.createObject();
  aJson.addItemToObject(object, "lightstates", lightstates);
  for (int i = 0; i < numLights; ++i) {
    sprintf(lightId, "%d", this->lights[i]->getId());
    aJson.addItemToObject(lightstates, lightId, this->infos[i].getJson(this->lights[i]->getType()));
  }
  
  return object;
}

void LightScene::addLight(Light *light) {
  if (numLights < MAX_LIGHTS)
    this->lights[numLights++] = light;
}

bool LightScene::addInfo(int lightId, HueLightInfo info) {
  for (int i = 0 ; i < numLights ; ++i) {
    if (this->lights[i]->getId() == lightId) {
      this->infos[i] = info;
      return true;
    }
  }
  return false;
}

HueLightInfo LightScene::getInfo(int index) {
  if (index >= 0 && index < numLights) {
    return this->infos[index];
  }
  HueLightInfo def;
  return def;
}

Light *LightScene::getLight(int index) {
  if (index >= 0 && index < numLights)
    return this->lights[index];
  return nullptr;
}

int LightScene::getNumLights() {
  return this->numLights;
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
