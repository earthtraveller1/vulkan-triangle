use std::sync::mpsc::Receiver;
use crate::backend::ffi::vulkan::*;
use std::ptr::null;

pub type EventHandler = fn(glfw::WindowEvent);

pub struct Window {
    glfw: glfw::Glfw,
    window: glfw::Window,
    events: Receiver<(f64, glfw::WindowEvent)>,
    event_handler: Option<EventHandler>,
}

fn glfw_error_callback<'r>(error: glfw::Error, message: String, _: &'r ()) {
    println!("[GLFW ERROR {}]: {}", error, message);
}

impl Window {
    pub fn new(
        width: u32,
        height: u32,
        title: &str,
        event_handler: Option<EventHandler>,
    ) -> Window {
        let mut glfw = glfw::init(Some(glfw::Callback {
            f: glfw_error_callback,
            data: (),
        }))
        .expect("Failed to initialize GLFW.");

        glfw.window_hint(glfw::WindowHint::ClientApi(glfw::ClientApiHint::NoApi));
        glfw.window_hint(glfw::WindowHint::Resizable(false));
        glfw.window_hint(glfw::WindowHint::Visible(false));

        let (mut window, events) = glfw
            .create_window(width, height, title, glfw::WindowMode::Windowed)
            .expect("Failed to create GLFW window.");

        window.set_key_polling(true);

        Window {
            glfw,
            window,
            events,
            event_handler,
        }
    }
    
    pub fn get_required_instance_extensions(&self) -> Option<Vec<String>> {
        self.glfw.get_required_instance_extensions()
    }
    
    pub fn create_window_surface(&self, instance: VkInstance, surface: *mut VkSurfaceKHR) -> VkResult {
        self.window.create_window_surface(instance as usize, null(), surface as *mut u64).try_into().unwrap()
    }
    
    pub fn show(&mut self) {
        self.window.show();
    }

    pub fn is_open(&self) -> bool {
        !self.window.should_close()
    }

    pub fn update(&mut self) {
        self.glfw.poll_events();
        glfw::flush_messages(&self.events).for_each(|(_, event)| {
            if let Some(handler) = self.event_handler {
                handler(event);
            }
        });
    }
}
