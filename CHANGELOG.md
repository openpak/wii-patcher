# Changelog — nn-wfc-patcher-wii

Generated from git history on 2026-09-15. `git log` stays the source
of truth; this file is the readable summary.

## Unreleased

- ci: release builds on `v*.*.*` tags only (was `openpak-v*`), artifacts kept 3 days
  [ee7bce6, bf37c89]; docs commits

## openpak-v0.1.1 — 2026-09-10

- release: python3-cryptography and zip from apt, gh from GitHub's apt repo (the devkitppc
  image has no pip) [6fb683b]

## openpak-v0.1.0 — 2026-09-10

- OpenPak: openpak.org domain, our payload signing key (public half only), release workflow
  with launcher, gecko codes and signed payloads [1bcdbdd]

## payload-0.3.1 — 2026-08-26



## payload-0.3.0 — 2026-08-26

- Payload: Fix stack overflow in DWCi_GPRecvBuddyMessageCallback [f5de8fb]
- Bump version to 0.2.0 [3e91666]
- Payload: Port Mii SAKE patch to MK channel [c1bdafa]
- Payload: Clear hidden Mii info for SAKE [efa978e]


## payload-0.2.5 — 2026-04-16

- Payload: Fix incorrect case in include path [42f380e]
- Payload: Add missing DABJD00 ports [516aa4a]
- ItemType: Fix KINOKO_3 hasQuantity to true [a0a7680]
- RaceManager: Fix pathetic m_state offset bug [45464fd]
- Payload: Rename EFadeDirection to ENextType [da6de92]
- Payload: Fix issue with open host page [d27a564]
- Payload: Rename EGG files to be accurate [e8f0d44]
- Payload: Apply new import formatting to MKW UI [4b1dab9]
- Payload: Move OpenHostPage into primary directory [459a331]
- Payload: Globally reformat enums in MKW imports [94e5f25]
- Payload: Rework Mario Kart Wii ItemType header [c1cbfdf]
- Payload: Fix import filename capitalization [33405b2]
- Payload: Remove generic namespaces from Mario Kart Wii imports [d81d7ce]
- Payload: Improve timer start hook [8e0d385]
- Payload: Bump version of 0.2.4 [8005a5a]
- Payload: Report actual final finish time [d2eafaf]
- Payload: Fix base time not being set on race start [45efcfc]
- Payload: Bump version to 0.2.3 [86fa656]
- Payload: Finish MKW kiosk demo ports [69e829f]
- Payload: Verify unsure MKW kiosk demo ports [9c00b1d]
- Payload: Mario Kart Wii Kiosk Demo ports [944eacc]
- Stage1: Fix exploit compilation errors [989efc1]
- Merge changes from new-stage1 [f214e1d]
- Update LICENSE [de298b2]
- Change domain from wiilink24.com to wiilink.ca [4925d04]
- Patch: Add MEM2 IBAT config to loader tool [8c7e402]
- Payload: Store tb->ms as u64 [3e44ced]
- Payload: Fix crash in offline Grand Prix [a5f5c02]
- Payload: Bump version to 0.2.2 [f6fcf50]
- Payload: Clear hidden Mii info from more records [8cdf946]
- Payload: Bump version to 0.2.1 [98ca906]
- Payload: Port Mii SAKE patch to MK channel [e261feb]
- Payload: Clear hidden Mii info for SAKE [b52b1c4]
- Patch: Add PES to the IBAT fix blacklist [b8b2322]
- Stage1: Fix for exploit and optimize size more [5c771a9]
- Stage1: Optimize size a bit [10848ee]
- Payload: Fix missing port in Mario Kart Wii [7838402]
- Payload: Bump version to 0.1.2 [74259d6]
- Payload: Report finish time to the server [d80ed0e]
- Payload: Add credit to acaruso for a patch [967a7d5]

- … 13 commits omitted (see `git log payload-0.3.0..payload-0.2.5`)

## payload-0.2.4 — 2026-04-13



## stage1v1 — 2026-04-10



## payload-0.2.3 — 2026-04-10



## payload-0.2.2 — 2026-03-29



## payload-0.2.1 — 2026-03-26



## payload-0.2.0 — 2026-03-26

- Bump version to 0.2.0 [3e91666]
- Payload: Port Mii SAKE patch to MK channel [c1bdafa]
- Payload: Clear hidden Mii info for SAKE [efa978e]


## payload-0.1.3 — 2026-01-02

