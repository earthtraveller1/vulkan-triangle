use vulkan_triangle::backend::Instance;
use vulkan_triangle::Window;

fn main() {
    let mut window = Window::new(800, 600, "Vulkan Triangle", None);

    Instance::enumerate_extension_names()
        .iter()
        .for_each(|extension| println!("[INFO]: Found Vulkan Instance extension {}", extension));

    let instance = Instance::new("Vulkan Triangle", &window, 0, 1, 0, true)
        .expect("Failed to create the Vulkan instance.");

    let _debug_messenger = instance
        .create_debug_messenger()
        .expect("[ERROR]: Failed to create the debug messenger!");

    let _surface = instance.create_window_surface(&window);

    instance
        .enumerate_physical_devices()
        .iter()
        .for_each(|physical_device| {
            println!(
                "[INFO]: Found physical device {}",
                physical_device.get_properties().name
            )
        });

    window.show();

    while window.is_open() {
        window.update();
    }
}
