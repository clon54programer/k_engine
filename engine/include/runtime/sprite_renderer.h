#pragma once

#include <cstddef>
#include <cstdint>

#include <vulkan/vulkan.h>

#include "runtime/content_loader.h"

namespace k_engine::runtime
{

class SpriteRenderer
{
  public:
    bool Initialize(VkPhysicalDevice physical_device, VkDevice device,
                    VkCommandPool command_pool, VkQueue queue, VkRenderPass render_pass,
                    VkExtent2D extent, const SpriteAsset &sprite);

    void DestroyPipeline() noexcept;
    bool CreatePipeline(VkRenderPass render_pass, VkExtent2D extent);

    bool Draw(VkCommandBuffer command_buffer, float position_x, float position_y,
              float width, float height) const;

    void Shutdown() noexcept;

  private:
    bool create_descriptor_set_layout();
    bool create_pipeline_layout();
    bool create_sampler();
    bool create_texture(const SpriteAsset &sprite);
    bool create_vertex_buffer();
    bool create_descriptor_pool();
    bool update_descriptor_set();
    bool create_pipeline(VkRenderPass render_pass, VkExtent2D extent);
    bool create_shader_module(const std::uint32_t *code, std::size_t size,
                              VkShaderModule &shader_module) const;

    bool find_memory_type(std::uint32_t type_filter, VkMemoryPropertyFlags properties,
                          std::uint32_t &memory_type_index) const;
    // Los tipos de Vulkan son convertibles entre sí; el orden se mantiene por
    // claridad y se documenta en la definición.
    // NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
    bool create_buffer(VkDeviceSize size, VkBufferUsageFlags usage,
                       VkMemoryPropertyFlags properties, VkBuffer &buffer,
                       VkDeviceMemory &memory);
    bool copy_buffer_to_image(VkBuffer buffer, VkImage image, std::uint32_t width,
                              std::uint32_t height);

    void destroy_texture() noexcept;

    VkPhysicalDevice _physical_device = VK_NULL_HANDLE;
    VkDevice _device = VK_NULL_HANDLE;
    VkCommandPool _command_pool = VK_NULL_HANDLE;
    VkQueue _queue = VK_NULL_HANDLE;

    VkDescriptorSetLayout _descriptor_set_layout = VK_NULL_HANDLE;
    VkPipelineLayout _pipeline_layout = VK_NULL_HANDLE;
    VkDescriptorPool _descriptor_pool = VK_NULL_HANDLE;
    VkDescriptorSet _descriptor_set = VK_NULL_HANDLE;

    VkSampler _sampler = VK_NULL_HANDLE;
    VkImage _texture_image = VK_NULL_HANDLE;
    VkDeviceMemory _texture_memory = VK_NULL_HANDLE;
    VkImageView _texture_view = VK_NULL_HANDLE;

    VkBuffer _vertex_buffer = VK_NULL_HANDLE;
    VkDeviceMemory _vertex_memory = VK_NULL_HANDLE;

    VkPipeline _pipeline = VK_NULL_HANDLE;
    VkExtent2D _extent{};
};

} // namespace k_engine::runtime
