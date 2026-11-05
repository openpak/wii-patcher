// OpenPak Wii Patcher: Homebrew Channel app. One screen, the Switch Patcher's flow and wording:
// status pill, Nintendo / OpenPak rows, automatic backup, exact rollback, reboot prompt.
// The Wii has no resident hook, so "OpenPak" means the active connection's DNS is OpenPak's
// resolver, which answers only OpenPak's names and forwards everything else.
#include "patcher.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fat.h>
#include <gccore.h>
#include <malloc.h>
#include <sys/stat.h>
#include <unistd.h>
#include <wiiuse/wpad.h>

#ifndef APP_VERSION
#define APP_VERSION "0.0.0"
#endif

static const NetConfig::Dns OPENPAK_DNS = {{145, 241, 199, 19}};
static const char *CONFIG = "/shared2/sys/net/02/config.dat";
static const char *TMPCFG = "/shared2/sys/net/02/config.tmp";
static const char *SD_DIR = "sd:/apps/openpak-wii-patcher/backup";

static void *xfb = nullptr;
static GXRModeObj *rmode = nullptr;

#define C_RESET "\x1b[0m"
#define C_OK "\x1b[32;1m"
#define C_WARN "\x1b[33;1m"
#define C_ACC "\x1b[36;1m"
#define C_MUTED "\x1b[30;1m"
#define C_INK "\x1b[37;1m"

static bool isfsOk = false;

// ISFS wants 32-byte aligned buffers.
struct NandIo : Io {
    bool readFile(const char *path, Bytes &out) {
        s32 fd = ISFS_Open(path, ISFS_OPEN_READ);
        if (fd < 0) return false;
        fstats *st = (fstats *)memalign(32, sizeof(fstats));
        bool ok = false;
        if (st && ISFS_GetFileStats(fd, st) >= 0 && st->file_length > 0 && st->file_length <= 65536) {
            u32 n = st->file_length;
            u8 *buf = (u8 *)memalign(32, (n + 31) & ~31u);
            if (buf && ISFS_Read(fd, buf, n) == (s32)n) { out.assign(buf, buf + n); ok = true; }
            free(buf);
        }
        free(st);
        ISFS_Close(fd);
        return ok;
    }
    bool writeNew(const char *path, const Bytes &d, u8 attr, u8 op, u8 gp, u8 otp) {
        ISFS_Delete(path);
        if (ISFS_CreateFile(path, attr, op, gp, otp) < 0) return false;
        s32 fd = ISFS_Open(path, ISFS_OPEN_RW);
        if (fd < 0) return false;
        u8 *buf = (u8 *)memalign(32, (d.size() + 31) & ~31u);
        bool ok = buf && (memcpy(buf, d.data(), d.size()), ISFS_Write(fd, buf, d.size()) == (s32)d.size());
        free(buf);
        ISFS_Close(fd);
        return ok;
    }
    bool nandRead(Bytes &out) override { return isfsOk && readFile(CONFIG, out); }
    bool nandWrite(const Bytes &d) override {
        if (!isfsOk) return false;
        Bytes old;
        u32 owner; u16 group; u8 attr, op, gp, otp;
        if (!readFile(CONFIG, old)) return false;
        if (ISFS_GetAttr(CONFIG, &owner, &group, &attr, &op, &gp, &otp) < 0) return false;
        // 1. temp file next to the real one, with the real file's owner, group and permissions
        if (!writeNew(TMPCFG, d, attr, op, gp, otp)) { ISFS_Delete(TMPCFG); return false; }
        ISFS_SetAttr(TMPCFG, owner, group, attr, op, gp, otp);
        // 2. verify it by reading it back
        Bytes back;
        u32 o2; u16 g2; u8 a2, op2, gp2, otp2;
        if (!readFile(TMPCFG, back) || back != d ||
            ISFS_GetAttr(TMPCFG, &o2, &g2, &a2, &op2, &gp2, &otp2) < 0 ||
            o2 != owner || g2 != group || op2 != op || gp2 != gp || otp2 != otp) {
            ISFS_Delete(TMPCFG);
            return false;
        }
        // 3. rename over config.dat; if the file system refuses to replace, remove it first
        if (ISFS_Rename(TMPCFG, CONFIG) < 0) {
            ISFS_Delete(CONFIG);
            if (ISFS_Rename(TMPCFG, CONFIG) < 0) {
                // put the previous file back rather than leave the Wii without settings
                writeNew(CONFIG, old, attr, op, gp, otp);
                ISFS_SetAttr(CONFIG, owner, group, attr, op, gp, otp);
                ISFS_Delete(TMPCFG);
                return false;
            }
        }
        return true;
    }
    static std::string sdPath(const char *n) { return std::string(SD_DIR) + "/" + n; }
    bool sdRead(const char *n, Bytes &out) override {
        FILE *f = fopen(sdPath(n).c_str(), "rb");
        if (!f) return false;
        out.clear();
        unsigned char b[1024]; size_t r;
        while ((r = fread(b, 1, sizeof(b), f)) > 0) out.insert(out.end(), b, b + r);
        fclose(f);
        return true;
    }
    bool sdWrite(const char *n, const Bytes &d) override {
        std::string p = sdPath(n), t = p + ".tmp";
        FILE *f = fopen(t.c_str(), "wb");
        if (!f) return false;
        bool ok = fwrite(d.data(), 1, d.size(), f) == d.size();
        ok = fclose(f) == 0 && ok;
        if (!ok) { remove(t.c_str()); return false; }
        remove(p.c_str());
        return rename(t.c_str(), p.c_str()) == 0;
    }
    void sdRemove(const char *n) override { remove(sdPath(n).c_str()); }
};