- Payload: Fix missing port in Mario Kart Wii [7838402]
- Payload: Bump version to 0.1.2 [74259d6]
- Payload: Report finish time to the server [d80ed0e]
- Payload: Add credit to acaruso for a patch [967a7d5]
- Payload: Don't calculate with fake finish time [7e45308]
- Payload: Only run time correction for local players [aeaa0c7]
- Revert "Payload: Only send the corrected finish time to remote players" [364a7ca]
- Payload: Only send the corrected finish time to remote players [52e7a67]
- Payload: Add bugfix to prevent forced disconnect [60ebbe9]
- Payload: Match in-game timer with real world time [7243f2c]
- Add build section to README (#82) [39753d6]
- Payload: Use ROOM packet size as minimum [f5fea6c]
- Add Mario Kart Wii demo (DABJD00) to gamedefs.csv [dcbbca1]
- Payload: Bump version to 0.1.1 [c1e916d]
- Payload: Remove lower bound size check on SELECT/ROOM [aba4a5c]
- Payload: Add newline to end of header file [a64979b]
- Payload: Print version string on startup [1cf0534]


## payload-0.1.2 — 2026-01-02



## payload-0.1.1 — 2025-12-31



## payload-0.1.0 — 2025-12-31



## payload-0.1.0 — 2025-12-31

- Payload: Fix (WWFC_)TITLE_TYPE discrepancy [03bcc4e]
- Payload: Bump format version to 2 [66eb68e]
- Payload: Bump version to 0.1.0 [ce49fc3]
- Stage1: Fix outdated macro name [7d51644]
- Payload: Add function [c521243]
- Payload: Refactor to avoid compiler luck [baaee99]
- Payload: Move to semantic-like versioning system [cae0c9d]
- Payload: Print WWFC Reports to OSReport [4d9f6bb]
- Payload: Separate game-specific code and remove STL dependency [59b2d6b]
- Update license to allow relicensing under GPL [96147b5]
- Payload: Display custom error codes in every game [d25190e]
- Update GP report to use the "wl:" namespace [9c07681]
- Reformat added records to use wl: namespace [742e01e]
- Payload: Increase Brawl latency limit [966370a]
- MKW: Remove unused decide engine class functions [f5c91dd]
- MKW: Remove engine class patch [3da4420]
- Payload: Compile with -fno-threadsafe-statics [a71c628]
- Payload: Fix auth token signing on Wii U VC [958c226]
- Patch: Fix MSCF HBM data address [7bb71bb]
- Fix 91010 on Fortune Street [3c4edba]
- Merge pull request #80 from MikeIsAStar/revert-fix-item-negotiation-bug [d7f9130]
- Revert commit "Fix a bug that leads to the rejection of one's item request without justification" [d66d0f0]
- Merge pull request #77 from MikeIsAStar/do-not-automatically-display-the-exception-console [decfd62]
- Merge pull request #78 from MikeIsAStar/translate-open-host-messages [236a2e0]
- Merge pull request #73 from ppebb/build-script [8a7e1c0]
- Merge pull request #79 from WiiLink24/dependabot/github_actions/dot-github/workflows/actions/download-artifact-4.1.7 [2e437ab]
- Bump actions/download-artifact from 4.1.0 to 4.1.7 in /.github/workflows [9eff58e]
- [MKW] Translate Open Host messages [bb21123]
- [MKW] Do not automatically display the exception console [00ff866]
- Merge pull request #76 from MikeIsAStar/refrain-from-reporting-the-same-player-to-the-server-repeatedly [0d993ae]
- [MKW] Refrain from reporting the same player to the server repeatedly [cc8eff1]
- Merge pull request #74 from MikeIsAStar/validate-thunder-cloud-used-event-data [fc0097e]
- [MKW] Prevent the game from crashing if a nonexistent player uses a Thunder Cloud [bbaac4f]
- Add exploit command [31783c7]
- Pass arguments after -- to each build script [393f9ea]
- add main build script [89e8486]
- Merge pull request #71 from MikeIsAStar/do-not-overwrite-gpiprocessconnect-return-value [6d21e50]
- Include import statements where needed [993bd43]
- [MKW] Do not overwrite the return value of the function 'gpiProcessConnect' [21112f9]
- Merge pull request #69 from MikeIsAStar/add-method-to-check-custom-region-usage [e10c127]

- … 203 earlier commits omitted (see `git log`)
