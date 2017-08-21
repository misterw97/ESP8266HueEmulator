#include "HueLightInfo.h"
#include "params.h"
#include <aJSON.h>

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
    Light(char* n, LightType t);
    
    void setId(int id);
    int getId();
    
    char* getTypeName();
    char* getTypeModelid();
    char* getName();

    // TODO : virtual ?
    HueLightInfo getInfo();
    /**
    * Do something when receiving informations for this Light
    * See HueLightInfo to know which information is available
    * For example : you could do something as " if (info.on) led.turnOn(); else led.turnOff(); "
    * REDEFINE THIS METHOD IN DAUGHTER CLASS
    */
    virtual void handleQuery(HueLightInfo info) {};

    aJsonObject *getStateJson();
    aJsonObject *getJson();
    
  private:
    int id;
    char name[25];
    LightType type;
    HueLightInfo info;
};

#endif
