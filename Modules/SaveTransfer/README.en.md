# MachDefense recovery code

<!-- language-nav:start -->
[한국어](README.md) | [English](README.en.md)
<!-- language-nav:end -->

`Codec.hpp` preserves all five original save slots and unknown bytes. `MD1-` contains version 1, CRC32 of the 42,496-byte plaintext, bounded PackBits and Base64url. The original save seed and checksum remain in the payload; decoding and re-encoding the save preserves its bytes.

There is no server, salt, encryption or platform-specific codec. The codec uses the existing `SaveData` accessors. The shared menu owns GET CODE / ENTER CODE and the confirmation dialog. Hosts only supply text entry and durable storage. Hosts that do not enable transfer retain their existing menu.

Apple, Android and Web use the same codec and C++ menu. GET CODE opens a read-only field; COPY copies its contents. ENTER CODE opens a writable field; CHECK validates it, then the original YES/NO buttons confirm replacement of all five slots. CLOSE and CANCEL leave the save unchanged. Hosts supply the text field, keyboard and clipboard.

`FileStore.hpp` implements the Apple/Android restore transaction: backup, pending payload, atomic replacement, re-read, durable commit marker and cleanup. Startup completes an interrupted transaction. A commit marker prevents an old journal from overwriting later gameplay saves. An uncertain I/O failure blocks further saves until restart. Web uses its existing IndexedDB transaction for the backup and replacement.

Single-slot selection and future settings extensions are proposals; the current feature transfers all five slots. See [implementation and checks](../../docs/evidence/save-transfer-all-platforms-20261106.md).
