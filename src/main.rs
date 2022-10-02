use vulkan_triangle::vulkan::*;

unsafe fn create_instance() -> VkInstance {
    let application_info = VkApplicationInfo {
        sType: VK_STRUCTURE_TYPE_APPLICATION_INFO,
        pNext: std::ptr::null(),
        pApplicationName: b"Vulkan Triangle".as_ptr() as *const i8,
        applicationVersion: 0,
        pEngineName: std::ptr::null(),
        engineVersion: 0,
        apiVersion: VK_MAKE_API_VERSION(0, 1, 2, 0)
    };
    
    let create_info = VkInstanceCreateInfo {
        sType: VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        pNext: std::ptr::null(),
        flags: 0,
        pApplicationInfo: &application_info,
        enabledLayerCount: 0,
        ppEnabledLayerNames: std::ptr::null(),
        enabledExtensionCount: 0,
        ppEnabledExtensionNames: std::ptr::null()
    };
    
    let mut instance = std::ptr::null_mut();
    let result = vkCreateInstance(&create_info, std::ptr::null(), &mut instance);
    
    if result != VK_SUCCESS {
        panic!("[FATAL ERROR]: Failed to create the Vulkan instance. Error Code {}", result);
    }
    
    instance
}

fn main() {
    unsafe {
        let instance = create_instance();
        
        vkDestroyInstance(instance, std::ptr::null());
    }
}