#include "LightService.h"

#if PRINT_BUFFER_LEN < 2048
#  error aJson print buffer length PRINT_BUFFER_LEN must be increased to at least 4096
#endif

// useful network / parsing functions

#define MIN(a,b) ((a<b)?a:b)
#define MAX(a,b) ((a>b)?a:b)

String macString;
String bridgeIDString;
String ipString;
String netmaskString;
String gatewayString;
String devicename = BRIDGE_NAME;
String username = "PaPEg4df!CXTnRT$";
String userdevicetype = "clientname#devicename";
String utc = "1997-07-27T03:00:00";

static const char* _ssdp_response_template =
  "HTTP/1.1 200 OK\r\n"
  "EXT:\r\n"
  "CACHE-CONTROL: max-age=%u\r\n" // SSDP_INTERVAL
  "LOCATION: http://%s:%u/%s\r\n" // WiFi.localIP(), _port, _schemaURL
  "SERVER: Arduino/1.0 UPNP/1.1 %s/%s\r\n" // _modelName, _modelNumber
  "hue-bridgeid: %s\r\n"
  "ST: %s\r\n"  // _deviceType
  "USN: uuid:%s\r\n" // _uuid
  "\r\n";

static const char* _ssdp_notify_template =
  "NOTIFY * HTTP/1.1\r\n"
  "HOST: 239.255.255.250:1900\r\n"
  "NTS: ssdp:alive\r\n"
  "CACHE-CONTROL: max-age=%u\r\n" // SSDP_INTERVAL
  "LOCATION: http://%s:%u/%s\r\n" // WiFi.localIP(), _port, _schemaURL
  "SERVER: Arduino/1.0 UPNP/1.1 %s/%s\r\n" // _modelName, _modelNumber
  "hue-bridgeid: %s\r\n"
  "NT: %s\r\n"  // _deviceType
  "USN: uuid:%s\r\n" // _uuid
  "\r\n";

ESP8266WebServer *HTTP;

// things about colors

struct rgbcolor {
  rgbcolor(uint8_t r, uint8_t g, uint8_t b) : r(r), g(g), b(b) {};
  uint8_t r;
  uint8_t g;
  uint8_t b;
};

struct hsvcolor {
  hsvcolor(const rgbcolor& color) {
    float r = ((float)color.r)/COLOR_SATURATION;
    float g = ((float)color.g)/COLOR_SATURATION;
    float b = ((float)color.b)/COLOR_SATURATION;
    float mi = MIN(MIN(r,g),b);
    float ma = MAX(MAX(r,g),b);
    float diff = ma - mi;
    v = ma;
    h = 0;
    s = (!v)?0:(diff/ma);
    if (diff) {
      if (r == v) {
            h = (g - b) / diff + (g < b ? 6.0f : 0.0f);
        } else if (g == v) {
            h = (b - r) / diff + 2.0f;
        } else {
            h = (r - g) / diff + 4.0f;
        }
        h /= 6.0f;
    }
  };
  float h;
  float s;
  float v;
};

// ==============================================================================================================
// Color Conversion
// ==============================================================================================================
// TODO: Consider switching to something along the lines of
// https://github.com/patdie421/mea-edomus/blob/master/src/philipshue_color.c
// and/ or https://github.com/kayno/arduinolifx/blob/master/color.h
// for color coversions instead
// ==============================================================================================================

// Based on http://stackoverflow.com/questions/22564187/rgb-to-philips-hue-hsb
// The code is based on this brilliant note: https://github.com/PhilipsHue/PhilipsHueSDK-iOS-OSX/commit/f41091cf671e13fe8c32fcced12604cd31cceaf3

rgbcolor getXYtoRGB(float x, float y, int brightness_raw) {
  float brightness = ((float)brightness_raw) / 255.0f;
  float bright_y = brightness / y;
  float X = x * bright_y;
  float Z = (1 - x - y) * bright_y;

  // convert to RGB (0.0-1.0) color space
  float R = X * 1.4628067 - brightness * 0.1840623 - Z * 0.2743606;
  float G = -X * 0.5217933 + brightness * 1.4472381 + Z * 0.0677227;
  float B = X * 0.0349342 - brightness * 0.0968930 + Z * 1.2884099;

  // apply inverse 2.2 gamma
  float inv_gamma = 1.0 / 2.4;
  float linear_delta = 0.055;
  float linear_interval = 1 + linear_delta;
  float r = R <= 0.0031308 ? 12.92 * R : (linear_interval) * pow(R, inv_gamma) - linear_delta;
  float g = G <= 0.0031308 ? 12.92 * G : (linear_interval) * pow(G, inv_gamma) - linear_delta;
  float b = B <= 0.0031308 ? 12.92 * B : (linear_interval) * pow(B, inv_gamma) - linear_delta;

  return rgbcolor(r * COLOR_SATURATION,
                  g * COLOR_SATURATION,
                  b * COLOR_SATURATION);
}

