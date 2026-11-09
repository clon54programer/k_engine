#include <cstdint>
#include <iostream>

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

int main()
{
    int glfw_major = 0;
    int glfw_minor = 0;
    int glfw_revision = 0;
    glfwGetVersion(&glfw_major, &glfw_minor, &glfw_revision);

    std::uint32_t vulkan_version = 0;
    if (vkEnumerateInstanceVersion(&vulkan_version) != VK_SUCCESS)
    {
        std::cerr << "No se pudo consultar la versión del loader de Vulkan.\n";
        return 1;
    }

    std::cout << "GLFW " << glfw_major << '.' << glfw_minor << '.' << glfw_revision
              << "; Vulkan loader " << VK_VERSION_MAJOR(vulkan_version) << '.'
              << VK_VERSION_MINOR(vulkan_version) << '.'
              << VK_VERSION_PATCH(vulkan_version) << '\n';
    return 0;
}
