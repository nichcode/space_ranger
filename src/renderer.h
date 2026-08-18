
#pragma once

#include "pal2/pal_graphics.h"
#include "pal2/pal_video.h"
#include "camera.h"

#define MAX_FRAMES_IN_FLIGHT 2

struct Vertex;
struct Texture;
class AssetManager;

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
    friend class AssetManager;
public:
    bool initialize(
        PalWindow* window, 
        PalAdapter* adapter, 
        PalDevice* device, 
        uint32_t windowWidth, 
        uint32_t windowHeight);
        
    void shutdown();

    void beginRendering(Camera* camera, const glm::vec4& clearColor);
    void drawQuad(const glm::vec2& position, const glm::vec2& size, Texture* texture);
    void endRendering();
    void resize(uint32_t width, uint32_t height);

private:
    PalShader* createShader(const char* path, PalShaderStage stage, PalShaderFormats format);
        
    bool createSwapchain(PalWindow* window);
    void resetBatch();
    void flushBatch();

    void nextBatch();
    uint32_t getTextureIndex(Texture* texture);

private:
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

    uint32_t m_FrameIndex = 0;
    PalImageSubresourceRange m_ImageRange;
    uint32_t m_ImageIndex = 0;
    glm::mat4 m_Projection;

    PalSampler* m_Sampler;
    PalViewport m_Viewport;
    Frame m_Frames[MAX_FRAMES_IN_FLIGHT];
    PalRect2D m_Scissor;
};