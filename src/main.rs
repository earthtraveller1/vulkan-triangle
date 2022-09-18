use vulkan_triangle::backend::Instance;
use vulkan_triangle::Window;

fn main() {
    let mut window = Window::new(800, 600, "Vulkan Triangle", None);

    Instance::enumerate_extension_names()
        .iter()
        .for_each(|extension| println!("[INFO]: Found Vulkan Instance extension {}", extension));

    let instance = Instance::new("Vulkan Triangle", &window, 0, 1, 0, true)
        .expect("Failed to create the Vulkan instance.");

    instance
        .create_debug_messenger()
        .expect("[ERROR]: Failed to create the debug messenger!");

    window.show();

    while window.is_open() {
        window.update();
    }
}
