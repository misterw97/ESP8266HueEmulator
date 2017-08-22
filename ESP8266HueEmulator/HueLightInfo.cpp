#include "HueLightInfo.h"

aJsonObject *HueLightInfo::getJson(LightType type) {
  aJsonObject *object = aJson.createObject();
  aJson.addBooleanToObject(object, "on", this->on);
  aJson.addNumberToObject(object, "bri", this->brightness);
  if (type == COLOR_LIGHT || type == EXTENDED_COLOR_LIGHT) {
    aJson.addNumberToObject(object, "hue", this->hue);
    aJson.addNumberToObject(object, "sat", this->saturation);
    aJson.addStringToObject(object, "effect", this->effect == EFFECT_COLORLOOP ? "colorloop" : "none");
    // TODO the current color mode 
    aJson.addStringToObject(object, "colormode", "hs");
  }
  if (type == EXTENDED_COLOR_LIGHT) {
    aJsonObject *xyArray = aJson.createArray();
    // TODO actually store this (xy mode: CIE 1931 color co-ordinates)
    double numbers[2] = {0.0, 0.0};
    aJson.addItemToObject(object, "xy", aJson.createFloatArray(numbers, 2));
    // TODO actually store this (ct mode: color temp (expressed in mireds range 154-500))
    aJson.addNumberToObject(object, "ct", 500);
  }
  aJson.addStringToObject(object, "alert", this->alert == ALERT_SELECT ? "select" : ( this->alert == ALERT_LSELECT ? "lselect" : "none" ));
  // lamp can be seen by the hub
  aJson.addBooleanToObject(object, "reachable", true);
  return object;  
}
