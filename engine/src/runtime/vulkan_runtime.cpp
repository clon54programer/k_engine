#include "runtime/vulkan_runtime.h"

#include <algorithm>
#include <array>
#include <cmath>
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

#include "resource/pak_reader.h"

namespace k_engine::runtime
{

namespace
{

constexpr std::uint32_t DEFAULT_WINDOW_WIDTH = 1280;
constexpr std::uint32_t DEFAULT_WINDOW_HEIGHT = 720;
constexpr std::uint32_t IMAGE_COUNT_EXTRA = 1;
constexpr std::uint32_t DEVICE_QUEUE_INDEX = 0;
constexpr float DEVICE_QUEUE_PRIORITY = 1.0F;
constexpr std::uint32_t AMD_VENDOR_ID = 0x1002;
constexpr std::uint32_t AMD_RENOIR_DEVICE_ID = 0x15E7;
constexpr const char *VALIDATION_LAYER_NAME = "VK_LAYER_KHRONOS_validation";
constexpr const char *WINDOW_TITLE = "K_engine — runtime Vulkan T2";
constexpr float CLEAR_COLOR_RED_BASE = 0.05F;
constexpr float CLEAR_COLOR_GREEN_BASE = 0.08F;
constexpr float CLEAR_COLOR_BLUE_BASE = 0.14F;
constexpr float CLEAR_COLOR_AMPLITUDE = 0.04F;
constexpr double SPRITE_MOVE_SPEED = 200.0;
constexpr double MAX_FRAME_DELTA_SECONDS = 0.1;
constexpr double HALF_DIVISOR = 2.0;

constexpr std::array<VkCompositeAlphaFlagBitsKHR, 4> COMPOSITE_ALPHA_CANDIDATES = {
    VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
    VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
    VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
    VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
};

constexpr VkPipelineStageFlags WAIT_STAGE = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

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
        if (format.format == VK_FORMAT_B8G8R8A8_SRGB &&
            format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            return format;
        }
    }

    if (formats.size() == 1 && formats.front().format == VK_FORMAT_UNDEFINED)
    {
        return VkSurfaceFormatKHR{VK_FORMAT_B8G8R8A8_SRGB,
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
    // La API de Vulkan entrega funciones mediante vkGetInstanceProcAddr y exige
    // esta conversión; no se usa para convertir datos.
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
              << "): " << (description != nullptr ? description : "sin descripción")
              << '\n';
}

} // namespace

VulkanRuntime::VulkanRuntime() = default;

VulkanRuntime::~VulkanRuntime()
{
    Shutdown();
}

bool VulkanRuntime::Initialize(const RuntimeOptions &options)
{
    _shutdown_complete = false;
    _engine_error_count = 0;
    _engine_warning_count = 0;
    _icd_display_warning_count = 0;

#ifndef NDEBUG
    _validation_enabled = true;
#else
    _validation_enabled = false;
#endif

    if (!initialize_glfw())
    {
        return false;
    }

    if (!create_instance())
    {
        return false;
    }

    if (!create_debug_messenger())
    {
        return false;
    }

    if (!create_window())
    {
        return false;
    }

    if (!create_surface())
    {
        return false;
    }

    if (!pick_physical_device())
    {
        return false;
    }

    if (!create_logical_device())
    {
        return false;
    }

    if (!create_swapchain())
    {
        return false;
    }

    if (!create_image_views())
    {
        return false;
    }

    if (!create_render_pass())
    {
        return false;
    }

    if (!create_framebuffers())
    {
        return false;
    }

    if (!create_command_pool())
    {
        return false;
    }

    if (!load_content(options))
    {
        return false;
    }

    if (_has_content && !create_sprite_renderer())
    {
        return false;
    }

    if (!create_command_buffer())
    {
        return false;
    }

    if (!create_sync_objects())
    {
        return false;
    }

    _last_frame_time = glfwGetTime();
    return true;
}

int VulkanRuntime::Run(const RuntimeOptions &options)
{
    if (_window == nullptr || _device == VK_NULL_HANDLE)
    {
        LogError("El runtime no está inicializado.");
        return 1;
    }

    _last_frame_time = glfwGetTime();
    std::uint32_t frames_presented = 0;
    while (glfwWindowShouldClose(_window) == GLFW_FALSE &&
           (options.frame_limit == 0 || frames_presented < options.frame_limit))
    {
        std::uint32_t image_index = 0;
        bool frame_skipped = false;
        if (!prepare_frame(image_index, frame_skipped))
        {
            return 1;
        }

        if (frame_skipped)
        {
            continue;
        }

        VkResult present_result = VK_SUCCESS;
        if (!submit_frame(image_index, present_result))
        {
            return 1;
        }

        ++frames_presented;
        if (!handle_frame_result(present_result))
        {
            return 1;
        }
    }

    if (!CheckVk(vkDeviceWaitIdle(_device), "vkDeviceWaitIdle"))
    {
        return 1;
    }

    if (_validation_enabled && (_engine_error_count > 0 || _engine_warning_count > 0))
    {
        LogError("La validación registró mensajes atribuibles al engine.");
        return 1;
    }

    return 0;
}

// El primer parámetro es el índice de imagen y el segundo indica si se omitió
// el frame; se mantienen separados para no introducir un tipo auxiliar.
// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
bool VulkanRuntime::prepare_frame(std::uint32_t &image_index, bool &frame_skipped)
{
    frame_skipped = false;
    glfwPollEvents();

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
        frame_skipped = true;
        return true;
    }

