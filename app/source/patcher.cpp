#include "patcher.h"
#include <cstdio>

static const char *ORIG = "config.dat.original", *MAN = "config.dat.managed", *STATE = "state.txt";

bool Patcher::fail(const char *m) { snprintf(err, sizeof(err), "%s", m); return false; }

std::string Patcher::wantState() const {
    char b[96];
    snprintf(b, sizeof(b), "dns=%u.%u.%u.%u\nversion=%s\n", dns.b[0], dns.b[1], dns.b[2], dns.b[3], version.c_str());
    return b;
}

Patcher::Status Patcher::status() {
    Bytes cur;
    if (!io.nandRead(cur) || NetConfig::selected(cur.data(), cur.size()) < 0) return UNKNOWN;
    return NetConfig::isOpenPak(cur.data(), cur.size(), dns) ? OPENPAK : NINTENDO;
}

bool Patcher::wasInstalled() { Bytes b; return io.sdRead(STATE, b); }

bool Patcher::outdated() {
    Bytes st;
    if (!io.sdRead(STATE, st) || status() != OPENPAK) return false;
    std::string want = wantState();
    return std::string(st.begin(), st.end()) != want;
}

bool Patcher::install() {
    err[0] = warn[0] = '\0';
    Bytes cur, man, orig, next;
    if (!io.nandRead(cur)) return fail("Could not read the Wii's network settings");
    if (NetConfig::selected(cur.data(), cur.size()) < 0)
        return fail("No active connection found: set one up in Wii Settings first");
    bool haveMan = io.sdRead(MAN, man), haveOrig = io.sdRead(ORIG, orig);
    bool on = NetConfig::isOpenPak(cur.data(), cur.size(), dns);
    bool ours = haveMan && man == cur;

    // Back up before anything is written. An earlier backup is kept while the console still
    // runs the setup OpenPak wrote, so the first original is never lost.
    if (!ours && !(on && haveOrig)) {
        if (!io.sdWrite(ORIG, cur)) return fail("Could not back up the network settings to the SD card");
        Bytes check;
        if (!io.sdRead(ORIG, check) || check != cur) return fail("Could not back up the network settings to the SD card");
    }
    next.resize(cur.size());
    if (!NetConfig::applyOpenPak(cur.data(), cur.size(), next.data(), dns)) return fail("Network settings not recognised");
    if (!io.sdWrite(MAN, next)) return fail("Could not record the OpenPak setup on the SD card");
    if (!io.nandWrite(next)) return fail("Could not write the network settings; nothing was changed");
    Bytes back;
    if (!io.nandRead(back) || back != next) {
        if (!io.nandWrite(cur)) return fail("Network settings changed unexpectedly; restore from the SD card backup");
        return fail("Could not verify the network settings; originals put back");
    }
    std::string s = wantState();
    io.sdWrite(STATE, Bytes(s.begin(), s.end()));
    return true;
}

bool Patcher::remove() {
    err[0] = warn[0] = '\0';
    Bytes cur, man, orig;
    if (!io.nandRead(cur)) return fail("Could not read the Wii's network settings");
    if (NetConfig::selected(cur.data(), cur.size()) < 0 && cur.size() != NetConfig::FILE_SIZE)
        return fail("Network settings not recognised");
    if (!io.sdRead(ORIG, orig)) {
        if (!NetConfig::isOpenPak(cur.data(), cur.size(), dns)) return true;   // nothing of ours
        return fail("No backup of your network settings was found");
    }
    bool haveMan = io.sdRead(MAN, man);
    if (cur == orig) {
        // already the original
    } else if (haveMan && cur == man) {
        if (!io.nandWrite(orig)) return fail("Could not restore the network settings; originals kept");
        Bytes back;
        if (!io.nandRead(back) || back != orig) return fail("Could not verify the restored network settings; originals kept");
    } else {
        Bytes rev(cur.size());
        int n = NetConfig::revertDns(cur.data(), orig.data(), cur.size(), rev.data(), dns);
        if (n < 0) return fail("Network settings not recognised");
        if (n > 0) {
            if (!io.nandWrite(rev)) return fail("Could not restore the network settings; originals kept");
            snprintf(warn, sizeof(warn), "Network settings changed since; only OpenPak's DNS was removed");
        } else {
            snprintf(warn, sizeof(warn), "Network settings changed since; originals kept in the app's backup folder");
            return true;   // nothing of ours in the file: leave the backup in place
        }
    }
    io.sdRemove(MAN);
    io.sdRemove(ORIG);
    io.sdRemove(STATE);
    return true;
}
