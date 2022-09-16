use std::{env, path::PathBuf};

fn main() {
    println!("cargo:rustc-link-lib=vulkan");
    println!("cargo:rerun-if-changed=src/ffi/vulkan.h");

    let out_path = PathBuf::from(env::var("OUT_DIR").unwrap());

    bindgen::Builder::default()
        .header("src/backend/ffi/vulkan.h")
        .prepend_enum_name(false)
        .parse_callbacks(Box::new(bindgen::CargoCallbacks))
        .generate()
        .expect("Unable to generate bindings for src/ffi/vulkan.h")
        .write_to_file(out_path.join("vulkan-ffi.rs"))
        .expect("Couldn't write bindings to file.");
}
