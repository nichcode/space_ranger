
#pragma once

#include "pal2/pal_graphics.h"
#include "helper.h"

struct Frame {
    PalBuffer* vertexBuffer;
    PalBuffer* uploadBuffer;
};

class Renderer {
public:
    bool initialize(
        PalDevice* device, 
        PalCommandBuffer* cmdBuffer, 
        PalQueue* queue);
        
    void shutdown();

private:
    PalDevice* m_Device;
    PalBuffer* m_IndexBuffer;
    Frame m_Frames[MAX_FRAMES_IN_FLIGHT];
};