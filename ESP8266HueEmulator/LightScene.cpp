#include "LightScene.h"

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
