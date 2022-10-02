use std::ffi::{c_void, CString, CStr};

use vulkan_triangle::vulkan::*;

const WINDOW_WIDTH: u32 = 1024;
const WINDOW_HEIGHT: u32 = 768;
const WINDOW_TITLE: &str = "Vulkan Triangle";

const VALIDATION_LAYERS: [*const i8; 1] = [b"VK_LAYER_KHRONOS_validation\0".as_ptr() as *const i8];

const ENABLE_VALIDATION: bool = true;

unsafe extern "C" fn debug_messenger_callback(
    _message_severity: VkDebugUtilsMessageSeverityFlagBitsEXT,
    _message_types: VkDebugUtilsMessageTypeFlagsEXT,
    callback_data: *const VkDebugUtilsMessengerCallbackDataEXT,
    _user_data: *mut c_void,
) -> VkBool32 {
    println!("[VULKAN]: {}", CStr::from_ptr((*callback_data).pMessage).to_str().unwrap_or("<INVALID STRING OR SMTH>"));
    
    VK_FALSE
}

unsafe fn get_debug_messenger_create_info() -> VkDebugUtilsMessengerCreateInfoEXT {
    VkDebugUtilsMessengerCreateInfoEXT {
        sType: VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
        pNext: std::ptr::null(),
        flags: 0,
        messageSeverity: (VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT
            | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
            | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT
            | VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
            .try_into()
            .unwrap(),
        messageType: (VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
            | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
            | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT)
            .try_into()
            .unwrap(),
        pfnUserCallback: Some(debug_messenger_callback),
        pUserData: std::ptr::null_mut(),
    }
}

unsafe fn create_instance(extensions: Vec<CString>) -> VkInstance {
    let application_info = VkApplicationInfo {
        sType: VK_STRUCTURE_TYPE_APPLICATION_INFO,
        pNext: std::ptr::null(),
        pApplicationName: b"Vulkan Triangle\0".as_ptr() as *const i8,
        applicationVersion: 0,
        pEngineName: std::ptr::null(),
        engineVersion: 0,
        apiVersion: VK_MAKE_API_VERSION(0, 1, 2, 0),
    };

    let mut extensions: Vec<*const i8> = extensions.iter().map(|e| e.as_ptr()).collect();
    
    if ENABLE_VALIDATION {
        extensions.push(VK_EXT_DEBUG_UTILS_EXTENSION_NAME.as_ptr() as *const i8);
    }

    let messenger_create_info = get_debug_messenger_create_info();

    let create_info = VkInstanceCreateInfo {
        sType: VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        pNext: if ENABLE_VALIDATION {
            &messenger_create_info as *const VkDebugUtilsMessengerCreateInfoEXT as *const c_void
        } else {
            std::ptr::null()
        },
        flags: 0,
        pApplicationInfo: &application_info,
        enabledLayerCount: if ENABLE_VALIDATION {
            VALIDATION_LAYERS.len().try_into().unwrap()
        } else {
            0
        },
        ppEnabledLayerNames: if ENABLE_VALIDATION {
            VALIDATION_LAYERS.as_ptr()
        } else {
            std::ptr::null()
        },
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

unsafe fn create_debug_messenger(instance: VkInstance) -> VkDebugUtilsMessengerEXT {
    let create_info = get_debug_messenger_create_info();
    
    let mut debug_messenger = std::ptr::null_mut();
    let result = vk_create_debug_utils_messenger_ext(instance, &create_info, std::ptr::null(), &mut debug_messenger);
    if result != VK_SUCCESS {
        panic!("Failed to create the debug messenger. Error Code {}", result);
    }
    
    debug_messenger
}

unsafe fn create_surface(instance: VkInstance, window: &glfw::Window) -> VkSurfaceKHR {
    let mut surface = std::ptr::null_mut();
    let result = window.create_window_surface(
        instance as usize,
        std::ptr::null(),
        std::mem::transmute(&mut surface),
    );
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
        let debug_messenger = create_debug_messenger(instance);

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
        vk_destroy_debug_utils_messenger_ext(instance, debug_messenger, std::ptr::null());
        vkDestroyInstance(instance, std::ptr::null());
    }
}
