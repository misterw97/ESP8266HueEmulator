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
bool parseHueLightInfo(HueLightInfo currentInfo, aJsonObject *parsedRoot, HueLightInfo *newInfo);

class Light;
class ESP8266WebServer;
class WcFnRequestHandler;
class LightServiceClass {
    public:

      /**
       * @return number of available lights
       */
      int getLightsAvailable();
      /**
       * @param *light pointer for the light to add to the bridge
       * @return true if the light has been added to the bridge
       */
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

      /**
       * Send XML description of the Bridge
       */
      void descriptionFn();
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
       * PUT a scene description for one light
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
       * PUT description for an action of a group
       */
      void groupsIdActionFn(WcFnRequestHandler *handler, String requestUri, HTTPMethod method);
      /**
       * GET existing lights
       * POST for asking the bridge to search for new lights
       */
      void lightsFn(WcFnRequestHandler *handler, String requestUri, HTTPMethod method);
      /**
       * GET information on one light
       * PUT information on a light
       */
      void lightsIdFn(WcFnRequestHandler *whandler, String requestUri, HTTPMethod method);
      /**
       * POST information on a light
       */
      void lightsIdStateFn(WcFnRequestHandler *whandler, String requestUri, HTTPMethod method);
      /**
       * Creates a new light
       */
      void lightsNewFn(WcFnRequestHandler *handler, String requestUri, HTTPMethod method);
      
      
    private:
      int currentNumLights;
      Light *lights_[MAX_LIGHTS] = {nullptr, };
      LightGroup *groups_[MAX_GROUPS] = {nullptr, };
      LightScene *scenes_[MAX_SCENES] = {nullptr, };
};

extern LightServiceClass LightService;

#endif
