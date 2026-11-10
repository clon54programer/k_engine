#include "editor/gfx/quad_renderer.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include "editor_shaders.h"

namespace k_engine::editor::gfx
{

namespace
{

constexpr std::size_t MAX_VERTICES = 262144;
constexpr std::size_t FLOATS_PER_VERTEX = 8;
constexpr std::uint32_t MAX_DESCRIPTOR_SETS = 1024;
constexpr std::uint32_t MAX_TEXTURE_DIMENSION = 4096;

struct QuadVertex
{
    float x = 0.0F;
    float y = 0.0F;
    float u = 0.0F;
    float v = 0.0F;
    float r = 1.0F;
    float g = 1.0F;
    float b = 1.0F;
    float a = 1.0F;
};

struct QuadPushConstants
{
    float viewport_width = 0.0F;
    float viewport_height = 0.0F;
};

static_assert(sizeof(QuadVertex) == FLOATS_PER_VERTEX * sizeof(float),
              "El vértice del quad debe ser interleaved sin padding");

bool CheckVk(VkResult result, const char *operation)
{
    if (result != VK_SUCCESS)
    {
        std::cerr << "Error de Vulkan en " << operation << " (VkResult "
                  << static_cast<int>(result) << ").\n";
        return false;
    }
    return true;
}

} // namespace

QuadRenderer::QuadRenderer() = default;

QuadRenderer::~QuadRenderer()
{
    Shutdown();
}

bool QuadRenderer::Initialize(VulkanContext &context, VkRenderPass render_pass,
                              VkExtent2D extent)
{
    Shutdown();
    _context = &context;
    _physical_device = context.PhysicalDevice();
    _device = context.Device();
    _command_pool = context.CommandPool();
    _queue = context.GraphicsQueue();

    if (!create_descriptor_set_layout() || !create_pipeline_layout() ||
        !create_sampler() || !create_descriptor_pool() || !create_vertex_buffer())
    {
        Shutdown();
        return false;
    }

    const std::array<std::uint8_t, 4> WHITE = {255, 255, 255, 255};
    Texture white;
    std::string error;
    if (!create_texture_internal(WHITE.data(), 1, 1, white, error))
    {
        std::cerr << "Error: " << error << '\n';
        Shutdown();
        return false;
    }
    _textures.push_back(white);

    if (!create_pipeline(render_pass, extent))
    {
        Shutdown();
        return false;
    }
    return true;
}

bool QuadRenderer::RecreatePipeline(VkRenderPass render_pass, VkExtent2D extent)
{
    if (_device == VK_NULL_HANDLE)
    {
        return false;
    }
    if (_pipeline != VK_NULL_HANDLE)
    {
        vkDestroyPipeline(_device, _pipeline, nullptr);
        _pipeline = VK_NULL_HANDLE;
    }
    return create_pipeline(render_pass, extent);
}

void QuadRenderer::Shutdown() noexcept
{
    if (_device != VK_NULL_HANDLE)
    {
        (void)vkDeviceWaitIdle(_device);

        if (_pipeline != VK_NULL_HANDLE)
        {
            vkDestroyPipeline(_device, _pipeline, nullptr);
            _pipeline = VK_NULL_HANDLE;
        }

        for (Texture &texture : _textures)
        {
            destroy_texture(texture);
        }
        _textures.clear();

        if (_descriptor_pool != VK_NULL_HANDLE)
        {
            vkDestroyDescriptorPool(_device, _descriptor_pool, nullptr);
            _descriptor_pool = VK_NULL_HANDLE;
        }
        if (_sampler != VK_NULL_HANDLE)
        {
            vkDestroySampler(_device, _sampler, nullptr);
            _sampler = VK_NULL_HANDLE;
        }

        if (_vertex_mapped != nullptr)
        {
            vkUnmapMemory(_device, _vertex_memory);
            _vertex_mapped = nullptr;
        }
        if (_vertex_buffer != VK_NULL_HANDLE)
        {
            vkDestroyBuffer(_device, _vertex_buffer, nullptr);
            _vertex_buffer = VK_NULL_HANDLE;
        }
        if (_vertex_memory != VK_NULL_HANDLE)
        {
            vkFreeMemory(_device, _vertex_memory, nullptr);
            _vertex_memory = VK_NULL_HANDLE;
        }

        if (_pipeline_layout != VK_NULL_HANDLE)
        {
            vkDestroyPipelineLayout(_device, _pipeline_layout, nullptr);
            _pipeline_layout = VK_NULL_HANDLE;
        }
        if (_descriptor_set_layout != VK_NULL_HANDLE)
        {
            vkDestroyDescriptorSetLayout(_device, _descriptor_set_layout, nullptr);
            _descriptor_set_layout = VK_NULL_HANDLE;
        }
    }

    _context = nullptr;
    _physical_device = VK_NULL_HANDLE;
    _device = VK_NULL_HANDLE;
    _command_pool = VK_NULL_HANDLE;
    _queue = VK_NULL_HANDLE;
    _vertex_capacity = 0;
    _extent = VkExtent2D{};
}

bool QuadRenderer::Draw(VkCommandBuffer command_buffer, const ui::DrawList &draw_list)
{
    if (_pipeline == VK_NULL_HANDLE || _vertex_mapped == nullptr ||
        command_buffer == VK_NULL_HANDLE)
    {
        return false;
    }
    if (draw_list.vertices.empty() || draw_list.commands.empty())
    {
        return true;
    }
    if (draw_list.vertices.size() > MAX_VERTICES)
    {
        std::cerr << "Error: la lista de dibujo supera " << MAX_VERTICES
                  << " vértices.\n";
        return false;
    }

    std::memcpy(_vertex_mapped, draw_list.vertices.data(),
                draw_list.vertices.size() * sizeof(ui::DrawVertex));

    const VkViewport VIEWPORT{
        0.0F, 0.0F, static_cast<float>(_extent.width), static_cast<float>(_extent.height),
        0.0F, 1.0F};
    const VkRect2D SCISSOR{{0, 0}, _extent};
    vkCmdSetViewport(command_buffer, 0, 1, &VIEWPORT);
    vkCmdSetScissor(command_buffer, 0, 1, &SCISSOR);

    const VkDeviceSize OFFSET = 0;
    vkCmdBindVertexBuffers(command_buffer, 0, 1, &_vertex_buffer, &OFFSET);
    vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline);