int getHue(hsvcolor hsb) {
  return hsb.h * 360 * 182.04;
}

int getSaturation(hsvcolor hsb) {
  return hsb.s * COLOR_SATURATION;
}

rgbcolor getMirektoRGB(int mirek) {
  int hectemp = 10000 / mirek;
  int r, g, b;
  if (hectemp <= 66) {
    r = COLOR_SATURATION;
    g = 99.4708025861 * log(hectemp) - 161.1195681661;
    b = hectemp <= 19 ? 0 : (138.5177312231 * log(hectemp - 10) - 305.0447927307);
  } else {
    r = 329.698727446 * pow(hectemp - 60, -0.1332047592);
    g = 288.1221695283 * pow(hectemp - 60, -0.0755148492);
    b = COLOR_SATURATION;
  }
  r = r > COLOR_SATURATION ? COLOR_SATURATION : r;
  g = g > COLOR_SATURATION ? COLOR_SATURATION : g;
  b = b > COLOR_SATURATION ? COLOR_SATURATION : b;
  return rgbcolor(r, g, b);
}

// end of colorfull functions

String removeSlashes(String uri) {
  if (uri[0] == '/') {
    uri = uri.substring(1);
  }
  if (uri.length() && uri[uri.length() - 1] == '/') {
    uri = uri.substring(0, uri.length() - 1);
  }
  return uri;
}

String getPathSegment(String uri) {
  // assume slashes removed
  int slash = uri.indexOf("/");
  if (slash == -1) {
    return uri;
  }
  return uri.substring(0, slash);
}

String removePathSegment(String uri) {
  // assume slashes removed
  int slash = uri.indexOf("/");
  if (slash == -1) {
    return "";
  }
  return uri.substring(slash);
}

String getWildCard(String requestUri, String wcUri, int n, char wildcard = '*') {
  wcUri = removeSlashes(wcUri);
  requestUri = removeSlashes(requestUri);
  String wildcardStr;
  wildcardStr += wildcard;
  int i = 0;
  while (1) {
    String uPath = getPathSegment(wcUri);
    String ruPath = getPathSegment(requestUri);
    if (uPath == wildcardStr) {
      if (i == n) {
        return ruPath;
      }
      i++;
    }
    wcUri = removeSlashes(removePathSegment(wcUri));
    requestUri = removeSlashes(removePathSegment(requestUri));
    if (!wcUri.length() && !requestUri.length()) {
      return "";
    }
    if (!wcUri.length() || !requestUri.length()) {
      return "";
    }
  }
  return "";
}

String StringIPaddress(IPAddress myaddr) {
  String LocalIP = "";
  for (int i = 0; i < 4; i++)
  {
    LocalIP += String(myaddr[i]);
    if (i < 3) LocalIP += ".";
  }
  return LocalIP;
}

int ssdpMsgFormatCallback(SSDPClass *ssdp, char *buffer, int buff_len,
                          bool isNotify, int interval, char *modelName,
                          char *modelNumber, char *uuid, char *deviceType,
                          uint32_t ip, uint16_t port, char *schemaURL) {
  if (isNotify) {
    return snprintf(buffer, buff_len,
      _ssdp_notify_template,
      interval,
      ipString.c_str(), port, schemaURL,
      modelName, modelNumber,
      bridgeIDString.c_str(),
      deviceType,
      uuid);
  } else {
    return snprintf(buffer, buff_len,
      _ssdp_response_template,
      interval,
      ipString.c_str(), port, schemaURL,
      modelName, modelNumber,
      bridgeIDString.c_str(),
      deviceType,
      uuid);
  }
}

class WcFnRequestHandler;

typedef std::function<void(WcFnRequestHandler *handler, String requestUri, HTTPMethod method)> HandlerFunction;

