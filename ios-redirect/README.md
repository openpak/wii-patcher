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

## Repack + fakesign (`repack.py`)

`repack.py` is the proven step-1 chain: given the stock TMD/ticket and the
decrypted SO content, it grows the RX `PT_LOAD` to map the cave, writes the
trampoline, re-encrypts the content, updates the TMD content record, and
fakesigns the TMD (trucha: zero the RSA signature, brute-force an unchecked pad
until `SHA1(body)[0]==0`). It contains **no Nintendo bytes**: the Wii common
key is read from `OPENPAK_WII_COMMON_KEY` (the patcher gets it on-console, from
OTP on vWii), never embedded. It refuses any SO binary whose sha1 is not one of
the two hooked targets. It assembles/installs nothing.

## Install model + scope (installer not built here)

The patcher derives the SO from the console's own IOS slot (vWii-exclusive
builds are not on the open NUS), bakes the ceiling-verified suffix list + IP,
applies the trampoline/cave, extends the RX program header, re-hashes and
fakesigns, and installs **slot 36 only** in place (never IOS80 / System-Menu
titles). Re-applies through the normal update prompt when the list changes.
Restores the stock slot exactly on request. DNS stays the fallback.

Scope: original Wii with a BootMii backup, and vWii (with a verified
SLCCMPT+OTP backup gate). Wii mini and new-boot1 are refused.

**vWii acceptance is UNPROVEN (gating).** vWii IOS have the trucha bug disabled
and the System Menu runs stock IOS80, so whether a fakesigned in-place IOS36 is
accepted at disc launch is undocumented and can only be settled by a hardware
probe (see research/wii/on-console-redirect.md -> owner test steps). The
release is HELD until that probe passes; until then vWii uses OpenPak DNS.
Recovery is Wii U-side (vWii NAND Restorer / Decaffeinator), no BootMii needed.

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
