# Charybdis Wireless Mini — ZMK Firmware

## Hardware

- **Keyboard**: Charybdis Wireless Mini — 3×6 split + trackball on the right half
- **Left half**: 3 thumb keys (36 = outer, 37 = center, 38 = inner)
- **Right half**: 2 thumb keys (39 = inner, 40 = outer)
- **Dongle**: nice_nano (no screen), `dongle_standard_nano` build target
- **OS**: Mac only

### Key positions

```
Left:   00  01  02  03  04  05
        12  13  14  15  16  17
        24  25  26  27  28  29
                36  37  38

Right:  06  07  08  09  10  11
        18  19  20  21  22  23
        30  31  32  33  34  35
                        39  40
```

## Build

```bash
docker compose -f local-build/docker-compose.yml run --rm builder
```

`build.yaml` is trimmed to only build:
- `settings_reset`
- `dongle_standard_nano colemak_dh`

Output: `firmwares/charybdis_dongle/colemak_dh/`
- `charybdis_left_dongle.uf2` → flash to left half
- `charybdis_right_dongle.uf2` → flash to right half
- `charybdis_dongle.uf2` → flash to dongle

## Key files

| File | Purpose |
|------|---------|
| `config/keymaps/colemak_dh.keymap` | Main keymap (only one being actively developed) |
| `config/keymap_features/behaviors.dtsi` | Shared behaviors (HRMs, tap-dance, mod-morph) |
| `config/keymap_features/combos.dtsi` | Shared combos — used by ALL keymaps |
| `config/keymap_features/macros.dtsi` | Shared macros — used by ALL keymaps |
| `config/trackball/charybdis_pointer.dtsi` | Trackball input processors |
| `build.yaml` | Build targets |
| `local-build/docker-compose.yml` | Docker build config |

**Important**: `behaviors.dtsi`, `combos.dtsi`, and `macros.dtsi` are shared across all keymaps
(qwerty, canary, focal, graphite, colemak_dh). Changes there affect all keymaps.

## Layer structure

| # | Name | Access | Purpose |
|---|------|--------|---------|
| 0 | BASE | — | Colemak-DH |
| 1 | NAV | hold RET (key 39) | Navigation + mouse |
| 2 | SYM | hold TAB (key 36) | Symbols + numbers; trackball scrolls |
| 3 | SHORTCUTS | hold ESC (key 38) | Mac shortcuts + clicks; slow trackball |
| 4 | ADJ | SHORTCUTS + NAV | Brightness, volume, F23/F24 |
| 5 | SYSTEM | SYM + NAV | BT profiles, output, power, bootloader |

ADJ and SYSTEM are ZMK conditional layers (`zmk,conditional-layers`) — no dedicated key needed.

### Trackball behaviour
- Default: normal pointer speed
- SYM active (hold TAB): trackball scrolls
- SHORTCUTS active (hold ESC): slow pointer

## colemak_dh keymap design

### BASE layer thumb cluster

```
36: lt 2 TAB               (tap=TAB, hold=SYM)
37: thumb_mt LSHIFT SPACE  (tap=SPACE, hold=LSHIFT)
38: lt 3 ESC               (tap=ESC, hold=SHORTCUTS)
39: lt 1 RET               (tap=RET, hold=NAV)
40: thumb_mt RSHIFT BSPC   (tap=BSPC, hold=RSHIFT)
```

### Home row mods (matches hillside/kyria `hml`/`hmr`)

- Left: A=plain, R=LCTRL, S=LALT, T=LGUI (using `ht_left`)
- Right: N=RGUI, E=RALT, I=RCTRL, O=plain (using `ht_right`)

### Outer columns

- Left outer: `key_repeat` (00), `LSHIFT` (12), `none` (24)
- Right outer: `caps_word` (11), `RSHIFT` (23), `none` (35)

### NAV layer (matches hillside NAV-MAC)

- Left: scroll right/up/left (top), mouse move L/D/R (home), scroll up/down (bottom)
- Right top: `Ctrl+Left` (prev desktop), LCLK, RCLK, `Ctrl+Right` (next desktop)
- Right home: arrow keys
- Right bottom: Home, PgDn, PgUp, End

### SYM layer (matches hillside SYMBOL_L)

- Left: symbols (PIPE, AT, LBKT, RBKT, CARET, DLLR/RCTRL, LBRC/RALT, RBRC/RGUI, BSLH, HASH, LPAR, RPAR)
- Right: numbers (N7–N9, N1/RGUI, N2/RALT, N3/RCTRL, N0, N4–N6)

### SHORTCUTS layer (hold ESC)

- Q=prev tab (`⇧⌘[`), W=LCLK, F=RCLK, P=next tab (`⇧⌘]`)
- A=Select All (`⌘A`), R=Save (`⌘S`), T=Find (`⌘F`)
- Z=Undo (`⌘Z`), X=Cut (`⌘X`), C=Copy (`⌘C`), D=Paste (`⌘V`)

### ADJ layer (SHORTCUTS + NAV)

- Right: brightness down/up (top), volume down/up/mute (home), F23/F24 (bottom)

### SYSTEM layer (SYM + NAV)

- Left home: BT profiles 0–4
- Left bottom outer: bootloader
- Right top: `OUT_USB`, `OUT_BLE`, ext power off/on
- Right bottom outer: bootloader

## Key behaviors

Defined in `config/keymap_features/behaviors.dtsi`:

| Behavior | Type | Notes |
|----------|------|-------|
| `ht_left` | hold-tap balanced 280ms | Left-side HRM |
| `ht_right` | hold-tap balanced 280ms | Right-side HRM |
| `ht_left_tp` | hold-tap tap-preferred 200ms | Fast-roll left HRM (used by other keymaps) |
| `ht_right_tp` | hold-tap tap-preferred 200ms | Fast-roll right HRM (used by other keymaps) |
| `thumb_mt` | hold-tap hold-preferred 220ms | Thumb SHIFT+SPACE/BSPC |
| `mm_cm_sc` | mod-morph shift | comma → semicolon |
| `mm_pr_col` | mod-morph shift | period → colon |

## Combos

Defined in `config/keymap_features/combos.dtsi` (shared by all keymaps):

| Combo | Keys | Layer | Output |
|-------|------|-------|--------|
| CapsWord | 17+18 (G+M) | BASE | `caps_word` |
| ESC left | 1+2 (Q+W) | BASE | ESC |
| ESC right | 9+10 (Y+SEMI) | BASE | ESC |
| Bootloader left | 0+24 | SYSTEM | bootloader |
| Bootloader right | 11+35 | SYSTEM | bootloader |

### Symbol combos (vertical same-column pairs, BASE only, 20ms timeout, 150ms prior-idle)

| Combo | Keys | Symbol |
|-------|------|--------|
| P+T | 4+16 | `-` |
| F+S | 3+15 | `+` |
| T+D | 16+28 | `=` |
| S+C | 15+27 | `!` |
| R+X | 14+26 | `` ` `` |
| W+R | 2+14 | `~` |
| L+N | 7+19 | `'` |
| U+E | 8+20 | `"` |
| N+H | 19+31 | `_` |
| E+, | 20+32 | `%` |
| Y+I | 9+21 | `*` |
| I+. | 21+33 | `&` |

## Reference keymap

The `colemak_dh` keymap is designed to match the hillside46 keymap at
`~/repos/zmk-config/config/hillside46.keymap` (Mac variant).
