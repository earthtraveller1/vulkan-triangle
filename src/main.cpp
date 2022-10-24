#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <fmt/format.h>

#ifdef _WIN32
#include <Windows.h>
#endif

#include <cstdint>
#include <cstdlib>

#include <algorithm>
#include <array>
#include <limits>
#include <optional>
#include <set>
#include <tuple>
#include <vector>

#define LOAD_VK_FUNCTION(function, instance)                                   \
    const auto hello56721_##function = reinterpret_cast<PFN_##function>(       \
        vkGetInstanceProcAddr(instance, #function))

namespace
{

constexpr uint16_t WINDOW_WIDTH = 1024;
constexpr uint16_t WINDOW_HEIGHT = 768;

constexpr bool ENABLE_VALIDATION = true;

constexpr auto DEVICE_EXTENSIONS =
    std::array<const char*, 1>{VK_KHR_SWAPCHAIN_EXTENSION_NAME};

struct swap_chain_support_details_t
{
    VkSurfaceCapabilitiesKHR surface_capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> present_modes;
};

auto do_nothing() {}

VkBool32 debug_messenger_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT p_severity,
    VkDebugUtilsMessageTypeFlagsEXT,
    const VkDebugUtilsMessengerCallbackDataEXT* p_callback_data, void*)
{
    if (p_severity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
    {
        do_nothing();
    }

    fmt::print("[VULKAN]: {}\n", p_callback_data->pMessage);

    return VK_FALSE;
}

constexpr VkDebugUtilsMessengerCreateInfoEXT DEBUG_MESSENGER_CREATE_INFO{
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

void glfw_error_callback(int p_error_code, const char* p_message)
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
    const char** glfw_vulkan_extensions =
        glfwGetRequiredInstanceExtensions(&glfw_vulkan_extension_count);

    std::vector<const char*> enabled_extensions(
        glfw_vulkan_extensions,
        glfw_vulkan_extensions + glfw_vulkan_extension_count);
    if (ENABLE_VALIDATION)
    {
        enabled_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    std::vector<const char*> enabled_layers;
    if (ENABLE_VALIDATION)
    {
        enabled_layers.push_back("VK_LAYER_KHRONOS_validation");
    }

    fmt::print("[INFO]: Enabling the following extensions:\n");
    for (const auto& extension : enabled_extensions)
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

VkSurfaceKHR create_surface(VkInstance p_instance, GLFWwindow* p_window)
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

auto query_swap_chain_support_details(VkPhysicalDevice p_physical_device,
                                      VkSurfaceKHR p_surface)
    -> swap_chain_support_details_t
{
    auto support_details = swap_chain_support_details_t{};

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
        p_physical_device, p_surface, &support_details.surface_capabilities);

    auto format_count = static_cast<uint32_t>(0);
    vkGetPhysicalDeviceSurfaceFormatsKHR(p_physical_device, p_surface,
                                         &format_count, nullptr);

    if (format_count != 0)
    {
        support_details.formats.resize(format_count);
        vkGetPhysicalDeviceSurfaceFormatsKHR(p_physical_device, p_surface,
                                             &format_count,
                                             support_details.formats.data());
    }

    auto present_mode_count = static_cast<uint32_t>(0);
    vkGetPhysicalDeviceSurfacePresentModesKHR(p_physical_device, p_surface,
                                              &present_mode_count, nullptr);

    if (present_mode_count != 0)
    {
        support_details.present_modes.resize(present_mode_count);
        vkGetPhysicalDeviceSurfacePresentModesKHR(
            p_physical_device, p_surface, &present_mode_count,
            support_details.present_modes.data());
    }

    return support_details;
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
    for (const auto& physical_device : physical_devices)
    {
        auto [graphics_family, present_family] =
            find_queue_families(physical_device, p_surface);

        auto has_required_extensions = true;

        auto available_extension_count = static_cast<std::uint32_t>(0);
        vkEnumerateDeviceExtensionProperties(
            physical_device, nullptr, &available_extension_count, nullptr);

        auto available_extensions =
            std::vector<VkExtensionProperties>(available_extension_count);
        vkEnumerateDeviceExtensionProperties(physical_device, nullptr,
                                             &available_extension_count,
                                             available_extensions.data());

        auto device_properties = VkPhysicalDeviceProperties{};
        vkGetPhysicalDeviceProperties(physical_device, &device_properties);

        fmt::print("[INFO]: Found physical device {}\n",
                   device_properties.deviceName);

        for (const auto& extension : DEVICE_EXTENSIONS)
        {
            auto extension_found = false;

            for (const auto& available_extension : available_extensions)
            {
                if (std::strcmp(available_extension.extensionName, extension) ==
                    0)
                {
                    extension_found = true;
                    break;
                }
            }

            if (!extension_found)
            {
                has_required_extensions = false;
                break;
            }
        }

        auto swap_chain_adequate = false;

        if (has_required_extensions && graphics_family.has_value() &&
            present_family.has_value())
        {
            const auto [surface_capabilities, formats, present_modes] =
                query_swap_chain_support_details(physical_device, p_surface);

            swap_chain_adequate = !formats.empty() && !present_modes.empty();
        }

        // A physical device must have both a present family and a graphics fa-
        // mily for it to be usable. And it must have all the required extensi-
        // ons, plus an adequate swap chain.
        if (graphics_family.has_value() && present_family.has_value() &&
            has_required_extensions && swap_chain_adequate)
        {
            usable_physical_devices.push_back(physical_device);
        }
    }

    if (usable_physical_devices.empty())
    {
        fmt::print(
            "[FATAL ERROR]: Failed to find any adequate physical devices!\n");
        std::exit(EXIT_FAILURE);
    }

    VkPhysicalDevice chosen_device = usable_physical_devices[0];
    for (const auto& physical_device : usable_physical_devices)
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
                           .enabledExtensionCount =
                               static_cast<uint32_t>(DEVICE_EXTENSIONS.size()),
                           .ppEnabledExtensionNames = DEVICE_EXTENSIONS.data(),
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

// The return types are like this
// - Format
// - Present Mode
// - Extent
auto choose_swap_chain_settings(
    GLFWwindow* p_window,
    const std::vector<VkSurfaceFormatKHR>& p_available_formats,
    const std::vector<VkPresentModeKHR>& p_available_present_modes,
    const VkSurfaceCapabilitiesKHR& p_surface_capabilties)
    -> std::tuple<VkSurfaceFormatKHR, VkPresentModeKHR, VkExtent2D>
{
    auto chosen_format = p_available_formats[0];

    for (const auto& format : p_available_formats)
    {
        if (format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR &&
            format.format == VK_FORMAT_B8G8R8A8_SRGB)
        {
            chosen_format = format;
            break;
        }
    }

    auto chosen_present_mode = VK_PRESENT_MODE_FIFO_KHR;

    for (const auto& present_mode : p_available_present_modes)
    {
        if (present_mode == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            chosen_present_mode = present_mode;
            fmt::print(
                "[INFO]: YAY! We get to use VK_PRESENT_MODE_MAILBOX_KHR!\n");
        }
    }

    auto swap_chain_extent = VkExtent2D{};

    if (p_surface_capabilties.currentExtent.width ==
        (std::numeric_limits<std::uint32_t>::max)())
    {
        auto width = 0, height = 0;
        glfwGetFramebufferSize(p_window, &width, &height);

        swap_chain_extent.width =
            std::clamp(static_cast<uint32_t>(width),
                       p_surface_capabilties.minImageExtent.width,
                       p_surface_capabilties.maxImageExtent.width);
        swap_chain_extent.height =
            std::clamp(static_cast<uint32_t>(height),
                       p_surface_capabilties.minImageExtent.height,
                       p_surface_capabilties.maxImageExtent.height);
    }
    else
    {
        swap_chain_extent = p_surface_capabilties.currentExtent;
    }

    return {chosen_format, chosen_present_mode, swap_chain_extent};
}

auto create_swap_chain(VkPhysicalDevice p_physical_device,
                       VkSurfaceKHR p_surface, GLFWwindow* p_window,
                       std::uint32_t p_graphics_family,
                       std::uint32_t p_present_family, VkDevice p_device)
    -> VkSwapchainKHR
{
    const auto [surface_capabilties, formats, present_modes] =
        query_swap_chain_support_details(p_physical_device, p_surface);
    const auto [format, present_mode, extent] = choose_swap_chain_settings(
        p_window, formats, present_modes, surface_capabilties);

    const auto queue_families =
        std::array<std::uint32_t, 2>{p_graphics_family, p_present_family};

    auto create_info = VkSwapchainCreateInfoKHR{
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .pNext = nullptr,
        .flags = 0,
        .surface = p_surface,
        .minImageCount = surface_capabilties.minImageCount + 1,
        .imageFormat = format.format,
        .imageColorSpace = format.colorSpace,
        .imageExtent = extent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices = nullptr,
        .preTransform = surface_capabilties.currentTransform,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = present_mode,
        .clipped = VK_FALSE,
        .oldSwapchain = VK_NULL_HANDLE};

    if (p_graphics_family != p_present_family)
    {
        create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        create_info.queueFamilyIndexCount = queue_families.size();
        create_info.pQueueFamilyIndices = queue_families.data();
    }

    if (surface_capabilties.maxImageCount > 0 &&
        create_info.minImageCount > surface_capabilties.maxImageCount)
    {
        create_info.minImageCount = surface_capabilties.maxImageCount;
    }

    auto swap_chain = static_cast<VkSwapchainKHR>(VK_NULL_HANDLE);
    const auto result =
        vkCreateSwapchainKHR(p_device, &create_info, nullptr, &swap_chain);
    if (result != VK_SUCCESS)
    {
        fmt::print(
            "[FATAL ERROR]: Failed to create the swap chain: Vulkan error {}.",
            result);
        std::exit(EXIT_FAILURE);
    }

    return swap_chain;
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

    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT,
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

    const auto swap_chain =
        create_swap_chain(physical_device, surface, window,
                          graphics_queue_family, present_queue_family, device);

    glfwShowWindow(window);

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
    }

    vkDestroySwapchainKHR(device, swap_chain, nullptr);
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