
#pragma once

#include "PAL_defines.h"

/// @brief Shader
struct PAL_Shader;

struct PAL_Context;

/// @brief Shader data types
enum PAL_ShaderDataTypes_
{
    PAL_ShaderDataTypes_Int,
    PAL_ShaderDataTypes_Int2,
    PAL_ShaderDataTypes_Int3,
    PAL_ShaderDataTypes_Int4,
    PAL_ShaderDataTypes_Float,
    PAL_ShaderDataTypes_Float2,
    PAL_ShaderDataTypes_Float3,
    PAL_ShaderDataTypes_Float4
};

/// @brief Shader source types
enum PAL_ShaderSourceTypes_
{
    PAL_ShaderSourceTypes_GLSL,
};

/// @brief Shader attribute
struct PAL_ShaderAttrib
{
    /// @brief The divisor of the attribute. 0 for default;
    u32 divisor = 0;

    /// @brief Normalize the attribue;
    b8 normalize = false;

    /// @brief The type of the attribute.
    u32 type = 0;
};

/// @brief Shader layout
struct PAL_ShaderLayout
{
    /// @brief attributes
    PAL_ShaderAttrib attribs[PAL_MAX_ATTRIB]{};
    
    /// @brief attributes count
    u32 count = 0;
};

/// @brief Shader descriptor
struct PAL_ShaderDesc
{
    /// @brief Shader shource type.
    u32 type = 0;

    /// @brief Shader vertex source
    const char* vertex_src = nullptr;

    /// @brief Shader pixel source
    const char* pixel_src = nullptr;

    /// @brief Load shader sources from disk
    b8 load = true;

    /// @brief Shader layout
    PAL_ShaderLayout layout;
};

/**
 * @brief Create a shader. 
 * @attention A Context must be bound.
 * @param desc The shader descriptor.
 * @returns If successful, a pointer to the created shader otherwise nullptr.
 */
PAL_API PAL_Shader* PAL_CreateShader(PAL_ShaderDesc desc);

/**
 * @brief Destroy the shader.
 * @attention A Context must be bound.
 * @param shader The shader
 */
PAL_API void PAL_DestroyShader(PAL_Shader* shader);

/**
 * @brief Bind the shader to the pipeline.
 * @param shader The shader
 */
PAL_API void PAL_BindShader(PAL_Shader* shader);

/**
 * @brief Set int data to the active shader.
 * @param name The layout name
 * @param data The data
 */
PAL_API void PAL_SetInt(const char* name, i32 data);

/**
 * @brief Set int array data to the active shader.
 * @param name The layout name
 * @param data The data array
 * @param count The count of elements in the array
 */
PAL_API void PAL_SetIntArray(const char* name, i32* data, u32 count);

/**
 * @brief Set float data to the active shader.
 * @param name The layout name
 * @param data The data
 */
PAL_API void PAL_SetFloat(const char* name, f32 data);

/**
 * @brief Set float2 data to the active shader.
 * @param name The layout name
 * @param data The data
 * @param data2 The data2
 */
PAL_API void PAL_SetFloat2(const char* name, f32 data, f32 data2);

/**
 * @brief Set float3 data to the active shader.
 * @param name The layout name
 * @param data The data
 * @param data2 The data2
 * @param data3 The data3
 */
PAL_API void PAL_SetFloat3(const char* name, f32 data, f32 data2, f32 data3);

/**
 * @brief Set float4 data to the active shader.
 * @param name The layout name
 * @param data The data
 * @param data2 The data2
 * @param data3 The data3
 * @param data4 The data4
 */
PAL_API void PAL_SetFloat4(const char* name, f32 data, f32 data2, f32 data3, f32 data4);

/**
 * @brief Set mat4 data to the active shader.
 * @param name The layout name
 * @param data The data
 */
PAL_API void PAL_SetMat4(const char* name, f32* data);