class WcFnRequestHandler : public RequestHandler {
public:
    WcFnRequestHandler(HandlerFunction fn, const String &uri, HTTPMethod method, char wildcard = '*')
    : _fn(fn)
    , _uri(uri)
    , _method(method)
    , _wildcard(wildcard)
    {
      assert(_wildcard != '/');
      // verify that the URI is reasonable (only contains wildcard at the beginning/end/whole path segments
      for(int i = 0; i < _uri.length(); i++) {
        if (_uri[i] == _wildcard) {
          if (i != 0 && i != _uri.length() - 1 && (_uri[i-1] != '/' || _uri[i+1] != '/')) {
            assert(false);
          }
        }
      }
    }

    bool canHandle(HTTPMethod requestMethod, String requestUri) override  {
        if (_method != HTTP_ANY && _method != requestMethod) {
          return false;
        }

        String uri = removeSlashes(_uri);
        requestUri = removeSlashes(requestUri);
        String wildcardStr;
        wildcardStr += _wildcard;
        while (1) {
          String uPath = getPathSegment(uri);
          String ruPath = getPathSegment(requestUri);
          if (uPath != ruPath && uPath != wildcardStr) {
            return false;
          }
          uri = removeSlashes(removePathSegment(uri));
          requestUri = removeSlashes(removePathSegment(requestUri));
          if (!uri.length() && !requestUri.length()) {
            return true;
          }
          if (!uri.length() || !requestUri.length()) {
            return false;
          }
        }

        return true;
    }

    bool canUpload(String requestUri) override  {
        return false;
    }

    bool handle(ESP8266WebServer& server, HTTPMethod requestMethod, String requestUri) override {
        currentReqUri = requestUri;
        _fn(this, requestUri, requestMethod);
        currentReqUri = "";
        return true;
    }

    void upload(ESP8266WebServer& server, String requestUri, HTTPUpload& upload) override {}

    String getWildCard(int wcIndex) {
      return ::getWildCard(currentReqUri, _uri, wcIndex);
    }
protected:
    String currentReqUri;
    HandlerFunction _fn;
    String _uri;
    HTTPMethod _method;
    char _wildcard;
};

void on(HandlerFunction fn, const String &wcUri, HTTPMethod method, char wildcard = '*') {
  HTTP->addHandler(new WcFnRequestHandler(fn, wcUri, method, wildcard));
}

aJsonObject *wrapWithSuccess(aJsonObject *body) {
  aJsonObject *success = aJson.createObject();
  aJson.addItemToObject(success, "success", body);
  return success;
}

// targetBase is assumed to have a trailing slash (/)
aJsonObject *generateTargetPutResponse(aJsonObject *body, String targetBase) {
  aJsonObject *root = aJson.createArray();
  for (int i = 0; i < aJson.getArraySize(body); i++) {
    aJsonObject *success = aJson.createObject();
    aJson.addItemToArray(root, wrapWithSuccess(success));
    aJsonObject *entry = aJson.getArrayItem(body, i);
    String target = targetBase + entry->name;
    switch (entry->type) {
      case aJson_Boolean:
        aJson.addBooleanToObject(success, target.c_str(), entry->valuebool);
        break;
      case aJson_Int:
        aJson.addNumberToObject(success, target.c_str(), entry->valueint);
        break;
      case aJson_String:
        aJson.addStringToObject(success, target.c_str(), entry->valuestring);
        break;
      case aJson_Float:
        aJson.addNumberToObject(success, target.c_str(), entry->valuefloat);
        break;
      case aJson_Array: {
        aJsonObject *xy = aJson.createArray();
        aJson.addItemToObject(success, target.c_str(), xy);
        for (int j = 0; j < aJson.getArraySize(entry); j++) {
          aJson.addItemToArray(xy, aJson.createItem(aJson.getArrayItem(entry, j)->valuefloat));
        }
        break;
      }
      default:
        break;
    }
  }
  return root;
}

void sendJson(aJsonObject *root) {
  // Take aJsonObject and print it to Serial and to WiFi
  // From https://github.com/pubnub/msp430f5529/blob/master/msp430f5529.ino
  char *msgStr = aJson.print(root);
  aJson.deleteItem(root);
  Serial.println(millis());
  Serial.println(msgStr);
  HTTP->send(200, "text/plain", msgStr);
  free(msgStr);
}

