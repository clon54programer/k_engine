#pragma once

#include <cstdint>
#include <vector>

#include <vulkan/vulkan.h>

struct GLFWwindow;

namespace k_engine::runtime
{

struct RuntimeOptions
{
    std::uint32_t frame_limit = 0;
};

class VulkanRuntime
{
  public:
    VulkanRuntime();
    ~VulkanRuntime();

    VulkanRuntime(const VulkanRuntime &) = delete;
    VulkanRuntime &operator=(const VulkanRuntime &) = delete;
    VulkanRuntime(VulkanRuntime &&) = delete;
    VulkanRuntime &operator=(VulkanRuntime &&) = delete;

    bool Initialize();
    int Run(const RuntimeOptions &options);
    void Shutdown() noexcept;

  private:
    static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
        VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
        VkDebugUtilsMessageTypeFlagsEXT message_types,
        const VkDebugUtilsMessengerCallbackDataEXT *callback_data, void *user_data);

    static void framebuffer_size_callback(GLFWwindow *window, int width, int height);
    static void key_callback(GLFWwindow *window, int key, int scancode, int action,
                             int mods);

    void handle_debug_message(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
                              VkDebugUtilsMessageTypeFlagsEXT message_types,
                              const VkDebugUtilsMessengerCallbackDataEXT *callback_data);

    bool initialize_glfw();
    bool create_instance();
    bool create_debug_messenger();
    bool create_window();
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
    bool prepare_frame(std::uint32_t &image_index, bool &frame_skipped);
    bool submit_frame(std::uint32_t image_index, VkResult &present_result);
    bool handle_frame_result(VkResult present_result);

    void cleanup_swapchain() noexcept;
    void destroy_sync_objects() noexcept;
    bool record_command_buffer(VkCommandBuffer command_buffer, std::uint32_t image_index);
    void report_validation_summary() const;

    GLFWwindow *_window = nullptr;
    bool _glfw_initialized = false;
    bool _validation_enabled = false;
    bool _framebuffer_resized = false;
    bool _shutdown_complete = false;

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

    VkRenderPass _render_pass = VK_NULL_HANDLE;
    std::vector<VkFramebuffer> _framebuffers;

    VkCommandPool _command_pool = VK_NULL_HANDLE;
    VkCommandBuffer _command_buffer = VK_NULL_HANDLE;
    VkSemaphore _image_available_semaphore = VK_NULL_HANDLE;
    std::vector<VkSemaphore> _render_finished_semaphores;
    VkFence _in_flight_fence = VK_NULL_HANDLE;

    std::uint32_t _engine_error_count = 0;
    std::uint32_t _engine_warning_count = 0;
    std::uint32_t _icd_display_warning_count = 0;
};

} // namespace k_engine::runtime
