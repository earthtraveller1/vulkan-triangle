#include <GLFW/glfw3.h>
#include <fmt/format.h>

#ifdef _WIN32
#include <Windows.h>
#endif

#include <cstdlib>

namespace
{

const uint16_t WINDOW_WIDTH = 1024;
const uint16_t WINDOW_HEIGHT = 768;

void glfw_error_callback(int p_error_code, const char *p_message)
{
    fmt::print("[GLFW ERROR {}]: {}\n", p_error_code, p_message);
}

// The actual main function
int real_main()
{
    if (!glfwInit())
    {
        fmt::print("[FATAL ERROR]: Failed to initialize GLFW.\n");
        return EXIT_FAILURE;
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

    glfwTerminate();
    return EXIT_SUCCESS;
}

} // namespace

int main() { return real_main(); }

#ifdef _WIN32
int WINAPI wWinMain(HINSTANCE, HINSTANCE, PWSTR, int) { return real_main(); }
#endif