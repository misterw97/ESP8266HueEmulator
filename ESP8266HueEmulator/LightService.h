#include "Light.h"
#include "LightGroup.h"
#include "LightScene.h"
#include "HueLightInfo.h"
#include "params.h"

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiUdp.h>
#include "SSDP.h"
#include <aJSON.h> // Replace avm/pgmspace.h with pgmspace.h there and set #define PRINT_BUFFER_LEN 4096 ################# IMPORTANT
#include <assert.h>

/**
 * The LightService is the interaction between network and Lights
 */
#ifndef LIGHTSERVICE
#define LIGHTSERVICE

class aJsonObject;
class Light;
class ESP8266WebServer;
class WcFnRequestHandler;

bool parseHueLightInfo(HueLightInfo currentInfo, aJsonObject *parsedRoot, HueLightInfo *newInfo);
bool applyConfigToLight(Light *light, HueLightInfo config);
bool applyConfigToLight(Light *light, aJsonObject *parsedRoot);

aJsonObject *getGroupsJson();
aJsonObject *getScenesJson();
aJsonObject *getLightsJson();
void addConfigJson(aJsonObject *config);

void putScene(int id);
void putGroup(int id);


// Fn Methods
/**
 * Send XML description of the Bridge
 */
void descriptionFn();
/**
* Empty response, for things that are not available
*/
void unimpFn(WcFnRequestHandler *handler, String requestUri, HTTPMethod method);
/**
* Send short description of the bridge
*/
void configFn(WcFnRequestHandler *handler, String requestUri, HTTPMethod method);
/**
* Confirm success of authentification (simulates a recent press on the button on the bridge)
*/
void authFn(WcFnRequestHandler *handler, String requestUri, HTTPMethod method);
/**
* Respond with complete json as in https://github.com/probonopd/ESP8266HueEmulator/wiki/Hue-API#get-all-information-about-the-bridge
*/
void wholeConfigFn(WcFnRequestHandler *handler, String requestUri, HTTPMethod method);
/**
* List or create scenes
*/
void scenesFn(WcFnRequestHandler *handler, String requestUri, HTTPMethod method);
/**
* GET, PUT or DELETE a scene
*/
void scenesIdFn(WcFnRequestHandler *handler, String requestUri, HTTPMethod method);
/**
* PUT a scene description for one light = lightstate
*/
void scenesIdLightFn(WcFnRequestHandler *handler, String requestUri, HTTPMethod method);
/**
* GET or POST group
*/
void groupsFn(WcFnRequestHandler *handler, String requestUri, HTTPMethod method);
/**
* GET, PUT or DELETE a group
*/
void groupsIdFn(WcFnRequestHandler *handler, String requestUri, HTTPMethod method);
/**
* Execute an action of a group or a scene (TODO : https://developers.meethue.com/documentation/groups-api#253_body_example)
*/
void groupsIdActionFn(WcFnRequestHandler *handler, String requestUri, HTTPMethod method);
/**
* GET existing lights
* POST for asking the bridge to search for new lights
*/
void lightsFn(WcFnRequestHandler *handler, String requestUri, HTTPMethod method);
/**
* GET information on one light
* PUT information on a light ( TODO )
*/
void lightsIdFn(WcFnRequestHandler *whandler, String requestUri, HTTPMethod method);
/**
* PUT information on light state
*/
void lightsIdStateFn(WcFnRequestHandler *whandler, String requestUri, HTTPMethod method);

class LightServiceClass {
    public:

      int getLightsAvailable();

      Light *getLight(int id);
      LightGroup *getGroup(int id);
      LightScene *getScene(int id);

      LightGroup *getGroup();
      LightScene *getScene();
      bool addLight(Light *light);
      
      /**
       * Start the server and listen 
       */
      void begin();
      void begin(ESP8266WebServer *svr);
      /**
       * Update state of things...
       */
      void update();
      
      String getUtc();

    private:
      int currentNumLights;
      Light *lights_[MAX_LIGHTS] = {nullptr, };
      LightGroup groups_[MAX_GROUPS];
      LightScene scenes_[MAX_SCENES];
};

extern LightServiceClass LightService;

#endif
