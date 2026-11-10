#include "runtime/sprite_renderer.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iostream>

#include "sprite_shaders.h"

namespace k_engine::runtime
{

namespace
{

constexpr std::size_t VERTEX_COUNT = 4;
constexpr std::size_t FLOAT_COMPONENTS_PER_VERTEX = 4;
constexpr std::size_t BYTES_PER_FLOAT = sizeof(float);

constexpr std::array<float, VERTEX_COUNT * FLOAT_COMPONENTS_PER_VERTEX> VERTICES = {
    0.0F, 0.0F, 0.0F, 0.0F, 1.0F, 0.0F, 1.0F, 0.0F,
    0.0F, 1.0F, 0.0F, 1.0F, 1.0F, 1.0F, 1.0F, 1.0F,
};

struct SpritePushConstants
{
    float position_x = 0.0F;
    float position_y = 0.0F;
    float size_x = 0.0F;
    float size_y = 0.0F;
    float viewport_width = 0.0F;
    float viewport_height = 0.0F;
};

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

bool SpriteRenderer::Initialize(VkPhysicalDevice physical_device, VkDevice device,
                                VkCommandPool command_pool, VkQueue queue,
                                VkRenderPass render_pass, VkExtent2D extent,
                                const SpriteAsset &sprite)
{
    Shutdown();

    _physical_device = physical_device;
    _device = device;
    _command_pool = command_pool;
    _queue = queue;

    if (!create_descriptor_set_layout() || !create_pipeline_layout() ||
        !create_sampler() || !create_texture(sprite) || !create_vertex_buffer() ||
        !create_descriptor_pool() || !update_descriptor_set() ||
        !create_pipeline(render_pass, extent))
    {
        Shutdown();
        return false;
    }

    return true;
}

void SpriteRenderer::DestroyPipeline() noexcept
{
    if (_device != VK_NULL_HANDLE && _pipeline != VK_NULL_HANDLE)
    {
        vkDestroyPipeline(_device, _pipeline, nullptr);
    }
    _pipeline = VK_NULL_HANDLE;
}

bool SpriteRenderer::CreatePipeline(VkRenderPass render_pass, VkExtent2D extent)
{
    DestroyPipeline();
    return create_pipeline(render_pass, extent);
}

bool SpriteRenderer::Draw(VkCommandBuffer command_buffer, float position_x,
                          float position_y, float width, float height) const
{
    if (_pipeline == VK_NULL_HANDLE || command_buffer == VK_NULL_HANDLE ||
        _extent.width == 0 || _extent.height == 0)
    {
        return false;
    }

    const VkViewport VIEWPORT{
        0.0F, 0.0F, static_cast<float>(_extent.width), static_cast<float>(_extent.height),
        0.0F, 1.0F};
    const VkRect2D SCISSOR{{0, 0}, _extent};

    vkCmdSetViewport(command_buffer, 0, 1, &VIEWPORT);
    vkCmdSetScissor(command_buffer, 0, 1, &SCISSOR);

    const VkDeviceSize OFFSET = 0;
    vkCmdBindVertexBuffers(command_buffer, 0, 1, &_vertex_buffer, &OFFSET);
    vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline);
    vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            _pipeline_layout, 0, 1, &_descriptor_set, 0, nullptr);

    const SpritePushConstants PUSH{position_x,
                                   position_y,
                                   width,
                                   height,
                                   static_cast<float>(_extent.width),
                                   static_cast<float>(_extent.height)};
    vkCmdPushConstants(command_buffer, _pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT, 0,
                       sizeof(PUSH), &PUSH);
    vkCmdDraw(command_buffer, static_cast<std::uint32_t>(VERTEX_COUNT), 1, 0, 0);
    return true;
}

