#include "netconfig.h"
#include <cstring>

namespace NetConfig {

static const uint8_t *blk(const uint8_t *b, int i) { return b + HEADER + (size_t)i * BLOCK; }
static uint8_t *blk(uint8_t *b, int i) { return b + HEADER + (size_t)i * BLOCK; }

int selected(const uint8_t *buf, size_t len) {
    if (!buf || len != FILE_SIZE) return -1;
    int sel = -1;
    for (int i = 0; i < CONNECTIONS; i++) {
        if (blk(buf, i)[OFF_FLAGS] & FLAG_SELECTED) {
            if (sel >= 0) return -1;
            sel = i;
        }
    }
    if (sel < 0) return -1;
    const uint8_t *c = blk(buf, sel);
    unsigned mtu = (c[OFF_MTU] << 8) | c[OFF_MTU + 1];
    if (mtu != 0 && (mtu < 576 || mtu > 1500)) return -1;
    return sel;
}

static bool dnsIs(const uint8_t *c, const Dns &d) {
    return !(c[OFF_FLAGS] & FLAG_DNS_DHCP) && !memcmp(c + OFF_DNS1, d.b, 4) && !memcmp(c + OFF_DNS2, d.b, 4);
}

bool isOpenPak(const uint8_t *buf, size_t len, const Dns &dns) {
    int s = selected(buf, len);
    return s >= 0 && dnsIs(blk(buf, s), dns);
}

bool applyOpenPak(const uint8_t *in, size_t len, uint8_t *out, const Dns &dns) {
    int s = selected(in, len);
    if (s < 0) return false;
    if (out != in) memcpy(out, in, len);
    uint8_t *c = blk(out, s);
    c[OFF_FLAGS] &= (uint8_t)~FLAG_DNS_DHCP;
    memcpy(c + OFF_DNS1, dns.b, 4);
    memcpy(c + OFF_DNS2, dns.b, 4);
    return true;
}

int revertDns(const uint8_t *cur, const uint8_t *original, size_t len, uint8_t *out, const Dns &dns) {
    if (!cur || !original || len != FILE_SIZE) return -1;
    if (out != cur) memcpy(out, cur, len);
    int n = 0;
    for (int i = 0; i < CONNECTIONS; i++) {
        const uint8_t *c = blk(cur, i), *o = blk(original, i);
        if (!dnsIs(c, dns)) continue;
        uint8_t *w = blk(out, i);
        w[OFF_FLAGS] = (uint8_t)((w[OFF_FLAGS] & ~FLAG_DNS_DHCP) | (o[OFF_FLAGS] & FLAG_DNS_DHCP));
        memcpy(w + OFF_DNS1, o + OFF_DNS1, 8);
        n++;
    }
    return n;
}

}  // namespace NetConfig
