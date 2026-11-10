#include "editor/gfx/vulkan_context.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <iterator>
#include <limits>
#include <string>
#include <string_view>
#include <vector>

#include <GLFW/glfw3.h>

namespace k_engine::editor::gfx
{

namespace
{

constexpr std::uint32_t DEVICE_QUEUE_INDEX = 0;
constexpr float DEVICE_QUEUE_PRIORITY = 1.0F;
constexpr std::uint32_t AMD_VENDOR_ID = 0x1002;
constexpr std::uint32_t AMD_RENOIR_DEVICE_ID = 0x15E7;
constexpr std::uint32_t IMAGE_COUNT_EXTRA = 1;
constexpr const char *VALIDATION_LAYER_NAME = "VK_LAYER_KHRONOS_validation";

constexpr std::array<VkCompositeAlphaFlagBitsKHR, 4> COMPOSITE_ALPHA_CANDIDATES = {
    VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
    VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
    VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
    VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
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

void LogInfo(const std::string &message)
{
    std::cout << message << '\n';
}

void LogError(const std::string &message)
{
    std::cerr << "Error: " << message << '\n';
}

bool HasInstanceLayer(const char *name)
{
    std::uint32_t count = 0;
    VkResult result = vkEnumerateInstanceLayerProperties(&count, nullptr);
    if (result != VK_SUCCESS || count == 0)
    {
        return false;
    }
    std::vector<VkLayerProperties> layers(count);
    result = vkEnumerateInstanceLayerProperties(&count, layers.data());
    if (result != VK_SUCCESS && result != VK_INCOMPLETE)
    {
        return false;
    }
    layers.resize(count);
    return std::any_of(layers.begin(), layers.end(),
                       [name](const VkLayerProperties &layer) {
                           return std::strcmp(std::data(layer.layerName), name) == 0;
                       });
}

bool HasInstanceExtension(const char *name)
{
    std::uint32_t count = 0;
    VkResult result = vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr);
    if (result != VK_SUCCESS || count == 0)
    {
        return false;
    }
    std::vector<VkExtensionProperties> extensions(count);
    result = vkEnumerateInstanceExtensionProperties(nullptr, &count, extensions.data());
    if (result != VK_SUCCESS && result != VK_INCOMPLETE)
    {
        return false;
    }
    extensions.resize(count);
    return std::any_of(extensions.begin(), extensions.end(),
                       [name](const VkExtensionProperties &extension) {
                           return std::strcmp(std::data(extension.extensionName), name) ==
                                  0;
                       });
}

bool HasDeviceExtension(VkPhysicalDevice device, const char *name)
{
    std::uint32_t count = 0;
    VkResult result =
        vkEnumerateDeviceExtensionProperties(device, nullptr, &count, nullptr);
    if (result != VK_SUCCESS || count == 0)
    {
        return false;
    }
    std::vector<VkExtensionProperties> extensions(count);
    result =
        vkEnumerateDeviceExtensionProperties(device, nullptr, &count, extensions.data());
    if (result != VK_SUCCESS && result != VK_INCOMPLETE)
    {
        return false;
    }
    extensions.resize(count);
    return std::any_of(extensions.begin(), extensions.end(),
                       [name](const VkExtensionProperties &extension) {
                           return std::strcmp(std::data(extension.extensionName), name) ==
                                  0;
                       });
}

bool FindQueueFamily(VkPhysicalDevice device, VkSurfaceKHR surface,
                     std::uint32_t &family_index)
{
    std::uint32_t count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &count, nullptr);
    if (count == 0)
    {
        return false;
    }
    std::vector<VkQueueFamilyProperties> families(count);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &count, families.data());
    for (std::uint32_t index = 0; index < count; ++index)
    {
        if ((families[index].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0)
        {
            continue;
        }
        VkBool32 present_supported = VK_FALSE;
        const VkResult RESULT = vkGetPhysicalDeviceSurfaceSupportKHR(
            device, index, surface, &present_supported);
        if (RESULT == VK_SUCCESS && present_supported == VK_TRUE)
        {
            family_index = index;
            return true;
        }
    }
    return false;
}

VkSurfaceFormatKHR ChooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &formats)
{
    for (const auto &format : formats)
    {
        if (format.format == VK_FORMAT_B8G8R8A8_UNORM &&
            format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            return format;
        }
    }
    for (const auto &format : formats)
    {
        if (format.format == VK_FORMAT_B8G8R8A8_SRGB &&
            format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            return format;
        }
    }
    if (formats.size() == 1 && formats.front().format == VK_FORMAT_UNDEFINED)
    {
        return VkSurfaceFormatKHR{VK_FORMAT_B8G8R8A8_UNORM,
                                  VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
    }
    return formats.front();
}

VkPresentModeKHR ChoosePresentMode(const std::vector<VkPresentModeKHR> &present_modes)
{
    for (const VkPresentModeKHR MODE : present_modes)
    {
        if (MODE == VK_PRESENT_MODE_FIFO_KHR)
        {
            return MODE;
        }
    }
    return present_modes.front();
}

VkExtent2D ChooseExtent(const VkSurfaceCapabilitiesKHR &capabilities, GLFWwindow *window)
{
    if (capabilities.currentExtent.width != std::numeric_limits<std::uint32_t>::max())
    {
        return capabilities.currentExtent;
    }
    int width = 0;
    int height = 0;
    glfwGetFramebufferSize(window, &width, &height);
    VkExtent2D extent{};
    extent.width = static_cast<std::uint32_t>(std::max(width, 0));
    extent.height = static_cast<std::uint32_t>(std::max(height, 0));
    extent.width = std::clamp(extent.width, capabilities.minImageExtent.width,
                              capabilities.maxImageExtent.width);
    extent.height = std::clamp(extent.height, capabilities.minImageExtent.height,
                               capabilities.maxImageExtent.height);
    return extent;
}

VkCompositeAlphaFlagBitsKHR ChooseCompositeAlpha(
    const VkSurfaceCapabilitiesKHR &capabilities)
{
    for (const VkCompositeAlphaFlagBitsKHR CANDIDATE : COMPOSITE_ALPHA_CANDIDATES)
    {
        if ((capabilities.supportedCompositeAlpha & CANDIDATE) != 0)
        {
            return CANDIDATE;
        }
    }
    return VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
}

template <typename FunctionType>
FunctionType LoadInstanceFunction(VkInstance instance, const char *name)
{
    const PFN_vkVoidFunction FUNCTION = vkGetInstanceProcAddr(instance, name);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    return reinterpret_cast<FunctionType>(FUNCTION);
}

bool IsIcdDisplayMessage(const char *message)
{
    if (message == nullptr)
    {
        return false;
    }
    const std::string_view TEXT(message);
    return TEXT.find("vkGetPhysicalDeviceDisplayPropertiesKHR") !=
               std::string_view::npos ||
           TEXT.find("vkGetPhysicalDeviceDisplayPlanePropertiesKHR") !=
               std::string_view::npos ||
           TEXT.find("VK_KHR_display") != std::string_view::npos;
}

void GlfwErrorCallback(int error_code, const char *description)
{
    std::cerr << "Error de GLFW (" << error_code
              << "): " << (description != nullptr ? description : "sin descripcion")
              << '\n';
}

} // namespace