void sendError(int type, String path, String description) {
  aJsonObject *root = aJson.createArray();
  aJsonObject *errorContainer = aJson.createObject();
  aJsonObject *errorObject = aJson.createObject();
  aJson.addItemToObject(errorObject, "type", aJson.createItem(type));
  aJson.addStringToObject(errorObject, "address", path.c_str());
  aJson.addStringToObject(errorObject, "description", description.c_str());
  aJson.addItemToObject(errorContainer, "error", errorObject);
  aJson.addItemToArray(root, errorContainer);
  sendJson(root);
}

void sendSuccess(String id, String value) {
  aJsonObject *search = aJson.createArray();
  aJsonObject *container = aJson.createObject();
  aJson.addItemToArray(search, container);
  aJsonObject *succeed = aJson.createObject();
  aJson.addItemToObject(container, "success", succeed);
  aJson.addStringToObject(succeed, id.c_str(), value.c_str());
  sendJson(search);
}

void sendSuccess(String value) {
  aJsonObject *search = aJson.createArray();
  aJsonObject *container = aJson.createObject();
  aJson.addItemToArray(search, container);
  aJsonObject *succeed = aJson.createObject();
  aJson.addStringToObject(container, "success", value.c_str());
  sendJson(search);
}

void sendUpdated() {
  Serial.println("Updated.");
  HTTP->send(200, "text/plain", "Updated.");
}

String methodToString(int method) {
  switch (method) {
    case HTTP_POST: return "POST";
    case HTTP_GET: return "GET";
    case HTTP_PUT: return "PUT";
    case HTTP_PATCH: return "PATCH";
    case HTTP_DELETE: return "DELETE";
    case HTTP_OPTIONS: return "OPTIONS";
    default: return "unknown";
  }
}

// end of useful network methods


// begin of LightServiceClass

// export an instance "static" of LightServiceClass
LightServiceClass LightService;

void LightServiceClass::begin() {
  begin(new ESP8266WebServer(80));
}

void LightServiceClass::begin(ESP8266WebServer *svr) {
  HTTP = svr;
  macString = WiFi.macAddress();
  bridgeIDString = macString;
  bridgeIDString.replace(":", "");
  bridgeIDString = bridgeIDString.substring(0, 6) + "FFFE" + bridgeIDString.substring(6);
  ipString = StringIPaddress(WiFi.localIP());
  netmaskString = StringIPaddress(WiFi.subnetMask());
  gatewayString = StringIPaddress(WiFi.gatewayIP());

  Serial.print("Starting HTTP at ");
  Serial.print(WiFi.localIP());
  Serial.print(":");
  Serial.println(80);

  HTTP->on("/description.xml", HTTP_GET, descriptionFn);
  on(configFn, "/api/*/config", HTTP_ANY);
  on(configFn, "/api/config", HTTP_GET);
  on(wholeConfigFn, "/api/*", HTTP_GET);
  on(wholeConfigFn, "/api/", HTTP_GET);
  on(authFn, "/api", HTTP_POST);
  on(unimpFn, "/api/*/schedules", HTTP_GET);
  on(unimpFn, "/api/*/rules", HTTP_GET);
  on(unimpFn, "/api/*/sensors", HTTP_GET);
  on(scenesFn, "/api/*/scenes", HTTP_ANY);
  on(scenesIdFn, "/api/*/scenes/*", HTTP_ANY);
  //on(scenesIdLightFn, "/api/*/scenes/*/lightstates/*", HTTP_ANY);
  //on(scenesIdLightFn, "/api/*/scenes/*/lights/*/state", HTTP_ANY);
  //on(groupsFn, "/api/*/groups", HTTP_ANY);
  //on(groupsIdFn, "/api/*/groups/*", HTTP_ANY);
  //on(groupsIdActionFn, "/api/*/groups/*/action", HTTP_ANY);
  on(lightsFn, "/api/*/lights", HTTP_ANY);
  on(unimpFn, "/api/*/lights/new", HTTP_ANY);
  on(lightsIdFn, "/api/*/lights/*", HTTP_ANY);
  on(lightsIdStateFn, "/api/*/lights/*/state", HTTP_ANY);

  HTTP->begin();

  Serial.println("Starting SSDP...");
  SSDP.begin();
  SSDP.setSchemaURL((char*)"description.xml");
  SSDP.setHTTPPort(80);
  SSDP.setName((char*)BRIDGE_NAME);
  SSDP.setSerialNumber(macString.c_str());
  SSDP.setURL((char*)"index.html");
  SSDP.setModelName((char*)"IpBridge");
  SSDP.setModelNumber((char*)"0.1");
  SSDP.setModelURL((char*)"http://www.meethue.com");
  SSDP.setManufacturer((char*)"Royal Philips Electronics");
  SSDP.setManufacturerURL((char*)"http://www.philips.com");
  SSDP.setDeviceType((char*)"upnp:rootdevice");
  SSDP.setMessageFormatCallback(ssdpMsgFormatCallback);
  Serial.println("SSDP Started");
}

