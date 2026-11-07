fn main() {
    tauri_build::try_build(tauri_build::Attributes::new().app_manifest(
        tauri_build::AppManifest::new().commands(&[
            "save_read",
            "save_write",
            "finish_close",
            "diagnostics_write",
            "update_check",
            "update_install",
        ]),
    ))
    .expect("MachDefense desktop build configuration");
}
