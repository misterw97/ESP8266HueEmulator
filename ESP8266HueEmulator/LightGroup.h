#include "params.h"
#include "Light.h"
#include <aJSON.h>

/**
 * A LightGroup is a group of Lights, usually a Room.
 * A LightGroup can contains scenes that creates an ambiance with these lights.
 */
#ifndef LIGHTGROUP
#define LIGHTGROUP

// https://developers.meethue.com/documentation/groups-api
enum LightGroupType {
  LUMINAIRE, LIGHT_SOURCE, LIGHT_GROUP, ROOM
};
// when LightGroupType is ROOM, then LightGroupClass must be defined
enum LightGroupClass {
  LIVING_ROOM, KITCHEN, DINING, BEDROOM, KIDS_BEDROOM, BATHROOM, NURSERY, RECREATION, OFFICE, GYM, HALLWAY, TOILET, FRONT_DOOR, GARAGE, TERRACE, GARDEN, DRIVEWAY, CARPORT, OTHER
};

class aJsonObject;

class LightGroup {
  public:
    void begin(int id);
    bool active;
    
    aJsonObject *getJson();
    
    void setId(int id);
    void setName(char* name);
    void setType(LightGroupType type);
    void setType(char* type);
    void setClass(LightGroupClass cl);
    void setClass(char* cl);
    void addLight(Light *light);

    char* getName();
    char* getType();
    char* getClass();
    int getId();
    
    Light *getLight(int index);
    int getNumLights();
    
    void removeLights();
    
  private:
    int id;
    char name[40];
    char groupclass[15];
    char type[15];
    int numLights;
    Light *lights[MAX_LIGHTS];
};

#endif
