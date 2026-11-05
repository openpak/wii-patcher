#include "../source/patcher.h"
#include <cassert>
#include <cstdio>
#include <cstring>
#include <map>
#include <string>

struct Mem : Io {
    Bytes nand; std::map<std::string, Bytes> sd; bool failNand = false, failSd = false, flipOnWrite = false;
    bool nandRead(Bytes &o) override { o = nand; return true; }
    bool nandWrite(const Bytes &d) override { if (failNand) return false; nand = d; if (flipOnWrite) nand[100] ^= 1; return true; }
    bool sdRead(const char *n, Bytes &o) override { auto i = sd.find(n); if (i == sd.end()) return false; o = i->second; return true; }
    bool sdWrite(const char *n, const Bytes &d) override { if (failSd) return false; sd[n] = d; return true; }
    void sdRemove(const char *n) override { sd.erase(n); }
};

static Bytes sample(bool dhcpDns) {
    Bytes b(NetConfig::FILE_SIZE, 0);
    for (size_t i = 0; i < b.size(); i++) b[i] = (uint8_t)(i * 7 + 3);
    for (int c = 0; c < 3; c++) {
        uint8_t *p = &b[8 + c * 0x91C];
        p[0] = (uint8_t)((c == 1 ? 0x80 : 0) | 0x24 | (dhcpDns ? 2 : 0));
        p[0x1A] = 0x05; p[0x1B] = 0xDC;   // MTU 1500
    }
    return b;
}

int main() {
    NetConfig::Dns d = {{145, 241, 199, 19}};
    // select + exact rollback
    for (int dhcp = 0; dhcp < 2; dhcp++) {
        Mem m; m.nand = sample(dhcp); Bytes orig = m.nand;
        Patcher p(m, d, "1");
        assert(p.status() == Patcher::NINTENDO);
        assert(p.install());
        assert(p.status() == Patcher::OPENPAK);
        // only the selected connection's DNS bytes and flag changed
        int diff = 0; for (size_t i = 0; i < orig.size(); i++) diff += orig[i] != m.nand[i];
        assert(diff <= 9);
        assert(m.sd["config.dat.original"] == orig);
        assert(p.install());                                  // idempotent
        assert(m.sd["config.dat.original"] == orig);
        assert(p.remove());
        assert(m.nand == orig);
        assert(m.sd.empty());
        assert(p.status() == Patcher::NINTENDO);
    }
    // outdated
    { Mem m; m.nand = sample(1); Patcher a(m, d, "1"); assert(a.install()); Patcher b(m, d, "2"); assert(b.outdated()); assert(!a.outdated());
      Bytes orig = m.sd["config.dat.original"]; assert(b.install()); assert(m.sd["config.dat.original"] == orig); assert(!b.outdated()); }
    // unrecognised files are refused before anything is written
    { Mem m; m.nand = sample(1); m.nand.pop_back(); Patcher p(m, d, "1"); assert(!p.install()); assert(m.sd.empty()); }
    { Mem m; m.nand = sample(1); m.nand[8 + 0x91C] &= 0x7f; Patcher p(m, d, "1"); assert(!p.install()); assert(m.sd.empty()); }
    // failures change nothing
    { Mem m; m.nand = sample(1); Bytes o = m.nand; m.failSd = true; Patcher p(m, d, "1"); assert(!p.install()); assert(m.nand == o); }
    { Mem m; m.nand = sample(1); Bytes o = m.nand; m.failNand = true; Patcher p(m, d, "1"); assert(!p.install()); assert(m.nand == o); }
    { Mem m; m.nand = sample(1); Bytes o = m.nand; m.flipOnWrite = true; Patcher p(m, d, "1"); assert(!p.install()); m.flipOnWrite = false; }
    // settings changed after OpenPak was selected: only the DNS is reverted
    { Mem m; m.nand = sample(1); Bytes o = m.nand; Patcher p(m, d, "1"); assert(p.install());
      m.nand[8 + 0x91C + 0x30] ^= 0xFF;   // user edits something else
      Bytes edited = m.nand;
      assert(p.remove()); assert(p.warn[0]);
      Bytes want = o; want[8 + 0x91C + 0x30] ^= 0xFF; assert(m.nand == want); }
    // remove without a backup
    { Mem m; m.nand = sample(1); NetConfig::applyOpenPak(m.nand.data(), m.nand.size(), m.nand.data(), d); Patcher p(m, d, "1"); assert(!p.remove()); }
    puts("ok");
}
