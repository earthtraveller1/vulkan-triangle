use vulkan_triangle::backend::Instance;

fn main() {
    Instance::enumerate_extension_names()
        .iter()
        .for_each(|extension| println!("[INFO]: Found Vulkan Instance extension {}", extension));

    Instance::new("Vulkan Triangle", 0, 1, 0, true).expect("Failed to create the Vulkan instance.");
}
