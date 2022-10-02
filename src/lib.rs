#[allow(nonstandard_style)]
pub mod vulkan {
    include!(concat!(env!("OUT_DIR"), "/vulkan-ffi.rs"));

    pub fn VK_MAKE_API_VERSION(variant: u32, major: u32, minor: u32, patch: u32) -> u32 {
        ((variant) << 29) | ((major) << 22) | ((minor) << 12) | (patch)
    }

    pub unsafe fn vk_create_debug_utils_messenger_ext(
        instance: VkInstance,
        create_info: *const VkDebugUtilsMessengerCreateInfoEXT,
        allocator: *const VkAllocationCallbacks,
        messenger: *mut VkDebugUtilsMessengerEXT,
    ) -> VkResult {
        let func: PFN_vkCreateDebugUtilsMessengerEXT = std::mem::transmute(vkGetInstanceProcAddr(
            instance,
            b"vkCreateDebugUtilsMessengerEXT\0".as_ptr() as *const i8,
        ));

        if let Some(func) = func {
            func(instance, create_info, allocator, messenger)
        } else {
            VK_ERROR_EXTENSION_NOT_PRESENT
        }
    }

    pub unsafe fn vk_destroy_debug_utils_messenger_ext(
        instance: VkInstance,
        messenger: VkDebugUtilsMessengerEXT,
        allocator: *const VkAllocationCallbacks,
    ) {
        let func: PFN_vkDestroyDebugUtilsMessengerEXT = std::mem::transmute(vkGetInstanceProcAddr(
            instance,
            b"vkDestroyDebugUtilsMessengerEXT\0".as_ptr() as *const i8,
        ));
        
        if let Some(func) = func {
            func(instance, messenger, allocator);
        }
    }
}
