use super::ffi::vulkan::*;
use std::ffi::CString;
use std::mem::transmute;
use std::ptr::{null, null_mut};

pub struct DebugMessenger {
    instance: VkInstance,
    raw_handle: VkDebugUtilsMessengerEXT
}

impl DebugMessenger {
    pub(super) fn new(raw_instance: VkInstance) -> Result<DebugMessenger, ()> {
        let create_info = super::get_debug_messenger_create_info();
        
        let func_name = CString::new("vkCreateDebugUtilsMessengerEXT").unwrap();
        let func = unsafe { vkGetInstanceProcAddr(raw_instance, func_name.as_ptr()) };
        let func: PFN_vkCreateDebugUtilsMessengerEXT = unsafe { transmute(func) };
        
        let mut messenger = null_mut();
        
        if let Some(func) = func {
            unsafe { func(raw_instance, &create_info, null(), &mut messenger) };
        } else {
            return Err(());
        }
        
        return Ok(DebugMessenger { raw_handle: messenger, instance: raw_instance });
    }
}

impl Drop for DebugMessenger {
    fn drop(&mut self) {
        let func_name = CString::new("vkDestroyDebugUtilsMessengerEXT").unwrap();
        let func = unsafe { vkGetInstanceProcAddr(self.instance, func_name.as_ptr()) };
        let func: PFN_vkDestroyDebugUtilsMessengerEXT = unsafe { transmute(func) };
        
        if let Some(func) = func {
            unsafe { func(self.instance, self.raw_handle, null()) };
        }
        
        // If it was None, we can't really do anything so...
    }
}