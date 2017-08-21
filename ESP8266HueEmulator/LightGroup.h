#include "params.h"
#include "Light.h"
#include <aJSON.h>

/**
 * A LightGroup is a group of Lights, usually a Room.
 * A LightGroup can contains scenes that creates an ambiance with these lights.
 */
#ifndef LIGHTGROUP
#define LIGHTGROUP

class aJsonObject;

class LightGroup {
  public:
    void begin(int id);
    bool active = false;
      
    aJsonObject *getJson();

    void setId(int id);
    int getId();
    void addLight(Light *light);
    void removeLights();
    void setName(char* name);
    void setOwner(const char* owner);
    
  private:
    int id;
    char name[40];
    char owner[40];
    int numLights;
    Light *lights[MAX_LIGHTS];
    
    
};

#endif
