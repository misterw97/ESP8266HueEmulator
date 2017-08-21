#include "LightGroup.h"

void LightGroup::begin(int id) {
  strcpy(this->name,"");
  this->id = id;
  this->removeLights();
  this->setType(LIGHT_GROUP);
  this->setClass(OTHER);
  
  this->active = true;
}

void LightGroup::removeLights() {
  for (int i = 0 ; i < MAX_LIGHTS; ++i)
    lights[i] = nullptr;
  numLights = 0;
}

aJsonObject *LightGroup::getJson() {
  aJsonObject *object = aJson.createObject();
  // A unique, editable name given to the group.
  aJson.addStringToObject(object, "name", this->name);
  // Can be "LightGroup", "Room" or either "Luminaire" or "LightSource" if a Multisource Luminaire is present in the system.
  aJson.addStringToObject(object, "type", this->type);
  // Category of Room types. Default is: Other.
  aJson.addStringToObject(object, "class", this->groupclass);
  if (this->numLights > 0) {
    aJsonObject *lightsArray = aJson.createArray();
    // The IDs of the lights that are in the group.
    aJson.addItemToObject(object, "lights", lightsArray);
    char lightId[5];
    for (int i = 0; i < numLights; ++i) {
      if (this->lights[i]) {
        sprintf(lightId, "%d", this->lights[i]->getId());
        aJson.addItemToArray(lightsArray, aJson.createItem(lightId));
      }
    }
    // The light state of one of the lamps in the group.
    aJson.addItemToObject(object, "action", this->lights[0]->getStateJson());
  }
  return object;
}

void LightGroup::addLight(Light *light) {
  if (numLights < MAX_LIGHTS)
    this->lights[numLights++] = light;
}

Light *LightGroup::getLight(int index) {
  if (index >= 0 && index < numLights)
    return this->lights[index];
  return nullptr;
}

int LightGroup::getNumLights() {
  return this->numLights;
}

void LightGroup::setId(int id) {
  this->id = id;
}

int LightGroup::getId() {
  return this->id;
}

void LightGroup::setName(char* name) {
  strcpy(this->name,name);
}

char* LightGroup::getName() {
  return this->name;
}

void LightGroup::setType(LightGroupType type) {
  switch(type) {
    case LUMINAIRE:
      strcpy(this->type,"Luminaire");
      break;
    case LIGHT_SOURCE:
      strcpy(this->type,"Lightsource");
      break;
    case ROOM:
      strcpy(this->type,"Room");
      break;
    case LIGHT_GROUP:
    default:
      strcpy(this->type,"LightGroup");
      break;
  }
}

void LightGroup::setType(char* type) {
  strcpy(this->type,type);
}

char* LightGroup::getType() {
  return this->type;
}

void LightGroup::setClass(char* cl) {
  strcpy(this->groupclass,cl);
}

void LightGroup::setClass(LightGroupClass cl) {
  switch (cl) {
    case LIVING_ROOM:
      strcpy(this->groupclass, "Living room");
      break;
    case KITCHEN:
      strcpy(this->groupclass, "Kitchen");
      break;
    case DINING:
      strcpy(this->groupclass, "Dining");
      break;
    case BEDROOM:
      strcpy(this->groupclass, "Bedroom");
      break;
    case KIDS_BEDROOM:
      strcpy(this->groupclass, "Kids bedroom");
      break;
    case BATHROOM:
      strcpy(this->groupclass, "Bathroom");
      break;
    case NURSERY:
      strcpy(this->groupclass, "Nursery");
      break;
    case RECREATION:
      strcpy(this->groupclass, "Recreation");
      break;
    case OFFICE:
      strcpy(this->groupclass, "Office");
      break;
    case GYM:
      strcpy(this->groupclass, "Gym");
      break;
    case HALLWAY:
      strcpy(this->groupclass, "Hallway");
      break;
    case TOILET:
      strcpy(this->groupclass, "Toilet");
      break;
    case FRONT_DOOR:
      strcpy(this->groupclass, "Front door");
      break;
    case GARAGE:
      strcpy(this->groupclass, "Garage");
      break;
    case TERRACE:
      strcpy(this->groupclass, "Terrace");
      break;
    case GARDEN:
      strcpy(this->groupclass, "Garden");
      break;
    case DRIVEWAY:
      strcpy(this->groupclass, "Driveway");
      break;
    case CARPORT:
      strcpy(this->groupclass, "Carport");
      break;
    default:
      strcpy(this->groupclass, "Other");
  }
}

char* LightGroup::getClass() {
  return this->groupclass;
}

