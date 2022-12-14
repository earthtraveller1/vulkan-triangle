#define LOAD_VK_FUNCTION(function, instance)                                   \
    const auto hello56721_##function = reinterpret_cast<PFN_##function>(       \
        vkGetInstanceProcAddr(instance, #function))

namespace
{

template <typename T, size_t S> using raw_array = T[S];

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

struct vertex_t
{
    glm::vec2 position;
    glm::vec3 color;

    constexpr static auto get_binding_description()
        -> VkVertexInputBindingDescription
    {
        return VkVertexInputBindingDescription{.binding = 0,
                                               .stride = sizeof(vertex_t),
                                               .inputRate =
                                                   VK_VERTEX_INPUT_RATE_VERTEX};
    }

    constexpr static auto get_attribute_descriptions()
        -> std::array<VkVertexInputAttributeDescription, 2>
    {
        return std::array<VkVertexInputAttributeDescription, 2>{
            VkVertexInputAttributeDescription{.location = 0,
                                              .binding = 0,
                                              .format = VK_FORMAT_R32G32_SFLOAT,
                                              .offset =
                                                  offsetof(vertex_t, position)},
            VkVertexInputAttributeDescription{
                .location = 1,
                .binding = 0,
                .format = VK_FORMAT_R32G32B32_SFLOAT,
                .offset = offsetof(vertex_t, color)},
        };
    }
};

auto do_nothing() {}

inline auto print_error(std::string_view p_msg, VkResult p_err)
{
    fmt::print(stderr, fmt::fg(fmt::color::red), p_msg, p_err);
}

VkBool32 debug_messenger_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT p_severity,
    VkDebugUtilsMessageTypeFlagsEXT,
    const VkDebugUtilsMessengerCallbackDataEXT* p_callback_data, void*)
{
    auto color = fmt::fg(fmt::color::yellow);
    if (p_severity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
    {
        color = fmt::fg(fmt::color::red);
    }

    fmt::print(color, "[VULKAN]: {}\n", p_callback_data->pMessage);

    if (p_severity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
    {
        do_nothing();
    }

    return VK_FALSE;
}

constexpr VkDebugUtilsMessengerCreateInfoEXT DEBUG_MESSENGER_CREATE_INFO{
    .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
    .pNext = nullptr,
    .flags = 0,
    .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT,
    //    VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
    //    VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT,
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

// Return values
// - the swapchain handle
// - the handles to the swapchain images
// - the format of the swap chain images
// - the swap chain's extent
auto create_swap_chain(VkPhysicalDevice p_physical_device,
                       VkSurfaceKHR p_surface, GLFWwindow* p_window,
                       std::uint32_t p_graphics_family,
                       std::uint32_t p_present_family, VkDevice p_device)
    -> std::tuple<VkSwapchainKHR, std::vector<VkImage>, VkFormat, VkExtent2D>
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
        create_info.queueFamilyIndexCount =
            static_cast<uint32_t>(queue_families.size());
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

    auto image_count = static_cast<std::uint32_t>(0);
    vkGetSwapchainImagesKHR(p_device, swap_chain, &image_count, nullptr);

    auto images = std::vector<VkImage>(image_count);
    vkGetSwapchainImagesKHR(p_device, swap_chain, &image_count, images.data());

    return {swap_chain, images, format.format, extent};
}

auto create_image_views(VkDevice p_device,
                        const std::vector<VkImage> p_swap_chain_images,
                        VkFormat p_format) -> std::vector<VkImageView>
{
    auto image_views = std::vector<VkImageView>(p_swap_chain_images.size());

    for (auto i = static_cast<decltype(p_swap_chain_images.size())>(0);
         i < p_swap_chain_images.size(); i++)
    {
        const auto create_info = VkImageViewCreateInfo{
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .image = p_swap_chain_images.at(i),
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = p_format,
            .components =
                VkComponentMapping{.r = VK_COMPONENT_SWIZZLE_IDENTITY,
                                   .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                                   .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                                   .a = VK_COMPONENT_SWIZZLE_IDENTITY},
            .subresourceRange =
                VkImageSubresourceRange{.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                                        .baseMipLevel = 0,
                                        .levelCount = 1,
                                        .baseArrayLayer = 0,
                                        .layerCount = 1}};

        auto image_view = static_cast<VkImageView>(VK_NULL_HANDLE);
        const auto result =
            vkCreateImageView(p_device, &create_info, nullptr, &image_view);

        if (result != VK_SUCCESS)
        {
            fmt::print(stderr,
                       "[FATAL ERROR]: Failed to create image view number {}. "
                       "Vulkan error {}.",
                       i, result);
            std::exit(EXIT_FAILURE);
        }

        image_views[i] = image_view;
    }

    return image_views;
}

auto load_binary_file(std::string_view p_path) -> std::vector<char>
{
    auto file =
        std::ifstream(p_path.data(), std::fstream::binary | std::fstream::ate);

    if (!file.is_open())
    {
        fmt::print(stderr,
                   "[ERROR]: Failed to either find or access {}. Check if the "
                   "file actually exists and if the user has the neccessary "
                   "permissions to access it.\n",
                   p_path);
        return {};
    }

    const auto file_size = file.tellg();
    auto buffer = std::vector<char>(file_size);

    file.seekg(0);
    file.read(buffer.data(), file_size);

    return buffer;
}

auto create_shader_module(VkDevice p_device, const std::vector<char>& p_code)
    -> VkShaderModule
{
    const auto create_info = VkShaderModuleCreateInfo{
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .codeSize = p_code.size(),
        .pCode = reinterpret_cast<const std::uint32_t*>(p_code.data())};

    auto shader_module = static_cast<VkShaderModule>(VK_NULL_HANDLE);
    const auto result =
        vkCreateShaderModule(p_device, &create_info, nullptr, &shader_module);
    if (result != VK_SUCCESS)
    {
        fmt::print(
            stderr,
            "[FATAL ERROR]: Failed to create a shader module. Vulkan error {}.",
            result);
        std::exit(EXIT_FAILURE);
    }

    return shader_module;
}

auto create_graphics_pipeline(VkDevice p_device, VkExtent2D p_swap_chain_extent,
                              VkRenderPass p_render_pass)
    -> std::tuple<VkPipeline, VkPipelineLayout>
{
    const auto vertex_shader_code = load_binary_file("shaders/shader.vert.spv");
    const auto fragment_shader_code =
        load_binary_file("shaders/shader.frag.spv");

    const auto vertex_shader_module =
        create_shader_module(p_device, vertex_shader_code);
    const auto fragment_shader_module =
        create_shader_module(p_device, fragment_shader_code);

    const auto vertex_shader_stage_info = VkPipelineShaderStageCreateInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .stage = VK_SHADER_STAGE_VERTEX_BIT,
        .module = vertex_shader_module,
        .pName = "main",
        .pSpecializationInfo = nullptr};

    const auto fragment_shader_stage_info = VkPipelineShaderStageCreateInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
        .module = fragment_shader_module,
        .pName = "main",
        .pSpecializationInfo = nullptr};

    const auto shader_stages = std::array<VkPipelineShaderStageCreateInfo, 2>{
        fragment_shader_stage_info, vertex_shader_stage_info};

    const auto dynamic_states = std::array<VkDynamicState, 2>{
        VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

    const auto dynamic_state = VkPipelineDynamicStateCreateInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .dynamicStateCount = static_cast<std::uint32_t>(dynamic_states.size()),
        .pDynamicStates = dynamic_states.data()};

    constexpr auto VERTEX_BINDING_DESCRIPTION =
        vertex_t::get_binding_description();
    constexpr auto VERTEX_ATTRIBUTE_DESCRIPTIONS =
        vertex_t::get_attribute_descriptions();

    const auto vertex_input = VkPipelineVertexInputStateCreateInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .vertexBindingDescriptionCount = 1,
        .pVertexBindingDescriptions = &VERTEX_BINDING_DESCRIPTION,
        .vertexAttributeDescriptionCount =
            static_cast<uint32_t>(VERTEX_ATTRIBUTE_DESCRIPTIONS.size()),
        .pVertexAttributeDescriptions = VERTEX_ATTRIBUTE_DESCRIPTIONS.data(),
    };

    const auto input_assembly = VkPipelineInputAssemblyStateCreateInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        .primitiveRestartEnable = VK_FALSE,
    };

    const auto viewport = VkViewport{
        .x = 0.0f,
        .y = 0.0f,
        .width = static_cast<float>(p_swap_chain_extent.width),
        .height = static_cast<float>(p_swap_chain_extent.height),
        .minDepth = 0.0f,
        .maxDepth = 1.0f,
    };

    const auto scissor = VkRect2D{
        .offset =
            VkOffset2D{
                .x = 0,
                .y = 0,
            },
        .extent = p_swap_chain_extent,
    };

    const auto viewport_state = VkPipelineViewportStateCreateInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .viewportCount = 1,
        .pViewports = &viewport,
        .scissorCount = 1,
        .pScissors = &scissor};

    const auto rasterization = VkPipelineRasterizationStateCreateInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .depthClampEnable = VK_FALSE,
        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode = VK_POLYGON_MODE_FILL,
        .cullMode = VK_CULL_MODE_BACK_BIT,
        .frontFace = VK_FRONT_FACE_CLOCKWISE,
        .depthBiasEnable = VK_FALSE,
        .depthBiasConstantFactor = 0.0f,
        .depthBiasClamp = 0.0f,
        .depthBiasSlopeFactor = 0.0f,
        .lineWidth = 1.0f,
    };

    const auto multisampling = VkPipelineMultisampleStateCreateInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
        .sampleShadingEnable = VK_FALSE,
        .minSampleShading = 0.0f,
        .pSampleMask = nullptr,
        .alphaToCoverageEnable = VK_FALSE,
        .alphaToOneEnable = VK_FALSE};

    const auto color_blend_attachment = VkPipelineColorBlendAttachmentState{
        .blendEnable = VK_FALSE,
        .srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
        .dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,
        .colorBlendOp = VK_BLEND_OP_ADD,
        .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
        .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
        .alphaBlendOp = VK_BLEND_OP_ADD,
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                          VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT};

    const auto color_blending = VkPipelineColorBlendStateCreateInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .logicOpEnable = VK_FALSE,
        .logicOp = VK_LOGIC_OP_COPY,
        .attachmentCount = 1,
        .pAttachments = &color_blend_attachment,
        .blendConstants = {0.0f, 0.0f, 0.0f, 0.0f}};

    const auto pipeline_layout_info = VkPipelineLayoutCreateInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .setLayoutCount = 0,
        .pSetLayouts = nullptr,
        .pushConstantRangeCount = 0,
        .pPushConstantRanges = nullptr};

    auto pipeline_layout = static_cast<VkPipelineLayout>(
        VK_NULL_HANDLE); // TODO: Clean this up afterwards.
    const auto pipeline_layout_create_result = vkCreatePipelineLayout(
        p_device, &pipeline_layout_info, nullptr, &pipeline_layout);
    if (pipeline_layout_create_result != VK_SUCCESS)
    {
        fmt::print("[FATAL ERROR]: Failed to create the pipeline layout. "
                   "Vulkan error {}",
                   pipeline_layout_create_result);
        std::exit(EXIT_FAILURE);
    }

    const auto create_info = VkGraphicsPipelineCreateInfo{
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .stageCount = static_cast<uint32_t>(shader_stages.size()),
        .pStages = shader_stages.data(),
        .pVertexInputState = &vertex_input,
        .pInputAssemblyState = &input_assembly,
        .pTessellationState = nullptr,
        .pViewportState = &viewport_state,
        .pRasterizationState = &rasterization,
        .pMultisampleState = &multisampling,
        .pDepthStencilState = nullptr,
        .pColorBlendState = &color_blending,
        .pDynamicState = &dynamic_state,
        .layout = pipeline_layout,
        .renderPass = p_render_pass,
        .subpass = 0,
        .basePipelineHandle = VK_NULL_HANDLE,
        .basePipelineIndex = 0};

    auto pipeline = static_cast<VkPipeline>(VK_NULL_HANDLE);
    const auto result = vkCreateGraphicsPipelines(
        p_device, VK_NULL_HANDLE, 1, &create_info, nullptr, &pipeline);
    if (result != VK_SUCCESS)
    {
        fmt::print("[FATAL ERROR]: Failed to create the graphics pipeline. "
                   "Vulkan error {}.\n",
                   result);
        std::exit(EXIT_FAILURE);
    }

    vkDestroyShaderModule(p_device, vertex_shader_module, nullptr);
    vkDestroyShaderModule(p_device, fragment_shader_module, nullptr);

    return {pipeline, pipeline_layout};
}

