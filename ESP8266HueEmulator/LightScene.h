#include <aJSON.h>
#include "Light.h"
#include "params.h"
#include "HueLightInfo.h"

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
    aJsonObject *getJson(bool full);

    void addLight(Light *light);
    bool addInfo(int lightId, HueLightInfo info);
    
    void setId(int id);
    void setName(char* name);
    void setOwner(const char* owner);
    
    int getId();
    Light *getLight(int index);
    int getNumLights();
    HueLightInfo getInfo(int index);
    
    void removeLights();
  private:
    int id;
    char name[40];
    char owner[40];
    int numLights;
    Light *lights[MAX_LIGHTS];
    HueLightInfo infos[MAX_LIGHTS];
    
};

#endif