void SpriteRenderer::Shutdown() noexcept
{
    DestroyPipeline();

    if (_device != VK_NULL_HANDLE)
    {
        if (_descriptor_pool != VK_NULL_HANDLE)
        {
            vkDestroyDescriptorPool(_device, _descriptor_pool, nullptr);
        }
        _descriptor_pool = VK_NULL_HANDLE;
        _descriptor_set = VK_NULL_HANDLE;

        if (_sampler != VK_NULL_HANDLE)
        {
            vkDestroySampler(_device, _sampler, nullptr);
        }
        _sampler = VK_NULL_HANDLE;

        destroy_texture();

        if (_vertex_buffer != VK_NULL_HANDLE)
        {
            vkDestroyBuffer(_device, _vertex_buffer, nullptr);
        }
        _vertex_buffer = VK_NULL_HANDLE;

        if (_vertex_memory != VK_NULL_HANDLE)
        {
            vkFreeMemory(_device, _vertex_memory, nullptr);
        }
        _vertex_memory = VK_NULL_HANDLE;

        if (_pipeline_layout != VK_NULL_HANDLE)
        {
            vkDestroyPipelineLayout(_device, _pipeline_layout, nullptr);
        }
        _pipeline_layout = VK_NULL_HANDLE;

        if (_descriptor_set_layout != VK_NULL_HANDLE)
        {
            vkDestroyDescriptorSetLayout(_device, _descriptor_set_layout, nullptr);
        }
        _descriptor_set_layout = VK_NULL_HANDLE;
    }

    _physical_device = VK_NULL_HANDLE;
    _device = VK_NULL_HANDLE;
    _command_pool = VK_NULL_HANDLE;
    _queue = VK_NULL_HANDLE;
    _extent = VkExtent2D{};
}

bool SpriteRenderer::create_descriptor_set_layout()
{
    VkDescriptorSetLayoutBinding binding{};
    binding.binding = 0;
    binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    binding.descriptorCount = 1;
    binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    binding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    create_info.bindingCount = 1;
    create_info.pBindings = &binding;

    return CheckVk(vkCreateDescriptorSetLayout(_device, &create_info, nullptr,
                                               &_descriptor_set_layout),
                   "vkCreateDescriptorSetLayout");
}

