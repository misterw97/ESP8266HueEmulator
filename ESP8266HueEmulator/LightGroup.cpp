#include "Light.h"
#include "LightGroup.h"

#include <aJSON.h>

LightGroup::LightGroup(aJsonObject *root) {
  aJsonObject* jName = aJson.getObjectItem(root, "name");
  aJsonObject* jLights = aJson.getObjectItem(root, "lights");
  // jName and jLights guaranteed to exist
  name = jName->valuestring;
  for (int i = 0; i < aJson.getArraySize(jLights) && i < MAX_LIGHTS; i++) {
    aJsonObject* jLight = aJson.getArrayItem(jLights, i);
    lights[i] = atoi(jLight->valuestring);
  }
}

aJsonObject *LightGroup::getJson() {
  aJsonObject *object = aJson.createObject();
  aJson.addStringToObject(object, "name", name);
  aJsonObject *lightsArray = aJson.createArray();
  aJson.addItemToObject(object, "lights", lightsArray);
  for (int i = 0; i < MAX_LIGHTS; i++) {
    if ( lights[i] != 0 ) {
      String lightNum = "";
      lightNum += lights[i];
      aJson.addItemToArray(lightsArray, aJson.createItem(lightNum.c_str()));
    }
  }
  return object;
}



/*
aJsonObject *LightScene::getSceneJson() {
  aJsonObject *object = aJson.createObject();
  aJson.addStringToObject(object, "name", name);
  aJson.addStringToObject(object, "owner", "api");
  aJson.addStringToObject(object, "picture", "");
  aJson.addStringToObject(object, "lastupdated", "");
  aJson.addBooleanToObject(object, "recycle", false);
  aJson.addBooleanToObject(object, "locked", false);
  aJson.addNumberToObject(object, "version", 2);
  aJsonObject *lightsArray = aJson.createArray();
  aJson.addItemToObject(object, "lights", lightsArray);
  for (int i = 0; i < 16; i++) {
    if (!((1 << i) & lights)) {
      continue;
    }
    // add light to list
    String lightNum = "";
    lightNum += (i + 1);
    aJson.addItemToArray(lightsArray, aJson.createItem(lightNum.c_str()));
  }
  return object;
}
*/

