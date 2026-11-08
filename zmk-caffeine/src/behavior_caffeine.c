/*
 * Caffeine behavior: a toggle that taps its bindings on a timer.
 *
 * SPDX-License-Identifier: MIT
 */

#define DT_DRV_COMPAT zmk_behavior_caffeine

#include <zephyr/device.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <drivers/behavior.h>
#include <zmk/behavior.h>
#include <zmk/behavior_queue.h>
#include <zmk/event_manager.h>
#include <zmk/events/keycode_state_changed.h>
#include <zmk/keymap.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

enum caffeine_mode {
    CAFFEINE_MODE_ROTATE,   // one binding per tick, in turn
    CAFFEINE_MODE_SEQUENCE, // all bindings per tick, back to back
};

struct behavior_caffeine_config {
    uint32_t interval_ms;
    uint32_t tap_ms;
    uint32_t wait_ms;
    enum caffeine_mode mode;
    bool cancel_on_keypress;
    uint32_t count;
    struct zmk_behavior_binding bindings[];
};

struct behavior_caffeine_data {
    const struct device *dev;
    struct k_work_delayable work;
    struct zmk_behavior_binding_event event;
    uint32_t index;
    bool active;
};

static void caffeine_stop(struct behavior_caffeine_data *data) {
    if (!data->active) {
        return;
    }
    LOG_INF("caffeine off");
    data->active = false;
    k_work_cancel_delayable(&data->work);
}

static void caffeine_tap(struct behavior_caffeine_data *data,
                         const struct behavior_caffeine_config *cfg,
                         const struct zmk_behavior_binding *binding, uint32_t release_wait) {
    zmk_behavior_queue_add(&data->event, *binding, true, cfg->tap_ms);
    zmk_behavior_queue_add(&data->event, *binding, false, release_wait);
}

static void caffeine_tick(struct k_work *work) {
    struct k_work_delayable *dwork = k_work_delayable_from_work(work);
    struct behavior_caffeine_data *data =
        CONTAINER_OF(dwork, struct behavior_caffeine_data, work);
    const struct behavior_caffeine_config *cfg = data->dev->config;

    if (!data->active) {
        return;
    }

    data->event.timestamp = k_uptime_get();

    if (cfg->mode == CAFFEINE_MODE_SEQUENCE) {
        for (uint32_t i = 0; i < cfg->count; i++) {
            // No wait after the last leg so the queue drains at once.
            uint32_t wait = (i + 1 < cfg->count) ? cfg->wait_ms : 0;
            caffeine_tap(data, cfg, &cfg->bindings[i], wait);
        }
    } else {
        caffeine_tap(data, cfg, &cfg->bindings[data->index], 0);
        data->index = (data->index + 1) % cfg->count;
    }

    k_work_schedule(&data->work, K_MSEC(cfg->interval_ms));
}

static int on_caffeine_binding_pressed(struct zmk_behavior_binding *binding,
                                       struct zmk_behavior_binding_event event) {
    const struct device *dev = zmk_behavior_get_binding(binding->behavior_dev);
    struct behavior_caffeine_data *data = dev->data;

    if (data->active) {
        caffeine_stop(data);
        return ZMK_BEHAVIOR_OPAQUE;
    }

    LOG_INF("caffeine on");
    data->active = true;
    data->index = 0;
    // Keep the toggle key's position and source so queued taps are attributed to it.
    data->event = event;
    k_work_schedule(&data->work, K_NO_WAIT);
    return ZMK_BEHAVIOR_OPAQUE;
}

static int on_caffeine_binding_released(struct zmk_behavior_binding *binding,
                                        struct zmk_behavior_binding_event event) {
    return ZMK_BEHAVIOR_OPAQUE;
}

static int behavior_caffeine_init(const struct device *dev) {
    struct behavior_caffeine_data *data = dev->data;
    data->dev = dev;
    k_work_init_delayable(&data->work, caffeine_tick);
    return 0;
}

static const struct behavior_driver_api behavior_caffeine_driver_api = {
    .binding_pressed = on_caffeine_binding_pressed,
    .binding_released = on_caffeine_binding_released,
#if IS_ENABLED(CONFIG_ZMK_BEHAVIOR_METADATA)
    .get_parameter_metadata = zmk_behavior_get_empty_param_metadata,
#endif
};

#define TRANSFORMED_BINDINGS(n)                                                                    \
    {LISTIFY(DT_INST_PROP_LEN(n, bindings), ZMK_KEYMAP_EXTRACT_BINDING, (, ), DT_DRV_INST(n))}

#define CAFFEINE_INST(n)                                                                           \
    static struct behavior_caffeine_data behavior_caffeine_data_##n = {};                          \
    static const struct behavior_caffeine_config behavior_caffeine_config_##n = {                  \
        .interval_ms = DT_INST_PROP(n, interval_ms),                                               \
        .tap_ms = DT_INST_PROP(n, tap_ms),                                                         \
        .wait_ms = DT_INST_PROP(n, wait_ms),                                                       \
        .mode = DT_INST_ENUM_IDX(n, mode),                                                         \
        .cancel_on_keypress = DT_INST_PROP(n, cancel_on_keypress),                                 \
        .count = DT_INST_PROP_LEN(n, bindings),                                                    \
        .bindings = TRANSFORMED_BINDINGS(n),                                                       \
    };                                                                                             \
    BEHAVIOR_DT_INST_DEFINE(n, behavior_caffeine_init, NULL, &behavior_caffeine_data_##n,         \
                            &behavior_caffeine_config_##n, POST_KERNEL,                            \
                            CONFIG_KERNEL_INIT_PRIORITY_DEFAULT, &behavior_caffeine_driver_api);

DT_INST_FOREACH_STATUS_OKAY(CAFFEINE_INST)

// Keycode events come from real typing. The toggle key and &mmv raise none,
// so neither turning caffeine on nor its own nudges cancel it.
static int caffeine_keycode_listener(const zmk_event_t *eh) {
    const struct zmk_keycode_state_changed *ev = as_zmk_keycode_state_changed(eh);
    if (ev == NULL || !ev->state) {
        return ZMK_EV_EVENT_BUBBLE;
    }

#define CAFFEINE_CANCEL(n)                                                                         \
    if (behavior_caffeine_config_##n.cancel_on_keypress) {                                         \
        caffeine_stop(&behavior_caffeine_data_##n);                                                \
    }
    DT_INST_FOREACH_STATUS_OKAY(CAFFEINE_CANCEL)
#undef CAFFEINE_CANCEL

    return ZMK_EV_EVENT_BUBBLE;
}

ZMK_LISTENER(behavior_caffeine, caffeine_keycode_listener);
ZMK_SUBSCRIPTION(behavior_caffeine, zmk_keycode_state_changed);