    const VkResult WAIT_RESULT =
        vkWaitForFences(_device, 1, &_in_flight_fence, VK_TRUE,
                        std::numeric_limits<std::uint64_t>::max());
    if (!CheckVk(WAIT_RESULT, "vkWaitForFences"))
    {
        return false;
    }

    const VkResult ACQUIRE_RESULT = vkAcquireNextImageKHR(
        _device, _swapchain, std::numeric_limits<std::uint64_t>::max(),
        _image_available_semaphore, VK_NULL_HANDLE, &image_index);

    if (ACQUIRE_RESULT == VK_ERROR_OUT_OF_DATE_KHR)
    {
        if (!recreate_swapchain())
        {
            return false;
        }
        frame_skipped = true;
        return true;
    }

    if (ACQUIRE_RESULT != VK_SUCCESS && ACQUIRE_RESULT != VK_SUBOPTIMAL_KHR)
    {
        LogError("vkAcquireNextImageKHR falló con VkResult " +
                 std::to_string(static_cast<int>(ACQUIRE_RESULT)) + ".");
        return false;
    }

    if (!CheckVk(vkResetFences(_device, 1, &_in_flight_fence), "vkResetFences"))
    {
        return false;
    }

    if (!CheckVk(vkResetCommandBuffer(_command_buffer, 0), "vkResetCommandBuffer"))
    {
        return false;
    }

    return record_command_buffer(_command_buffer, image_index);
}

bool VulkanRuntime::submit_frame(std::uint32_t image_index, VkResult &present_result)
{
    if (image_index >= _render_finished_semaphores.size())
    {
        LogError("El índice de imagen del swapchain está fuera de rango.");
        return false;
    }

    VkSubmitInfo submit_info{};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = &_image_available_semaphore;
    submit_info.pWaitDstStageMask = &WAIT_STAGE;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &_command_buffer;
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = &_render_finished_semaphores[image_index];

    if (!CheckVk(vkQueueSubmit(_graphics_queue, 1, &submit_info, _in_flight_fence),
                 "vkQueueSubmit"))
    {
        return false;
    }

    VkPresentInfoKHR present_info{};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = &_render_finished_semaphores[image_index];
    present_info.swapchainCount = 1;
    present_info.pSwapchains = &_swapchain;
    present_info.pImageIndices = &image_index;

    present_result = vkQueuePresentKHR(_present_queue, &present_info);
    return true;
}