void LightServiceClass::update() {
  HTTP->handleClient();
}


// Fn Methods (functions)
// Methods called by HTTP requests

/**
 * Route : /description.xml
 */
void descriptionFn() {
  String str = "<root><specVersion><major>1</major><minor>0</minor></specVersion><URLBase>http://" + ipString + ":80/</URLBase><device><deviceType>urn:schemas-upnp-org:device:Basic:1</deviceType><friendlyName>" + BRIDGE_NAME + " (" + ipString + ")</friendlyName><manufacturer>Royal Philips Electronics</manufacturer><manufacturerURL>http://www.philips.com</manufacturerURL><modelDescription>Philips hue Personal Wireless Lighting</modelDescription><modelName>Philips hue bridge 2012</modelName><modelNumber>929000226503</modelNumber><modelURL>http://www.meethue.com</modelURL><serialNumber>"+macString+"</serialNumber><UDN>uuid:2f402f80-da50-11e1-9b23-"+macString+"</UDN><presentationURL>index.html</presentationURL><iconList><icon><mimetype>image/png</mimetype><height>48</height><width>48</width><depth>24</depth><url>hue_logo_0.png</url></icon><icon><mimetype>image/png</mimetype><height>120</height><width>120</width><depth>24</depth><url>hue_logo_3.png</url></icon></iconList></device></root>";
  HTTP->send(200, "text/plain", str);
}

void unimpFn(WcFnRequestHandler *handler, String requestUri, HTTPMethod method) {
  String str = "{}";
  HTTP->send(200, "text/plain", str);
}

/**
 * Routes 
 *  /api/<username>/config
 *  /api/config
 */
void configFn(WcFnRequestHandler *handler, String requestUri, HTTPMethod method) {
  switch (method) {
    case HTTP_GET: {
      Serial.println("GET config ");
      aJsonObject *root;
      root = aJson.createObject();
      addConfigJson(root);
      sendJson(root);
      break;
    }
    case HTTP_PUT: {
      Serial.print("PUT config ");
      Serial.println(HTTP->arg("plain"));
      aJsonObject* body = aJson.parse(( char*) HTTP->arg("plain").c_str());
      if (body) {
        
        aJsonObject* jName = aJson.getObjectItem(body, "name");
        if (jName)
          devicename = jName->valuestring;
          
        aJsonObject* jUTC = aJson.getObjectItem(body, "UTC");
        if (jUTC)
          utc = jUTC->valuestring;

        sendJson(generateTargetPutResponse(body, "/config/"));
        aJson.deleteItem(body);
      } else {
        sendError(2, "config", "Bad JSON body : empty");
      }
      break;
    }
    default:
      sendError(4, requestUri, "Config method not supported");
      break;
  }
}

/**
 * Route /api Method POST
 * It should test if button has been pressed for security reasons, but we are not going to check now... TODO ?
 */
void authFn(WcFnRequestHandler *handler, String requestUri, HTTPMethod method) {
  // TODO : pourquoi ça fait planter quand envoi POST (même vide) ?
  Serial.print("POST auth ");
  Serial.println(HTTP->arg("plain"));
  aJsonObject* body = aJson.parse(( char*) HTTP->arg("plain").c_str());
  if (body) {
    aJsonObject* devicetype = aJson.getObjectItem(body, "devicetype");
    aJsonObject* _username = aJson.getObjectItem(body, "username");
    if (devicetype) {
      userdevicetype = devicetype->valuestring;
      if (_username) {
        username = _username->valuestring;
      } else {
        aJson.addStringToObject(body, "username", username.c_str());
      }
      sendSuccess("username", username);
    } else {
      sendError(5, requestUri, "devicetype required !");
    }
    aJson.deleteItem(body);
  } else {
    sendError(2, "api", "Bad JSON body : empty");
  }
}

/**
 * Route /api/<username>
 */