VulkanContext::VulkanContext() = default;

VulkanContext::~VulkanContext()
{
    Shutdown();
}

bool VulkanContext::Initialize(const ContextOptions &options)
{
    if (!_shutdown_complete)
    {
        return false;
    }
    _shutdown_complete = false;
    _engine_error_count = 0;
    _engine_warning_count = 0;
    _icd_display_warning_count = 0;

#ifndef NDEBUG
    _validation_enabled = true;
#else
    _validation_enabled = false;
#endif

    if (!initialize_glfw() || !create_instance() || !create_debug_messenger() ||
        !create_window(options) || !create_surface() || !pick_physical_device() ||
        !create_logical_device() || !create_swapchain() || !create_image_views() ||
        !create_render_pass() || !create_framebuffers() || !create_command_pool() ||
        !create_command_buffer() || !create_sync_objects())
    {
        LogError("No se pudo inicializar el contexto Vulkan del editor.");
        return false;
    }

    ++_swapchain_generation;
    return true;
}

void VulkanContext::Shutdown() noexcept
{
    if (_shutdown_complete)
    {
        return;
    }

    if (_device != VK_NULL_HANDLE)
    {
        (void)vkDeviceWaitIdle(_device);
    }

    cleanup_swapchain();

    if (_device != VK_NULL_HANDLE)
    {
        if (_command_pool != VK_NULL_HANDLE)
        {
            vkDestroyCommandPool(_device, _command_pool, nullptr);
            _command_pool = VK_NULL_HANDLE;
            _command_buffer = VK_NULL_HANDLE;
        }
        destroy_sync_objects();
        vkDestroyDevice(_device, nullptr);
        _device = VK_NULL_HANDLE;
    }

    if (_surface != VK_NULL_HANDLE && _instance != VK_NULL_HANDLE)
    {
        vkDestroySurfaceKHR(_instance, _surface, nullptr);
        _surface = VK_NULL_HANDLE;
    }

    if (_debug_messenger != VK_NULL_HANDLE && _destroy_debug_messenger != nullptr)
    {
        _destroy_debug_messenger(_instance, _debug_messenger, nullptr);
        _debug_messenger = VK_NULL_HANDLE;
    }
    _destroy_debug_messenger = nullptr;

    if (_instance != VK_NULL_HANDLE)
    {
        vkDestroyInstance(_instance, nullptr);
        _instance = VK_NULL_HANDLE;
    }

    if (_window != nullptr)
    {
        glfwDestroyWindow(_window);
        _window = nullptr;
    }
    if (_glfw_initialized)
    {
        glfwTerminate();
        _glfw_initialized = false;
    }

    PrintValidationSummary();
    _shutdown_complete = true;
}