bool VulkanRuntime::handle_frame_result(VkResult present_result)
{
    if (present_result == VK_ERROR_OUT_OF_DATE_KHR ||
        present_result == VK_SUBOPTIMAL_KHR || _framebuffer_resized)
    {
        _framebuffer_resized = false;
        return recreate_swapchain();
    }

    if (present_result != VK_SUCCESS)
    {
        LogError("vkQueuePresentKHR falló con VkResult " +
                 std::to_string(static_cast<int>(present_result)) + ".");
        return false;
    }

    return true;
}

void VulkanRuntime::Shutdown() noexcept
{
    if (_shutdown_complete)
    {
        return;
    }

    if (_device != VK_NULL_HANDLE)
    {
        (void)vkDeviceWaitIdle(_device);
    }

    _sprite_renderer.Shutdown();
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

    _has_content = false;
    _content = LoadedContent{};
    _input = InputState{};
    report_validation_summary();
    _shutdown_complete = true;
}

bool VulkanRuntime::initialize_glfw()
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
        LogError("GLFW no está usando la plataforma X11.");
        return false;
    }

    return true;
}

bool VulkanRuntime::create_instance()
{
    std::uint32_t glfw_extension_count = 0;
    const char *const *glfw_extensions =
        glfwGetRequiredInstanceExtensions(&glfw_extension_count);
    if (glfw_extensions == nullptr || glfw_extension_count == 0)
    {
        LogError("GLFW no proporcionó las extensiones de instancia de Vulkan.");
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
            LogError(std::string("Falta la extensión de instancia ") + EXTENSION + ".");
            return false;
        }
    }

    std::vector<const char *> layers;
    if (_validation_enabled)
    {
        if (!HasInstanceLayer(VALIDATION_LAYER_NAME))
        {
            LogError(std::string("Falta la capa de validación ") + VALIDATION_LAYER_NAME +
                     ".");
            return false;
        }
        layers.push_back(VALIDATION_LAYER_NAME);
    }

    VkApplicationInfo application_info{};
    application_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    application_info.pApplicationName = "K_engine";
    application_info.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
    application_info.pEngineName = "K_engine";
    application_info.engineVersion = VK_MAKE_VERSION(0, 1, 0);
    application_info.apiVersion = VK_API_VERSION_1_3;

    VkDebugUtilsMessengerCreateInfoEXT debug_create_info{};
    debug_create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    debug_create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                                        VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
                                        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    debug_create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                    VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                    VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    debug_create_info.pfnUserCallback = &VulkanRuntime::debug_callback;
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

    LogInfo("Instancia Vulkan 1.3 creada.");
    return true;
}

bool VulkanRuntime::create_debug_messenger()
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
    create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                                  VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
                                  VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                  VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                              VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                              VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    create_info.pfnUserCallback = &VulkanRuntime::debug_callback;
    create_info.pUserData = this;

    if (!CheckVk(CREATE_MESSENGER(_instance, &create_info, nullptr, &_debug_messenger),
                 "vkCreateDebugUtilsMessengerEXT"))
    {
        return false;
    }

    LogInfo("Validation Layer y debug messenger activos.");
    return true;
}

bool VulkanRuntime::create_window()
{
    glfwDefaultWindowHints();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    _window = glfwCreateWindow(static_cast<int>(DEFAULT_WINDOW_WIDTH),
                               static_cast<int>(DEFAULT_WINDOW_HEIGHT), WINDOW_TITLE,
                               nullptr, nullptr);
    if (_window == nullptr)
    {
        LogError("No se pudo crear la ventana GLFW.");
        return false;
    }

    glfwSetWindowUserPointer(_window, this);
    glfwSetFramebufferSizeCallback(_window, &VulkanRuntime::framebuffer_size_callback);
    glfwSetKeyCallback(_window, &VulkanRuntime::key_callback);
    glfwSetCursorPosCallback(_window, &VulkanRuntime::cursor_position_callback);
    glfwSetMouseButtonCallback(_window, &VulkanRuntime::mouse_button_callback);
    return true;
}

