use super::{ffi::vulkan::*, Surface};
use core::mem::MaybeUninit;
use std::{convert::TryInto, ffi::CStr, ptr::null_mut};

pub struct PhysicalDevice {
    pub(super) raw_handle: VkPhysicalDevice,
}

pub enum DeviceType {
    Other,
    IntegratedGPU,
    DiscreteGPU,
    VirtualGPU,
    CPU,
}

pub struct Properties {
    pub name: String,
    pub device_type: DeviceType,
}

pub struct QueueFamilyIndices {
    pub graphics_family: Option<u32>,
    pub present_family: Option<u32>,
}

impl QueueFamilyIndices {
    pub fn is_complete(&self) -> bool {
        if let Some(_) = self.graphics_family {
            if let Some(_) = self.present_family {
                true
            } else {
                false
            }
        } else {
            false
        }
    }
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
                    _ => DeviceType::Other,
                },
            }
        }
    }

    pub fn find_queue_families(&self, surface: &Surface) -> QueueFamilyIndices {
        let mut queue_family_count = 0;
        unsafe {
            vkGetPhysicalDeviceQueueFamilyProperties(
                self.raw_handle,
                &mut queue_family_count,
                null_mut(),
            )
        };

        let mut queue_family_properties =
            Vec::with_capacity(queue_family_count.try_into().unwrap());
        unsafe {
            vkGetPhysicalDeviceQueueFamilyProperties(
                self.raw_handle,
                &mut queue_family_count,
                queue_family_properties.as_mut_ptr(),
            );
            queue_family_properties.set_len(queue_family_count.try_into().unwrap());
        }

        let mut indices = QueueFamilyIndices {
            graphics_family: None,
            present_family: None,
        };

        for i in 0..queue_family_properties.len() {
            {
                let a: u32 = queue_family_properties[i].queueFlags;
                let b: u32 = VK_QUEUE_GRAPHICS_BIT.try_into().unwrap();
                let c: u32 = a & b;

                if c > 0 {
                    indices.graphics_family = Some(i.try_into().unwrap());
                }
            }

            if indices.is_complete() {
                break;
            }

            let mut present_support = VK_FALSE;
            unsafe {
                vkGetPhysicalDeviceSurfaceSupportKHR(
                    self.raw_handle,
                    i.try_into().unwrap(),
                    surface.raw_handle,
                    &mut present_support,
                )
            };

            if present_support == VK_TRUE {
                indices.present_family = Some(i.try_into().unwrap());
            }
        }

        indices
    }
}

// I might add more stuff here later.