static void mkdirs() {
    mkdir("sd:/apps", 0777);
    mkdir("sd:/apps/openpak-wii-patcher", 0777);
    mkdir(SD_DIR, 0777);
}

enum { A = 1, B = 2, UP = 4, DOWN = 8, HOME = 16 };

static u32 buttons() {
    WPAD_ScanPads(); PAD_ScanPads();
    u32 w = WPAD_ButtonsDown(0), p = PAD_ButtonsDown(0), r = 0;
    if ((w & (WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A)) || (p & PAD_BUTTON_A)) r |= A;
    if ((w & (WPAD_BUTTON_B | WPAD_CLASSIC_BUTTON_B)) || (p & PAD_BUTTON_B)) r |= B;
    if ((w & (WPAD_BUTTON_UP | WPAD_CLASSIC_BUTTON_UP)) || (p & PAD_BUTTON_UP)) r |= UP;
    if ((w & (WPAD_BUTTON_DOWN | WPAD_CLASSIC_BUTTON_DOWN)) || (p & PAD_BUTTON_DOWN)) r |= DOWN;
    if ((w & (WPAD_BUTTON_HOME | WPAD_CLASSIC_BUTTON_HOME)) || (p & PAD_BUTTON_START)) r |= HOME;
    return r;
}

struct UI {
    int sel = 0;                 // 0 Nintendo, 1 OpenPak
    Patcher::Status st = Patcher::UNKNOWN;
    char status[200] = "";
    char note[200] = "";
    enum Mode { NORMAL, REBOOT, UPDATE, SWITCHBACK } mode = NORMAL;
};

static void render(const UI &ui) {
    printf("\x1b[2J\x1b[2;0H");
    const char *pill = ui.st == Patcher::OPENPAK ? C_OK "[ OPENPAK ]" : ui.st == Patcher::NINTENDO ? C_WARN "[ NINTENDO ]" : C_MUTED "[ ? ]";
    printf(C_ACC "  OpenPak Wii Patcher" C_RESET "                         %s" C_RESET "\n", pill);
    printf(C_MUTED "  Network selector\n" C_RESET);
    printf("  ----------------------------------------------------\n\n");
    const char *names[2] = {"Nintendo", "OpenPak"};
    for (int i = 0; i < 2; i++) {
        bool active = ui.st != Patcher::UNKNOWN && ((i == 1) == (ui.st == Patcher::OPENPAK));
        printf("  %s %s%-12s" C_RESET " %s%s" C_RESET "\n\n", i == ui.sel ? C_ACC ">" : " ",
               i == ui.sel ? C_INK : C_RESET, names[i], i == 1 ? C_OK : C_WARN, active ? "Active" : "");
    }
    printf("  ----------------------------------------------------\n");
    if (ui.status[0]) printf("\n  " C_INK "%s" C_RESET "\n", ui.status);
    else {
        printf(C_MUTED "\n  Changes apply to the connection the Wii uses now.\n");
        printf("  OpenPak answers only OpenPak's names; the rest is forwarded.\n" C_RESET);
    }
    if (ui.note[0]) printf("\n  " C_WARN "%s" C_RESET "\n", ui.note);
    printf("\x1b[26;2H");
    if (ui.mode == UI::REBOOT) printf(C_INK "(A)" C_RESET " Reboot now   " C_INK "(B)" C_RESET " Later");
    else if (ui.mode == UI::UPDATE || ui.mode == UI::SWITCHBACK) printf(C_INK "(A)" C_RESET " Yes   " C_INK "(B)" C_RESET " No");
    else printf(C_INK "(A)" C_RESET " Select   " C_INK "(B)" C_RESET " Exit");
    printf(C_MUTED "      v" APP_VERSION C_RESET);
}

