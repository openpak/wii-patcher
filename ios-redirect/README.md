# OpenPak Wii IOS SO-module redirect -- spike prototype

Status: **research/prototype only** (board card openpak/prds#129). Nothing here
is shipped, installed, signed, or run on hardware. No installer. This folder
holds the public-safe patch *source*; it contains **no Nintendo bytes**.

## What it does

The Wii resolves hostnames inside the IOS `SO`/IP module (`/dev/net/ip/top`
ioctl 0x11, `SOGetHostByName`). This hook makes only names on an embedded,
already-verified suffix list resolve to the OpenPak primary IP; everything else
resolves normally.

- `hook.c` -- the matcher. `openpak_hook(name)` returns an embedded
  dotted-decimal IP string when `name` is on the ceiling suffix list (matched on
  a DNS label boundary, case-folded), otherwise returns `name` unchanged. The
  suffix table and IP are **baked by the patcher** after it verifies the signed
  ceiling envelope; the values in `hook.c` are spike placeholders (RFC5737 IP).
- `stub.s` -- the trampoline glue: calls `openpak_hook`, then replays the
  resolver's displaced prologue and jumps back into the stock resolver body.
  On a match the stock resolver is handed the IP string and builds the hostent
  via its own IP-literal (`inet_aton`) path -- i.e. the console's own code does
  the hostent construction; we only change which name it sees.
- `cave.ld` / `build.sh` -- link the cave at the per-binary base address and
  emit a raw big-endian Thumb blob.
- `harness.py` / `so.py` -- a Unicorn proof harness (see below). Point it at a
  **locally decrypted** SO module that you obtained from your own console or
  from NUS; no such binary is included.

## Install model (later, not built here)

The patcher downloads the stock IOS from NUS, decrypts the SO content, derives
the two per-binary offsets, bakes the verified suffix list + IP into the cave,
writes an 8-byte trampoline over the resolver prologue, places the cave in the
~2 KB gap before the data segment, extends the RX program header, re-hashes and
fakesigns the title, and installs it in place (RiiConnect24 model). When the
ceiling list changes the patcher re-applies through its normal update prompt.
Scope: original Wii with a BootMii backup only -- not vWii, Wii mini, or
new-boot1 consoles.

## Proof (spike)

`build.sh` then `python harness.py` runs three cases against each of the two
real SO binaries, fully offline:

1. a listed name is handed to the stock resolver as the OpenPak IP;
2. an unlisted name is passed through unchanged;
3. the hook writes zero bytes to module data (only stack); the code image
   changes only at the 8-byte trampoline.

The harness deliberately stops at the hook/stock-resolver hand-off: the stock
IP-literal -> hostent path runs ARM-mode (BE32) libc that Unicorn does not
emulate reliably, and that path is stock Nintendo code exercised on every
literal-IP connection, not ours to prove.

## Licensing

Our code (`hook.c`, `stub.s`, `cave.ld`, harness) is OpenPak's. The eventual
installer may reuse d2x trust-chain/install code, which is GPLv3 -- acceptable
per the owner's 2026-11-05 decision. No Nintendo binary is ever redistributed.