bool VulkanRuntime::create_surface()
{
    const VkResult RESULT =
        glfwCreateWindowSurface(_instance, _window, nullptr, &_surface);
    if (!CheckVk(RESULT, "glfwCreateWindowSurface"))
    {
        return false;
    }

    LogInfo("Superficie Vulkan creada sobre X11.");
    return true;
}

bool VulkanRuntime::pick_physical_device()
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
        LogError("vkEnumeratePhysicalDevices falló al leer los dispositivos.");
        return false;
    }
    devices.resize(device_count);

    for (const VkPhysicalDevice DEVICE : devices)
    {
        VkPhysicalDeviceProperties properties{};
        vkGetPhysicalDeviceProperties(DEVICE, &properties);
        const std::string DEVICE_NAME(std::data(properties.deviceName));
        LogInfo(std::string("Dispositivo Vulkan: ") + DEVICE_NAME + " (vendor " +
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
            LogError("La GPU AMD no tiene una cola gráfica compatible con presentación.");
            continue;
        }

        _physical_device = DEVICE;
        _graphics_queue_family = family_index;
        LogInfo(std::string("GPU seleccionada: ") + DEVICE_NAME + ".");
        return true;
    }

    LogError("No se encontró la GPU AMD Radeon Graphics 1002:15e7 compatible.");
    return false;
}

bool VulkanRuntime::create_logical_device()
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
    LogInfo("Dispositivo lógico creado con la cola gráfica y de presentación.");
    return true;
}

bool VulkanRuntime::create_swapchain()
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
        LogError("La superficie no expone modos de presentación.");
        return false;
    }

    std::vector<VkPresentModeKHR> present_modes(present_mode_count);
    result = vkGetPhysicalDeviceSurfacePresentModesKHR(
        _physical_device, _surface, &present_mode_count, present_modes.data());
    if (result != VK_SUCCESS && result != VK_INCOMPLETE)
    {
        LogError("No se pudieron leer los modos de presentación.");
        return false;
    }
    present_modes.resize(present_mode_count);

    const VkSurfaceFormatKHR SURFACE_FORMAT = ChooseSurfaceFormat(formats);
    const VkPresentModeKHR PRESENT_MODE = ChoosePresentMode(present_modes);
    const VkExtent2D EXTENT = ChooseExtent(capabilities, _window);
    if (EXTENT.width == 0 || EXTENT.height == 0)
    {
        LogError("La ventana no tiene un tamaño válido para el swapchain.");
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
        LogError("No se pudieron leer las imágenes del swapchain.");
        return false;
    }
    _swapchain_images.resize(swapchain_image_count);

    _swapchain_format = SURFACE_FORMAT.format;
    _swapchain_extent = EXTENT;

    LogInfo("Swapchain creado: " + std::to_string(EXTENT.width) + "x" +
            std::to_string(EXTENT.height) + ", " + std::to_string(swapchain_image_count) +
            " imágenes, formato " + std::to_string(static_cast<int>(_swapchain_format)) +
            ".");
    return true;
}

bool VulkanRuntime::create_image_views()
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

bool VulkanRuntime::create_render_pass()
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
    dependency.srcAccessMask = 0;
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

bool VulkanRuntime::create_framebuffers()
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

bool VulkanRuntime::create_command_pool()
{
    VkCommandPoolCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    create_info.queueFamilyIndex = _graphics_queue_family;

    return CheckVk(vkCreateCommandPool(_device, &create_info, nullptr, &_command_pool),
                   "vkCreateCommandPool");
}

bool VulkanRuntime::create_command_buffer()
{
    VkCommandBufferAllocateInfo allocate_info{};
    allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocate_info.commandPool = _command_pool;
    allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocate_info.commandBufferCount = 1;

    return CheckVk(vkAllocateCommandBuffers(_device, &allocate_info, &_command_buffer),
                   "vkAllocateCommandBuffers");
}