static void finish(Patcher &p, UI &ui, bool wantOpenPak, bool again) {
    bool ok = wantOpenPak ? p.install() : p.remove();
    ui.note[0] = '\0';
    if (!ok) { snprintf(ui.status, sizeof(ui.status), "Failed: %s", p.err); return; }
    ui.st = p.status();
    snprintf(ui.status, sizeof(ui.status), again ? "%s setup updated. Reboot to apply?" : "%s selected. Reboot to apply?",
             wantOpenPak ? "OpenPak" : "Nintendo");
    if (p.warn[0]) snprintf(ui.note, sizeof(ui.note), "%s", p.warn);
    ui.mode = UI::REBOOT;
}

int main() {
    VIDEO_Init();
    rmode = VIDEO_GetPreferredMode(nullptr);
    xfb = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));
    console_init(xfb, 20, 20, rmode->fbWidth, rmode->xfbHeight, rmode->fbWidth * VI_DISPLAY_PIX_SZ);
    VIDEO_Configure(rmode); VIDEO_SetNextFramebuffer(xfb); VIDEO_SetBlack(false); VIDEO_Flush(); VIDEO_WaitVSync();
    if (rmode->viTVMode & VI_NON_INTERLACE) VIDEO_WaitVSync();
    WPAD_Init(); PAD_Init();

    UI ui;
    NandIo io;
    Patcher p(io, OPENPAK_DNS, APP_VERSION);
    bool sd = fatInitDefault();
    if (sd) mkdirs();
    isfsOk = ISFS_Initialize() >= 0;
    if (!sd) snprintf(ui.status, sizeof(ui.status), "Failed: the SD card could not be opened");
    else if (!isfsOk) snprintf(ui.status, sizeof(ui.status), "Failed: the Wii's file system could not be opened");
    ui.st = sd && isfsOk ? p.status() : Patcher::UNKNOWN;
    if (sd && isfsOk && ui.st == Patcher::UNKNOWN)
        snprintf(ui.note, sizeof(ui.note), "No active connection found: set one up in Wii Settings first.");
    ui.sel = ui.st == Patcher::OPENPAK ? 1 : 0;
    // Older setup than this app: ask to install the update, and on No offer to switch back.
    if (sd && isfsOk && p.outdated()) {
        ui.mode = UI::UPDATE;
        snprintf(ui.status, sizeof(ui.status), "OpenPak update available. Install it?");
    }

    for (;;) {
        render(ui);
        VIDEO_WaitVSync();
        u32 k = buttons();
        if (k & HOME) break;
        if (ui.mode == UI::REBOOT) {
            if (k & A) SYS_ResetSystem(SYS_RETURNTOMENU, 0, 0);
            else if (k & B) { ui.mode = UI::NORMAL; ui.status[0] = ui.note[0] = '\0'; }
        } else if (ui.mode == UI::UPDATE) {
            if (k & A) { ui.mode = UI::NORMAL; finish(p, ui, true, true); }
            else if (k & B) { ui.mode = UI::SWITCHBACK; snprintf(ui.status, sizeof(ui.status), "Switch back to the official network?"); }
        } else if (ui.mode == UI::SWITCHBACK) {
            if (k & A) { ui.mode = UI::NORMAL; finish(p, ui, false, false); }
            else if (k & B) { ui.mode = UI::NORMAL; ui.status[0] = '\0'; }
        } else {
            if (k & (UP | DOWN)) ui.sel ^= 1;
            else if (k & B) break;
            else if (k & A) {
                if (!sd || !isfsOk) continue;
                bool want = ui.sel == 1;
                finish(p, ui, want, want == (ui.st == Patcher::OPENPAK));
            }
        }
    }
    if (isfsOk) ISFS_Deinitialize();
    return 0;
}