auto create_render_pass(VkFormat p_format, VkDevice p_device) -> VkRenderPass
{
    const auto color_attachment = VkAttachmentDescription{
        .format = p_format,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR};

    const auto color_attachment_reference = VkAttachmentReference{
        .attachment = 0, .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

    const auto subpass = VkSubpassDescription{
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .colorAttachmentCount = 1,
        .pColorAttachments = &color_attachment_reference};

    const auto dependency = VkSubpassDependency{
        .srcSubpass = VK_SUBPASS_EXTERNAL,
        .dstSubpass = 0,
        .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .srcAccessMask = 0,
        .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT};

    const auto create_info = VkRenderPassCreateInfo{
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .attachmentCount = 1,
        .pAttachments = &color_attachment,
        .subpassCount = 1,
        .pSubpasses = &subpass,
        .dependencyCount = 1,
        .pDependencies = &dependency};

    auto render_pass = static_cast<VkRenderPass>(VK_NULL_HANDLE);
    const auto result =
        vkCreateRenderPass(p_device, &create_info, nullptr, &render_pass);
    if (result != VK_SUCCESS)
    {
        fmt::print("[FATAL ERROR]: Failed to create the render pass. Vulkan "
                   "error {}\n",
                   result);
        std::exit(EXIT_FAILURE);
    }

    return render_pass;
}

auto create_framebuffers(VkDevice p_device, VkRenderPass p_render_pass,
                         const std::vector<VkImageView>& p_image_views,
                         const VkExtent2D& p_extent)
    -> std::vector<VkFramebuffer>
{
    auto framebuffers = std::vector<VkFramebuffer>(p_image_views.size());

    for (size_t i = 0; i < framebuffers.size(); i++)
    {
        const auto create_info = VkFramebufferCreateInfo{
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .renderPass = p_render_pass,
            .attachmentCount = 1,
            .pAttachments = &p_image_views[i],
            .width = p_extent.width,
            .height = p_extent.height,
            .layers = 1};

        const auto result = vkCreateFramebuffer(p_device, &create_info, nullptr,
                                                &framebuffers[i]);
        if (result != VK_SUCCESS)
        {
            fmt::print("[FATAL ERROR]: Failed to create framebuffer {}. Vulkan "
                       "error {}\n",
                       i, result);
            std::exit(EXIT_FAILURE);
        }
    }

    return framebuffers;
}

auto create_command_pool(VkDevice p_device,
                         std::uint32_t p_graphics_queue_family) -> VkCommandPool
{
    const auto create_info = VkCommandPoolCreateInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = p_graphics_queue_family};

    auto command_pool = (VkCommandPool)VK_NULL_HANDLE;
    const auto result =
        vkCreateCommandPool(p_device, &create_info, nullptr, &command_pool);
    if (result != VK_SUCCESS)
    {
        fmt::print(stderr,
                   "[FATAL ERROR]: Failed to create a command pool. Vulkan "
                   "error {}.\n",
                   result);
        std::exit(EXIT_SUCCESS);
    }

    return command_pool;
}

