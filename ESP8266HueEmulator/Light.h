#include "HueLightInfo.h"
#include "params.h"

/**
 * Light is an object that should be able to handle queries containing informations about lightning
 */
#ifndef LIGHT
#define LIGHT

// https://developers.meethue.com/documentation/supported-lights
enum LightType {
  EXTENDED_COLOR_LIGHT,
  DIMMABLE_LIGHT,
  COLOR_LIGHT
};

class Light {
  
  public:

    unsigned int id;// index of the light in array
    
    Light(char* n, LightType t);

    char* getTypeName();
    char* getTypeModelid();
    char* getName();
    HueLightInfo getInfo();

    /**
    * Do something when receiving informations for this Light
    * See HueLightInfo to know which information is available
    * For example : you could do something as " if (info.on) led.turnOn(); else led.turnOff(); "
    */
    void handleQuery(HueLightInfo info) {
      this->info = info;
      return;
    }
    
  private:
    char* name;
    LightType type;
    HueLightInfo info;
  
};

#endif
