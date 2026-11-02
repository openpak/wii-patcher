# Protocol: wii-patcher

Fork of WiiLink's `wfc-patcher-wii` (BSD-style or GPL-2.0+, as upstream). Built for
`openpak.org` (same-length swap for `nintendowifi.net`, so every patched string fits):

- **launcher** (`openpak-wfc-launcher.dol`): the Wii's setup file, the counterpart of
  `openpak.nro`. Boots a disc with the stage-0 patch applied.
- **stage 0** gecko codes per game (`openpak-wfc-gecko-codes.zip`) for USB loaders and Riivolution.
- **stage 1 / stage 2 payloads** (`wfc-payload-openpak.tar.gz`): the server hands these out;
  unpack into `wfc-nas`'s `payload/` directory. Stage 2 is RSA-signed with the key in the
  `WFC_PAYLOAD_PRIVATE_KEY` secret; its public half is `include/wwfcPayloadPublicKey.hpp`.

## Landed stage-0 rows (verified two ways)

These rows were reverse-engineered, cross-checked between independent
analysis and `misc/address-search.py`, and committed to `gamedefs.csv`
with matching `GameAddresses.cpp` launcher entries:

- `RVYPD00` (CoD 3 PAL, RVYP52 rev 0)
- `RFFPD00` (FFCC Echoes of Time PAL, RFFPGD rev 0)
- `SEPTD00` (Tetris Party Deluxe, STEPTR rev 0)

## Variant-DWC games (need variant patch paths)

The stock stage-0 patch (`patch/wwfcPatch.s`, driven by
`misc/address-search.py`) only fits classic DWC builds: MD5-based auth,
`%s.available.gs` URL, full GameSpy GP/GT2 stack. The five games below
deviate, so filling `gamedefs.csv` rows is insufficient; the patch
sources need variant code paths, and every variant needs console
validation before it ships. Partial rows below are leads; each variant needs console validation before its row ships.

Scratch artifacts live outside the repo (workspace `scratch/tmp/`):
per-game disassembly caches, xref scripts, and `*_row.csv` fragments.

### SC7P52 Black Ops PAL → SC7PD00 (10 gap groups)

DWC version-C-request/response-D hybrid (Treyarch libogc omits
`GetSockName`). No MD5, no GameSpy availability URL, no GP SDK.
`misc/address-search.py` cannot run on it. Needs a Demonware-family
auth path in the patch.

### SM8P52 Modern Warfare 3 PAL → SM8PD00 (62/100 mapped)

New DWC auth version ("E"). 12 auth-cluster offsets are identical to
the SC7PD00 work-in-progress (Send +0xB40, MAC +0x798, CSNUM +0x9AC,
HOOK +0xCC0, CONTINUE +0xCD8, ERROR +0x1434, OUT +0x1488,
NHTTP +0x318/+0x404, Startup Create-0x3BC), independently validated
against the SM8P52 DOL (`bl@MAC+0x48`, `bl@CSNUM+0x14`, hook bytes,
hook-branch shape). Same engine family as Black Ops: one redesign
covers both. Partial row: `scratch/tmp/sm8p-work/SM8PD00_row.txt`.
Still missing: MD5, AVAILABLE_URL, GP/GT2 cluster, DNS-cache chain,
allocators, misc.

### SJBP52 GoldenEye PAL → SJBPD00 (64/100 mapped)

Apr-2010 DWC version C/D with SHA1 (`SHA1Init` at `0x803C5668`); no
MD5 code, constants, or strings anywhere (BE and LE searched), so
`ADDRESS_MD5Digest` has no natural value and the stock generator
exits. Every nonzero value was cross-validated against the RMCPD00
reference with an independent xref cache plus objdump spot checks.
Partial row: `scratch/tmp/sjbp-work/sjbpd00_row.csv`.
Still missing: MD5/SHA1 handling, AVAILABLE_URL, GP/GT2 cluster,
DNS-cache chain, allocators, misc. Needs an SHA1-aware patch path.

### RM2P69 MoH Heroes 2 PAL → RM2PD00 (blocked on target)

`DATA/sys/main.dol` (286 KB) is EA's bootloader stub (MetroTRK,
Apr-2007 SDK, strap screens, loads `game.dol`), not the game: no DWC
anchors at all. The real binary is `DATA/files/game.dol` (7.2 MB,
extracted to `scratch/tmp/redol/RM2P69-full/`). It has NASWII strings
but no MD5 signature and no `available.gs` URL at the usual pattern,
so the stock generator returns no row. Needs manual variant work
against `game.dol`.

### RMHP08 Monster Hunter Tri PAL → RMHPD00 (67/100 mapped)

Reduced DWC: NAS-auth + NHTTP + SO + natneg, no GameSpy GP/GT2 code.
GP/GT2 zeros are legitimate (precedent in committed rows); MD5,
allocators, and misc columns still open. Partial row:
`scratch/tmp/mhp-analysis/RMHPD00_row.csv`. Needs a reduced-stack
patch path (payload-source changes per round-two analysis).
