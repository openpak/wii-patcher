# Deploy: wii-patcher

## Player route

Extract `OpenPak-Wii-Patcher.zip` to the root of the SD card (it holds `apps/openpak-wii-patcher` and
`apps/openpak-wfc-launcher`), start **OpenPak Wii Patcher** from the Homebrew Channel, choose **OpenPak**,
accept the reboot prompt. Choose **Nintendo** to go back. Backups live in
`sd:/apps/openpak-wii-patcher/backup/` (`config.dat.original`, `config.dat.managed`, `state.txt`).

## What it writes

Only `/shared2/sys/net/02/config.dat` in NAND, and only the DNS source flag and DNS1/DNS2 of the
selected connection ([WiiBrew layout](https://wiibrew.org/wiki//shared2/sys/net/02/config.dat):
7004 bytes, three 0x91C blocks at 8, flags byte at +0, bit 7 selected, bit 1 DNS from DHCP, DNS1 at +0x10,
DNS2 at +0x14). It refuses any file that is not 7004 bytes with exactly one selected connection.
The write goes to `config.tmp` with the real file's owner, group and permissions, is read back and
compared, then renamed over `config.dat`. If the file system will not replace the file it is removed
first and, on failure, the previous bytes are written back. Selecting Nintendo restores the exact saved
file; if the settings changed after OpenPak was selected only OpenPak's DNS is reverted.

OpenPak's resolver is 145.241.199.19 and answers only OpenPak names, forwarding the rest.

## Build

`app/`: devkitPPC (`docker.io/devkitpro/devkitppc`), `make APP_VERSION=x.y.z` gives `boot.dol`.
Host test of the logic: `app/tests/run.sh` (needs g++).

## Objdump invocation for raw DOL sections

Bare `-m powerpc` decodes little-endian garbage and no `-m` fails.
Use: `powerpc-eabi-objdump -m powerpc:common --endian=big -b binary
--adjust-vma=ADDR -D`.