    const QuadPushConstants PUSH{static_cast<float>(_extent.width),
                                 static_cast<float>(_extent.height)};
    vkCmdPushConstants(command_buffer, _pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT, 0,
                       sizeof(PUSH), &PUSH);

    for (const ui::DrawCommand &command : draw_list.commands)
    {
        if (command.vertex_count == 0)
        {
            continue;
        }

        const float CLIP_LEFT = std::max(0.0F, std::floor(command.clip.x));
        const float CLIP_TOP = std::max(0.0F, std::floor(command.clip.y));
        const float CLIP_RIGHT = std::min(static_cast<float>(_extent.width),
                                          std::ceil(command.clip.x + command.clip.width));
        const float CLIP_BOTTOM =
            std::min(static_cast<float>(_extent.height),
                     std::ceil(command.clip.y + command.clip.height));
        if (CLIP_RIGHT <= CLIP_LEFT || CLIP_BOTTOM <= CLIP_TOP)
        {
            continue;
        }

        const VkRect2D COMMAND_SCISSOR{
            {static_cast<std::int32_t>(CLIP_LEFT), static_cast<std::int32_t>(CLIP_TOP)},
            {static_cast<std::uint32_t>(CLIP_RIGHT - CLIP_LEFT),
             static_cast<std::uint32_t>(CLIP_BOTTOM - CLIP_TOP)}};
        vkCmdSetScissor(command_buffer, 0, 1, &COMMAND_SCISSOR);

        VkDescriptorSet descriptor = _textures.front().descriptor;
        if (command.texture.value < _textures.size() &&
            _textures[command.texture.value].view != VK_NULL_HANDLE)
        {
            descriptor = _textures[command.texture.value].descriptor;
        }
        vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                _pipeline_layout, 0, 1, &descriptor, 0, nullptr);
        vkCmdDraw(command_buffer, command.vertex_count, 1, command.first_vertex, 0);
    }
    return true;
}

