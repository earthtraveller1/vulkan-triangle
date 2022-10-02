use std::ffi::CString;

use vulkan_triangle::vulkan::*;

const WINDOW_WIDTH: u32 = 1024;
const WINDOW_HEIGHT: u32 = 768;
const WINDOW_TITLE: &str = "Vulkan Triangle";

unsafe fn create_instance(extensions: Vec<CString>) -> VkInstance {
    let application_info = VkApplicationInfo {
        sType: VK_STRUCTURE_TYPE_APPLICATION_INFO,
        pNext: std::ptr::null(),
        pApplicationName: b"Vulkan Triangle".as_ptr() as *const i8,
        applicationVersion: 0,
        pEngineName: std::ptr::null(),
        engineVersion: 0,
        apiVersion: VK_MAKE_API_VERSION(0, 1, 2, 0),
    };

    let extensions: Vec<*const i8> = extensions.iter().map(|e| e.as_ptr()).collect();

    let create_info = VkInstanceCreateInfo {
        sType: VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        pNext: std::ptr::null(),
        flags: 0,
        pApplicationInfo: &application_info,
        enabledLayerCount: 0,
        ppEnabledLayerNames: std::ptr::null(),
        enabledExtensionCount: extensions.len().try_into().unwrap(),
        ppEnabledExtensionNames: extensions.as_ptr(),
    };

    let mut instance = std::ptr::null_mut();
    let result = vkCreateInstance(&create_info, std::ptr::null(), &mut instance);

    if result != VK_SUCCESS {
        panic!(
            "Failed to create the Vulkan instance. Error Code {}",
            result
        );
    }

    instance
}

unsafe fn create_surface(instance: VkInstance, window: &glfw::Window) -> VkSurfaceKHR {
    let mut surface = std::ptr::null_mut();
    let result = window.create_window_surface(instance as usize, std::ptr::null(), std::mem::transmute(&mut surface));
    if result != VK_SUCCESS.try_into().unwrap() {
        panic!("Failed to create the window surface.");
    }
    surface
}

fn main() {
    unsafe {
        let mut glfw = glfw::init(glfw::FAIL_ON_ERRORS).expect("Failed to initialize GLFW.");
        let instance = create_instance(
            glfw.get_required_instance_extensions()
                .expect("Failed to get required instance extensions")
                .iter()
                .map(|e| {
                    CString::new(e.as_str())
                        .expect(format!("Cannot convert {} to CString.", e).as_str())
                })
                .collect(),
        );

        glfw.window_hint(glfw::WindowHint::Visible(false));
        glfw.window_hint(glfw::WindowHint::Resizable(false));
        glfw.window_hint(glfw::WindowHint::ClientApi(glfw::ClientApiHint::NoApi));

        let (mut window, _events) = glfw
            .create_window(
                WINDOW_WIDTH,
                WINDOW_HEIGHT,
                WINDOW_TITLE,
                glfw::WindowMode::Windowed,
            )
            .expect("Failed to create the GLFW window.");
        
        let surface = create_surface(instance, &window);

        window.show();
        while !window.should_close() {
            glfw.poll_events();
        }
        
        vkDestroySurfaceKHR(instance, surface, std::ptr::null());
        vkDestroyInstance(instance, std::ptr::null());
    }
}
