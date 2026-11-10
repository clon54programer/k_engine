#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include <vulkan/vulkan.h>

struct GLFWwindow;

namespace k_engine::editor::gfx
{

struct ContextOptions
{
    std::string title = "K_engine - editor";
    std::uint32_t width = 1280;
    std::uint32_t height = 720;
};

// Contexto Vulkan mínimo del editor: ventana X11, superficie, swapchain,
// render pass, command buffer y sincronización. No contiene lógica de edición.
class VulkanContext
{
  public:
    VulkanContext();
    ~VulkanContext();

    VulkanContext(const VulkanContext &) = delete;
    VulkanContext &operator=(const VulkanContext &) = delete;
    VulkanContext(VulkanContext &&) = delete;
    VulkanContext &operator=(VulkanContext &&) = delete;

    bool Initialize(const ContextOptions &options);
    void Shutdown() noexcept;

    bool BeginFrame();
    bool EndFrame();

    void MarkFramebufferResized() noexcept;
    void SetClearColor(float red, float green, float blue, float alpha) noexcept;
    void WaitIdle() const;
    void PrintValidationSummary() const;

    [[nodiscard]] GLFWwindow *Window() const noexcept;
    [[nodiscard]] VkCommandBuffer CommandBuffer() const noexcept;
    [[nodiscard]] VkExtent2D Extent() const noexcept;
    [[nodiscard]] VkRenderPass RenderPass() const noexcept;
    [[nodiscard]] VkPhysicalDevice PhysicalDevice() const noexcept;
    [[nodiscard]] VkDevice Device() const noexcept;
    [[nodiscard]] VkCommandPool CommandPool() const noexcept;
    [[nodiscard]] VkQueue GraphicsQueue() const noexcept;
    [[nodiscard]] bool ValidationEnabled() const noexcept;
    [[nodiscard]] std::uint64_t SwapchainGeneration() const noexcept;
    [[nodiscard]] std::uint32_t EngineErrorCount() const noexcept;
    [[nodiscard]] std::uint32_t EngineWarningCount() const noexcept;

  private:
    static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
        VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
        VkDebugUtilsMessageTypeFlagsEXT message_types,
        const VkDebugUtilsMessengerCallbackDataEXT *callback_data, void *user_data);

    void handle_debug_message(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
                              VkDebugUtilsMessageTypeFlagsEXT message_types,
                              const VkDebugUtilsMessengerCallbackDataEXT *callback_data);

    bool initialize_glfw();
    bool create_instance();
    bool create_debug_messenger();
    bool create_window(const ContextOptions &options);
    bool create_surface();
    bool pick_physical_device();
    bool create_logical_device();
    bool create_swapchain();
    bool create_image_views();
    bool create_render_pass();
    bool create_framebuffers();
    bool create_command_pool();
    bool create_command_buffer();
    bool create_sync_objects();
    bool recreate_swapchain();

    void cleanup_swapchain() noexcept;
    void destroy_sync_objects() noexcept;

    bool _glfw_initialized = false;
    bool _validation_enabled = false;
    bool _framebuffer_resized = false;
    bool _recreate_after_present = false;
    bool _shutdown_complete = true;

    GLFWwindow *_window = nullptr;
    VkInstance _instance = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT _debug_messenger = VK_NULL_HANDLE;
    PFN_vkDestroyDebugUtilsMessengerEXT _destroy_debug_messenger = nullptr;
    VkSurfaceKHR _surface = VK_NULL_HANDLE;
    VkPhysicalDevice _physical_device = VK_NULL_HANDLE;
    VkDevice _device = VK_NULL_HANDLE;

    std::uint32_t _graphics_queue_family = 0;
    VkQueue _graphics_queue = VK_NULL_HANDLE;
    VkQueue _present_queue = VK_NULL_HANDLE;

    VkSwapchainKHR _swapchain = VK_NULL_HANDLE;
    std::vector<VkImage> _swapchain_images;
    std::vector<VkImageView> _swapchain_image_views;
    VkFormat _swapchain_format = VK_FORMAT_UNDEFINED;
    VkExtent2D _swapchain_extent{};
    std::uint64_t _swapchain_generation = 0;

    VkRenderPass _render_pass = VK_NULL_HANDLE;
    std::vector<VkFramebuffer> _framebuffers;

    VkCommandPool _command_pool = VK_NULL_HANDLE;
    VkCommandBuffer _command_buffer = VK_NULL_HANDLE;
    VkSemaphore _image_available_semaphore = VK_NULL_HANDLE;
    std::vector<VkSemaphore> _render_finished_semaphores;
    VkFence _in_flight_fence = VK_NULL_HANDLE;
    std::uint32_t _current_image_index = 0;

    bool _in_frame = false;
    float _clear_color[4] = {0.06F, 0.07F, 0.09F, 1.0F};

    std::uint32_t _engine_error_count = 0;
    std::uint32_t _engine_warning_count = 0;
    std::uint32_t _icd_display_warning_count = 0;
};

} // namespace k_engine::editor::gfx
