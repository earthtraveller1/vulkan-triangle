use std::ptr::null;

use super::ffi::vulkan::*;
use super::Instance;

pub struct Surface<'a> {
    pub(super) instance: &'a Instance,
    pub(super) raw_handle: VkSurfaceKHR,
}

impl<'a> Drop for Surface<'a> {
    fn drop(&mut self) {
        unsafe { vkDestroySurfaceKHR(self.instance.raw_handle, self.raw_handle, null()) };
    }
}