bool VulkanRuntime::create_sync_objects()
{
    destroy_sync_objects();

    if (_swapchain_images.empty())
    {
        LogError("No hay imágenes de swapchain para crear la sincronización.");
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

void VulkanRuntime::destroy_sync_objects() noexcept
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

bool VulkanRuntime::load_content(const RuntimeOptions &options)
{
    _has_content = false;
    _content = LoadedContent{};

    if (options.package_path.empty())
    {
        LogInfo("Sin paquete: se presenta solo el color de limpieza.");
        return true;
    }

    resource::PakReader pak;
    if (!pak.Open(options.package_path))
    {
        LogError(pak.LastError());
        return false;
    }

    std::string error;
    if (!ContentLoader::Load(pak, options.level_id, _content, error))
    {
        LogError(error);
        return false;
    }

    _has_content = true;
    _sprite_x = static_cast<double>(_content.sprite_x);
    _sprite_y = static_cast<double>(_content.sprite_y);

    LogInfo("Paquete cargado: " + options.package_path + " (" +
            std::to_string(pak.Entries().size()) + " recursos).");
    LogInfo("Nivel cargado: " + std::to_string(_content.level_id) + " \"" +
            _content.level_name + "\".");
    LogInfo("Sprite cargado: PNG " + std::to_string(_content.sprite.image_id) + " (" +
            std::to_string(_content.sprite.width) + "x" +
            std::to_string(_content.sprite.height) + ").");
    return true;
}

bool VulkanRuntime::create_sprite_renderer()
{
    if (!_has_content)
    {
        return true;
    }

    if (!_sprite_renderer.Initialize(_physical_device, _device, _command_pool,
                                     _graphics_queue, _render_pass, _swapchain_extent,
                                     _content.sprite))
    {
        LogError("No se pudo crear el pipeline y la textura del sprite.");
        return false;
    }

    return true;
}

float VulkanRuntime::frame_delta_seconds() noexcept
{
    const double NOW = glfwGetTime();
    double delta = NOW - _last_frame_time;
    _last_frame_time = NOW;

    if (delta < 0.0)
    {
        delta = 0.0;
    }
    else if (delta > MAX_FRAME_DELTA_SECONDS)
    {
        delta = MAX_FRAME_DELTA_SECONDS;
    }

    return static_cast<float>(delta);
}

void VulkanRuntime::clamp_sprite_position()
{
    if (!_has_content)
    {
        return;
    }

    const auto WIDTH = static_cast<double>(_content.sprite.width);
    const auto HEIGHT = static_cast<double>(_content.sprite.height);
    const double MAX_X =
        std::max(0.0, static_cast<double>(_swapchain_extent.width) - WIDTH);
    const double MAX_Y =
        std::max(0.0, static_cast<double>(_swapchain_extent.height) - HEIGHT);

    _sprite_x = std::clamp(_sprite_x, 0.0, MAX_X);
    _sprite_y = std::clamp(_sprite_y, 0.0, MAX_Y);
}

void VulkanRuntime::update_scene(float delta_seconds)
{
    if (!_has_content)
    {
        return;
    }

    const double DISTANCE = SPRITE_MOVE_SPEED * static_cast<double>(delta_seconds);
    if (_input.IsPressed(InputAction::MOVE_LEFT))
    {
        _sprite_x -= DISTANCE;
    }
    if (_input.IsPressed(InputAction::MOVE_RIGHT))
    {
        _sprite_x += DISTANCE;
    }
    if (_input.IsPressed(InputAction::MOVE_UP))
    {
        _sprite_y -= DISTANCE;
    }
    if (_input.IsPressed(InputAction::MOVE_DOWN))
    {
        _sprite_y += DISTANCE;
    }

    double click_x = 0.0;
    double click_y = 0.0;
    if (_input.ConsumeLeftClick(click_x, click_y))
    {
        _sprite_x = click_x - (static_cast<double>(_content.sprite.width) / HALF_DIVISOR);
        _sprite_y =
            click_y - (static_cast<double>(_content.sprite.height) / HALF_DIVISOR);
    }

    clamp_sprite_position();
}

bool VulkanRuntime::recreate_swapchain()
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
    _sprite_renderer.DestroyPipeline();
    cleanup_swapchain();

    if (!create_swapchain() || !create_image_views() || !create_render_pass())
    {
        return false;
    }

    if (_has_content && !_sprite_renderer.CreatePipeline(_render_pass, _swapchain_extent))
    {
        return false;
    }

    return create_framebuffers() && create_sync_objects();
}

void VulkanRuntime::cleanup_swapchain() noexcept
{
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
    _swapchain_format = VK_FORMAT_UNDEFINED;
    _swapchain_extent = VkExtent2D{};
}

bool VulkanRuntime::record_command_buffer(VkCommandBuffer command_buffer,
                                          std::uint32_t image_index)
{
    if (image_index >= _framebuffers.size())
    {
        LogError("El índice de imagen del swapchain no tiene framebuffer.");
        return false;
    }

    update_scene(frame_delta_seconds());

    VkCommandBufferBeginInfo begin_info{};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (!CheckVk(vkBeginCommandBuffer(command_buffer, &begin_info),
                 "vkBeginCommandBuffer"))
    {
        return false;
    }

    const auto TIME = static_cast<float>(glfwGetTime());
    VkClearValue clear_value{};
    clear_value.color.float32[0] =
        CLEAR_COLOR_RED_BASE + CLEAR_COLOR_AMPLITUDE * std::sin(TIME);
    clear_value.color.float32[1] =
        CLEAR_COLOR_GREEN_BASE + CLEAR_COLOR_AMPLITUDE * std::cos(TIME);
    clear_value.color.float32[2] = CLEAR_COLOR_BLUE_BASE;
    clear_value.color.float32[3] = 1.0F;

    VkRenderPassBeginInfo render_pass_info{};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_info.renderPass = _render_pass;
    render_pass_info.framebuffer = _framebuffers[image_index];
    render_pass_info.renderArea.offset = VkOffset2D{0, 0};
    render_pass_info.renderArea.extent = _swapchain_extent;
    render_pass_info.clearValueCount = 1;
    render_pass_info.pClearValues = &clear_value;

    vkCmdBeginRenderPass(command_buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

    if (_has_content &&
        !_sprite_renderer.Draw(command_buffer, static_cast<float>(_sprite_x),
                               static_cast<float>(_sprite_y),
                               static_cast<float>(_content.sprite.width),
                               static_cast<float>(_content.sprite.height)))
    {
        LogError("No se pudo dibujar el sprite del paquete.");
        return false;
    }

    vkCmdEndRenderPass(command_buffer);

    return CheckVk(vkEndCommandBuffer(command_buffer), "vkEndCommandBuffer");
}

void VulkanRuntime::report_validation_summary() const
{
    if (!_validation_enabled)
    {
        return;
    }

    std::cout << "Validación: " << _engine_error_count << " errores, "
              << _engine_warning_count << " advertencias del engine, "
              << _icd_display_warning_count
              << " advertencias del ICD sobre VK_KHR_display.\n";
}

VKAPI_ATTR VkBool32 VKAPI_CALL VulkanRuntime::debug_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
    VkDebugUtilsMessageTypeFlagsEXT message_types,
    const VkDebugUtilsMessengerCallbackDataEXT *callback_data, void *user_data)
{
    auto *runtime = static_cast<VulkanRuntime *>(user_data);
    if (runtime != nullptr)
    {
        runtime->handle_debug_message(message_severity, message_types, callback_data);
    }

    return VK_FALSE;
}

void VulkanRuntime::handle_debug_message(
    VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
    VkDebugUtilsMessageTypeFlagsEXT message_types,
    const VkDebugUtilsMessengerCallbackDataEXT *callback_data)
{
    const char *const MESSAGE =
        (callback_data != nullptr && callback_data->pMessage != nullptr)
            ? callback_data->pMessage
            : "";

    if (IsIcdDisplayMessage(MESSAGE))
    {
        ++_icd_display_warning_count;
        std::cerr << "[ICD VK_KHR_display] " << MESSAGE << '\n';
        return;
    }

    if ((message_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) != 0)
    {
        ++_engine_error_count;
        std::cerr << "[Validación ERROR] " << MESSAGE << '\n';
    }
    else if ((message_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) != 0)
    {
        ++_engine_warning_count;
        std::cerr << "[Validación WARNING] " << MESSAGE << '\n';
    }
    else if ((message_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) != 0 &&
             (message_types & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT) != 0)
    {
        std::cout << "[Validación INFO] " << MESSAGE << '\n';
    }
}

// NOLINTBEGIN(bugprone-easily-swappable-parameters)
// Las firmas de estas callbacks están impuestas por GLFW.
void VulkanRuntime::framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    (void)width;
    (void)height;

    auto *runtime = static_cast<VulkanRuntime *>(glfwGetWindowUserPointer(window));
    if (runtime != nullptr)
    {
        runtime->_framebuffer_resized = true;
    }
}

