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
    LightGroup(aJsonObject *root);

    /**
     * @return Json representation of the group
     */
    aJsonObject *getJson();
    aJsonObject *getSceneJson();

    unsigned int lights[MAX_LIGHTS];
    
  private:
    char* name;
    Light *lights_[MAX_LIGHTS];     
};

#endif
