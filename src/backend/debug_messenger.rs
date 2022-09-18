use super::ffi::vulkan::*;
use std::ffi::CString;
use std::mem::transmute;
use std::ptr::{null, null_mut};
use super::Instance;

pub struct DebugMessenger<'a> {
    instance: &'a Instance,
    raw_handle: VkDebugUtilsMessengerEXT
}

impl<'a> DebugMessenger<'a> {
    pub(super) fn new(instance: &Instance) -> Result<DebugMessenger, ()> {
        let create_info = super::get_debug_messenger_create_info();
        
        let func_name = CString::new("vkCreateDebugUtilsMessengerEXT").unwrap();
        let func = unsafe { vkGetInstanceProcAddr(instance.raw_handle, func_name.as_ptr()) };
        let func: PFN_vkCreateDebugUtilsMessengerEXT = unsafe { transmute(func) };
        
        let mut messenger = null_mut();
        
        if let Some(func) = func {
            unsafe { func(instance.raw_handle, &create_info, null(), &mut messenger) };
        } else {
            return Err(());
        }
        
        return Ok(DebugMessenger { raw_handle: messenger, instance });
    }
}

impl<'a> Drop for DebugMessenger<'a> {
    fn drop(&mut self) {
        let func_name = CString::new("vkDestroyDebugUtilsMessengerEXT").unwrap();
        let func = unsafe { vkGetInstanceProcAddr(self.instance.raw_handle, func_name.as_ptr()) };
        let func: PFN_vkDestroyDebugUtilsMessengerEXT = unsafe { transmute(func) };
        
        if let Some(func) = func {
            unsafe { func(self.instance.raw_handle, self.raw_handle, null()) };
        }
        
        // If it was None, we can't really do anything so...
    }
}