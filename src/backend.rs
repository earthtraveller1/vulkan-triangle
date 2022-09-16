pub mod ffi;

pub mod debug_messenger;
pub mod instance;

use std::{ptr::{null, null_mut}, ffi::c_void};

pub use debug_messenger::DebugMessenger;
pub use instance::Instance;

use ffi::vulkan::*;

extern "C" fn debug_callback(_: VkDebugUtilsMessageSeverityFlagBitsEXT, _: VkDebugUtilsMessageTypeFlagBitsEXT, _callback_data: *const VkDebugUtilsMessengerCallbackDataEXT, _: *mut c_void) -> VkBool32 {
    0
}

pub fn get_debug_messenger_create_info() /* -> VkDebugUtilsMessengerCreateInfoEXT */ {
    VkDebugUtilsMessengerCreateInfoEXT {
        sType: VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
        pNext: null(),
        flags: 0,
        messageSeverity: VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT
            | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
            | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT
            | VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT,
        messageType: VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
            | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT
            | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT,
        pfnUserCallback: Some(debug_callback),
        pUserData: null_mut()
    };
}