void VulkanRuntime::key_callback(GLFWwindow *window, int key, int scancode, int action,
                                 int mods)
{
    (void)scancode;
    (void)mods;

    auto *runtime = static_cast<VulkanRuntime *>(glfwGetWindowUserPointer(window));
    if (runtime == nullptr)
    {
        return;
    }

    const bool PRESSED = action != GLFW_RELEASE;
    switch (key)
    {
    case GLFW_KEY_A:
    case GLFW_KEY_LEFT:
        runtime->_input.SetPressed(InputAction::MOVE_LEFT, PRESSED);
        break;
    case GLFW_KEY_D:
    case GLFW_KEY_RIGHT:
        runtime->_input.SetPressed(InputAction::MOVE_RIGHT, PRESSED);
        break;
    case GLFW_KEY_W:
    case GLFW_KEY_UP:
        runtime->_input.SetPressed(InputAction::MOVE_UP, PRESSED);
        break;
    case GLFW_KEY_S:
    case GLFW_KEY_DOWN:
        runtime->_input.SetPressed(InputAction::MOVE_DOWN, PRESSED);
        break;
    case GLFW_KEY_ESCAPE:
        if (action == GLFW_PRESS)
        {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }
        break;
    default:
        break;
    }
}

void VulkanRuntime::cursor_position_callback(GLFWwindow *window, double cursor_x,
                                             double cursor_y)
{
    auto *runtime = static_cast<VulkanRuntime *>(glfwGetWindowUserPointer(window));
    if (runtime != nullptr)
    {
        runtime->_input.SetCursor(cursor_x, cursor_y);
    }
}

