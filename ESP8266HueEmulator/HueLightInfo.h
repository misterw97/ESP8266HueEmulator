#include <aJSON.h>

/**
 * HueLightInfo represents all the information that could be available on a (hue) light
 */
#ifndef HUELIGHTINFO
#define HUELIGHTINFO

enum HueColorType {
  TYPE_HUE_SAT, TYPE_CT, TYPE_XY
};

enum HueAlert {
  ALERT_NONE, ALERT_SELECT, ALERT_LSELECT
};

enum HueEffect {
  EFFECT_NONE, EFFECT_COLORLOOP
};

class HueLightInfo {
  public:
    aJsonObject *getJson();
  
    bool on = false;
    int brightness = 0;
    HueColorType type = TYPE_HUE_SAT;
    int hue = 0, saturation = 0;
    HueAlert alert = ALERT_NONE;
    HueEffect effect = EFFECT_NONE;
    unsigned int transitionTime = 800; // by default there is a transition time to the new state of 400 milliseconds
};

#endif
