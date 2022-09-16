use super::ffi::vulkan::*;
use std::{
    convert::TryInto,
    ptr::{null, null_mut},
};

pub struct DebugMessenger {
    raw_handle: VkDebugUtilsMessengerEXT
}

impl DebugMessenger {
    fn new() -> Result<DebugMessenger, ()> {
        Err(())
    }
}