class aJsonObject;

class LightGroup {
  public:
    LightGroup(aJsonObject *root);
    
    aJsonObject *getJson();
    aJsonObject *getSceneJson();

    unsigned int lights[MAX_LIGHTS];
    
  private:
    char* name;
     
};

class LightScene {
  
};