void VulkanRuntime::mouse_button_callback(GLFWwindow *window, int button, int action,
                                          int mods)
{
    (void)mods;

    if (button != GLFW_MOUSE_BUTTON_LEFT || action != GLFW_PRESS)
    {
        return;
    }

    auto *runtime = static_cast<VulkanRuntime *>(glfwGetWindowUserPointer(window));
    if (runtime == nullptr)
    {
        return;
    }

    int window_width = 0;
    int window_height = 0;
    int framebuffer_width = 0;
    int framebuffer_height = 0;
    glfwGetWindowSize(window, &window_width, &window_height);
    glfwGetFramebufferSize(window, &framebuffer_width, &framebuffer_height);

    double cursor_x = 0.0;
    double cursor_y = 0.0;
    glfwGetCursorPos(window, &cursor_x, &cursor_y);

    const double SCALE_X =
        window_width > 0 ? static_cast<double>(framebuffer_width) / window_width : 1.0;
    const double SCALE_Y =
        window_height > 0 ? static_cast<double>(framebuffer_height) / window_height : 1.0;

    runtime->_input.RequestLeftClick(cursor_x * SCALE_X, cursor_y * SCALE_Y);
}
// NOLINTEND(bugprone-easily-swappable-parameters)

} // namespace k_engine::runtime
