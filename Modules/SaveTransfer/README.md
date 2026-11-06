# MachDefense recovery code

`Codec.hpp` preserves all five original save slots and unknown bytes. `MD1-` contains version 1, CRC32 of the 42,496-byte plaintext, bounded PackBits and Base64url. The original save seed and checksum remain in the payload; decoding and re-encoding the save preserves its bytes.

There is no server, salt, encryption or platform-specific codec. The codec uses the existing `SaveData` accessors. The shared menu owns GET CODE / ENTER CODE and the confirmation dialog. Hosts only supply text entry and durable storage. Hosts that do not enable transfer retain their existing menu.

The initial PWA UI restores all five slots. Single-slot selection and native Apple/Android transfer UI connections remain follow-up work; raw iPhone save copies can already be encoded by the same module for PWA tests.
