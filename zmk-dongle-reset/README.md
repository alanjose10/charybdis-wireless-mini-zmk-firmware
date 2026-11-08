# zmk-dongle-reset

ZMK module. Adds `zmk,behavior-dongle-reset`: the same as ZMK's `&sys_reset`
and `&bootloader`, but with central locality. The reset acts on the dongle,
not on the half whose key was pressed.

Include the ready-made nodes in your keymap:

```dts
#include <behaviors/dongle_reset.dtsi>
```

Then bind `&dongle_bootloader` or `&dongle_sys_reset` to a key.

Add the module folder to `ZMK_EXTRA_MODULES` when you build.
