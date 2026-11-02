# Deploy: wii-patcher

The console still resolves `*.openpak.org` through DNS, so point the Wii at
[`nn-sssl-dns`](https://github.com/openpak/nn-sssl-dns) or publish records. Releases on `vX.Y.Z` tags.

## Objdump invocation for raw DOL sections

Bare `-m powerpc` decodes little-endian garbage and no `-m` fails.
Use: `powerpc-eabi-objdump -m powerpc:common --endian=big -b binary
--adjust-vma=ADDR -D`.
