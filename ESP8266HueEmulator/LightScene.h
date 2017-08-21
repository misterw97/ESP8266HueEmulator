#include <aJSON.h>
#include "Light.h"
#include "params.h"

/**
 * A scene is a list of saved HueLightInfo for each Light
 */
#ifndef LIGHTSCENE
#define LIGHTSCENE

// TODO use polymorphism making LightScene a child of LightGroup
class LightScene {
  public:
    void begin(int id);
    bool active = false;
    
    aJsonObject *getJson();
    
    void setId(int id);
    void addLight(Light *light);
    void setName(char* name);
    void setOwner(const char* owner);
    
    int getId();
    Light* *getLights();
    
    void removeLights();
  private:
    int id;
    char name[40];
    char owner[40];
    int numLights;
    Light *lights[MAX_LIGHTS];
    
    
};

#endif