bool VulkanContext::BeginFrame()
{
    if (_window == nullptr || _device == VK_NULL_HANDLE || _swapchain == VK_NULL_HANDLE)
    {
        return false;
    }

    if (_framebuffer_resized)
    {
        _framebuffer_resized = false;
        if (!recreate_swapchain())
        {
            return false;
        }
    }

    if (_swapchain_extent.width == 0 || _swapchain_extent.height == 0)
    {
        glfwWaitEvents();
        return false;
    }

    const VkResult WAIT_RESULT =
        vkWaitForFences(_device, 1, &_in_flight_fence, VK_TRUE,
                        std::numeric_limits<std::uint64_t>::max());
    if (!CheckVk(WAIT_RESULT, "vkWaitForFences"))
    {
        return false;
    }

    std::uint32_t image_index = 0;
    const VkResult ACQUIRE_RESULT = vkAcquireNextImageKHR(
        _device, _swapchain, std::numeric_limits<std::uint64_t>::max(),
        _image_available_semaphore, VK_NULL_HANDLE, &image_index);

    if (ACQUIRE_RESULT == VK_ERROR_OUT_OF_DATE_KHR)
    {
        (void)recreate_swapchain();
        return false;
    }
    if (ACQUIRE_RESULT != VK_SUCCESS && ACQUIRE_RESULT != VK_SUBOPTIMAL_KHR)
    {
        LogError("vkAcquireNextImageKHR fallo con VkResult " +
                 std::to_string(static_cast<int>(ACQUIRE_RESULT)) + ".");
        return false;
    }
    if (ACQUIRE_RESULT == VK_SUBOPTIMAL_KHR)
    {
        _recreate_after_present = true;
    }

    if (!CheckVk(vkResetFences(_device, 1, &_in_flight_fence), "vkResetFences") ||
        !CheckVk(vkResetCommandBuffer(_command_buffer, 0), "vkResetCommandBuffer"))
    {
        return false;
    }

    VkCommandBufferBeginInfo begin_info{};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    if (!CheckVk(vkBeginCommandBuffer(_command_buffer, &begin_info),
                 "vkBeginCommandBuffer"))
    {
        return false;
    }

    std::array<VkClearValue, 1> clear_values{};
    clear_values[0].color.float32[0] = _clear_color[0];
    clear_values[0].color.float32[1] = _clear_color[1];
    clear_values[0].color.float32[2] = _clear_color[2];
    clear_values[0].color.float32[3] = _clear_color[3];

    VkRenderPassBeginInfo render_pass_info{};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_info.renderPass = _render_pass;
    render_pass_info.framebuffer = _framebuffers[image_index];
    render_pass_info.renderArea.offset = VkOffset2D{0, 0};
    render_pass_info.renderArea.extent = _swapchain_extent;
    render_pass_info.clearValueCount = static_cast<std::uint32_t>(clear_values.size());
    render_pass_info.pClearValues = clear_values.data();

    vkCmdBeginRenderPass(_command_buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);
    _current_image_index = image_index;
    _in_frame = true;
    return true;
}

bool VulkanContext::EndFrame()
{
    if (!_in_frame)
    {
        return false;
    }

    vkCmdEndRenderPass(_command_buffer);
    if (!CheckVk(vkEndCommandBuffer(_command_buffer), "vkEndCommandBuffer"))
    {
        _in_frame = false;
        return false;
    }

    VkSubmitInfo submit_info{};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    const VkPipelineStageFlags WAIT_STAGE = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = &_image_available_semaphore;
    submit_info.pWaitDstStageMask = &WAIT_STAGE;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &_command_buffer;
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = &_render_finished_semaphores[_current_image_index];

    if (!CheckVk(vkQueueSubmit(_graphics_queue, 1, &submit_info, _in_flight_fence),
                 "vkQueueSubmit"))
    {
        _in_frame = false;
        return false;
    }

    VkPresentInfoKHR present_info{};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = &_render_finished_semaphores[_current_image_index];
    present_info.swapchainCount = 1;
    present_info.pSwapchains = &_swapchain;
    present_info.pImageIndices = &_current_image_index;

    const VkResult PRESENT_RESULT = vkQueuePresentKHR(_present_queue, &present_info);
    _in_frame = false;

    if (PRESENT_RESULT == VK_ERROR_OUT_OF_DATE_KHR ||
        PRESENT_RESULT == VK_SUBOPTIMAL_KHR || _recreate_after_present ||
        _framebuffer_resized)
    {
        _recreate_after_present = false;
        _framebuffer_resized = false;
        return recreate_swapchain();
    }
    if (PRESENT_RESULT != VK_SUCCESS)
    {
        LogError("vkQueuePresentKHR fallo con VkResult " +
                 std::to_string(static_cast<int>(PRESENT_RESULT)) + ".");
        return false;
    }
    return true;
}

