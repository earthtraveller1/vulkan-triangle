use std::{env, path::PathBuf};

const FFI_HEADER: &str = "src/ffi.h";

fn main() {
    let target_os = env::var("CARGO_CFG_TARGET_OS").unwrap();

    if target_os == "linux" {
        println!("cargo:rustc-link-lib=vulkan");
        println!("cargo:rerun-if-changed=src/ffi/vulkan.h");

        let out_path = PathBuf::from(env::var("OUT_DIR").unwrap());

        bindgen::Builder::default()
            .header(FFI_HEADER)
            .prepend_enum_name(false)
            .parse_callbacks(Box::new(bindgen::CargoCallbacks))
            .generate()
            .expect("Unable to generate bindings for src/ffi/vulkan.h")
            .write_to_file(out_path.join("vulkan-ffi.rs"))
            .expect("Couldn't write bindings to file.");
    } else if target_os == "windows" {
        let vulkan_sdk = env::var("VULKAN_SDK");
        if let Ok(vulkan_sdk_path) = vulkan_sdk {
            println!("cargo:rustc-link-search={}/Lib", vulkan_sdk_path);
            println!("cargo:rustc-link-lib=vulkan-1");
            println!("cargo:rerun-if-changed=src/ffi/vulkan.h");

            let out_path = PathBuf::from(env::var("OUT_DIR").unwrap());

            bindgen::Builder::default()
                .header(FFI_HEADER)
                .prepend_enum_name(false)
                .clang_arg(format!("-I{}/Include", vulkan_sdk_path))
                .parse_callbacks(Box::new(bindgen::CargoCallbacks))
                .generate()
                .expect("Unable to generate bindings for src/ffi/vulkan.h")
                .write_to_file(out_path.join("vulkan-ffi.rs"))
                .expect("Couldn't write bindings to file.");
        } else {
            // If the VULKAN_SDK variable is not set, we simply try to find the
            // SDK where it might be.
            let sdks: Vec<PathBuf> = std::fs::read_dir("C:/VulkanSDK")
                .expect("C:/VulkanSDK does not appear to exist!")
                .map(|x| {
                    let x = x.unwrap();
                    x.path()
                })
                .collect();

            // In production applications we might wanted to find the latest v-
            // ersion of the SDK to use, but for now I'll just use the first o-
            // ne in the directory. This is because most people usually only h-
            // ave one version of the Vulkan SDK installed.
            let sdk = &sdks[0];

            println!("cargo:rustc-link-search={}/Lib", sdk.to_str().unwrap());
            println!("cargo:rustc-link-lib=vulkan-1");
            println!("cargo:rerun-if-changed=src/ffi/vulkan.h");

            let out_path = PathBuf::from(env::var("OUT_DIR").unwrap());

            bindgen::Builder::default()
                .header(FFI_HEADER)
                .prepend_enum_name(false)
                .clang_arg(format!("-I{}/Include", sdk.to_str().unwrap()))
                .parse_callbacks(Box::new(bindgen::CargoCallbacks))
                .generate()
                .expect("Unable to generate bindings for src/ffi/vulkan.h")
                .write_to_file(out_path.join("vulkan-ffi.rs"))
                .expect("Couldn't write bindings to file.");
        }
    }
}
