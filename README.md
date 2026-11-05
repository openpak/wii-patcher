# Wii Patcher

OpenPak's Wii Patcher.

| | |
| --- | --- |
| Console | Wii |
| Kind | Patcher: Homebrew Channel app (`OpenPak-Wii-Patcher.zip`), launcher (`openpak-wfc-launcher.dol`), gecko codes and stage payloads |
| Download | [Releases](https://github.com/openpak/wii-patcher/releases) (`vX.Y.Z` tags) |
| Based on | [WiiLink `wfc-patcher-wii`](https://wfc.wiilink.ca) (custom BSD-style or GPL-2.0-or-later, as upstream) |
| Licence | Custom BSD-style or GPL-2.0-or-later, as upstream; see [LICENSE](LICENSE) |

## What it does

- Builds the launcher (`openpak-wfc-launcher.dol`), the Wii's setup file, which boots a disc with the stage 0 patch applied.
- Ships stage 0 gecko codes per game (`openpak-wfc-gecko-codes.zip`) for USB loaders and Riivolution.
- Hands out stage 1 and stage 2 payloads (`wfc-payload-openpak.tar.gz`) that the console downloads; stage 2 is RSA-signed.
- Resolves `*.openpak.org` through DNS; the Wii Patcher app sets that DNS entry on the console.

## Install

Players: extract `OpenPak-Wii-Patcher.zip` to the SD card and run **OpenPak Wii Patcher** from the
Homebrew Channel; it sets the DNS for you and backs up and restores your settings
(see [deploy](docs/deploy.md)). Operators: unpack `wfc-payload-openpak.tar.gz` into the `payload/`
directory of `wfc-nas`.

## Build

Needs devkitPPC and Python 3.11+ with the `cryptography` module.

```sh
./make.sh --all -- -j8 -DWWFC_DOMAIN=\"${DOMAIN}\"
```

The output lands in `dist/`. The long form is in [provenance](docs/provenance.md#building).

Signing keys are never tracked. Run `./make.sh --keys` once to generate a fresh `misc/private-key-OUT.pem` and the matching public key header (both gitignored). Nothing in the build or tests uses a checked-in dev key.

## Docs

- [Protocol](docs/protocol.md)
- [Deploy](docs/deploy.md)
- [Provenance](docs/provenance.md)

## Licence

Custom BSD-style licence from WiiLink, see [LICENSE](LICENSE). The upstream README also offers GPL-2.0-or-later as an alternative, see [provenance](docs/provenance.md#license).

PRDs: openpak/prds (private)