void VulkanContext::MarkFramebufferResized() noexcept
{
    _framebuffer_resized = true;
}

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
void VulkanContext::SetClearColor(float red, float green, float blue,
                                  float alpha) noexcept
{
    _clear_color[0] = red;
    _clear_color[1] = green;
    _clear_color[2] = blue;
    _clear_color[3] = alpha;
}

void VulkanContext::WaitIdle() const
{
    if (_device != VK_NULL_HANDLE)
    {
        (void)vkDeviceWaitIdle(_device);
    }
}

void VulkanContext::PrintValidationSummary() const
{
    std::cout << "Validacion: " << _engine_error_count << " errores, "
              << _engine_warning_count << " advertencias del engine, "
              << _icd_display_warning_count
              << " advertencias del ICD sobre VK_KHR_display.\n";
}

GLFWwindow *VulkanContext::Window() const noexcept
{
    return _window;
}

VkCommandBuffer VulkanContext::CommandBuffer() const noexcept
{
    return _command_buffer;
}

VkExtent2D VulkanContext::Extent() const noexcept
{
    return _swapchain_extent;
}

VkRenderPass VulkanContext::RenderPass() const noexcept
{
    return _render_pass;
}

VkPhysicalDevice VulkanContext::PhysicalDevice() const noexcept
{
    return _physical_device;
}

VkDevice VulkanContext::Device() const noexcept
{
    return _device;
}

VkCommandPool VulkanContext::CommandPool() const noexcept
{
    return _command_pool;
}

VkQueue VulkanContext::GraphicsQueue() const noexcept
{
    return _graphics_queue;
}

bool VulkanContext::ValidationEnabled() const noexcept
{
    return _validation_enabled;
}

std::uint64_t VulkanContext::SwapchainGeneration() const noexcept
{
    return _swapchain_generation;
}

std::uint32_t VulkanContext::EngineErrorCount() const noexcept
{
    return _engine_error_count;
}

std::uint32_t VulkanContext::EngineWarningCount() const noexcept
{
    return _engine_warning_count;
}

VKAPI_ATTR VkBool32 VKAPI_CALL VulkanContext::debug_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
    VkDebugUtilsMessageTypeFlagsEXT message_types,
    const VkDebugUtilsMessengerCallbackDataEXT *callback_data, void *user_data)
{
    auto *const CONTEXT = static_cast<VulkanContext *>(user_data);
    if (CONTEXT != nullptr)
    {
        CONTEXT->handle_debug_message(message_severity, message_types, callback_data);
    }
    return VK_FALSE;
}

void VulkanContext::handle_debug_message(
    VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
    VkDebugUtilsMessageTypeFlagsEXT message_types,
    const VkDebugUtilsMessengerCallbackDataEXT *callback_data)
{
    (void)message_types;
    if (callback_data == nullptr || callback_data->pMessage == nullptr)
    {
        return;
    }

    const std::string_view MESSAGE(callback_data->pMessage);
    if ((message_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) != 0)
    {
        ++_engine_error_count;
        std::cerr << "Validation error: " << MESSAGE << '\n';
    }
    else if ((message_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) != 0)
    {
        if (IsIcdDisplayMessage(callback_data->pMessage))
        {
            ++_icd_display_warning_count;
        }
        else
        {
            ++_engine_warning_count;
            std::cerr << "Validation warning: " << MESSAGE << '\n';
        }
    }
}

bool VulkanContext::initialize_glfw()
{
    glfwSetErrorCallback(&GlfwErrorCallback);
    glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_X11);
    if (glfwInit() != GLFW_TRUE)
    {
        LogError("No se pudo inicializar GLFW.");
        return false;
    }
    _glfw_initialized = true;
    if (glfwGetPlatform() != GLFW_PLATFORM_X11)
    {
        LogError("GLFW no esta usando la plataforma X11.");
        return false;
    }
    return true;
}

