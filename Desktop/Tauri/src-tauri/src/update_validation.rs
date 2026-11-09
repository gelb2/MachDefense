use serde_json::{json, Value};
use std::{fs, path::Path};
use tauri_plugin_updater::UpdaterExt;
pub fn run(app: &tauri::AppHandle, dir: &Path) -> Value {
    let result = (|| -> Result<Value, Box<dyn std::error::Error>> {
        // The caller creates this disposable bundle; never target the installed app.
        if !dir.join("disposable-update-test").is_file() {
            return Err("Missing test marker".into());
        }
        let exe = dir.join("MachDefense Desktop.app/Contents/MacOS/machdefense-desktop");
        let before = fs::read(&exe)?;
        let bytes = fs::read(dir.join("update.app.tar.gz"))?;
        let signature = fs::read_to_string(dir.join("update.app.tar.gz.sig"))?;
        let metadata = json!({"version":env!("CARGO_PKG_VERSION"),"platforms":{"darwin-aarch64":{"url":"https://localhost.invalid/update.app.tar.gz","signature":signature.trim()}}});
        let updater = app
            .updater_builder()
            .executable_path(&exe)
            .version_comparator(|_, _| true)
            .build()?;
        let update = updater
            .restore_update(metadata.clone())?
            .ok_or("No fixture update")?;
        update.verify(&bytes)?;
        let mut damaged = bytes.clone();
        let n = damaged.len() / 2;
        damaged[n] ^= 1;
        assert!(update.install(&damaged).is_err());
        assert_eq!(fs::read(&exe)?, before);
        assert!(update.install(&bytes[..bytes.len() / 2]).is_err());
        assert_eq!(fs::read(&exe)?, before);
        let mut wrong = metadata.clone();
        wrong["version"] = json!("999.0.0");
        assert!(updater
            .restore_update(wrong)?
            .unwrap()
            .verify(&bytes)
            .is_err());
        let mut older = metadata.clone();
        older["version"] = json!("0.5.0");
        assert!(app.updater()?.restore_update(older)?.is_none());
        update.install(&bytes)?;
        assert_eq!(fs::read(&exe)?, fs::read(dir.join("expected-executable"))?);
        assert_eq!(
            fs::read(dir.join("save-marker"))?,
            b"progress outside bundle"
        );
        Ok(
            json!({"pass":true,"validSignature":true,"damagedRejected":true,"truncatedRejected":true,"versionMismatchRejected":true,"downgradeIgnored":true,"bundleInstalled":true,"outsideSavePreserved":true}),
        )
    })();
    match result {
        Ok(v) => v,
        Err(e) => json!({"pass":false,"error":e.to_string()}),
    }
}