auto create_command_buffer(VkDevice p_device, VkCommandPool p_pool)
    -> VkCommandBuffer
{
    const auto allocate_info = VkCommandBufferAllocateInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = nullptr,
        .commandPool = p_pool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1};

    auto command_buffer = (VkCommandBuffer)VK_NULL_HANDLE;
    const auto result =
        vkAllocateCommandBuffers(p_device, &allocate_info, &command_buffer);
    if (result != VK_SUCCESS)
    {
        fmt::print(stderr,
                   "[FATAL ERROR]: Failed to allocate a command buffer. Vulkan "
                   "error {}.\n",
                   result);
        std::exit(EXIT_SUCCESS);
    }

    return command_buffer;
}

// The return values for this function is
// - buffer
// - the buffer's memory
auto create_vertex_buffer(VkPhysicalDevice p_physical_device, VkDevice p_device,
                          size_t p_size, const vertex_t* p_vertices)
    -> std::tuple<VkBuffer, VkDeviceMemory>
{
    const auto create_info =
        VkBufferCreateInfo{.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
                           .pNext = nullptr,
                           .flags = 0,
                           .size = p_size,
                           .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                           .sharingMode = VK_SHARING_MODE_EXCLUSIVE};

    auto buffer = (VkBuffer)VK_NULL_HANDLE;
    const auto result =
        vkCreateBuffer(p_device, &create_info, nullptr, &buffer);
    if (result != VK_SUCCESS)
    {
        fmt::print(stderr, fmt::fg(fmt::color::red),
                   "[FATAL ERROR]: Failed to create a buffer. Vulkan error {}.",
                   result);
        std::exit(EXIT_FAILURE);
    }

    auto memory_requirements = VkMemoryRequirements{};
    vkGetBufferMemoryRequirements(p_device, buffer, &memory_requirements);

    auto memory_properties = VkPhysicalDeviceMemoryProperties{};
    vkGetPhysicalDeviceMemoryProperties(p_physical_device, &memory_properties);

    const auto properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

    auto memory_type = (uint32_t)0;
    for (auto i = (uint32_t)0; i < memory_properties.memoryTypeCount; i++)
    {
        if ((memory_requirements.memoryTypeBits & (1 << i)) &&
            (memory_properties.memoryTypes[i].propertyFlags & properties) ==
                properties)
        {
            memory_type = i;
            break;
        }
    }

    const auto allocate_info =
        VkMemoryAllocateInfo{.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
                             .pNext = nullptr,
                             .allocationSize = memory_requirements.size,
                             .memoryTypeIndex = memory_type};

    auto memory = (VkDeviceMemory)VK_NULL_HANDLE;
    const auto result2 =
        vkAllocateMemory(p_device, &allocate_info, nullptr, &memory);

    if (result != VK_SUCCESS)
    {
        fmt::print(stderr, fmt::fg(fmt::color::red),
                   "[FATAL ERROR]: Failed to allocate some GPU memory. Vulkan "
                   "error {}.\n",
                   result2);
        std::exit(EXIT_FAILURE);
    }

    vkBindBufferMemory(p_device, buffer, memory, 0);

    auto data = (void*)nullptr;
    vkMapMemory(p_device, memory, 0, p_size, 0, &data);
    std::memcpy(data, p_vertices, p_size);
    vkUnmapMemory(p_device, memory);

    return {buffer, memory};
}