bool QuadRenderer::CreateTexture(const std::uint8_t *rgba, std::uint32_t width,
                                 std::uint32_t height, ui::TextureId &texture,
                                 std::string &error)
{
    if (rgba == nullptr || width == 0 || height == 0 || width > MAX_TEXTURE_DIMENSION ||
        height > MAX_TEXTURE_DIMENSION)
    {
        error = "La textura tiene dimensiones inválidas.";
        return false;
    }

    Texture created;
    if (!create_texture_internal(rgba, width, height, created, error))
    {
        return false;
    }
    _textures.push_back(created);
    texture.value = static_cast<std::uint32_t>(_textures.size() - 1);
    return true;
}

void QuadRenderer::DestroyTexture(ui::TextureId texture) noexcept
{
    if (_device == VK_NULL_HANDLE || texture.value == 0 ||
        texture.value >= _textures.size())
    {
        return;
    }
    destroy_texture(_textures[texture.value]);
}

ui::TextureId QuadRenderer::WhiteTexture() noexcept
{
    return ui::TextureId{0};
}

bool QuadRenderer::create_descriptor_set_layout()
{
    VkDescriptorSetLayoutBinding binding{};
    binding.binding = 0;
    binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    binding.descriptorCount = 1;
    binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    create_info.bindingCount = 1;
    create_info.pBindings = &binding;
    return CheckVk(vkCreateDescriptorSetLayout(_device, &create_info, nullptr,
                                               &_descriptor_set_layout),
                   "vkCreateDescriptorSetLayout");
}

bool QuadRenderer::create_pipeline_layout()
{
    VkPushConstantRange push_range{};
    push_range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    push_range.offset = 0;
    push_range.size = static_cast<std::uint32_t>(sizeof(QuadPushConstants));

    VkPipelineLayoutCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    create_info.setLayoutCount = 1;
    create_info.pSetLayouts = &_descriptor_set_layout;
    create_info.pushConstantRangeCount = 1;
    create_info.pPushConstantRanges = &push_range;
    return CheckVk(
        vkCreatePipelineLayout(_device, &create_info, nullptr, &_pipeline_layout),
        "vkCreatePipelineLayout");
}

bool QuadRenderer::create_sampler()
{
    VkSamplerCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    create_info.magFilter = VK_FILTER_NEAREST;
    create_info.minFilter = VK_FILTER_NEAREST;
    create_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
    create_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    create_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    create_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    create_info.anisotropyEnable = VK_FALSE;
    create_info.maxAnisotropy = 1.0F;
    create_info.compareEnable = VK_FALSE;
    create_info.compareOp = VK_COMPARE_OP_ALWAYS;
    create_info.borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
    create_info.unnormalizedCoordinates = VK_FALSE;
    return CheckVk(vkCreateSampler(_device, &create_info, nullptr, &_sampler),
                   "vkCreateSampler");
}

bool QuadRenderer::create_descriptor_pool()
{
    VkDescriptorPoolSize pool_size{};
    pool_size.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    pool_size.descriptorCount = MAX_DESCRIPTOR_SETS;

    VkDescriptorPoolCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    create_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    create_info.poolSizeCount = 1;
    create_info.pPoolSizes = &pool_size;
    create_info.maxSets = MAX_DESCRIPTOR_SETS;
    return CheckVk(
        vkCreateDescriptorPool(_device, &create_info, nullptr, &_descriptor_pool),
        "vkCreateDescriptorPool");
}

bool QuadRenderer::create_vertex_buffer()
{
    _vertex_capacity = static_cast<VkDeviceSize>(MAX_VERTICES * sizeof(QuadVertex));
    if (!create_buffer(_vertex_capacity, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                       VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                           VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                       _vertex_buffer, _vertex_memory))
    {
        return false;
    }
    return CheckVk(
        vkMapMemory(_device, _vertex_memory, 0, _vertex_capacity, 0, &_vertex_mapped),
        "vkMapMemory");
}