void wholeConfigFn(WcFnRequestHandler *handler, String requestUri, HTTPMethod method) {
  aJsonObject *root;
  root = aJson.createObject();
  // lights
  aJson.addItemToObject(root, "lights", getLightsJson());
  // groups
  // TODO aJson.addItemToObject(root, "groups", getGroupsJson());
  // scenes
  aJson.addItemToObject(root, "scenes", getScenesJson());
  // configuration
  aJsonObject *config;
  aJson.addItemToObject(root, "config", config = aJson.createObject());
  addConfigJson(config);
  // TODO add support for schedules
  aJsonObject *schedules;
  aJson.addItemToObject(root, "schedules", schedules = aJson.createObject());
  // TODO add support for sensors
  aJsonObject *sensors;
  aJson.addItemToObject(root, "sensors", sensors = aJson.createObject());
  // TODO add support for rules
  aJsonObject *rules;
  aJson.addItemToObject(root, "rules", rules = aJson.createObject());
  sendJson(root);
}

/**
 * Route /api/<username>/scenes
 */
void scenesFn(WcFnRequestHandler *handler, String requestUri, HTTPMethod method) {
  switch (method) {
    case HTTP_GET:
      sendJson(getScenesJson());
      break;
    case HTTP_POST:
    case HTTP_PUT:
      putScene(-1);
      break;
    default:
      sendError(4, requestUri, "Scene method not supported");
      break;
  }
}

/**
 * Route /api/<username>/scenes/<id>
 */
void scenesIdFn(WcFnRequestHandler *handler, String requestUri, HTTPMethod method) {
  String sceneStringId = handler->getWildCard(1);
  int sceneId = -1;
  if (sceneStringId.length() > 0) {
    sceneId = atoi(sceneStringId.c_str());
  }
  switch (method) {
    case HTTP_GET:
      if (LightService.getScene(sceneId)) {
        sendJson(LightService.getScene(sceneId)->getJson());
      } else {
        sendError(3, "/scenes/"+sceneId, "Cannot retrieve scene that does not exist");
      }
      break;
    case HTTP_PUT:
      putScene(sceneId);
      sendUpdated();
      break;
    case HTTP_DELETE:
      if (LightService.getScene(sceneId)) {
        LightService.getScene(sceneId)->active = false;
      } else {
        sendError(3, requestUri, "Cannot delete scene that does not exist");
      }
      sendSuccess(requestUri+" deleted");
      break;
    default:
      sendError(4, requestUri, "Scene method not supported");
      break;
  }
}

/**
 * Route /api/<username>/lights
 */
void lightsFn(WcFnRequestHandler *handler, String requestUri, HTTPMethod method) {
  switch (method) {
    case HTTP_GET: {
      sendJson(getLightsJson());
      break;
    }
    case HTTP_POST:
      // "start" a "search" for "new" lights
      sendSuccess("/lights", "Searching for new devices");
      break;
    default:
      sendError(4, requestUri, "Light method not supported");
      break;
  }
}

/**
 * Route /api/<username>/lights/<id>
 */
void lightsIdFn(WcFnRequestHandler *whandler, String requestUri, HTTPMethod method) {
  int lightId = atoi(whandler->getWildCard(1).c_str());
  switch (method) {
    case HTTP_GET: {
      if (LightService.getLight(lightId))
        sendJson(LightService.getLight(lightId)->getJson());
      else
        sendError(3, "/lights/"+lightId, "Cannot retrieve light that does not exist");
      break;
    }
    case HTTP_PUT:
      // TODO update light ? 
      sendUpdated();
      break;
    default:
      sendError(4, requestUri, "Light method not supported");
      break;
  }
}

/**
 * Route /api/<username>/lights/<id>/state
 */
void lightsIdStateFn(WcFnRequestHandler *whandler, String requestUri, HTTPMethod method) {
  int lightId = atoi(whandler->getWildCard(1).c_str());
 
  if (!LightService.getLight(lightId)) {
    sendError(3, requestUri, "Requested light not available");
    return;
  }

  switch (method) {
    case HTTP_POST:
    case HTTP_PUT: {
      Serial.print("PUT or POST lightstate ");
      Serial.println(HTTP->arg("plain"));
      
      aJsonObject* parsedRoot = aJson.parse(( char*) HTTP->arg("plain").c_str());
      if (!parsedRoot) {
        // unparseable json
        sendError(2, requestUri, "Bad JSON body in request");
        return;
      }
      HueLightInfo currentInfo = LightService.getLight(lightId)->getInfo();
      HueLightInfo newInfo;
      if (!parseHueLightInfo(currentInfo, parsedRoot, &newInfo)) {
        aJson.deleteItem(parsedRoot);
        return;
      }

      LightService.getLight(lightId)->handleQuery(newInfo);
      
      sendJson(generateTargetPutResponse(parsedRoot, "/lights/" + whandler->getWildCard(1) + "/state/"));
      aJson.deleteItem(parsedRoot);
      break;
    }
    default:
      sendError(4, requestUri, "Light method not supported");
      break;
  }
  
}



