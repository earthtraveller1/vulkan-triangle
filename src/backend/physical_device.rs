use super::ffi::vulkan::*;
use core::mem::MaybeUninit;
use std::ffi::CStr;

pub struct PhysicalDevice {
    pub(super) raw_handle: VkPhysicalDevice,
}

pub enum DeviceType {
    Other,
    IntegratedGPU,
    DiscreteGPU,
    VirtualGPU,
    CPU
}

pub struct Properties {
    pub name: String,
    pub device_type: DeviceType
}

impl PhysicalDevice {
    pub fn get_properties(&self) -> Properties {
        unsafe {
            let mut properties = MaybeUninit::<VkPhysicalDeviceProperties>::uninit();
            vkGetPhysicalDeviceProperties(self.raw_handle, properties.as_mut_ptr());
            let properties = properties.assume_init();

            Properties {
                name: CStr::from_ptr(&(properties.deviceName[0]))
                    .to_str()
                    .unwrap()
                    .to_owned(),
                device_type: match properties.deviceType {
                    VK_PHYSICAL_DEVICE_TYPE_OTHER => DeviceType::Other,
                    VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU => DeviceType::IntegratedGPU,
                    VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU => DeviceType::DiscreteGPU,
                    VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU => DeviceType::VirtualGPU,
                    VK_PHYSICAL_DEVICE_TYPE_CPU => DeviceType::CPU,
                    _ => DeviceType::Other
                }
            }
        }
    }
}

// I might add more stuff here later.