bool VulkanContext::create_instance()
{
    std::uint32_t glfw_extension_count = 0;
    const char *const *glfw_extensions =
        glfwGetRequiredInstanceExtensions(&glfw_extension_count);
    if (glfw_extensions == nullptr || glfw_extension_count == 0)
    {
        LogError("GLFW no proporciono las extensiones de instancia de Vulkan.");
        return false;
    }

    std::vector<const char *> extensions;
    extensions.reserve(glfw_extension_count + 1);
    std::copy_n(glfw_extensions, glfw_extension_count, std::back_inserter(extensions));

    if (_validation_enabled)
    {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }
    for (const char *const EXTENSION : extensions)
    {
        if (!HasInstanceExtension(EXTENSION))
        {
            LogError(std::string("Falta la extension de instancia ") + EXTENSION + ".");
            return false;
        }
    }

    std::vector<const char *> layers;
    if (_validation_enabled)
    {
        if (!HasInstanceLayer(VALIDATION_LAYER_NAME))
        {
            LogError(std::string("Falta la capa de validacion ") + VALIDATION_LAYER_NAME +
                     ".");
            return false;
        }
        layers.push_back(VALIDATION_LAYER_NAME);
    }

    VkApplicationInfo application_info{};
    application_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    application_info.pApplicationName = "K_engine editor";
    application_info.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
    application_info.pEngineName = "K_engine";
    application_info.engineVersion = VK_MAKE_VERSION(0, 1, 0);
    application_info.apiVersion = VK_API_VERSION_1_3;

    VkDebugUtilsMessengerCreateInfoEXT debug_create_info{};
    debug_create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    debug_create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    debug_create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                    VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                    VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    debug_create_info.pfnUserCallback = &VulkanContext::debug_callback;
    debug_create_info.pUserData = this;

    VkInstanceCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.pNext = _validation_enabled ? &debug_create_info : nullptr;
    create_info.pApplicationInfo = &application_info;
    create_info.enabledLayerCount = static_cast<std::uint32_t>(layers.size());
    create_info.ppEnabledLayerNames = layers.empty() ? nullptr : layers.data();
    create_info.enabledExtensionCount = static_cast<std::uint32_t>(extensions.size());
    create_info.ppEnabledExtensionNames = extensions.data();

    if (!CheckVk(vkCreateInstance(&create_info, nullptr, &_instance), "vkCreateInstance"))
    {
        return false;
    }
    LogInfo("Instancia Vulkan 1.3 del editor creada.");
    return true;
}

bool VulkanContext::create_debug_messenger()
{
    if (!_validation_enabled)
    {
        return true;
    }

    const auto CREATE_MESSENGER =
        LoadInstanceFunction<PFN_vkCreateDebugUtilsMessengerEXT>(
            _instance, "vkCreateDebugUtilsMessengerEXT");
    const auto DESTROY_MESSENGER =
        LoadInstanceFunction<PFN_vkDestroyDebugUtilsMessengerEXT>(
            _instance, "vkDestroyDebugUtilsMessengerEXT");
    if (CREATE_MESSENGER == nullptr || DESTROY_MESSENGER == nullptr)
    {
        LogError("No se pudieron cargar las funciones de VK_EXT_debug_utils.");
        return false;
    }
    _destroy_debug_messenger = DESTROY_MESSENGER;

    VkDebugUtilsMessengerCreateInfoEXT create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                  VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                              VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                              VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    create_info.pfnUserCallback = &VulkanContext::debug_callback;
    create_info.pUserData = this;

    return CheckVk(CREATE_MESSENGER(_instance, &create_info, nullptr, &_debug_messenger),
                   "vkCreateDebugUtilsMessengerEXT");
}

bool VulkanContext::create_window(const ContextOptions &options)
{
    glfwDefaultWindowHints();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    _window = glfwCreateWindow(static_cast<int>(options.width),
                               static_cast<int>(options.height), options.title.c_str(),
                               nullptr, nullptr);
    if (_window == nullptr)
    {
        LogError("No se pudo crear la ventana GLFW del editor.");
        return false;
    }
    return true;
}

bool VulkanContext::create_surface()
{
    return CheckVk(glfwCreateWindowSurface(_instance, _window, nullptr, &_surface),
                   "glfwCreateWindowSurface");
}

