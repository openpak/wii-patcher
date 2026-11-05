#!/usr/bin/env python3
# Unicorn proof of the OpenPak Wii SO-module redirect hook, on the real
# decrypted SO binaries. Fully offline. Emits no Nintendo bytes.
#
# Proof boundary: our patch (trampoline + cave) is pure Thumb and its job is
# to (a) decide list membership and (b) hand the stock resolver either the
# OpenPak IP string (match) or the original name (no match). We prove exactly
# that. We deliberately do NOT execute the stock resolver body in Unicorn:
# it runs ARM-mode (BE32) libc veneers (memcpy/strtoul/ctype) that Unicorn's
# ARM BE32 path does not emulate reliably; converting a dotted-decimal literal
# into a hostent is stock Nintendo code exercised on every literal-IP connect,
# not ours to prove.
import os, sys, subprocess
from unicorn import *
from unicorn.arm_const import *
from so import SO

# Point these at your own locally decrypted SO modules (content index 5 of the
# respective IOS). No such binary is shipped in this repo.
#   OPENPAK_SO_IOS36 -> 171900-byte SO (sha1 6141c36f...), IOS36/55
#   OPENPAK_SO_IOS56 -> 172344-byte SO (sha1 00ef2f8b...), IOS56/58/61/80
CFG = {
    "IOS36/55 (171900B, 6141c36f)": dict(
        bin=os.environ.get("OPENPAK_SO_IOS36", ""), cave="cave_ios36.bin",
        resolver=0x13b512ac, resume=0x13b512b4, cave_va=0x13b69760),
    "IOS56/58/61/80 (172344B, 00ef2f8b)": dict(
        bin=os.environ.get("OPENPAK_SO_IOS56", ""), cave="cave_ios56.bin",
        resolver=0x13b513ac, resume=0x13b513b4, cave_va=0x13b69920),
}
NAME_VA = 0x30000000
STACK_TOP = 0x20010000
SENTINEL = 0xdead0000

def sym(elf, name):
    out = subprocess.check_output(["llvm-nm", elf]).decode()
    for ln in out.splitlines():
        if ln.endswith(" " + name):
            return int(ln.split()[0], 16)
    raise KeyError(name)

def trampoline(cave_va):
    import struct
    return struct.pack(">HH", 0x4b00, 0x4718) + struct.pack(">I", cave_va | 1)

class Harness:
    def __init__(self, cfg):
        self.cfg = cfg
        self.s = SO(cfg["bin"])
        self.cave = open(cfg["cave"], "rb").read()
        self.hook_fn = sym(cfg["cave"].replace(".bin", ".elf"), "openpak_hook")

    def fresh(self, patched=True):
        uc = Uc(UC_ARCH_ARM, UC_MODE_THUMB | UC_MODE_BIG_ENDIAN)
        rx = self.s.rx
        uc.mem_map(rx["va"], 0x13b6a000 - rx["va"], UC_PROT_ALL)
        uc.mem_write(rx["va"], self.s.d[rx["off"]:rx["off"] + rx["fsz"]])
        if patched:
            uc.mem_write(self.cfg["resolver"], trampoline(self.cfg["cave_va"]))
            uc.mem_write(self.cfg["cave_va"], self.cave)
        for g in self.s.segs:
            if not (g["flg"] & 1) and g["fsz"]:
                uc.mem_map(g["va"] & ~0xfff, (g["fsz"] + 0xfff) & ~0xfff, UC_PROT_READ | UC_PROT_WRITE)
                uc.mem_write(g["va"], self.s.d[g["off"]:g["off"] + g["fsz"]])
        uc.mem_map(0x13b6b000, 0xd5000, UC_PROT_READ | UC_PROT_WRITE)  # BSS / static buffers
        uc.mem_map(STACK_TOP - 0x20000, 0x20000, UC_PROT_READ | UC_PROT_WRITE)
        uc.mem_map(NAME_VA & ~0xfff, 0x1000, UC_PROT_READ | UC_PROT_WRITE)
        uc.mem_map(SENTINEL & ~0xfff, 0x1000, UC_PROT_ALL)  # landing pad for hook return
        return uc

    def call_hook(self, name):
        """Unit test: run the cave's openpak_hook(name) in isolation (pure Thumb)."""
        uc = self.fresh()
        uc.mem_write(NAME_VA, name.encode() + b"\0")
        uc.reg_write(UC_ARM_REG_R0, NAME_VA)
        uc.reg_write(UC_ARM_REG_SP, STACK_TOP)
        uc.reg_write(UC_ARM_REG_LR, SENTINEL | 1)
        uc.hook_add(UC_HOOK_CODE, lambda u, a, s, x: u.emu_stop(),
                    begin=SENTINEL & ~1, end=(SENTINEL & ~1) + 2)
        uc.emu_start(self.hook_fn | 1, 0, count=100000)
        r0 = uc.reg_read(UC_ARM_REG_R0)
        return r0, read_cstr(uc, r0)

    def run_to_handoff(self, name):
        """Integration: enter the real resolver entry; stop where the stock
        resolver body begins, capturing the argument our hook handed it."""
        uc = self.fresh()
        uc.mem_write(NAME_VA, name.encode() + b"\0")
        uc.reg_write(UC_ARM_REG_R0, NAME_VA)
        uc.reg_write(UC_ARM_REG_SP, STACK_TOP)
        uc.reg_write(UC_ARM_REG_LR, SENTINEL | 1)
        cap = {}
        def at_resume(u, a, s, x):
            cap["r0"] = u.reg_read(UC_ARM_REG_R0)
            cap["sp"] = u.reg_read(UC_ARM_REG_SP)
            u.emu_stop()
        uc.hook_add(UC_HOOK_CODE, at_resume, begin=self.cfg["resume"], end=self.cfg["resume"])
        uc.emu_start(self.cfg["resolver"] | 1, 0, count=100000)
        cap["uc"] = uc
        return cap

