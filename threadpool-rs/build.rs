// Generate bindings the the C libthreadpool on the fly
//   based on the bindgen manual
//
// Austin Shafer - 2019

extern crate bindgen;

fn main () {
    // tell cargo to link with libthreadpool.so
    println!("cargo:rustc-env=DYLD_LIBRARY_PATH=/Users/AShafer/git/threadbox/threadpool/");
    println!("cargo:rustc-link-search=native=/Users/AShafer/git/threadbox/threadpool/");
    println!("cargo:rustc-link-lib=dylib=threadpool");

    // update the generated bindings if the wrapper changes
    println!("cargo:rerun-if-changed=wrapper.h");

    let bindings = bindgen::Builder::default()
        .header("wrapper.h")
        .rustfmt_bindings(true)
        .whitelist_function("pool_init")
        .whitelist_function("pool_exec")
        .whitelist_function("pool_destroy")
        .generate()
        .expect("Could not generate bindings for libthreadpool");

    bindings.write_to_file("src/threadpool_bindings.rs")
        .expect("Could not write bindings to ./threadpool_bindings.rs");
}
