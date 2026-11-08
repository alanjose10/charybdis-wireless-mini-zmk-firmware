# zmk-caffeine

ZMK module. Adds `zmk,behavior-caffeine`, a toggle that taps a list of
bindings on a timer so the host does not go idle.

```dts
ca: caffeine {
    compatible = "zmk,behavior-caffeine";
    #binding-cells = <0>;
    bindings = <&mmv MOVE_UP>, <&mmv MOVE_LEFT>,
               <&mmv MOVE_DOWN>, <&mmv MOVE_RIGHT>;
    mode = "sequence";
    interval-ms = <30000>;
    tap-ms = <100>;
    cancel-on-keypress;
};
```

| Property | Default | Meaning |
|---|---|---|
| `bindings` | required | Behaviors to tap |
| `mode` | `rotate` | `rotate`: one binding per tick, in turn. `sequence`: all bindings per tick, back to back |
| `interval-ms` | 30000 | Time between ticks |
| `tap-ms` | 30 | Press duration of each tap. Longer `&mmv` taps travel further |
| `wait-ms` | 0 | Pause between taps inside one tick (sequence mode) |
| `cancel-on-keypress` | off | Turn off when any keycode is pressed. The toggle key and `&mmv` raise no keycode, so they never cancel it |

Press `&ca` to start. Press again to stop. The behavior runs on the central
(the dongle), so the toggle does not reset when a half sleeps.

Add the module folder to `ZMK_EXTRA_MODULES` when you build.
