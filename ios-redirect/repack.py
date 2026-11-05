#!/usr/bin/env python3
# Repack + fakesign an OpenPak-patched SO module in place (spike prototype,
# card #129). Public-safe: NO Nintendo bytes. The Wii common key is NOT
# embedded -- it is read from the environment (the patcher obtains it on the
# console: from OTP on vWii, or the value the user's own tools already hold):
#     OPENPAK_WII_COMMON_KEY = 32 hex chars
#
# Given the stock TMD, ticket, the decrypted SO content, and the matching cave
# blob, this: grows the RX PT_LOAD to map the cave, writes the trampoline,
# re-encrypts the content, updates the TMD content record, and fakesigns the
# TMD (trucha). It does not assemble/install a WAD and performs nothing on a
# console. NOTE: vWii IOS have the trucha bug disabled; a fakesigned title is
# not accepted by a near-stock vWii -- see README (vWii acceptance blocker).
import os, sys, struct, hashlib
from Cryptodome.Cipher import AES

# Per-SO-binary parameters (from the spike RE; offsets are facts, not bytes).
TARGETS = {
    # sha1 of the decrypted SO content : config
    "6141c36f825a62093b4ba28b07054680ecd469a7": dict(      # IOS36/55
        rx_foff=0x128, rx_va=0x13b40000, rx_fsz=0x29748,
        cave_va=0x13b69760, resolver=0x13b512ac, cave="cave_ios36.bin"),
    "00ef2f8bbcd208eb5e64cd916554405ad34ecfd5": dict(      # IOS56/58/61/80
        rx_foff=0x128, rx_va=0x13b40000, rx_fsz=0x29904,
        cave_va=0x13b69920, resolver=0x13b513ac, cave="cave_ios56.bin"),
}
ALIGN = 0x20


def common_key():
    v = os.environ.get("OPENPAK_WII_COMMON_KEY", "")
    if len(v) != 32:
        sys.exit("set OPENPAK_WII_COMMON_KEY to the 32-hex-char Wii common key")
    return bytes.fromhex(v)


def grow_and_patch(orig, cfg):
    cave = open(cfg["cave"], "rb").read()
    tramp = struct.pack(">HH", 0x4b00, 0x4718) + struct.pack(">I", cfg["cave_va"] | 1)
    need = (cfg["cave_va"] - cfg["rx_va"]) + len(cave)
    delta = ((need - cfg["rx_fsz"]) + ALIGN - 1) & ~(ALIGN - 1)
    new_fsz = cfg["rx_fsz"] + delta
    rx = bytearray(orig[cfg["rx_foff"]:cfg["rx_foff"] + cfg["rx_fsz"]]) + b"\0" * delta
    coff = cfg["cave_va"] - cfg["rx_va"]
    rx[coff:coff + len(cave)] = cave
    roff = cfg["resolver"] - cfg["rx_va"]
    rx[roff:roff + len(tramp)] = tramp
    out = bytearray(orig[:cfg["rx_foff"]]) + rx + orig[cfg["rx_foff"] + cfg["rx_fsz"]:]
    phoff = struct.unpack(">I", orig[0x1c:0x20])[0]
    phnum = struct.unpack(">H", orig[0x2c:0x2e])[0]
    old_end = cfg["rx_foff"] + cfg["rx_fsz"]
    for i in range(phnum):
        b = phoff + i * 0x20
        p_off = struct.unpack(">I", orig[b + 4:b + 8])[0]
        p_va = struct.unpack(">I", orig[b + 8:b + 12])[0]
        if p_va == cfg["rx_va"]:
            struct.pack_into(">I", out, b + 16, new_fsz)
            struct.pack_into(">I", out, b + 20, new_fsz)
        elif p_off >= old_end:
            struct.pack_into(">I", out, b + 4, p_off + delta)
    return bytes(out)


def main():
    if len(sys.argv) != 5:
        sys.exit("usage: repack.py <tmd> <cetk> <decrypted-content5> <out-prefix>")
    tmd = bytearray(open(sys.argv[1], "rb").read())
    cetk = open(sys.argv[2], "rb").read()
    content = open(sys.argv[3], "rb").read()
    out_prefix = sys.argv[4]

    sha = hashlib.sha1(content).hexdigest()
    if sha not in TARGETS:
        sys.exit("unknown SO binary sha1 %s -- refusing (needs its own hooked cave + offsets)" % sha)
    cfg = TARGETS[sha]

    tid = cetk[0x1DC:0x1DC + 8]
    tkey = AES.new(common_key(), AES.MODE_CBC, tid + b"\0" * 8).decrypt(cetk[0x1BF:0x1BF + 16])

    off = 0x140
    num = struct.unpack(">H", tmd[off + 0x9e:off + 0xa0])[0]
    cs = off + 0xa4
    rec = None
    for i in range(num):
        if struct.unpack(">H", tmd[cs + i * 0x24 + 4:cs + i * 0x24 + 6])[0] == 5:
            rec = cs + i * 0x24
    if rec is None:
        sys.exit("content index 5 not found in TMD")

    patched = grow_and_patch(content, cfg)
    new_sha = hashlib.sha1(patched).digest()
    iv = struct.pack(">H", 5) + b"\0" * 14
    enc = AES.new(tkey, AES.MODE_CBC, iv).encrypt(patched + b"\0" * ((-len(patched)) % 16))
    struct.pack_into(">Q", tmd, rec + 8, len(patched))
    tmd[rec + 16:rec + 36] = new_sha

    # fakesign (trucha): zero the RSA signature, brute-force an unchecked pad
    tmd[0x04:0x104] = b"\0" * 0x100
    fill = off + 0xa2
    for v in range(0x10000):
        struct.pack_into(">H", tmd, fill, v)
        if hashlib.sha1(tmd[off:]).digest()[0] == 0:
            break
    else:
        sys.exit("fakesign: no trucha fill found (unexpected)")

    open(out_prefix + ".content5.app", "wb").write(patched)
    open(out_prefix + ".content5.enc", "wb").write(enc)
    open(out_prefix + ".tmd", "wb").write(tmd)
    print("ok: %s (%d B) re-encrypted + TMD fakesigned (trucha fill 0x%04x)"
          % (sha[:12], len(patched), v))


if __name__ == "__main__":
    main()
