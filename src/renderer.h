
#pragma once

#include "pal2/pal_graphics.h"
#include "pal2/pal_video.h"

#define MAX_FRAMES_IN_FLIGHT 2

struct Vertex;

struct Frame {
    uint32_t vertexCount;
    uint32_t indexCount;
    uint32_t offset;
    Vertex* ptr;
    PalBuffer* vertexBuffer;
    PalBuffer* uploadBuffer;
    PalSemaphore* semaphore;
    PalCommandBuffer* cmdBuffer;
    PalFence* fence;
};

class Renderer {
public:
    void initialize(PalWindow* window); 
    void shutdown();

    void beginRendering(float r, float g, float b, float a);
    void endRendering();

private:
    PalShader* createShader(const char* path, PalShaderStage stage);
    void createDevice();
    void createSwapchain(PalWindow* window);
    void createPipeline();

    void createBuffers();
    void createSyncObjects();
    void createDescriptorObjects();

    void resetBatch();
    void flushBatch();

private:
    PalAdapter* m_Adapter = nullptr;
    PalDevice* m_Device = nullptr;
    PalQueue* m_Queue = nullptr;
    PalSurface* m_Surface = nullptr;
    PalSwapchain* m_Swapchain = nullptr;

    uint32_t m_ImageCount = 0;
    PalImageView** m_ImageViews = nullptr;
    PalSemaphore** m_RenderFinishedSemaphores = nullptr;
    PalFence** m_InFlightImages = nullptr;

    PalCommandPool* m_CmdPool = nullptr;
    PalDescriptorSetLayout* m_DescriptorSetLayout = nullptr;
    PalDescriptorPool* m_DescriptorPool = nullptr;
    PalDescriptorSet* m_DescriptorSet = nullptr;

    PalPipelineLayout* m_PipelineLayout = nullptr;
    PalPipeline* m_QuadPipeline = nullptr;
    PalBuffer* m_IndexBuffer = nullptr;
    PalBuffer* m_IndexStagingBuffer = nullptr;

    PalShaderFormats m_ShaderFormats = 0;
    uint32_t m_MaxGraphicsQueues = 0;
    PalFormat m_ImageFormat;
    uint32_t m_FrameIndex = 0;

    uint32_t m_ImageIndex = 0;
    Frame m_Frames[MAX_FRAMES_IN_FLIGHT];
    PalImageSubresourceRange m_ImageRange;
};