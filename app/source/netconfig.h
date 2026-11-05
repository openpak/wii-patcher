// The Wii's network settings file, /shared2/sys/net/02/config.dat (System Menu 2.0 and later).
// Layout source: https://wiibrew.org/wiki//shared2/sys/net/02/config.dat
//   7004 bytes = 8-byte header + 3 connection blocks of 0x91C bytes, block n at 8 + n * 0x91C.
//   In a block: flags at +0x00 (bit 7 selected, bit 1 DNS source: 0 manual, 1 DHCP),
//   DNS1 at +0x10, DNS2 at +0x14 (IPv4, network order).
// OpenPak only ever touches those three fields of the selected connection. Pure logic, no I/O,
// so it is tested on the host (tests/run.sh).
#pragma once
#include <cstddef>
#include <cstdint>

namespace NetConfig {

constexpr size_t FILE_SIZE = 7004;
constexpr size_t HEADER = 8;
constexpr size_t BLOCK = 0x91C;
constexpr int CONNECTIONS = 3;
constexpr size_t OFF_FLAGS = 0x00, OFF_DNS1 = 0x10, OFF_DNS2 = 0x14, OFF_MTU = 0x1A;
constexpr uint8_t FLAG_SELECTED = 0x80, FLAG_DNS_DHCP = 0x02;

struct Dns { uint8_t b[4]; };
inline bool operator==(const Dns &a, const Dns &b) { return a.b[0]==b.b[0] && a.b[1]==b.b[1] && a.b[2]==b.b[2] && a.b[3]==b.b[3]; }

// Returns the index of the one selected connection, or -1 when the file does not look like a
// Wii network config (wrong size, no or several selected connections, impossible MTU).
int selected(const uint8_t *buf, size_t len);

// True when the selected connection uses manual DNS and both servers are `dns`.
bool isOpenPak(const uint8_t *buf, size_t len, const Dns &dns);

// Copies `in` to `out` with the selected connection's DNS set manual, DNS1 = DNS2 = dns.
// Nothing else changes. False when the file is not recognised.
bool applyOpenPak(const uint8_t *in, size_t len, uint8_t *out, const Dns &dns);

// Brings OpenPak's DNS fields back to what `original` had, for every connection that still holds
// OpenPak's DNS in `cur`. Used when the settings changed after OpenPak was selected, so the
// exact file can not simply be put back. Returns how many connections were reverted.
int revertDns(const uint8_t *cur, const uint8_t *original, size_t len, uint8_t *out, const Dns &dns);

}  // namespace NetConfig