bool QuadRenderer::create_pipeline(VkRenderPass render_pass, VkExtent2D extent)
{
    if (_device == VK_NULL_HANDLE || render_pass == VK_NULL_HANDLE ||
        _pipeline_layout == VK_NULL_HANDLE)
    {
        return false;
    }

    VkShaderModule vertex_module = VK_NULL_HANDLE;
    VkShaderModule fragment_module = VK_NULL_HANDLE;
    if (!create_shader_module(shaders::QUAD_VERT.data(),
                              shaders::QUAD_VERT.size() * sizeof(std::uint32_t),
                              vertex_module) ||
        !create_shader_module(shaders::QUAD_FRAG.data(),
                              shaders::QUAD_FRAG.size() * sizeof(std::uint32_t),
                              fragment_module))
    {
        if (vertex_module != VK_NULL_HANDLE)
        {
            vkDestroyShaderModule(_device, vertex_module, nullptr);
        }
        return false;
    }

    VkPipelineShaderStageCreateInfo vertex_stage{};
    vertex_stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertex_stage.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertex_stage.module = vertex_module;
    vertex_stage.pName = "main";

    VkPipelineShaderStageCreateInfo fragment_stage{};
    fragment_stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragment_stage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragment_stage.module = fragment_module;
    fragment_stage.pName = "main";

    const std::array<VkPipelineShaderStageCreateInfo, 2> STAGES = {vertex_stage,
                                                                   fragment_stage};

    VkVertexInputBindingDescription binding{};
    binding.binding = 0;
    binding.stride = static_cast<std::uint32_t>(sizeof(QuadVertex));
    binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    const std::array<VkVertexInputAttributeDescription, 3> ATTRIBUTES = {
        VkVertexInputAttributeDescription{0, 0, VK_FORMAT_R32G32_SFLOAT, 0},
        VkVertexInputAttributeDescription{1, 0, VK_FORMAT_R32G32_SFLOAT,
                                          static_cast<std::uint32_t>(2 * sizeof(float))},
        VkVertexInputAttributeDescription{2, 0, VK_FORMAT_R32G32B32A32_SFLOAT,
                                          static_cast<std::uint32_t>(4 * sizeof(float))},
    };

    VkPipelineVertexInputStateCreateInfo vertex_input{};
    vertex_input.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_input.vertexBindingDescriptionCount = 1;
    vertex_input.pVertexBindingDescriptions = &binding;
    vertex_input.vertexAttributeDescriptionCount =
        static_cast<std::uint32_t>(ATTRIBUTES.size());
    vertex_input.pVertexAttributeDescriptions = ATTRIBUTES.data();

    VkPipelineInputAssemblyStateCreateInfo input_assembly{};
    input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    input_assembly.primitiveRestartEnable = VK_FALSE;

    VkPipelineViewportStateCreateInfo viewport_state{};
    viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewport_state.viewportCount = 1;
    viewport_state.scissorCount = 1;

    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.cullMode = VK_CULL_MODE_NONE;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.lineWidth = 1.0F;

    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineColorBlendAttachmentState color_attachment{};
    color_attachment.blendEnable = VK_TRUE;
    color_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    color_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    color_attachment.colorBlendOp = VK_BLEND_OP_ADD;
    color_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    color_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    color_attachment.alphaBlendOp = VK_BLEND_OP_ADD;
    color_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
                                      VK_COLOR_COMPONENT_G_BIT |
                                      VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

    VkPipelineColorBlendStateCreateInfo color_blending{};
    color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    color_blending.logicOpEnable = VK_FALSE;
    color_blending.attachmentCount = 1;
    color_blending.pAttachments = &color_attachment;

    constexpr std::array<VkDynamicState, 2> DYNAMIC_STATES = {VK_DYNAMIC_STATE_VIEWPORT,
                                                              VK_DYNAMIC_STATE_SCISSOR};
    VkPipelineDynamicStateCreateInfo dynamic_state{};
    dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamic_state.dynamicStateCount = static_cast<std::uint32_t>(DYNAMIC_STATES.size());
    dynamic_state.pDynamicStates = DYNAMIC_STATES.data();

    VkGraphicsPipelineCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    create_info.stageCount = static_cast<std::uint32_t>(STAGES.size());
    create_info.pStages = STAGES.data();
    create_info.pVertexInputState = &vertex_input;
    create_info.pInputAssemblyState = &input_assembly;
    create_info.pViewportState = &viewport_state;
    create_info.pRasterizationState = &rasterizer;
    create_info.pMultisampleState = &multisampling;
    create_info.pColorBlendState = &color_blending;
    create_info.pDynamicState = &dynamic_state;
    create_info.layout = _pipeline_layout;
    create_info.renderPass = render_pass;
    create_info.subpass = 0;

    const VkResult RESULT = vkCreateGraphicsPipelines(_device, VK_NULL_HANDLE, 1,
                                                      &create_info, nullptr, &_pipeline);
    vkDestroyShaderModule(_device, fragment_module, nullptr);
    vkDestroyShaderModule(_device, vertex_module, nullptr);
    if (!CheckVk(RESULT, "vkCreateGraphicsPipelines"))
    {
        _pipeline = VK_NULL_HANDLE;
        return false;
    }
    _extent = extent;
    return true;
}

