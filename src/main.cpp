#include <GLFW/glfw3.h>
#include <fmt/format.h>
#include <vulkan/vulkan.h>

#ifdef _WIN32
#include <Windows.h>
#endif

#include <cstdlib>

namespace
{

constexpr uint16_t WINDOW_WIDTH = 1024;
constexpr uint16_t WINDOW_HEIGHT = 768;

void glfw_error_callback(int p_error_code, const char *p_message)
{
    fmt::print("[GLFW ERROR {}]: {}\n", p_error_code, p_message);
}

VkInstance create_instance()
{
    VkApplicationInfo application_info
    {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pNext = nullptr,
        .pApplicationName = "Vulkan Triangle",
        .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
        .pEngineName = nullptr,
        .engineVersion = 0,
        .apiVersion = VK_API_VERSION_1_2
    };
    
    VkInstanceCreateInfo create_info 
    {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .pApplicationInfo = &application_info,
        .enabledLayerCount = 0,
        .ppEnabledLayerNames = nullptr,
        .enabledExtensionCount = 0,
        .ppEnabledExtensionNames = nullptr
    };
    
    VkInstance instance;
    VkResult result = vkCreateInstance(&create_info, nullptr, &instance);
    if (result != VK_SUCCESS)
    {
        fmt::print("[FATAL ERROR]: Failed to create the Vulkan instance. Vulkan Error {}", result);
        std::exit(EXIT_FAILURE);
    }
    
    return instance;
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
    
    vkDestroyInstance(instance, nullptr);

    glfwTerminate();
    return EXIT_SUCCESS;
}

} // namespace

int main() { return real_main(); }

#ifdef _WIN32
int WINAPI wWinMain(HINSTANCE, HINSTANCE, PWSTR, int) { return real_main(); }
#endif