// data methods / interacts with structures

LightScene *LightServiceClass::getScene(int id) {
  if (id < MAX_SCENES && id >= 0)
    if (this->scenes_[id].active)
      return &this->scenes_[id];
  return nullptr;
}

LightScene *LightServiceClass::getScene() {
  for (int id = 0 ; id < MAX_SCENES ; ++id)
    if (!this->scenes_[id].active) {
      this->scenes_[id].begin(id);
      return &this->scenes_[id];      
    }
  return nullptr;
}

int LightServiceClass::getLightsAvailable() {
  return this->currentNumLights;
}

bool LightServiceClass::addLight(Light *light) {
  if (currentNumLights < MAX_LIGHTS) {
    light->setId(currentNumLights+1);
    this->lights_[currentNumLights] = light;
    ++currentNumLights;
    return true;
  }
  return false;
}

Light *LightServiceClass::getLight(int i) {
  if (i <= currentNumLights && i > 0) {
    return this->lights_[i-1];
  }
  return nullptr;
}

String LightServiceClass::getUtc() {
  return utc;
}



bool parseHueLightInfo(HueLightInfo currentInfo, aJsonObject *parsedRoot, HueLightInfo *newInfo) {
  *newInfo = currentInfo;
  aJsonObject* onState = aJson.getObjectItem(parsedRoot, "on");
  if (onState) {
    newInfo->on = onState->valuebool;
  }

  // pull brightness
  aJsonObject* briState = aJson.getObjectItem(parsedRoot, "bri");
  if (briState) {
    newInfo->brightness = briState->valueint;
  }

  // pull effect
  aJsonObject* effectState = aJson.getObjectItem(parsedRoot, "effect");
  if (effectState) {
    const char *effect = effectState->valuestring;
    if (!strcmp(effect, "colorloop")) {
      newInfo->effect = EFFECT_COLORLOOP;
    } else {
      newInfo->effect = EFFECT_NONE;
    }
  }
  
  // pull alert
  aJsonObject* alertState = aJson.getObjectItem(parsedRoot, "alert");
  if (alertState) {
    const char *alert = alertState->valuestring;
    if (!strcmp(alert, "select")) {
      newInfo->alert = ALERT_SELECT;
    } else if (!strcmp(alert, "lselect")) {
      newInfo->alert = ALERT_LSELECT;
    } else {
      newInfo->alert = ALERT_NONE;
    }
  }

  // pull transitiontime
  aJsonObject* transitiontimeState = aJson.getObjectItem(parsedRoot, "transitiontime");
  if (transitiontimeState) {
    newInfo->transitionTime = transitiontimeState->valueint;
  }

  aJsonObject* hueState = aJson.getObjectItem(parsedRoot, "hue");
  aJsonObject* satState = aJson.getObjectItem(parsedRoot, "sat");
  aJsonObject* ctState = aJson.getObjectItem(parsedRoot, "ct");
  aJsonObject* xyState = aJson.getObjectItem(parsedRoot, "xy");
  if (xyState) {
    aJsonObject* elem0 = aJson.getArrayItem(xyState, 0);
    aJsonObject* elem1 = aJson.getArrayItem(xyState, 1);
    if (!elem0 || !elem1) {
      sendError(5, "/api/api/lights/?/state", "xy color coordinates incomplete");
      return false;
    }
    hsvcolor hsb = getXYtoRGB(elem0->valuefloat, elem1->valuefloat, newInfo->brightness);
    newInfo->hue = getHue(hsb);
    newInfo->saturation = getSaturation(hsb);
  } else if (ctState) {
    int mirek = ctState->valueint;
    if (mirek > 500 || mirek < 153) {
      sendError(7, "/api/api/lights/?/state", "Invalid vaule for color temperature");
      return false;
    }

    hsvcolor hsb = getMirektoRGB(mirek);
    newInfo->hue = getHue(hsb);
    newInfo->saturation = getSaturation(hsb);
  } else if (hueState || satState) {
    if (hueState) newInfo->hue = hueState->valueint;
    if (satState) newInfo->saturation = satState->valueint;
  }
  return true;
}

