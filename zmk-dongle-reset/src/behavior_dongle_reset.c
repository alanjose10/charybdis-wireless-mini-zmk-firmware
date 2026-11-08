/*
 * Dongle reset behavior: ZMK's reset behavior with central locality.
 *
 * SPDX-License-Identifier: MIT
 */

#define DT_DRV_COMPAT zmk_behavior_dongle_reset

#include <zephyr/device.h>
#include <zephyr/sys/reboot.h>
#include <zephyr/logging/log.h>

#include <drivers/behavior.h>
#include <zmk/behavior.h>

#if IS_ENABLED(CONFIG_RETENTION_BOOT_MODE)
#include <zephyr/retention/bootmode.h>
#endif

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

struct behavior_dongle_reset_config {
#if IS_ENABLED(CONFIG_RETENTION_BOOT_MODE)
    enum BOOT_MODE_TYPES boot_mode;
#else
    int type;
#endif
};

static int on_dongle_reset_binding_pressed(struct zmk_behavior_binding *binding,
                                           struct zmk_behavior_binding_event event) {
    const struct device *dev = zmk_behavior_get_binding(binding->behavior_dev);
    const struct behavior_dongle_reset_config *cfg = dev->config;

#if IS_ENABLED(CONFIG_RETENTION_BOOT_MODE)
    int ret = bootmode_set(cfg->boot_mode);
    if (ret < 0) {
        LOG_ERR("Failed to set the boot mode (%d)", ret);
        return ZMK_BEHAVIOR_OPAQUE;
    }
    sys_reboot(SYS_REBOOT_WARM);
#else
    // Adafruit nRF52 bootloader reads the reboot reason: 0x57 enters UF2 mode.
    sys_reboot(cfg->type);
#endif

    return ZMK_BEHAVIOR_OPAQUE;
}

static const struct behavior_driver_api behavior_dongle_reset_driver_api = {
    .binding_pressed = on_dongle_reset_binding_pressed,
    .locality = BEHAVIOR_LOCALITY_CENTRAL,
#if IS_ENABLED(CONFIG_ZMK_BEHAVIOR_METADATA)
    .get_parameter_metadata = zmk_behavior_get_empty_param_metadata,
#endif
};

#define DONGLE_RST_INST(n)                                                                         \
    static const struct behavior_dongle_reset_config behavior_dongle_reset_config_##n = {          \
        COND_CODE_1(                                                                               \
            IS_ENABLED(CONFIG_RETENTION_BOOT_MODE),                                                \
            (DT_INST_PROP(n, bootloader) ? BOOT_MODE_TYPE_BOOTLOADER : BOOT_MODE_TYPE_NORMAL),     \
            (.type = DT_INST_PROP(n, type))),                                                      \
    };                                                                                             \
    BEHAVIOR_DT_INST_DEFINE(n, NULL, NULL, NULL, &behavior_dongle_reset_config_##n, POST_KERNEL,   \
                            CONFIG_KERNEL_INIT_PRIORITY_DEFAULT,                                   \
                            &behavior_dongle_reset_driver_api);

DT_INST_FOREACH_STATUS_OKAY(DONGLE_RST_INST)
