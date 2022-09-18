use super::super::Window;
use super::ffi::vulkan::*;
use super::PhysicalDevice;
use super::Surface;
use std::{
    convert::TryInto,
    ffi::{CStr, CString},
    os::raw::c_void,
    ptr::{null, null_mut},
};

// A structure to represent a Vulkan instance.
pub struct Instance {
    pub(super) raw_handle: VkInstance,
}

// Validation Layers
const VALIDATION_LAYERS: [*const i8; 1] = [b"VK_LAYER_KHRONOS_validation\0".as_ptr() as *const i8];

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
                unsafe { CStr::from_ptr(&(extension.extensionName[0])) }
                    .to_str()
                    .unwrap_or("ERROR")
                    .to_string()
            })
            .collect()
    }

    pub fn new(
        application_name: &str,
        application_window: &Window,
        app_version_major: u32,
        app_version_minor: u32,
        app_version_patch: u32,
        enable_validation: bool,
    ) -> Result<Instance, ()> {
        let application_name = CString::new(application_name).unwrap();

        let application_info = VkApplicationInfo {
            sType: VK_STRUCTURE_TYPE_APPLICATION_INFO,
            pNext: null(),
            pApplicationName: application_name.as_ptr(),
            applicationVersion: VK_MAKE_VERSION(
                app_version_major,
                app_version_minor,
                app_version_patch,
            ),
            pEngineName: null(),
            engineVersion: 0,
            apiVersion: VK_MAKE_API_VERSION(0, 1, 2, 0),
        };

        let debug_messenger_info = super::get_debug_messenger_create_info();

        let mut create_info = VkInstanceCreateInfo {
            sType: VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            pNext: null(),
            flags: 0,
            pApplicationInfo: &application_info,
            enabledLayerCount: 0,
            ppEnabledLayerNames: null(),
            enabledExtensionCount: 0,
            ppEnabledExtensionNames: null(),
        };

        let mut enabled_extensions = Vec::new();

        let window_extensions: Vec<CString> = application_window
            .get_required_instance_extensions()
            .unwrap()
            .iter()
            .map(|extension| CString::new(extension.clone()).unwrap())
            .collect();

        window_extensions
            .iter()
            .for_each(|extension| enabled_extensions.push(extension.as_ptr()));

        if enable_validation {
            create_info.pNext =
                &debug_messenger_info as *const VkDebugUtilsMessengerCreateInfoEXT as *const c_void;

            create_info.enabledLayerCount = VALIDATION_LAYERS.len().try_into().unwrap();
            create_info.ppEnabledLayerNames = VALIDATION_LAYERS.as_ptr();

            enabled_extensions.push(VK_EXT_DEBUG_UTILS_EXTENSION_NAME.as_ptr() as *const i8);
        }

        if enabled_extensions.len() > 0 {
            create_info.enabledExtensionCount = enabled_extensions.len().try_into().unwrap();
            create_info.ppEnabledExtensionNames = enabled_extensions.as_ptr();
        }

        let mut raw_handle = null_mut();
        let result = unsafe { vkCreateInstance(&create_info, null(), &mut raw_handle) };

        if result == VK_SUCCESS {
            Ok(Instance { raw_handle })
        } else {
            Err(())
        }
    }

    pub fn create_debug_messenger(&self) -> Result<super::DebugMessenger, ()> {
        super::DebugMessenger::new(self)
    }

    pub fn create_window_surface(&self, window: &Window) -> Surface {
        let mut surface = null_mut();
        window.create_window_surface(self.raw_handle, &mut surface);
        Surface {
            instance: &self,
            raw_handle: surface,
        }
    }

    pub fn enumerate_physical_devices(&self) -> Vec<PhysicalDevice> {
        let mut device_count = 0;
        unsafe { vkEnumeratePhysicalDevices(self.raw_handle, &mut device_count, null_mut()) };

        let mut devices = Vec::with_capacity(device_count.try_into().unwrap());
        unsafe {
            vkEnumeratePhysicalDevices(self.raw_handle, &mut device_count, devices.as_mut_ptr());
            devices.set_len(device_count.try_into().unwrap());
        }

        devices
            .iter()
            .map(|device| PhysicalDevice {
                raw_handle: device.clone(),
            })
            .collect()
    }
}

impl Drop for Instance {
    fn drop(&mut self) {
        unsafe { vkDestroyInstance(self.raw_handle, null()) };
    }
}
