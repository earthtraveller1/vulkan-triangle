#[allow(nonstandard_style)]
pub mod vulkan {
    include!(concat!(env!("OUT_DIR"), "/vulkan-ffi.rs"));
}