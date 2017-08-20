enum HueColorType {
  TYPE_HUE_SAT, TYPE_CT, TYPE_XY
};

enum HueAlert {
  ALERT_NONE, ALERT_SELECT, ALERT_LSELECT
};

enum HueEffect {
  EFFECT_NONE, EFFECT_COLORLOOP
};

struct HueLightInfo {
  bool on = false;
  int brightness = 0;
  HueColorType type = TYPE_HUE_SAT;
  int hue = 0, saturation = 0;
  HueAlert alert = ALERT_NONE;
  HueEffect effect = EFFECT_NONE;
  unsigned int transitionTime = 800; // by default there is a transition time to the new state of 400 milliseconds
};

class aJsonObject;
bool parseHueLightInfo(HueLightInfo currentInfo, aJsonObject *parsedRoot, HueLightInfo *newInfo);

class Light;
class ESP8266WebServer;
class LightServiceClass {
    public:
      int getLightsAvailable();
      bool addLight(Light *light);
      void begin();
      void begin(ESP8266WebServer *svr);
      void update();
    private:
      int currentNumLights;
};

extern LightServiceClass LightService;