bool QuadRenderer::create_shader_module(const std::uint32_t *code, std::size_t size,
                                        VkShaderModule &shader_module) const
{
    if (code == nullptr || size == 0 || (size % sizeof(std::uint32_t)) != 0)
    {
        return false;
    }
    VkShaderModuleCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    create_info.codeSize = size;
    create_info.pCode = code;
    return CheckVk(vkCreateShaderModule(_device, &create_info, nullptr, &shader_module),
                   "vkCreateShaderModule");
}

bool QuadRenderer::create_texture_internal(const std::uint8_t *rgba, std::uint32_t width,
                                           std::uint32_t height, Texture &texture,
                                           std::string &error)
{
    const VkDeviceSize IMAGE_BYTES = static_cast<VkDeviceSize>(width) * height * 4ULL;

    VkImageCreateInfo image_info{};
    image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_info.imageType = VK_IMAGE_TYPE_2D;
    image_info.extent.width = width;
    image_info.extent.height = height;
    image_info.extent.depth = 1;
    image_info.mipLevels = 1;
    image_info.arrayLayers = 1;
    image_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_info.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    image_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (!CheckVk(vkCreateImage(_device, &image_info, nullptr, &texture.image),
                 "vkCreateImage"))
    {
        error = "No se pudo crear la imagen de la textura.";
        return false;
    }

    VkMemoryRequirements memory_requirements{};
    vkGetImageMemoryRequirements(_device, texture.image, &memory_requirements);
    std::uint32_t memory_type_index = 0;
    if (!find_memory_type(memory_requirements.memoryTypeBits,
                          VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, memory_type_index))
    {
        error = "No hay memoria de dispositivo para la textura.";
        destroy_texture(texture);
        return false;
    }

    VkMemoryAllocateInfo allocate_info{};
    allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocate_info.allocationSize = memory_requirements.size;
    allocate_info.memoryTypeIndex = memory_type_index;
    if (!CheckVk(vkAllocateMemory(_device, &allocate_info, nullptr, &texture.memory),
                 "vkAllocateMemory") ||
        !CheckVk(vkBindImageMemory(_device, texture.image, texture.memory, 0),
                 "vkBindImageMemory"))
    {
        error = "No se pudo reservar memoria para la textura.";
        destroy_texture(texture);
        return false;
    }

    VkImageViewCreateInfo view_info{};
    view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    view_info.image = texture.image;
    view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    view_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    view_info.subresourceRange.baseMipLevel = 0;
    view_info.subresourceRange.levelCount = 1;
    view_info.subresourceRange.baseArrayLayer = 0;
    view_info.subresourceRange.layerCount = 1;
    if (!CheckVk(vkCreateImageView(_device, &view_info, nullptr, &texture.view),
                 "vkCreateImageView"))
    {
        error = "No se pudo crear la vista de la textura.";
        destroy_texture(texture);
        return false;
    }

    texture.width = width;
    texture.height = height;
    if (!update_descriptor(texture))
    {
        error = "No se pudo actualizar el descriptor de la textura.";
        destroy_texture(texture);
        return false;
    }

    VkBuffer staging_buffer = VK_NULL_HANDLE;
    VkDeviceMemory staging_memory = VK_NULL_HANDLE;
    if (!create_buffer(IMAGE_BYTES, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                       VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                           VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                       staging_buffer, staging_memory))
    {
        error = "No se pudo crear el staging de la textura.";
        destroy_texture(texture);
        return false;
    }

    void *mapped = nullptr;
    if (!CheckVk(vkMapMemory(_device, staging_memory, 0, IMAGE_BYTES, 0, &mapped),
                 "vkMapMemory"))
    {
        vkDestroyBuffer(_device, staging_buffer, nullptr);
        vkFreeMemory(_device, staging_memory, nullptr);
        error = "No se pudo mapear el staging de la textura.";
        destroy_texture(texture);
        return false;
    }
    std::memcpy(mapped, rgba, static_cast<std::size_t>(IMAGE_BYTES));
    vkUnmapMemory(_device, staging_memory);

    const bool COPIED =
        copy_buffer_to_image(staging_buffer, texture.image, width, height);
    vkDestroyBuffer(_device, staging_buffer, nullptr);
    vkFreeMemory(_device, staging_memory, nullptr);
    if (!COPIED)
    {
        error = "No se pudo subir la textura a la GPU.";
        destroy_texture(texture);
        return false;
    }
    return true;
}

