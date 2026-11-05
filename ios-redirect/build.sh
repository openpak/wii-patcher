#!/usr/bin/env bash
# Build the OpenPak SO-module redirect cave (spike prototype, card #129).
# Produces position-fixed, big-endian Thumb cave blobs for each target SO
# binary. Requires clang + ld.lld + llvm-objcopy (LLVM with ARM backend).
#
# NOTE: this emits ONLY our own hook code. It does not contain, download, or
# redistribute any Nintendo bytes. The per-binary link base (BASE) and the
# resume address (RESUME_THUMB) are the offsets the patcher derives from the
# user's own decrypted SO module (see ../README and research/wii).
set -euo pipefail
cd "$(dirname "$0")"
TGT="--target=armebv5te-none-eabi -mthumb -mfloat-abi=soft"

build() { # name BASE RESUME_THUMB
  clang $TGT -O2 -ffreestanding -fno-builtin -c hook.c -o hook.o
  clang $TGT -c stub.s -o stub.o
  ld.lld -EB -T cave.ld --defsym BASE="$2" --defsym RESUME_THUMB="$3" stub.o hook.o -o "cave_$1.elf"
  llvm-objcopy -O binary "cave_$1.elf" "cave_$1.bin"
  echo "cave_$1.bin: $(stat -c%s cave_$1.bin) bytes  base=$2 resume=$3"
}

# IOS36/55 SO binary (171900 B, sha1 6141c36f...): gap 0x8b8 before data seg.
build ios36 0x13b69760 0x13b512b5
# IOS56/58/61/80 SO binary (172344 B, sha1 00ef2f8b...): gap 0x6fc before data seg.
build ios56 0x13b69920 0x13b513b5
