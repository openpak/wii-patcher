# Protocol: wii-patcher

Fork of WiiLink's `wfc-patcher-wii` (BSD-style or GPL-2.0+, as upstream). Built for
`openpak.org` (same-length swap for `nintendowifi.net`, so every patched string fits):

- **launcher** (`openpak-wfc-launcher.dol`): the Wii's setup file, the counterpart of
  `openpak.nro`. Boots a disc with the stage-0 patch applied.
- **stage 0** gecko codes per game (`openpak-wfc-gecko-codes.zip`) for USB loaders and Riivolution.
- **stage 1 / stage 2 payloads** (`wfc-payload-openpak.tar.gz`): the server hands these out;
  unpack into `nn-wfc`'s `payload/` directory. Stage 2 is RSA-signed with the key in the
  `WFC_PAYLOAD_PRIVATE_KEY` secret; its public half is `include/wwfcPayloadPublicKey.hpp`.