bool VulkanContext::pick_physical_device()
{
    std::uint32_t device_count = 0;
    VkResult result = vkEnumeratePhysicalDevices(_instance, &device_count, nullptr);
    if (!CheckVk(result, "vkEnumeratePhysicalDevices") || device_count == 0)
    {
        LogError("No hay dispositivos Vulkan disponibles.");
        return false;
    }

    std::vector<VkPhysicalDevice> devices(device_count);
    result = vkEnumeratePhysicalDevices(_instance, &device_count, devices.data());
    if (result != VK_SUCCESS && result != VK_INCOMPLETE)
    {
        LogError("vkEnumeratePhysicalDevices fallo al leer los dispositivos.");
        return false;
    }
    devices.resize(device_count);

    for (const VkPhysicalDevice DEVICE : devices)
    {
        VkPhysicalDeviceProperties properties{};
        vkGetPhysicalDeviceProperties(DEVICE, &properties);
        const std::string DEVICE_NAME(std::data(properties.deviceName));
        LogInfo("Dispositivo Vulkan: " + DEVICE_NAME + " (vendor " +
                std::to_string(properties.vendorID) + ", device " +
                std::to_string(properties.deviceID) + ").");

        if (properties.vendorID != AMD_VENDOR_ID ||
            properties.deviceID != AMD_RENOIR_DEVICE_ID)
        {
            continue;
        }
        if (properties.apiVersion < VK_API_VERSION_1_3)
        {
            LogError("La GPU AMD no expone Vulkan 1.3.");
            continue;
        }
        if (!HasDeviceExtension(DEVICE, VK_KHR_SWAPCHAIN_EXTENSION_NAME))
        {
            LogError("La GPU AMD no expone VK_KHR_swapchain.");
            continue;
        }
        std::uint32_t family_index = 0;
        if (!FindQueueFamily(DEVICE, _surface, family_index))
        {
            LogError("La GPU AMD no tiene una cola grafica compatible con presentacion.");
            continue;
        }
        _physical_device = DEVICE;
        _graphics_queue_family = family_index;
        LogInfo("GPU seleccionada: " + DEVICE_NAME + ".");
        return true;
    }

    LogError("No se encontro la GPU AMD Radeon Graphics 1002:15e7 compatible.");
    return false;
}

bool VulkanContext::create_logical_device()
{
    const float QUEUE_PRIORITY = DEVICE_QUEUE_PRIORITY;
    VkDeviceQueueCreateInfo queue_create_info{};
    queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_create_info.queueFamilyIndex = _graphics_queue_family;
    queue_create_info.queueCount = 1;
    queue_create_info.pQueuePriorities = &QUEUE_PRIORITY;

    const std::array<const char *, 1> DEVICE_EXTENSIONS = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    };
    VkPhysicalDeviceFeatures features{};
    VkDeviceCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    create_info.queueCreateInfoCount = 1;
    create_info.pQueueCreateInfos = &queue_create_info;
    create_info.pEnabledFeatures = &features;
    create_info.enabledExtensionCount =
        static_cast<std::uint32_t>(DEVICE_EXTENSIONS.size());
    create_info.ppEnabledExtensionNames = DEVICE_EXTENSIONS.data();

    if (!CheckVk(vkCreateDevice(_physical_device, &create_info, nullptr, &_device),
                 "vkCreateDevice"))
    {
        return false;
    }
    vkGetDeviceQueue(_device, _graphics_queue_family, DEVICE_QUEUE_INDEX,
                     &_graphics_queue);
    _present_queue = _graphics_queue;
    return true;
}

