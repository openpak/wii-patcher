// Select / remove logic on top of NetConfig, with every file access behind Io so the host test
// can run it against memory. Backup naming follows the Switch Patcher: <name>.original is what
// the console had, <name>.managed is what OpenPak wrote.
#pragma once
#include "netconfig.h"
#include <string>
#include <vector>

typedef std::vector<uint8_t> Bytes;

struct Io {
    virtual ~Io() {}
    virtual bool nandRead(Bytes &out) = 0;
    // Must write a temp file, verify it, then replace config.dat. False leaves config.dat as it was.
    virtual bool nandWrite(const Bytes &data) = 0;
    virtual bool sdRead(const char *name, Bytes &out) = 0;   // false when absent
    virtual bool sdWrite(const char *name, const Bytes &data) = 0;
    virtual void sdRemove(const char *name) = 0;
};

struct Patcher {
    Io &io;
    NetConfig::Dns dns;
    std::string version;   // the app's version, recorded in state.txt
    char err[160] = "";
    char warn[160] = "";

    Patcher(Io &i, NetConfig::Dns d, const char *v) : io(i), dns(d), version(v) {}

    enum Status { UNKNOWN, NINTENDO, OPENPAK };
    Status status();
    // True when OpenPak was selected by an older app version or another resolver address.
    bool outdated();
    bool install();   // OpenPak selected / setup updated
    bool remove();    // Nintendo selected
    bool wasInstalled();

private:
    bool fail(const char *m);
    std::string wantState() const;
};
