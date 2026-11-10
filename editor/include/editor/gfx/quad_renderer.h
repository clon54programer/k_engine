#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include <vulkan/vulkan.h>

#include "editor/gfx/vulkan_context.h"
#include "ui/ui_types.h"

namespace k_engine::editor::gfx
{

// Renderer de quads para la UI, los tiles y la previsualización. Un solo
// pipeline y un descriptor set por textura.
class QuadRenderer
{
  public:
    QuadRenderer();
    ~QuadRenderer();

    QuadRenderer(const QuadRenderer &) = delete;
    QuadRenderer &operator=(const QuadRenderer &) = delete;
    QuadRenderer(QuadRenderer &&) = delete;
    QuadRenderer &operator=(QuadRenderer &&) = delete;

    bool Initialize(VulkanContext &context, VkRenderPass render_pass, VkExtent2D extent);
    bool RecreatePipeline(VkRenderPass render_pass, VkExtent2D extent);
    void Shutdown() noexcept;

    bool Draw(VkCommandBuffer command_buffer, const ui::DrawList &draw_list);

    bool CreateTexture(const std::uint8_t *rgba, std::uint32_t width,
                       std::uint32_t height, ui::TextureId &texture, std::string &error);
    void DestroyTexture(ui::TextureId texture) noexcept;
    [[nodiscard]] static ui::TextureId WhiteTexture() noexcept;

  private:
    struct Texture
    {
        VkImage image = VK_NULL_HANDLE;
        VkDeviceMemory memory = VK_NULL_HANDLE;
        VkImageView view = VK_NULL_HANDLE;
        VkDescriptorSet descriptor = VK_NULL_HANDLE;
        std::uint32_t width = 0;
        std::uint32_t height = 0;
    };

    bool create_descriptor_set_layout();
    bool create_pipeline_layout();
    bool create_sampler();
    bool create_descriptor_pool();
    bool create_vertex_buffer();
    bool create_pipeline(VkRenderPass render_pass, VkExtent2D extent);
    bool create_shader_module(const std::uint32_t *code, std::size_t size,
                              VkShaderModule &shader_module) const;
    bool create_texture_internal(const std::uint8_t *rgba, std::uint32_t width,
                                 std::uint32_t height, Texture &texture,
                                 std::string &error);
    bool find_memory_type(std::uint32_t type_filter, VkMemoryPropertyFlags properties,
                          std::uint32_t &memory_type_index) const;
    bool create_buffer(VkDeviceSize size, VkBufferUsageFlags usage,
                       VkMemoryPropertyFlags properties, VkBuffer &buffer,
                       VkDeviceMemory &memory);
    bool copy_buffer_to_image(VkBuffer buffer, VkImage image, std::uint32_t width,
                              std::uint32_t height);
    bool update_descriptor(Texture &texture);
    void destroy_texture(Texture &texture) noexcept;

    VulkanContext *_context = nullptr;
    VkPhysicalDevice _physical_device = VK_NULL_HANDLE;
    VkDevice _device = VK_NULL_HANDLE;
    VkCommandPool _command_pool = VK_NULL_HANDLE;
    VkQueue _queue = VK_NULL_HANDLE;

    VkDescriptorSetLayout _descriptor_set_layout = VK_NULL_HANDLE;
    VkPipelineLayout _pipeline_layout = VK_NULL_HANDLE;
    VkDescriptorPool _descriptor_pool = VK_NULL_HANDLE;
    VkSampler _sampler = VK_NULL_HANDLE;

    VkBuffer _vertex_buffer = VK_NULL_HANDLE;
    VkDeviceMemory _vertex_memory = VK_NULL_HANDLE;
    void *_vertex_mapped = nullptr;
    VkDeviceSize _vertex_capacity = 0;

    VkPipeline _pipeline = VK_NULL_HANDLE;
    VkExtent2D _extent{};

    std::vector<Texture> _textures;
};

} // namespace k_engine::editor::gfx
