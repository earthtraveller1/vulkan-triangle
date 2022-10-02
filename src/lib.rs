#[allow(nonstandard_style)]
pub mod vulkan {
    include!(concat!(env!("OUT_DIR"), "/vulkan-ffi.rs"));
    
    pub fn VK_MAKE_API_VERSION(variant: u32, major: u32, minor: u32, patch: u32) -> u32 {
        (((variant)) << 29) | (((major)) << 22) | (((minor)) << 12) | ((patch))
    }
}