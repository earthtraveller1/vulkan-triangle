#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <fmt/format.h>

#ifdef _WIN32
#include <Windows.h>
#endif

#include <cstdlib>

#include <optional>
#include <set>
#include <tuple>
#include <vector>

#define LOAD_VK_FUNCTION(function, instance)                                   \
    const auto hello56721_##function = reinterpret_cast<PFN_##function>(       \
        vkGetInstanceProcAddr(instance, #function))

namespace
{

const uint16_t WINDOW_WIDTH = 1024;
const uint16_t WINDOW_HEIGHT = 768;

const bool ENABLE_VALIDATION = false;

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

    std::vector<const char *> enabled_extensions(
        glfw_vulkan_extensions,
        glfw_vulkan_extensions + glfw_vulkan_extension_count);
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
    for (const auto &extension : enabled_extensions)
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

VkSurfaceKHR create_surface(VkInstance p_instance, GLFWwindow *p_window)
{
    VkSurfaceKHR surface;
    VkResult result =
        glfwCreateWindowSurface(p_instance, p_window, nullptr, &surface);
    if (result != VK_SUCCESS)
    {
        fmt::print("[FATAL ERROR]: Failed to create the window surface. Vulkan "
                   "error {}",
                   result);
        std::exit(EXIT_FAILURE);
    }

    return surface;
}

auto find_queue_families(VkPhysicalDevice p_physical_device,
                         VkSurfaceKHR p_surface)
    -> std::tuple<std::optional<uint32_t>, std::optional<uint32_t>>
{
    std::optional<uint32_t> graphics_family;
    std::optional<uint32_t> present_family;

    uint32_t queue_family_count;
    vkGetPhysicalDeviceQueueFamilyProperties(p_physical_device,
                                             &queue_family_count, nullptr);

    std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(
        p_physical_device, &queue_family_count, queue_families.data());

    for (uint32_t i = 0; i < queue_families.size(); i++)
    {
        if (queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            graphics_family = i;
        }

        VkBool32 present_support;
        vkGetPhysicalDeviceSurfaceSupportKHR(p_physical_device, i, p_surface,
                                             &present_support);

        if (present_support)
        {
            present_family = i;
        }
    }

    return {graphics_family, present_family};
}

VkPhysicalDevice pick_physical_device(VkInstance p_instance,
                                      VkSurfaceKHR p_surface)
{
    uint32_t physical_device_count;
    vkEnumeratePhysicalDevices(p_instance, &physical_device_count, nullptr);

    std::vector<VkPhysicalDevice> physical_devices(physical_device_count);
    vkEnumeratePhysicalDevices(p_instance, &physical_device_count,
                               physical_devices.data());

    std::vector<VkPhysicalDevice> usable_physical_devices;
    for (const auto &physical_device : physical_devices)
    {
        auto [graphics_family, present_family] =
            find_queue_families(physical_device, p_surface);

        // A physical device must have both a present family and a graphics fa-
        // mily for it to be usable.
        if (graphics_family.has_value() && present_family.has_value())
        {
            usable_physical_devices.push_back(physical_device);
        }
    }

    VkPhysicalDevice chosen_device = usable_physical_devices[0];
    for (const auto &physical_device : usable_physical_devices)
    {
        VkPhysicalDeviceProperties device_properties;
        vkGetPhysicalDeviceProperties(physical_device, &device_properties);

        // This is assuming that you either have a single dedicated GPU, a ded-
        // icated GPU and an integrated GPU, or a single integrated GPU and no-
        // thing else.
        if (device_properties.deviceType ==
            VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
        {
            chosen_device = physical_device;
            break;
        }
    }

    VkPhysicalDeviceProperties device_properties;
    vkGetPhysicalDeviceProperties(chosen_device, &device_properties);

    fmt::print("[INFO]: We chose to use the {} graphics card.\n",
               device_properties.deviceName);

    return chosen_device;
}

// Return values:
// - Logical device handle
// - Graphics queue handle
// - Present queue handle
auto create_logical_device(VkPhysicalDevice p_physical_device,
                           std::uint32_t p_graphics_family,
                           std::uint32_t p_present_family)
    -> std::tuple<VkDevice, VkQueue, VkQueue>
{
    auto queue_create_infos = std::vector<VkDeviceQueueCreateInfo>();

    const auto queue_priority = 1.0f;

    if (p_graphics_family == p_present_family)
    {
        const auto queue_create_info = VkDeviceQueueCreateInfo{
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .flags = 0,
            .queueFamilyIndex = p_graphics_family,
            .queueCount = 1,
            .pQueuePriorities = &queue_priority,
        };

        queue_create_infos.push_back(queue_create_info);
    }
    else
    {
        auto queue_create_info = VkDeviceQueueCreateInfo{
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .flags = 0,
            .queueFamilyIndex = p_graphics_family,
            .queueCount = 1,
            .pQueuePriorities = &queue_priority,
        };

        queue_create_infos.push_back(queue_create_info);

        queue_create_info.queueFamilyIndex = p_present_family;
        queue_create_infos.push_back(queue_create_info);
    }

    const auto create_info =
        VkDeviceCreateInfo{.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
                           .pNext = nullptr,
                           .flags = 0,
                           .queueCreateInfoCount =
                               static_cast<uint32_t>(queue_create_infos.size()),
                           .pQueueCreateInfos = queue_create_infos.data(),
                           .enabledLayerCount = 0,
                           .ppEnabledLayerNames = nullptr,
                           .enabledExtensionCount = 0,
                           .ppEnabledExtensionNames = nullptr,
                           .pEnabledFeatures = nullptr};

    auto device = static_cast<VkDevice>(nullptr);
    const auto result =
        vkCreateDevice(p_physical_device, &create_info, nullptr, &device);
    if (result != VK_SUCCESS)
    {
        fmt::print("[FATAL ERROR]: Failed to create the logical device.\n");
        std::exit(EXIT_FAILURE);
    }

    auto graphics_queue = static_cast<VkQueue>(nullptr);
    auto present_queue = static_cast<VkQueue>(nullptr);

    vkGetDeviceQueue(device, p_graphics_family, 0, &graphics_queue);
    vkGetDeviceQueue(device, p_present_family, 0, &present_queue);

    return {device, graphics_queue, present_queue};
}

// The actual main function
int real_main()
{
    if (!glfwInit())
    {
        fmt::print("[FATAL ERROR]: Failed to initialize GLFW.\n");
        return EXIT_FAILURE;
    }

    const VkInstance instance = create_instance();

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

    const VkSurfaceKHR surface = create_surface(instance, window);
    const VkPhysicalDevice physical_device =
        pick_physical_device(instance, surface);

    const auto [graphics_queue_family_opt, present_queue_family_opt] =
        find_queue_families(physical_device, surface);
    const auto graphics_queue_family = graphics_queue_family_opt.value();
    const auto present_queue_family = present_queue_family_opt.value();

    const auto [device, graphics_queue, present_queue] = create_logical_device(
        physical_device, graphics_queue_family, present_queue_family);

    glfwShowWindow(window);

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
    }
    
    vkDestroyDevice(device, nullptr);
    vkDestroySurfaceKHR(instance, surface, nullptr);

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