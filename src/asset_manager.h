
#pragma once

#include "pal2/pal_graphics.h"

class Renderer;
class AssetManager;

class Texture {
    friend class Renderer;
    friend class AssetManager;
    
private:
    PalImage* image;
    PalImageView* imageView;
};

class AssetManager {
public:
    static void initialize(PalDevice* device);
    static void shutdown();
    static Texture* loadTexture(const char* path);
    static void destroyTexture(Texture* texture);
};