bool QuadRenderer::find_memory_type(std::uint32_t type_filter,
                                    VkMemoryPropertyFlags properties,
                                    std::uint32_t &memory_type_index) const
{
    VkPhysicalDeviceMemoryProperties memory_properties{};
    vkGetPhysicalDeviceMemoryProperties(_physical_device, &memory_properties);
    for (std::uint32_t index = 0; index < memory_properties.memoryTypeCount; ++index)
    {
        const std::uint32_t BIT = 1U << index;
        if ((type_filter & BIT) != 0U &&
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
            (memory_properties.memoryTypes[index].propertyFlags & properties) ==
                properties)
        {
            memory_type_index = index;
            return true;
        }
    }
    return false;
}

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
bool QuadRenderer::create_buffer(VkDeviceSize size, VkBufferUsageFlags usage,
                                 VkMemoryPropertyFlags properties, VkBuffer &buffer,
                                 VkDeviceMemory &memory)
{
    VkBufferCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    create_info.size = size;
    create_info.usage = usage;
    create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkBuffer new_buffer = VK_NULL_HANDLE;
    if (!CheckVk(vkCreateBuffer(_device, &create_info, nullptr, &new_buffer),
                 "vkCreateBuffer"))
    {
        return false;
    }

    VkMemoryRequirements memory_requirements{};
    vkGetBufferMemoryRequirements(_device, new_buffer, &memory_requirements);
    std::uint32_t memory_type_index = 0;
    if (!find_memory_type(memory_requirements.memoryTypeBits, properties,
                          memory_type_index))
    {
        vkDestroyBuffer(_device, new_buffer, nullptr);
        return false;
    }

    VkMemoryAllocateInfo allocate_info{};
    allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocate_info.allocationSize = memory_requirements.size;
    allocate_info.memoryTypeIndex = memory_type_index;
    VkDeviceMemory new_memory = VK_NULL_HANDLE;
    if (!CheckVk(vkAllocateMemory(_device, &allocate_info, nullptr, &new_memory),
                 "vkAllocateMemory"))
    {
        vkDestroyBuffer(_device, new_buffer, nullptr);
        return false;
    }
    if (!CheckVk(vkBindBufferMemory(_device, new_buffer, new_memory, 0),
                 "vkBindBufferMemory"))
    {
        vkDestroyBuffer(_device, new_buffer, nullptr);
        vkFreeMemory(_device, new_memory, nullptr);
        return false;
    }
    buffer = new_buffer;
    memory = new_memory;
    return true;
}