auto record_command_buffer(VkCommandBuffer p_command_buffer,
                           VkRenderPass p_render_pass,
                           VkFramebuffer p_framebuffer,
                           const VkExtent2D& p_swap_chain_extent,
                           VkPipeline p_graphics_pipeline,
                           VkBuffer p_vertex_buffer)
{
    const auto begin_info = VkCommandBufferBeginInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = nullptr,
        .flags = 0,
        .pInheritanceInfo = nullptr};

    const auto result = vkBeginCommandBuffer(p_command_buffer, &begin_info);
    if (result != VK_SUCCESS)
    {
        print_error("[FATAL ERROR]: Failed to begin recording the command "
                    "buffer. Vulkan error {}.\n",
                    result);
        std::exit(EXIT_FAILURE);
    }

    const auto clear_color = VkClearValue{{{0.0f, 0.0f, 0.0f, 1.0f}}};

    const auto render_pass_begin_info = VkRenderPassBeginInfo{
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .pNext = nullptr,
        .renderPass = p_render_pass,
        .framebuffer = p_framebuffer,
        .renderArea = VkRect2D{.offset = VkOffset2D{.x = 0, .y = 0},
                               .extent = p_swap_chain_extent},
        .clearValueCount = 1,
        .pClearValues = &clear_color};

    vkCmdBeginRenderPass(p_command_buffer, &render_pass_begin_info,
                         VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(p_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                      p_graphics_pipeline);

    const auto offset = (VkDeviceSize)0;
    vkCmdBindVertexBuffers(p_command_buffer, 0, 1, &p_vertex_buffer, &offset);

    const auto viewport =
        VkViewport{.x = 0.0f,
                   .y = 0.0f,
                   .width = static_cast<float>(p_swap_chain_extent.width),
                   .height = static_cast<float>(p_swap_chain_extent.height),
                   .minDepth = 0.0f,
                   .maxDepth = 1.0f};
    vkCmdSetViewport(p_command_buffer, 0, 1, &viewport);

    const auto scissor = VkRect2D{.offset = VkOffset2D{.x = 0, .y = 0},
                                  .extent = p_swap_chain_extent};
    vkCmdSetScissor(p_command_buffer, 0, 1, &scissor);

    vkCmdDraw(p_command_buffer, 3, 1, 0, 0);

    vkCmdEndRenderPass(p_command_buffer);

    const auto end_result = vkEndCommandBuffer(p_command_buffer);
    if (end_result != VK_SUCCESS)
    {
        print_error("[FATAL ERROR]: Failed to stop recording the command "
                    "buffer. Vulkan error {}.\n",
                    result);
        std::exit(EXIT_FAILURE);
    }
}

