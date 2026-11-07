#![cfg_attr(not(debug_assertions), windows_subsystem = "windows")]
mod store;
#[cfg(feature = "validation")]
mod update_validation;
use std::sync::{
    atomic::{AtomicBool, Ordering},
    Arc, Mutex,
};
use tauri::{Manager, WebviewUrl, WebviewWindow};
use tauri_plugin_updater::UpdaterExt;
struct State {
    writes: Arc<Mutex<()>>,
    allow_exit: AtomicBool,
}
fn root(w: &WebviewWindow, cheat: bool) -> Result<std::path::PathBuf, String> {
    let url = w.url().map_err(|e| e.to_string())?;
    if w.label() != "main"
        || !((url.scheme() == "tauri" && url.host_str() == Some("localhost"))
            || (url.scheme() == "http" && url.host_str() == Some("tauri.localhost")))
    {
        return Err("Local game window required".into());
    }
    let base = w
        .app_handle()
        .path()
        .app_data_dir()
        .map_err(|e| e.to_string())?;
    let base = if cfg!(feature = "validation") {
        base.join("validation")
    } else {
        base
    };
    Ok(base.join(if cheat { "saves-cheat" } else { "saves" }))
}
#[tauri::command]
async fn save_read(
    w: WebviewWindow,
    state: tauri::State<'_, State>,
    cheat: bool,
) -> Result<Option<store::Snapshot>, String> {
    let dir = root(&w, cheat)?;
    let lock = state.writes.clone();
    tauri::async_runtime::spawn_blocking(move || {
        let _guard = lock.lock().map_err(|e| e.to_string())?;
        store::read(&dir)
    })
    .await
    .map_err(|e| e.to_string())?
}
#[tauri::command]
async fn save_write(
    w: WebviewWindow,
    state: tauri::State<'_, State>,
    cheat: bool,
    bytes: Vec<u8>,
    expected: u64,
    restoring: bool,
) -> Result<store::Snapshot, String> {
    let dir = root(&w, cheat)?;
    let lock = state.writes.clone();
    tauri::async_runtime::spawn_blocking(move || {
        let _guard = lock.lock().map_err(|e| e.to_string())?;
        store::write(&dir, bytes, expected, restoring)
    })
    .await
    .map_err(|e| e.to_string())?
}
#[tauri::command]
fn finish_close(w: WebviewWindow, state: tauri::State<'_, State>) -> Result<(), String> {
    root(&w, false)?;
    state.allow_exit.store(true, Ordering::SeqCst);
    w.app_handle().exit(0);
    Ok(())
}
#[tauri::command]
async fn update_check(w: WebviewWindow) -> Result<Option<String>, String> {
    root(&w, false)?;
    let update = w
        .app_handle()
        .updater()
        .map_err(|e| e.to_string())?
        .check()
        .await
        .map_err(|e| e.to_string())?;
    Ok(update.map(|u| u.version))
}
#[tauri::command]
async fn update_install(
    w: WebviewWindow,
    state: tauri::State<'_, State>,
    version: String,
) -> Result<(), String> {
    root(&w, false)?;
    let update = w
        .app_handle()
        .updater()
        .map_err(|e| e.to_string())?
        .check()
        .await
        .map_err(|e| e.to_string())?
        .ok_or("No update available")?;
    if update.version != version {
        return Err("Update changed. Reopen the launcher.".into());
    }
    update
        .download_and_install(|_, _| {}, || {})
        .await
        .map_err(|e| e.to_string())?;
    state.allow_exit.store(true, Ordering::SeqCst);
    w.app_handle().restart()
}
#[tauri::command]
fn diagnostics_write(w: WebviewWindow, report: String) -> Result<(), String> {
    let dir = root(&w, true)?;
    if report.len() > 1048576 {
        return Err("Report too large".into());
    }
    std::fs::create_dir_all(&dir).map_err(|e| e.to_string())?;
    std::fs::write(dir.join("diagnostics.json"), report).map_err(|e| e.to_string())
}
fn close(w: &WebviewWindow) {
    let _ =
        w.eval("window.MDDesktop?MDDesktop.close():window.__TAURI__.core.invoke('finish_close')");
}
fn main() {
    let app = tauri::Builder::default()
        .plugin(tauri_plugin_updater::Builder::new().build())
        .plugin(tauri_plugin_single_instance::init(|app, _, _| {
            if let Some(w) = app.get_webview_window("main") {
                let _ = w.set_focus();
            }
        }))
        .manage(State {
            writes: Arc::new(Mutex::new(())),
            allow_exit: AtomicBool::new(false),
        })
        .invoke_handler(tauri::generate_handler![
            save_read,
            save_write,
            finish_close,
            diagnostics_write,
            update_check,
            update_install
        ])
        .setup(|app| {
            #[cfg(feature = "validation")]
            if let Some(dir) = std::env::var_os("MD_DESKTOP_UPDATE_TEST") {
                let dir = std::path::PathBuf::from(dir);
                let result = update_validation::run(app.handle(), &dir);
                std::fs::write(
                    dir.join("result.json"),
                    serde_json::to_vec_pretty(&result).unwrap(),
                )?;
                app.state::<State>()
                    .allow_exit
                    .store(true, Ordering::SeqCst);
                app.handle()
                    .exit(if result["pass"] == true { 0 } else { 1 });
                return Ok(());
            }
            use tauri::menu::{MenuBuilder, MenuItemBuilder, SubmenuBuilder};
            let quit = MenuItemBuilder::with_id("quit", "Quit MachDefense Desktop")
                .accelerator("CmdOrCtrl+Q")
                .build(app)?;
            let application = SubmenuBuilder::new(app, "MachDefense Desktop")
                .hide()
                .hide_others()
                .show_all()
                .separator()
                .item(&quit)
                .build()?;
            let edit = SubmenuBuilder::new(app, "Edit")
                .undo()
                .redo()
                .separator()
                .cut()
                .copy()
                .paste()
                .select_all()
                .build()?;
            let window = SubmenuBuilder::new(app, "Window")
                .minimize()
                .maximize()
                .build()?;
            app.set_menu(
                MenuBuilder::new(app)
                    .items(&[&application, &edit, &window])
                    .build()?,
            )?;
            app.on_menu_event(|app, event| {
                if event.id().as_ref() == "quit" {
                    if let Some(w) = app.get_webview_window("main") {
                        close(&w);
                    }
                }
            });
            let w =
                tauri::WebviewWindowBuilder::new(app, "main", WebviewUrl::App("index.html".into()))
                    .title("MachDefense Desktop")
                    .inner_size(700., 1000.)
                    .min_inner_size(320., 480.)
                    .on_new_window(|_, _| tauri::webview::NewWindowResponse::Deny)
                    .on_navigation(|u| {
                        (u.scheme() == "tauri" && u.host_str() == Some("localhost"))
                            || (u.scheme() == "http" && u.host_str() == Some("tauri.localhost"))
                    })
                    .build()?;
            #[cfg(feature = "validation")]
            if std::env::var_os("MD_DESKTOP_AUTOTEST").is_some() {
                let code = std::env::var_os("MD_DESKTOP_RECOVERY_FILE")
                    .and_then(|p| std::fs::read_to_string(p).ok());
                let script = format!(
                    "window.MDValidationRecovery={};\n{}",
                    serde_json::to_string(&code).unwrap(),
                    include_str!("../../frontend/validation.js")
                );
                let test_window = w.clone();
                std::thread::spawn(move || {
                    std::thread::sleep(std::time::Duration::from_secs(2));
                    let _ = test_window.eval(&script);
                });
            }
            let handle = w.clone();
            w.on_window_event(move |event| {
                if let tauri::WindowEvent::CloseRequested { api, .. } = event {
                    api.prevent_close();
                    close(&handle);
                }
            });
            Ok(())
        })
        .build(tauri::generate_context!())
        .expect("Create MachDefense Desktop");
    app.run(|app, event| {
        if let tauri::RunEvent::ExitRequested { api, .. } = event {
            if !app.state::<State>().allow_exit.load(Ordering::SeqCst) {
                api.prevent_exit();
                if let Some(w) = app.get_webview_window("main") {
                    close(&w);
                }
            }
        }
    });
}
