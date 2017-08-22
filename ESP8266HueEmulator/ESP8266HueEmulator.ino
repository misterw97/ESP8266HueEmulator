/**
 * Emulate Philips Hue Bridge
 **/
#include "Light.h"
#include "LightGroup.h"
#include "LightService.h"

#include "SSDP.h"

#include <aJSON.h> // Replace avm/pgmspace.h with pgmspace.h there and set #define PRINT_BUFFER_LEN 4096 ################# IMPORTANT


// Include ssid and password
#include "secrets.h"
//const char* ssid = "********";
//const char* password = "********";


class SerialLight : public Light {
  public :
  
    SerialLight(char* n, LightType t) : Light(n,t) {}
    
    void turn(boolean on) {
      if (on)
        Serial.println("Light turned on");
      else
        Serial.println("Light turned off");
    }
    
    void handleQuery(HueLightInfo info) {
      // NEEDED !
      this->info = info;
      turn(info.on);
    }
    
};

SerialLight myLight("Test myLight",DIMMABLE_LIGHT);

void setup() {
  Serial.begin(9600);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  LightService.begin();
  LightService.addLight(&myLight);
  LightGroup *group = LightService.getGroup();
  group->setName("Groupe init");
  group->setType(ROOM);
  group->setClass(OFFICE);
  group->addLight(&myLight);
}

void loop() {
  
  LightService.update();

  yield();
  
}
