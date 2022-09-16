use super::ffi::vulkan::*;
use std::{
    convert::TryInto,
    ptr::{null, null_mut},
};

// A structure to represent a Vulkan instance.
pub struct Instance {
    raw_handle: VkInstance,
}

// Public methods
impl Instance {
    pub fn enumerate_extension_names() -> Vec<String> {
        let mut extension_count = 0;
        unsafe { vkEnumerateInstanceExtensionProperties(null(), &mut extension_count, null_mut()) };

        let mut extensions = Vec::with_capacity(extension_count.try_into().unwrap());
        unsafe {
            vkEnumerateInstanceExtensionProperties(
                null(),
                &mut extension_count,
                extensions.as_mut_ptr(),
            )
        };
        unsafe { extensions.set_len(extension_count.try_into().unwrap()) };

        // If this looks very verbose, it's becuase it's very complicated. Sorry.
        extensions
            .iter()
            .map(|extension| {
                String::from_utf8(
                    extension
                        .extensionName
                        .to_vec()
                        .iter()
                        .map(|c| (*c) as u8)
                        .collect(),
                )
                .unwrap_or("<UTF8 ENCODING ERROR>".to_string())
            })
            .collect()
    }

    pub fn new(
        application_name: &str,
        app_version_major: u32,
        app_version_minor: u32,
        app_version_patch: u32,
    ) -> Result<Instance, ()> {
        let application_name = application_name.to_owned() + "\0";

        let application_info = VkApplicationInfo {
            sType: VK_STRUCTURE_TYPE_APPLICATION_INFO,
            pNext: null(),
            pApplicationName: application_name.as_ptr() as *const i8,
            applicationVersion: VK_MAKE_VERSION(
                app_version_major,
                app_version_minor,
                app_version_patch,
            ),
            pEngineName: null(),
            engineVersion: 0,
            apiVersion: VK_MAKE_API_VERSION(0, 1, 2, 0),
        };

        let create_info = VkInstanceCreateInfo {
            sType: VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            pNext: null(),
            flags: 0,
            pApplicationInfo: &application_info,
            enabledLayerCount: 0,
            ppEnabledLayerNames: null(),
            enabledExtensionCount: 0,
            ppEnabledExtensionNames: null(),
        };

        let mut raw_handle = null_mut();
        let result = unsafe { vkCreateInstance(&create_info, null(), &mut raw_handle) };

        if result == VK_SUCCESS {
            Ok(Instance { raw_handle })
        } else {
            Err(())
        }
    }
}

impl Drop for Instance {
    fn drop(&mut self) {
        unsafe { vkDestroyInstance(self.raw_handle, null()) };
    }
}
