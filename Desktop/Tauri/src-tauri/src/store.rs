use serde::{Deserialize, Serialize};
use sha2::{Digest, Sha256};
use std::{fs, io::Write, path::Path};
#[derive(Clone, Debug, Serialize, Deserialize)]
pub struct Snapshot {
    pub bytes: Vec<u8>,
    pub revision: u64,
    pub sha256: String,
}
impl Snapshot {
    fn checked(&self) -> Result<(), String> {
        if self.bytes.len() != 42496 || format!("{:x}", Sha256::digest(&self.bytes)) != self.sha256
        {
            return Err("Saved game checksum mismatch. The file has been kept.".into());
        }
        Ok(())
    }
}
pub fn read(root: &Path) -> Result<Option<Snapshot>, String> {
    let p = root.join("current.json");
    if !p.exists() {
        return Ok(None);
    }
    let metadata = fs::metadata(&p).map_err(|e| e.to_string())?;
    if metadata.len() > 262144 {
        return Err("Saved game is too large. The file has been kept.".into());
    }
    let s: Snapshot = serde_json::from_slice(&fs::read(p).map_err(|e| e.to_string())?)
        .map_err(|e| e.to_string())?;
    s.checked()?;
    Ok(Some(s))
}
fn atomic(root: &Path, name: &str, bytes: &[u8]) -> Result<(), String> {
    let mut temp = tempfile::NamedTempFile::new_in(root).map_err(|e| e.to_string())?;
    temp.write_all(bytes)
        .and_then(|_| temp.as_file().sync_all())
        .map_err(|e| e.to_string())?;
    temp.persist(root.join(name)).map_err(|e| e.to_string())?;
    fs::File::open(root)
        .and_then(|f| f.sync_all())
        .map_err(|e| e.to_string())
}
pub fn write(
    root: &Path,
    bytes: Vec<u8>,
    expected: u64,
    restoring: bool,
) -> Result<Snapshot, String> {
    if bytes.len() != 42496 {
        return Err("Invalid saved game length".into());
    }
    fs::create_dir_all(root).map_err(|e| e.to_string())?;
    let previous = read(root)?;
    if previous.as_ref().map_or(0, |s| s.revision) != expected {
        return Err("Newer progress exists. Reopen MachDefense.".into());
    }
    let revision = expected.checked_add(1).ok_or("Save revision overflow")?;
    if let Some(old) = previous {
        let data = serde_json::to_vec(&old).map_err(|e| e.to_string())?;
        atomic(root, "backup.json", &data)?;
        if restoring {
            atomic(root, "restore-backup.json", &data)?;
        }
    }
    let next = Snapshot {
        sha256: format!("{:x}", Sha256::digest(&bytes)),
        bytes,
        revision,
    };
    atomic(
        root,
        "current.json",
        &serde_json::to_vec(&next).map_err(|e| e.to_string())?,
    )?;
    let verified = read(root)?.ok_or("Saved game not found after write")?;
    if verified.sha256 != next.sha256 || verified.revision != next.revision {
        return Err("Saved game verification failed".into());
    }
    Ok(next)
}
#[cfg(test)]
mod tests {
    use super::*;
    #[test]
    fn roundtrip_backup_conflict_corruption() {
        let dir = tempfile::tempdir().unwrap();
        let p = dir.path();
        assert!(read(p).unwrap().is_none());
        let a = write(p, vec![1; 42496], 0, false).unwrap();
        assert_eq!(read(p).unwrap().unwrap().bytes, a.bytes);
        assert!(write(p, vec![2; 42496], 0, false).is_err());
        assert!(write(p, vec![0; 10], 1, false).is_err());
        let b = write(p, vec![2; 42496], 1, true).unwrap();
        assert_eq!(b.revision, 2);
        let backup: Snapshot =
            serde_json::from_slice(&fs::read(p.join("restore-backup.json")).unwrap()).unwrap();
        assert_eq!(backup.bytes, a.bytes);
        fs::write(p.join("current.json"), b"damaged").unwrap();
        assert!(read(p).is_err());
        assert!(write(p, vec![3; 42496], 2, false).is_err());
        assert_eq!(fs::read(p.join("current.json")).unwrap(), b"damaged");
    }
    #[test]
    fn interrupted_temp_does_not_replace() {
        let dir = tempfile::tempdir().unwrap();
        let p = dir.path();
        write(p, vec![7; 42496], 0, false).unwrap();
        fs::write(p.join("uncommitted.tmp"), b"partial").unwrap();
        assert_eq!(read(p).unwrap().unwrap().bytes, vec![7; 42496]);
    }
    #[test]
    fn checksum_detects_valid_json_damage() {
        let dir = tempfile::tempdir().unwrap();
        let p = dir.path();
        let mut s = write(p, vec![7; 42496], 0, false).unwrap();
        s.bytes[100] = 8;
        fs::write(p.join("current.json"), serde_json::to_vec(&s).unwrap()).unwrap();
        assert!(read(p).is_err());
    }
}
