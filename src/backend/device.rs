use std::collections::HashSet;
use std::convert::TryInto;
use std::mem::MaybeUninit;
use std::ptr::null;
use std::ptr::null_mut;

use super::ffi::vulkan::*;
use super::physical_device::QueueFamilyIndices;
use super::PhysicalDevice;

pub struct Device {
    raw_handle: VkDevice,
}

impl Device {
    pub(super) fn new(
        physical_device: &PhysicalDevice,
        queue_family_indices: &QueueFamilyIndices,
    ) -> Result<Device, ()> {
        let (graphics_queue_family, present_queue_family) = if !queue_family_indices.is_complete() {
            return Err(());
        } else {
            (
                if let Some(x) = queue_family_indices.present_family {
                    x
                } else {
                    0
                },
                if let Some(x) = queue_family_indices.graphics_family {
                    x
                } else {
                    0
                },
            )
        };

        let mut unique_queue_families = HashSet::new();
        unique_queue_families.insert(graphics_queue_family);
        unique_queue_families.insert(present_queue_family);

        let queue_priority = 1.0f32;

        let queue_create_infos = unique_queue_families
            .iter()
            .map(|queue_family| VkDeviceQueueCreateInfo {
                sType: VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                pNext: null(),
                flags: 0,
                queueFamilyIndex: queue_family.clone(),
                queueCount: 1,
                pQueuePriorities: &queue_priority,
            })
            .collect::<Vec<VkDeviceQueueCreateInfo>>();
            
        let device_features = MaybeUninit::<VkPhysicalDeviceFeatures>::uninit();
        
        let create_info = VkDeviceCreateInfo {
            sType: VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            pNext: null(),
            flags: 0,
            queueCreateInfoCount: queue_create_infos.len().try_into().unwrap(),
            pQueueCreateInfos: queue_create_infos.as_ptr(),
            enabledLayerCount: 0,
            ppEnabledLayerNames: null(),
            enabledExtensionCount: 0,
            ppEnabledExtensionNames: null(),
            pEnabledFeatures: device_features.as_ptr()
        };
        
        let mut raw_handle = null_mut();
        let result = unsafe { vkCreateDevice(physical_device.raw_handle, &create_info, null(), &mut raw_handle) };
        if result != VK_SUCCESS {
            Err(())
        } else {
            Ok(Device { raw_handle })
        }
    }
}

impl Drop for Device {
    fn drop(&mut self) {
        unsafe { vkDestroyDevice(self.raw_handle, null()) };
    }
}