def read_cstr(uc, addr, n=32):
    if not addr:
        return None
    return bytes(uc.mem_read(addr, n)).split(b"\0")[0].decode("latin1", "replace")

def mem_writes(before, after):
    runs = []
    for lo in before:
        a, b = before[lo], after[lo]
        i = 0
        while i < len(a):
            if a[i] != b[i]:
                j = i
                while j < len(a) and a[j] != b[j]:
                    j += 1
                runs.append((lo + i, j - i)); i = j
            else:
                i += 1
    return runs

def writable_snapshot(uc):
    return {lo: bytes(uc.mem_read(lo, hi - lo + 1))
            for lo, hi, p in uc.mem_regions() if p & UC_PROT_WRITE}

IP = "203.0.113.9"
LISTED = ["gs.klei.com", "naex.nintendowifi.net", "openpak.org", "KLEI.COM"]
UNLISTED = ["connect.example.com", "mastiff-klei.com", "notklei.com.evil.net", "google.com"]


allok = True
for title, cfg in CFG.items():
    print("=" * 72)
    print(title)
    h = Harness(cfg)
    tr = trampoline(cfg["cave_va"])
    gap = 0x13b6a000 - (h.s.rx["va"] + h.s.rx["fsz"])
    print("  resolver=%08x resume=%08x cave=%08x hook_fn=%08x"
          % (cfg["resolver"], cfg["resume"], cfg["cave_va"], h.hook_fn))
    print("  patch footprint: 8-byte trampoline + %d-byte cave in a %d-byte gap"
          % (len(h.cave), gap))

    # UNIT: pure-Thumb matcher
    print("  -- UNIT: cave openpak_hook(name) --")
    unit_ok = True
    for nm in LISTED:
        r0, st = h.call_hook(nm)
        ok = st == IP
        unit_ok &= ok
        print("     %-24s -> '%s' %s" % (nm, st, "MATCH->IP" if ok else "FAIL"))
    for nm in UNLISTED:
        r0, st = h.call_hook(nm)
        ok = (r0 == NAME_VA and st == nm)
        unit_ok &= ok
        print("     %-24s -> passthrough %s" % (nm, "OK" if ok else "FAIL(r0=%08x '%s')" % (r0, st)))

    # CASE 1: listed name handed to stock resolver as the OpenPak IP
    base = writable_snapshot(h.fresh())
    c1 = h.run_to_handoff("gs.klei.com")
    c1s = read_cstr(c1["uc"], c1.get("r0"))
    ok1 = c1s == IP
    print("  -- CASE 1 (listed 'gs.klei.com') -- stock resolver receives r0->'%s'  %s"
          % (c1s, "PASS" if ok1 else "FAIL"))

    # CASE 2: unlisted name handed through unchanged
    c2 = h.run_to_handoff("connect.example.com")
    c2s = read_cstr(c2["uc"], c2.get("r0"))
    ok2 = c2.get("r0") == NAME_VA and c2s == "connect.example.com"
    print("  -- CASE 2 (unlisted) -- stock resolver receives r0->'%s' (original)  %s"
          % (c2s, "PASS" if ok2 else "FAIL"))

    # CASE 3: nothing else disturbed
    after = writable_snapshot(c1["uc"])
    runs = mem_writes(base, after)
    # exclude the stack and the NAME page (the latter is our test input, not a hook write)
    stray = [r for r in runs
             if not ((STACK_TOP - 0x20000) <= r[0] < STACK_TOP)
             and not ((NAME_VA & ~0xfff) <= r[0] < (NAME_VA & ~0xfff) + 0x1000)]
    code_after = bytes(c1["uc"].mem_read(h.s.rx["va"], h.s.rx["fsz"]))
    orig = h.s.d[h.s.rx["off"]:h.s.rx["off"] + h.s.rx["fsz"]]
    code_runs = mem_writes({h.s.rx["va"]: orig}, {h.s.rx["va"]: code_after})
    nontramp = [r for r in code_runs if not (cfg["resolver"] <= r[0] < cfg["resolver"] + 8)]
    ok3 = not stray and not nontramp
    print("  -- CASE 3 (memory) -- hook wrote 0 bytes to module data (only stack); "
          "code image changed only at the 8-byte trampoline  %s" % ("PASS" if ok3 else "FAIL"))
    if stray:
        print("     stray:", [(hex(a), n) for a, n in stray])
    if nontramp:
        print("     code:", [(hex(a), n) for a, n in nontramp])

    ok = unit_ok and ok1 and ok2 and ok3
    allok &= ok
    print("  RESULT: %s" % ("ALL PASS" if ok else "FAIL"))

print("=" * 72)
print("OVERALL:", "ALL PASS" if allok else "FAIL")