// Return values
// 2 semaphores
// 1 fence
auto create_sync_objects(VkDevice p_device)
    -> std::tuple<VkSemaphore, VkSemaphore, VkFence>
{
    const auto semaphore_create_info =
        VkSemaphoreCreateInfo{.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};

    const auto fence_create_info =
        VkFenceCreateInfo{.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
                          .flags = VK_FENCE_CREATE_SIGNALED_BIT};

    auto semaphore1 = (VkSemaphore)VK_NULL_HANDLE,
         semaphore2 = (VkSemaphore)VK_NULL_HANDLE;
    auto fence = (VkFence)VK_NULL_HANDLE;

    const auto results = std::array<VkResult, 3>{
        vkCreateSemaphore(p_device, &semaphore_create_info, nullptr,
                          &semaphore1),
        vkCreateSemaphore(p_device, &semaphore_create_info, nullptr,
                          &semaphore2),
        vkCreateFence(p_device, &fence_create_info, nullptr, &fence)};

    for (const auto& result : results)
    {
        if (result != VK_SUCCESS)
        {
            print_error("[FATAL ERROR]: Failed to create synchronization "
                        "objects for Vulkan. Vulkan error {}",
                        result);
            std::exit(EXIT_FAILURE);
        }
    }

    return {semaphore1, semaphore2, fence};
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

    const auto [swap_chain, swap_chain_images, swap_chain_format,
                swap_chain_extent] =
        create_swap_chain(physical_device, surface, window,
                          graphics_queue_family, present_queue_family, device);

    const auto swap_chain_image_views =
        create_image_views(device, swap_chain_images, swap_chain_format);

    const auto render_pass = create_render_pass(swap_chain_format, device);

    const auto [graphics_pipeline, pipeline_layout] =
        create_graphics_pipeline(device, swap_chain_extent, render_pass);

    const auto swap_chain_framebuffers = create_framebuffers(
        device, render_pass, swap_chain_image_views, swap_chain_extent);

    const auto command_pool =
        create_command_pool(device, graphics_queue_family);

    const auto command_buffer = create_command_buffer(device, command_pool);

    const auto vertices = std::array<vertex_t, 3>{
        vertex_t{glm::vec2{0.0f, -0.5f}, glm::vec3{1.0f, 0.0f, 0.0f}},
        vertex_t{glm::vec2{0.5f, 0.5f}, glm::vec3{0.0f, 1.0f, 0.0f}},
        vertex_t{glm::vec2{-0.5f, 0.5f}, glm::vec3{0.0f, 0.0f, 1.0f}}};

    const auto [vertex_buffer, vertex_buffer_memory] = create_vertex_buffer(
        physical_device, device, vertices.size() * sizeof(vertex_t),
        vertices.data());

    const auto [image_available_semaphore, render_finished_semaphore,
                in_flight_fence] = create_sync_objects(device);

    glfwShowWindow(window);

    while (!glfwWindowShouldClose(window))
    {
        vkWaitForFences(device, 1, &in_flight_fence, VK_TRUE, UINT64_MAX);
        vkResetFences(device, 1, &in_flight_fence);

        auto image_index = (uint32_t)0;
        vkAcquireNextImageKHR(device, swap_chain, UINT64_MAX,
                              image_available_semaphore, VK_NULL_HANDLE,
                              &image_index);

        vkResetCommandBuffer(command_buffer, 0);
        record_command_buffer(
            command_buffer, render_pass, swap_chain_framebuffers[image_index],
            swap_chain_extent, graphics_pipeline, vertex_buffer);

        const auto wait_stages = raw_array<VkPipelineStageFlags, 1>{
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

        const auto submit_info =
            VkSubmitInfo{.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
                         .waitSemaphoreCount = 1,
                         .pWaitSemaphores = &image_available_semaphore,
                         .pWaitDstStageMask = wait_stages,
                         .commandBufferCount = 1,
                         .pCommandBuffers = &command_buffer,
                         .signalSemaphoreCount = 1,
                         .pSignalSemaphores = &render_finished_semaphore};

        const auto submit_result =
            vkQueueSubmit(graphics_queue, 1, &submit_info, in_flight_fence);

        const auto present_info =
            VkPresentInfoKHR{.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
                             .pNext = nullptr,
                             .waitSemaphoreCount = 1,
                             .pWaitSemaphores = &render_finished_semaphore,
                             .swapchainCount = 1,
                             .pSwapchains = &swap_chain,
                             .pImageIndices = &image_index,
                             .pResults = nullptr};

        vkQueuePresentKHR(present_queue, &present_info);

        if (submit_result != VK_SUCCESS)
        {
            print_error("[FATAL ERROR]: Failed to submit the command buffer. "
                        "Vulkan error {}\n",
                        submit_result);
            std::exit(EXIT_FAILURE);
        }

        glfwPollEvents();
    }

    vkDeviceWaitIdle(device);

    vkDestroySemaphore(device, image_available_semaphore, nullptr);
    vkDestroySemaphore(device, render_finished_semaphore, nullptr);
    vkDestroyFence(device, in_flight_fence, nullptr);
    vkFreeMemory(device, vertex_buffer_memory, nullptr);
    vkDestroyBuffer(device, vertex_buffer, nullptr);
    vkDestroyCommandPool(device, command_pool, nullptr);

    for (const auto framebuffer : swap_chain_framebuffers)
    {
        vkDestroyFramebuffer(device, framebuffer, nullptr);
    }

    vkDestroyPipeline(device, graphics_pipeline, nullptr);
    vkDestroyPipelineLayout(device, pipeline_layout, nullptr);
    vkDestroyRenderPass(device, render_pass, nullptr);

    for (const auto& image_view : swap_chain_image_views)
    {
        vkDestroyImageView(device, image_view, nullptr);
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
