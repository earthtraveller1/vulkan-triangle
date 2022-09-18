pub mod ffi;

pub mod debug_messenger;
pub mod instance;
pub mod surface;

use std::{
    ffi::{c_void, CStr},
    ptr::{null, null_mut},
};

pub use debug_messenger::DebugMessenger;
pub use instance::Instance;
pub use surface::Surface;

use ffi::vulkan::*;

unsafe extern "C" fn debug_callback(
    _message_severity: VkDebugUtilsMessageSeverityFlagBitsEXT,
    _message_type: VkDebugUtilsMessageTypeFlagsEXT,
    callback_data: *const VkDebugUtilsMessengerCallbackDataEXT,
    _user_data: *mut c_void,
) -> VkBool32 {
    println!(
        "[VULKAN]: {}",
        CStr::from_ptr((*callback_data).pMessage)
            .to_str()
            .unwrap_or("")
    );
    
    VK_FALSE
}

fn get_debug_messenger_create_info() -> VkDebugUtilsMessengerCreateInfoEXT {
    VkDebugUtilsMessengerCreateInfoEXT {
        sType: VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
        pNext: null(),
        flags: 0,
        messageSeverity: (VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT
            | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
            | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT
            | VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
            .try_into()
            .unwrap(),
        messageType: (VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
            | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT
            | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT)
            .try_into()
            .unwrap(),
        pfnUserCallback: Some(debug_callback),
        pUserData: null_mut(),
    }
}