bool VulkanContext::create_swapchain()
{
    VkSurfaceCapabilitiesKHR capabilities{};
    if (!CheckVk(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(_physical_device, _surface,
                                                           &capabilities),
                 "vkGetPhysicalDeviceSurfaceCapabilitiesKHR"))
    {
        return false;
    }

    std::uint32_t format_count = 0;
    VkResult result = vkGetPhysicalDeviceSurfaceFormatsKHR(_physical_device, _surface,
                                                           &format_count, nullptr);
    if (!CheckVk(result, "vkGetPhysicalDeviceSurfaceFormatsKHR") || format_count == 0)
    {
        LogError("La superficie no expone formatos Vulkan.");
        return false;
    }
    std::vector<VkSurfaceFormatKHR> formats(format_count);
    result = vkGetPhysicalDeviceSurfaceFormatsKHR(_physical_device, _surface,
                                                  &format_count, formats.data());
    if (result != VK_SUCCESS && result != VK_INCOMPLETE)
    {
        LogError("No se pudieron leer los formatos de superficie.");
        return false;
    }
    formats.resize(format_count);

    std::uint32_t present_mode_count = 0;
    result = vkGetPhysicalDeviceSurfacePresentModesKHR(_physical_device, _surface,
                                                       &present_mode_count, nullptr);
    if (!CheckVk(result, "vkGetPhysicalDeviceSurfacePresentModesKHR") ||
        present_mode_count == 0)
    {
        LogError("La superficie no expone modos de presentacion.");
        return false;
    }
    std::vector<VkPresentModeKHR> present_modes(present_mode_count);
    result = vkGetPhysicalDeviceSurfacePresentModesKHR(
        _physical_device, _surface, &present_mode_count, present_modes.data());
    if (result != VK_SUCCESS && result != VK_INCOMPLETE)
    {
        LogError("No se pudieron leer los modos de presentacion.");
        return false;
    }
    present_modes.resize(present_mode_count);

    const VkSurfaceFormatKHR SURFACE_FORMAT = ChooseSurfaceFormat(formats);
    const VkPresentModeKHR PRESENT_MODE = ChoosePresentMode(present_modes);
    const VkExtent2D EXTENT = ChooseExtent(capabilities, _window);
    if (EXTENT.width == 0 || EXTENT.height == 0)
    {
        LogError("La ventana no tiene un tamano valido para el swapchain.");
        return false;
    }

    std::uint32_t image_count = capabilities.minImageCount + IMAGE_COUNT_EXTRA;
    if (capabilities.maxImageCount > 0 && image_count > capabilities.maxImageCount)
    {
        image_count = capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    create_info.surface = _surface;
    create_info.minImageCount = image_count;
    create_info.imageFormat = SURFACE_FORMAT.format;
    create_info.imageColorSpace = SURFACE_FORMAT.colorSpace;
    create_info.imageExtent = EXTENT;
    create_info.imageArrayLayers = 1;
    create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    create_info.preTransform = capabilities.currentTransform;
    create_info.compositeAlpha = ChooseCompositeAlpha(capabilities);
    create_info.presentMode = PRESENT_MODE;
    create_info.clipped = VK_TRUE;
    create_info.oldSwapchain = VK_NULL_HANDLE;

    if (!CheckVk(vkCreateSwapchainKHR(_device, &create_info, nullptr, &_swapchain),
                 "vkCreateSwapchainKHR"))
    {
        return false;
    }

    std::uint32_t swapchain_image_count = 0;
    result =
        vkGetSwapchainImagesKHR(_device, _swapchain, &swapchain_image_count, nullptr);
    if (!CheckVk(result, "vkGetSwapchainImagesKHR") || swapchain_image_count == 0)
    {
        return false;
    }
    _swapchain_images.resize(swapchain_image_count);
    result = vkGetSwapchainImagesKHR(_device, _swapchain, &swapchain_image_count,
                                     _swapchain_images.data());
    if (result != VK_SUCCESS && result != VK_INCOMPLETE)
    {
        LogError("No se pudieron leer las imagenes del swapchain.");
        return false;
    }
    _swapchain_images.resize(swapchain_image_count);
    _swapchain_format = SURFACE_FORMAT.format;
    _swapchain_extent = EXTENT;
    LogInfo("Swapchain del editor: " + std::to_string(EXTENT.width) + "x" +
            std::to_string(EXTENT.height) + ", " + std::to_string(swapchain_image_count) +
            " imagenes, formato " + std::to_string(static_cast<int>(_swapchain_format)) +
            ".");
    return true;
}

bool VulkanContext::create_image_views()
{
    _swapchain_image_views.resize(_swapchain_images.size(), VK_NULL_HANDLE);
    for (std::size_t index = 0; index < _swapchain_images.size(); ++index)
    {
        VkImageViewCreateInfo create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        create_info.image = _swapchain_images[index];
        create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        create_info.format = _swapchain_format;
        create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        create_info.subresourceRange.baseMipLevel = 0;
        create_info.subresourceRange.levelCount = 1;
        create_info.subresourceRange.baseArrayLayer = 0;
        create_info.subresourceRange.layerCount = 1;
        if (!CheckVk(vkCreateImageView(_device, &create_info, nullptr,
                                       &_swapchain_image_views[index]),
                     "vkCreateImageView"))
        {
            return false;
        }
    }
    return true;
}

bool VulkanContext::create_render_pass()
{
    VkAttachmentDescription color_attachment{};
    color_attachment.format = _swapchain_format;
    color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference color_reference{};
    color_reference.attachment = 0;
    color_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_reference;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    create_info.attachmentCount = 1;
    create_info.pAttachments = &color_attachment;
    create_info.subpassCount = 1;
    create_info.pSubpasses = &subpass;
    create_info.dependencyCount = 1;
    create_info.pDependencies = &dependency;

    return CheckVk(vkCreateRenderPass(_device, &create_info, nullptr, &_render_pass),
                   "vkCreateRenderPass");
}

bool VulkanContext::create_framebuffers()
{
    _framebuffers.resize(_swapchain_image_views.size(), VK_NULL_HANDLE);
    for (std::size_t index = 0; index < _swapchain_image_views.size(); ++index)
    {
        VkFramebufferCreateInfo create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        create_info.renderPass = _render_pass;
        create_info.attachmentCount = 1;
        create_info.pAttachments = &_swapchain_image_views[index];
        create_info.width = _swapchain_extent.width;
        create_info.height = _swapchain_extent.height;
        create_info.layers = 1;
        if (!CheckVk(vkCreateFramebuffer(_device, &create_info, nullptr,
                                         &_framebuffers[index]),
                     "vkCreateFramebuffer"))
        {
            return false;
        }
    }
    return true;
}

bool VulkanContext::create_command_pool()
{
    VkCommandPoolCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    create_info.queueFamilyIndex = _graphics_queue_family;
    return CheckVk(vkCreateCommandPool(_device, &create_info, nullptr, &_command_pool),
                   "vkCreateCommandPool");
}

bool VulkanContext::create_command_buffer()
{
    VkCommandBufferAllocateInfo allocate_info{};
    allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocate_info.commandPool = _command_pool;
    allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocate_info.commandBufferCount = 1;
    return CheckVk(vkAllocateCommandBuffers(_device, &allocate_info, &_command_buffer),
                   "vkAllocateCommandBuffers");
}

bool VulkanContext::create_sync_objects()
{
    destroy_sync_objects();
    if (_swapchain_images.empty())
    {
        return false;
    }

    VkSemaphoreCreateInfo semaphore_info{};
    semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    VkFenceCreateInfo fence_info{};
    fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    if (!CheckVk(vkCreateSemaphore(_device, &semaphore_info, nullptr,
                                   &_image_available_semaphore),
                 "vkCreateSemaphore"))
    {
        return false;
    }

    _render_finished_semaphores.assign(_swapchain_images.size(), VK_NULL_HANDLE);
    for (VkSemaphore &semaphore : _render_finished_semaphores)
    {
        if (!CheckVk(vkCreateSemaphore(_device, &semaphore_info, nullptr, &semaphore),
                     "vkCreateSemaphore"))
        {
            return false;
        }
    }
    return CheckVk(vkCreateFence(_device, &fence_info, nullptr, &_in_flight_fence),
                   "vkCreateFence");
}

bool VulkanContext::recreate_swapchain()
{
    int width = 0;
    int height = 0;
    glfwGetFramebufferSize(_window, &width, &height);
    while (width == 0 || height == 0)
    {
        glfwWaitEvents();
        glfwGetFramebufferSize(_window, &width, &height);
    }

    if (!CheckVk(vkDeviceWaitIdle(_device), "vkDeviceWaitIdle"))
    {
        return false;
    }

    destroy_sync_objects();
    cleanup_swapchain();

    if (!create_swapchain() || !create_image_views() || !create_render_pass() ||
        !create_framebuffers() || !create_sync_objects())
    {
        return false;
    }

    ++_swapchain_generation;
    return true;
}

void VulkanContext::cleanup_swapchain() noexcept
{
    if (_device == VK_NULL_HANDLE)
    {
        return;
    }

    for (const VkFramebuffer FRAMEBUFFER : _framebuffers)
    {
        if (FRAMEBUFFER != VK_NULL_HANDLE)
        {
            vkDestroyFramebuffer(_device, FRAMEBUFFER, nullptr);
        }
    }
    _framebuffers.clear();

    if (_render_pass != VK_NULL_HANDLE)
    {
        vkDestroyRenderPass(_device, _render_pass, nullptr);
        _render_pass = VK_NULL_HANDLE;
    }

    for (const VkImageView IMAGE_VIEW : _swapchain_image_views)
    {
        if (IMAGE_VIEW != VK_NULL_HANDLE)
        {
            vkDestroyImageView(_device, IMAGE_VIEW, nullptr);
        }
    }
    _swapchain_image_views.clear();

    if (_swapchain != VK_NULL_HANDLE)
    {
        vkDestroySwapchainKHR(_device, _swapchain, nullptr);
        _swapchain = VK_NULL_HANDLE;
    }
    _swapchain_images.clear();
}

void VulkanContext::destroy_sync_objects() noexcept
{
    if (_device == VK_NULL_HANDLE)
    {
        _image_available_semaphore = VK_NULL_HANDLE;
        _render_finished_semaphores.clear();
        _in_flight_fence = VK_NULL_HANDLE;
        return;
    }

    if (_in_flight_fence != VK_NULL_HANDLE)
    {
        vkDestroyFence(_device, _in_flight_fence, nullptr);
        _in_flight_fence = VK_NULL_HANDLE;
    }
    for (const VkSemaphore SEMAPHORE : _render_finished_semaphores)
    {
        if (SEMAPHORE != VK_NULL_HANDLE)
        {
            vkDestroySemaphore(_device, SEMAPHORE, nullptr);
        }
    }
    _render_finished_semaphores.clear();

    if (_image_available_semaphore != VK_NULL_HANDLE)
    {
        vkDestroySemaphore(_device, _image_available_semaphore, nullptr);
        _image_available_semaphore = VK_NULL_HANDLE;
    }
}

} // namespace k_engine::editor::gfx
