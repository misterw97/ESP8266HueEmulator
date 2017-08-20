// Max number of exposed lights is directly related to aJSON PRINT_BUFFER_LEN, 14 for 4096
#define MAX_LIGHTS 14
#define COLOR_SATURATION 255.0f
#define BRIDGE_NAME "Reveil"

#include "LightService.h"

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

    /** Do something when receiving informations for this Light */
    virtual void handleQuery(HueLightInfo info) {}
    
  private:
    char* name;
    LightType type;
    HueLightInfo info;
  
};