bool QuadRenderer::copy_buffer_to_image(VkBuffer buffer, VkImage image,
                                        std::uint32_t width, std::uint32_t height)
{
    VkCommandBufferAllocateInfo allocate_info{};
    allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocate_info.commandPool = _command_pool;
    allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocate_info.commandBufferCount = 1;
    VkCommandBuffer command_buffer = VK_NULL_HANDLE;
    if (!CheckVk(vkAllocateCommandBuffers(_device, &allocate_info, &command_buffer),
                 "vkAllocateCommandBuffers"))
    {
        return false;
    }

    VkCommandBufferBeginInfo begin_info{};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    if (!CheckVk(vkBeginCommandBuffer(command_buffer, &begin_info),
                 "vkBeginCommandBuffer"))
    {
        vkFreeCommandBuffers(_device, _command_pool, 1, &command_buffer);
        return false;
    }

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    vkCmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                         VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1,
                         &barrier);

    VkBufferImageCopy region{};
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageExtent = VkExtent3D{width, height, 1};
    vkCmdCopyBufferToImage(command_buffer, buffer, image,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    vkCmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
                         VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr,
                         1, &barrier);

    if (!CheckVk(vkEndCommandBuffer(command_buffer), "vkEndCommandBuffer"))
    {
        vkFreeCommandBuffers(_device, _command_pool, 1, &command_buffer);
        return false;
    }

    VkSubmitInfo submit_info{};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &command_buffer;
    bool success =
        CheckVk(vkQueueSubmit(_queue, 1, &submit_info, VK_NULL_HANDLE), "vkQueueSubmit");
    if (success)
    {
        success = CheckVk(vkQueueWaitIdle(_queue), "vkQueueWaitIdle");
    }
    vkFreeCommandBuffers(_device, _command_pool, 1, &command_buffer);
    return success;
}

bool QuadRenderer::update_descriptor(Texture &texture)
{
    VkDescriptorSetAllocateInfo allocate_info{};
    allocate_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocate_info.descriptorPool = _descriptor_pool;
    allocate_info.descriptorSetCount = 1;
    allocate_info.pSetLayouts = &_descriptor_set_layout;
    if (!CheckVk(vkAllocateDescriptorSets(_device, &allocate_info, &texture.descriptor),
                 "vkAllocateDescriptorSets"))
    {
        return false;
    }

    VkDescriptorImageInfo image_info{};
    image_info.sampler = _sampler;
    image_info.imageView = texture.view;
    image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkWriteDescriptorSet write{};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.dstSet = texture.descriptor;
    write.dstBinding = 0;
    write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    write.descriptorCount = 1;
    write.pImageInfo = &image_info;
    vkUpdateDescriptorSets(_device, 1, &write, 0, nullptr);
    return true;
}

void QuadRenderer::destroy_texture(Texture &texture) noexcept
{
    if (_device == VK_NULL_HANDLE)
    {
        texture = Texture{};
        return;
    }
    if (texture.view != VK_NULL_HANDLE)
    {
        vkDestroyImageView(_device, texture.view, nullptr);
    }
    if (texture.image != VK_NULL_HANDLE)
    {
        vkDestroyImage(_device, texture.image, nullptr);
    }
    if (texture.memory != VK_NULL_HANDLE)
    {
        vkFreeMemory(_device, texture.memory, nullptr);
    }
    if (texture.descriptor != VK_NULL_HANDLE && _descriptor_pool != VK_NULL_HANDLE)
    {
        (void)vkFreeDescriptorSets(_device, _descriptor_pool, 1, &texture.descriptor);
    }
    texture = Texture{};
}

} // namespace k_engine::editor::gfx
