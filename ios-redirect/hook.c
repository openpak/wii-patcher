/* OpenPak Wii IOS SO-module redirect hook (spike prototype, card #129).
 * Public-safe: contains NO Nintendo bytes. Hooks the internal resolver
 * entry (SOGetHostByName core). On a suffix-list match it swaps the
 * queried name for an embedded dotted-decimal IP string, then falls into
 * the module's own IP-literal path which builds the hostent. On no match
 * the original name is passed through unchanged to the normal DNS path.
 *
 * The suffix table and IP string below are BAKED by the patcher after it
 * verifies the signed ceiling envelope; the values here are spike
 * placeholders (RFC5737 documentation IP).
 */

/* ---- baked data (patcher overwrites these at install time) ---- */
/* table: entries of {u8 len; bytes}; terminated by a zero-length entry. */
__attribute__((section(".caved"), used))
static const unsigned char openpak_suffixes[] = {
    8,'k','l','e','i','.','c','o','m',
    16,'n','i','n','t','e','n','d','o','w','i','f','i','.','n','e','t',
    11,'o','p','e','n','p','a','k','.','o','r','g',
    0
};
__attribute__((section(".caved"), used))
static const char openpak_ip[] = "203.0.113.9";

/* strlen, local (no libc in IOS) */
static int slen(const char *s){ int n=0; while(s[n]) n++; return n; }

/* Returns replacement name on a ceiling match, else the original name. */
__attribute__((section(".cavetext"), used))
const char *openpak_hook(const char *name)
{
    int nl = slen(name);
    const unsigned char *t = openpak_suffixes;
    while (*t) {
        int sl = *t++;
        const unsigned char *suf = t;
        t += sl;
        /* Match on a DNS label boundary: the name must equal the suffix, or
         * the character just before the suffix must be '.'. This stops
         * look-alikes like "mastiff-klei.com" matching "klei.com". */
        if (sl <= nl && (sl == nl || name[nl - sl - 1] == '.')) {
            const char *tail = name + nl - sl;
            int i, ok = 1;
            for (i = 0; i < sl; i++) {
                char a = tail[i];
                if (a >= 'A' && a <= 'Z') a = (char)(a + 32); /* fold case */
                if (a != (char)suf[i]) { ok = 0; break; }
            }
            if (ok) return openpak_ip;
        }
    }
    return name;
}
