# Wii Patcher

OpenPak's Wii Patcher.

| | |
| --- | --- |
| Console | Wii |
| Kind | Patcher: launcher (`openpak-wfc-launcher.dol`), gecko codes and stage payloads |
| Download | [Releases](https://github.com/openpak/wii-patcher/releases) (`vX.Y.Z` tags) |
| Based on | [WiiLink `wfc-patcher-wii`](https://wfc.wiilink.ca) (custom BSD-style or GPL-2.0-or-later, as upstream) |
| Licence | Custom BSD-style or GPL-2.0-or-later, as upstream; see [LICENSE](LICENSE) |

## What it does

- Builds the launcher (`openpak-wfc-launcher.dol`), the Wii's setup file, which boots a disc with the stage 0 patch applied.
- Ships stage 0 gecko codes per game (`openpak-wfc-gecko-codes.zip`) for USB loaders and Riivolution.
- Hands out stage 1 and stage 2 payloads (`wfc-payload-openpak.tar.gz`) that the console downloads; stage 2 is RSA-signed.
- Resolves `*.openpak.org` through DNS, so the console needs a DNS entry that points there.

## Install

1. Unpack `wfc-payload-openpak.tar.gz` into the `payload/` directory of `nn-wfc`.
2. Point the Wii at `*.openpak.org` through DNS, see [deploy](docs/deploy.md).

## Build

Needs devkitPPC and Python 3.11+ with the `cryptography` module.

```sh
./make.sh --all -- -j8 -DWWFC_DOMAIN=\"${DOMAIN}\"
```

The output lands in `dist/`. The long form is in [upstream](docs/upstream.md#building).

## Docs

- [Protocol](docs/protocol.md)
- [Evidence](docs/evidence.md)
- [Deploy](docs/deploy.md)
- [Notes](docs/notes.md)
- [Upstream README](docs/upstream.md)
- [Docs index](docs/README.md)

## Licence

Custom BSD-style licence from WiiLink, see [LICENSE](LICENSE). The upstream README also offers GPL-2.0-or-later as an alternative, see [upstream](docs/upstream.md#license).
