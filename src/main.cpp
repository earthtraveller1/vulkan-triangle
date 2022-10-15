#include <GLFW/glfw3.h>
#include <fmt/format.h>
#include <vulkan/vulkan.h>

#ifdef _WIN32
#include <Windows.h>
#endif

#include <cstdlib>

#include <vector>

#define LOAD_VK_FUNCTION(function, instance)                                   \
    const auto hello56721_##function = reinterpret_cast<PFN_##function>(       \
        vkGetInstanceProcAddr(instance, #function))

namespace
{

const uint16_t WINDOW_WIDTH = 1024;
const uint16_t WINDOW_HEIGHT = 768;

const bool ENABLE_VALIDATION = true;

VkBool32 debug_messenger_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT, VkDebugUtilsMessageTypeFlagsEXT,
    const VkDebugUtilsMessengerCallbackDataEXT *p_callback_data, void *)
{
    fmt::print("[VULKAN]: {}\n", p_callback_data->pMessage);

    return VK_FALSE;
}

const VkDebugUtilsMessengerCreateInfoEXT DEBUG_MESSENGER_CREATE_INFO{
    .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
    .pNext = nullptr,
    .flags = 0,
    .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT,
    .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                   VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                   VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
    .pfnUserCallback = debug_messenger_callback,
    .pUserData = nullptr};

void glfw_error_callback(int p_error_code, const char *p_message)
{
    fmt::print("[GLFW ERROR {}]: {}\n", p_error_code, p_message);
}

VkInstance create_instance()
{
    VkApplicationInfo application_info{
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pNext = nullptr,
        .pApplicationName = "Vulkan Triangle",
        .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
        .pEngineName = nullptr,
        .engineVersion = 0,
        .apiVersion = VK_API_VERSION_1_2};

    uint32_t glfw_vulkan_extension_count;
    const char **glfw_vulkan_extensions =
        glfwGetRequiredInstanceExtensions(&glfw_vulkan_extension_count);

    std::vector<const char *> enabled_extensions(glfw_vulkan_extensions,
                                             glfw_vulkan_extensions +
                                                 glfw_vulkan_extension_count);
    if (ENABLE_VALIDATION)
    {
        enabled_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    std::vector<const char *> enabled_layers;
    if (ENABLE_VALIDATION)
    {
        enabled_layers.push_back("VK_LAYER_KHRONOS_validation");
    }
    
    fmt::print("[INFO]: Enabling the following extensions:\n");
    for (const auto& extension: enabled_extensions)
    {
        fmt::print("\t{}\n", extension);
    }

    VkInstanceCreateInfo create_info{
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .pApplicationInfo = &application_info,
        .enabledLayerCount = static_cast<uint32_t>(enabled_layers.size()),
        .ppEnabledLayerNames = enabled_layers.data(),
        .enabledExtensionCount =
            static_cast<uint32_t>(enabled_extensions.size()),
        .ppEnabledExtensionNames = enabled_extensions.data()};

    if (ENABLE_VALIDATION)
    {
        create_info.pNext = &DEBUG_MESSENGER_CREATE_INFO;
    }

    VkInstance instance;
    VkResult result = vkCreateInstance(&create_info, nullptr, &instance);
    if (result != VK_SUCCESS)
    {
        fmt::print("[FATAL ERROR]: Failed to create the Vulkan instance. "
                   "Vulkan Error {}",
                   result);
        std::exit(EXIT_FAILURE);
    }

    return instance;
}

VkDebugUtilsMessengerEXT create_debug_messenger(VkInstance p_instance)
{
    LOAD_VK_FUNCTION(vkCreateDebugUtilsMessengerEXT, p_instance);
    VkDebugUtilsMessengerEXT debug_messenger;
    VkResult result = hello56721_vkCreateDebugUtilsMessengerEXT(
        p_instance, &DEBUG_MESSENGER_CREATE_INFO, nullptr, &debug_messenger);
    if (result != VK_SUCCESS)
    {
        fmt::print("[FATAL ERROR]: Failed to create the debug messenger. "
                   "Vulkan error {}",
                   result);
        std::exit(EXIT_FAILURE);
    }

    return debug_messenger;
}

// The actual main function
int real_main()
{
    if (!glfwInit())
    {
        fmt::print("[FATAL ERROR]: Failed to initialize GLFW.\n");
        return EXIT_FAILURE;
    }

    VkInstance instance = create_instance();

    VkDebugUtilsMessengerEXT debug_messenger;
    if (ENABLE_VALIDATION)
    {
        debug_messenger = create_debug_messenger(instance);
    }

    glfwSetErrorCallback(glfw_error_callback);

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    GLFWwindow *window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT,
                                          "Vulkan Triangle", nullptr, nullptr);
    if (window == nullptr)
    {
        fmt::print("[FATAL ERROR]: Failed to create the GLFW window.\n");
        glfwTerminate();
        return EXIT_FAILURE;
    }

    glfwShowWindow(window);

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
    }

    if (ENABLE_VALIDATION)
    {
        LOAD_VK_FUNCTION(vkDestroyDebugUtilsMessengerEXT, instance);
        hello56721_vkDestroyDebugUtilsMessengerEXT(instance, debug_messenger,
                                                   nullptr);
    }

    vkDestroyInstance(instance, nullptr);

    glfwTerminate();
    return EXIT_SUCCESS;
}

} // namespace

int main() { return real_main(); }

#ifdef _WIN32
int WINAPI wWinMain(HINSTANCE, HINSTANCE, PWSTR, int) { return real_main(); }
#endif