void addConfigJson(aJsonObject *root) {
  aJson.addStringToObject(root, "name", devicename.c_str());
  aJson.addStringToObject(root, "swversion", "81012917");
  aJson.addStringToObject(root, "bridgeid", bridgeIDString.c_str());
  // TODO add support for portalservices
  aJson.addBooleanToObject(root, "portalservices", false);
  aJson.addBooleanToObject(root, "linkbutton", true);
  aJson.addStringToObject(root, "mac", macString.c_str());
  aJson.addBooleanToObject(root, "dhcp", true);
  aJson.addStringToObject(root, "ipaddress", ipString.c_str());
  aJson.addStringToObject(root, "netmask", netmaskString.c_str());
  aJson.addStringToObject(root, "gateway", gatewayString.c_str());
  aJson.addStringToObject(root, "apiversion", "1.3.0");
  aJson.addStringToObject(root, "timezone", "Europe/London");
  aJsonObject *whitelist;
  aJson.addItemToObject(root, "whitelist", whitelist = aJson.createObject());
  aJsonObject *whitelistFirstEntry;
  aJson.addItemToObject(whitelist, username.c_str(), whitelistFirstEntry = aJson.createObject());
  aJson.addStringToObject(whitelistFirstEntry, "name", userdevicetype.c_str());
  aJsonObject *swupdate;
  aJson.addItemToObject(root, "swupdate", swupdate = aJson.createObject());
  aJson.addStringToObject(swupdate, "text", "");
  aJson.addBooleanToObject(swupdate, "notify", false);// Otherwise client app shows update notice
  aJson.addNumberToObject(swupdate, "updatestate", 0);
  aJson.addStringToObject(swupdate, "url", "");
}

aJsonObject *getScenesJson() {
  aJsonObject *root = aJson.createObject();
  String sceneId = "";
  for (int i = 0; i < MAX_SCENES; ++i)
    if(LightService.getScene(i)) {
      sceneId = "";
      sceneId += LightService.getScene(i)->getId();
      aJson.addItemToObject(root, sceneId.c_str(), LightService.getScene(i)->getJson());
    }
  return root;
}

aJsonObject *getLightsJson() {
  aJsonObject *root = aJson.createObject();
  String lightId = "";
  for (int i = 1; i <= LightService.getLightsAvailable(); ++i)
    if(LightService.getLight(i)) {
      lightId = "";
      lightId += LightService.getLight(i)->getId();
      aJson.addItemToObject(root, lightId.c_str(), LightService.getLight(i)->getJson());
    }
  return root;
}

void putScene(int id) {
  // Save HTTP data
  String body = HTTP->arg("plain");

  Serial.print("PUT scene ");
  Serial.println(body);
  
  if (body != "") {
    aJsonObject* root = aJson.parse(( char*) body.c_str());
    if (root) {
      
      aJsonObject* jName = aJson.getObjectItem(root, "name");
      aJsonObject* jLights = aJson.getObjectItem(root, "lights");
      
      // if we are creating new Scene, we need name and lights, else not
      if ( (id >= 0 ) || (jName && jLights) ) {
        
        LightScene *scene;
        if (id < 0) {
          // retrieve a "new" LightScene
          scene = LightService.getScene();
        } else {
          scene = LightService.getScene(id);
        }
        
        if (scene) {
          // update Name
          if (jName)
            scene->setName(jName->valuestring);
          // update Owner with now registered user
          scene->setOwner(username.c_str());
          // update lights
          if (jLights) {
            scene->removeLights();
            for (int i = 0; i < aJson.getArraySize(jLights); ++i) {
              // retrieve the light id
              aJsonObject* jLight = aJson.getArrayItem(jLights, i);
              int lightId = atoi(jLight->valuestring);
              // if it designs a registered light in the bridge
              if (LightService.getLight(lightId))
                // then add the light to the LightScene
                scene->addLight(LightService.getLight(lightId));
            }
          }

          String newId = "";
          newId += scene->getId();
          sendSuccess("id", newId);
          
        }
        else {
          sendError(301, "scenes", "Scenes table full");
        }
        
      } else {
        sendError(5, "scenes", "name and lights are required for a scene");
      }

      aJson.deleteItem(root);
    } else {
      sendError(2, "scenes", "Bad JSON body");
    }
  } else {
    sendError(2, "scenes", "Bad JSON body : empty");
  }
}