bool SpriteRenderer::create_pipeline_layout()
{
    VkPushConstantRange push_range{};
    push_range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    push_range.offset = 0;
    push_range.size = static_cast<std::uint32_t>(sizeof(SpritePushConstants));

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

bool SpriteRenderer::create_sampler()
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

bool SpriteRenderer::create_texture(const SpriteAsset &sprite)
{
    if (sprite.width == 0 || sprite.height == 0 || sprite.rgba.empty())
    {
        return false;
    }

    const VkDeviceSize IMAGE_BYTES =
        static_cast<VkDeviceSize>(sprite.width) * sprite.height * 4ULL;
    if (sprite.rgba.size() != IMAGE_BYTES)
    {
        return false;
    }

    VkImageCreateInfo image_info{};
    image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_info.imageType = VK_IMAGE_TYPE_2D;
    image_info.extent.width = sprite.width;
    image_info.extent.height = sprite.height;
    image_info.extent.depth = 1;
    image_info.mipLevels = 1;
    image_info.arrayLayers = 1;
    image_info.format = VK_FORMAT_R8G8B8A8_SRGB;
    image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_info.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    image_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (!CheckVk(vkCreateImage(_device, &image_info, nullptr, &_texture_image),
                 "vkCreateImage"))
    {
        return false;
    }

    VkMemoryRequirements memory_requirements{};
    vkGetImageMemoryRequirements(_device, _texture_image, &memory_requirements);

    std::uint32_t memory_type_index = 0;
    if (!find_memory_type(memory_requirements.memoryTypeBits,
                          VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, memory_type_index))
    {
        return false;
    }

    VkMemoryAllocateInfo allocate_info{};
    allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocate_info.allocationSize = memory_requirements.size;
    allocate_info.memoryTypeIndex = memory_type_index;

    if (!CheckVk(vkAllocateMemory(_device, &allocate_info, nullptr, &_texture_memory),
                 "vkAllocateMemory"))
    {
        return false;
    }

    if (!CheckVk(vkBindImageMemory(_device, _texture_image, _texture_memory, 0),
                 "vkBindImageMemory"))
    {
        return false;
    }

    VkImageViewCreateInfo view_info{};
    view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    view_info.image = _texture_image;
    view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    view_info.format = VK_FORMAT_R8G8B8A8_SRGB;
    view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    view_info.subresourceRange.baseMipLevel = 0;
    view_info.subresourceRange.levelCount = 1;
    view_info.subresourceRange.baseArrayLayer = 0;
    view_info.subresourceRange.layerCount = 1;

    if (!CheckVk(vkCreateImageView(_device, &view_info, nullptr, &_texture_view),
                 "vkCreateImageView"))
    {
        return false;
    }

    VkBuffer staging_buffer = VK_NULL_HANDLE;
    VkDeviceMemory staging_memory = VK_NULL_HANDLE;
    if (!create_buffer(IMAGE_BYTES, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                       VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                           VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                       staging_buffer, staging_memory))
    {
        return false;
    }

    void *mapped = nullptr;
    if (!CheckVk(vkMapMemory(_device, staging_memory, 0, IMAGE_BYTES, 0, &mapped),
                 "vkMapMemory"))
    {
        vkDestroyBuffer(_device, staging_buffer, nullptr);
        vkFreeMemory(_device, staging_memory, nullptr);
        return false;
    }

    std::memcpy(mapped, sprite.rgba.data(), static_cast<std::size_t>(IMAGE_BYTES));
    vkUnmapMemory(_device, staging_memory);

    const bool COPIED =
        copy_buffer_to_image(staging_buffer, _texture_image, sprite.width, sprite.height);

    vkDestroyBuffer(_device, staging_buffer, nullptr);
    vkFreeMemory(_device, staging_memory, nullptr);
    return COPIED;
}

bool SpriteRenderer::create_vertex_buffer()
{
    const VkDeviceSize BUFFER_SIZE = sizeof(VERTICES);

    if (!create_buffer(BUFFER_SIZE, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                       VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                           VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                       _vertex_buffer, _vertex_memory))
    {
        return false;
    }

    void *mapped = nullptr;
    if (!CheckVk(vkMapMemory(_device, _vertex_memory, 0, BUFFER_SIZE, 0, &mapped),
                 "vkMapMemory"))
    {
        return false;
    }

    std::memcpy(mapped, VERTICES.data(), sizeof(VERTICES));
    vkUnmapMemory(_device, _vertex_memory);
    return true;
}

bool SpriteRenderer::create_descriptor_pool()
{
    VkDescriptorPoolSize pool_size{};
    pool_size.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    pool_size.descriptorCount = 1;

    VkDescriptorPoolCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    create_info.poolSizeCount = 1;
    create_info.pPoolSizes = &pool_size;
    create_info.maxSets = 1;

    return CheckVk(
        vkCreateDescriptorPool(_device, &create_info, nullptr, &_descriptor_pool),
        "vkCreateDescriptorPool");
}

bool SpriteRenderer::update_descriptor_set()
{
    VkDescriptorSetAllocateInfo allocate_info{};
    allocate_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocate_info.descriptorPool = _descriptor_pool;
    allocate_info.descriptorSetCount = 1;
    allocate_info.pSetLayouts = &_descriptor_set_layout;

    if (!CheckVk(vkAllocateDescriptorSets(_device, &allocate_info, &_descriptor_set),
                 "vkAllocateDescriptorSets"))
    {
        return false;
    }

    VkDescriptorImageInfo image_info{};
    image_info.sampler = _sampler;
    image_info.imageView = _texture_view;
    image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkWriteDescriptorSet write{};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.dstSet = _descriptor_set;
    write.dstBinding = 0;
    write.dstArrayElement = 0;
    write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    write.descriptorCount = 1;
    write.pImageInfo = &image_info;

    vkUpdateDescriptorSets(_device, 1, &write, 0, nullptr);
    return true;
}

bool SpriteRenderer::create_shader_module(const std::uint32_t *code, std::size_t size,
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

bool SpriteRenderer::create_pipeline(VkRenderPass render_pass, VkExtent2D extent)
{
    if (_device == VK_NULL_HANDLE || render_pass == VK_NULL_HANDLE ||
        _pipeline_layout == VK_NULL_HANDLE || _descriptor_set_layout == VK_NULL_HANDLE)
    {
        return false;
    }

    VkShaderModule vertex_module = VK_NULL_HANDLE;
    VkShaderModule fragment_module = VK_NULL_HANDLE;
    if (!create_shader_module(shaders::SPRITE_VERT.data(),
                              shaders::SPRITE_VERT.size() * sizeof(std::uint32_t),
                              vertex_module) ||
        !create_shader_module(shaders::SPRITE_FRAG.data(),
                              shaders::SPRITE_FRAG.size() * sizeof(std::uint32_t),
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
    binding.stride =
        static_cast<std::uint32_t>(FLOAT_COMPONENTS_PER_VERTEX * BYTES_PER_FLOAT);
    binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    const std::array<VkVertexInputAttributeDescription, 2> ATTRIBUTES = {
        VkVertexInputAttributeDescription{0, 0, VK_FORMAT_R32G32_SFLOAT, 0},
        VkVertexInputAttributeDescription{
            1, 0, VK_FORMAT_R32G32_SFLOAT,
            static_cast<std::uint32_t>(2U * BYTES_PER_FLOAT)},
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
    input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
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

bool SpriteRenderer::find_memory_type(std::uint32_t type_filter,
                                      VkMemoryPropertyFlags properties,
                                      std::uint32_t &memory_type_index) const
{
    if (_physical_device == VK_NULL_HANDLE)
    {
        return false;
    }

    VkPhysicalDeviceMemoryProperties memory_properties{};
    vkGetPhysicalDeviceMemoryProperties(_physical_device, &memory_properties);

    for (std::uint32_t index = 0; index < memory_properties.memoryTypeCount; ++index)
    {
        const std::uint32_t BIT = 1U << index;
        // memoryTypes es un arreglo C de tamaño fijo; el índice se comprueba
        // contra memoryTypeCount antes de acceder.
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
bool SpriteRenderer::create_buffer(VkDeviceSize size, VkBufferUsageFlags usage,
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

bool SpriteRenderer::copy_buffer_to_image(VkBuffer buffer, VkImage image,
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
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

    vkCmdPipelineBarrier(command_buffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                         VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1,
                         &barrier);

    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = VkOffset3D{0, 0, 0};
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

    const VkResult SUBMIT_RESULT = vkQueueSubmit(_queue, 1, &submit_info, VK_NULL_HANDLE);
    bool success = CheckVk(SUBMIT_RESULT, "vkQueueSubmit");
    if (success)
    {
        success = CheckVk(vkQueueWaitIdle(_queue), "vkQueueWaitIdle");
    }

    vkFreeCommandBuffers(_device, _command_pool, 1, &command_buffer);
    return success;
}

void SpriteRenderer::destroy_texture() noexcept
{
    if (_device == VK_NULL_HANDLE)
    {
        _texture_image = VK_NULL_HANDLE;
        _texture_memory = VK_NULL_HANDLE;
        _texture_view = VK_NULL_HANDLE;
        return;
    }

    if (_texture_view != VK_NULL_HANDLE)
    {
        vkDestroyImageView(_device, _texture_view, nullptr);
    }
    _texture_view = VK_NULL_HANDLE;

    if (_texture_image != VK_NULL_HANDLE)
    {
        vkDestroyImage(_device, _texture_image, nullptr);
    }
    _texture_image = VK_NULL_HANDLE;

    if (_texture_memory != VK_NULL_HANDLE)
    {
        vkFreeMemory(_device, _texture_memory, nullptr);
    }
    _texture_memory = VK_NULL_HANDLE;
}

} // namespace k_engine::runtime
