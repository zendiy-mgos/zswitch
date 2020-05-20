# ZenSwitch
## Overview
A Mongoose OS library for Zen Switches ecosystem.
## C/C++ API Reference
### mgos_zswitch
```c
struct mgos_zswitch {
  char *id;
  int type;
};
```

Switch handle.

|Fields||
|--|--|
|id|Switch unique ID|
|type|Fixed value: `MGOS_ZTHING_SWITCH`.|
### mgos_zswitch_create()
```c
struct mgos_zswitch *mgos_zswitch_create(const char *id, struct mgos_zswitch_cfg *cfg);
```
Create and initialize the switch instance. Returns the instance handle, or `NULL` on error.

|Parameter||
|--|--|
|id|Unique ZenThing ID.|
|cfg|Optional. Switch configuration.|

**Example 1** - Create a switch using default configuration.
```c
struct mgos_zswitch *sw = mgos_zswitch_create("sw-1", NULL);
```
**Example 2** - Create two switches and put them in the same (inching) group.
```c
struct mgos_zswitch_cfg cfg = MGOS_ZSWITCH_CFG;
cfg.group_id = 1;

struct mgos_zswitch *sw1 = mgos_zswitch_create("sw-1", &cfg);
struct mgos_zswitch *sw2 = mgos_zswitch_create("sw-2", &cfg);
```
### mgos_zswitch_close()
```c
void mgos_zswitch_close(struct mgos_zswitch *handle);
```
Close and destroy the switch instance.

|Parameter||
|--|--|
|handle|Switch handle.|
### mgos_zswitch_state
```c
struct mgos_zswitch_state {
  struct mgos_zswitch *handle;
  bool value;
};
```

Switch state for `(*mgos_zswitch_state_handler_t)`.  

|Fields||
|--|--|
|handle|Switch handle|
|state|Switch state.|
### (*mgos_zswitch_state_handler_t)
```c
typedef bool (*mgos_zswitch_state_handler_t)(enum mgos_zthing_state_act act,
                                             struct mgos_zswitch_state *state,
                                             void *user_data);
```

Handler signature for `mgos_zswitch_state_handler_set()`.  

|Parameter||
|--|--|
|act|The action the handler must manage: `MGOS_ZTHING_STATE_SET` or `MGOS_ZTHING_STATE_GET`|
|state|Switch state.|
|user_data|Handler's user-data.|

### mgos_zswitch_state_handler_set()
```c
bool mgos_zswitch_state_handler_set(struct mgos_zswitch *handle,
                                    mgos_zswitch_state_handler_t handler,
                                    void *user_data);
```
Set the handler for managing switch state (get or set). Returns `true` on success, `false` otherwise.  

|Parameter||
|--|--|
|handle|Switch handle.|
|handler|State handler.|
|user_data|Handler's user-data.|

**Example** - Set and handler that manages (read/write) switch state unsing a GPIO.
```c
bool zswitch_state_handler(enum mgos_zthing_state_act act, struct mgos_zswitch_state *state, void *user_data) {
  int pin = 5;
  if (act == MGOS_ZTHING_STATE_SET) {
    mgos_gpio_write(pin, state->value);
    mgos_msleep(10);
    if (mgos_gpio_read(pin) != state->value) {
      LOG(LL_ERROR, ("Unexpected GPIO value reading '%s' state.", state->handle->id));
      return false;
    }
  } else if (act == MGOS_ZTHING_STATE_GET) {
    state->value = mgos_gpio_read(pin);
  }
  return true;
  (void) user_data;
}

struct mgos_zswitch *sw = mgos_zswitch_create("sw-1", NULL);
mgos_zswitch_state_handler_set(sw, zswitch_state_handler, NULL);
```
### mgos_zswitch_state_handler_reset()
```c
void mgos_zswitch_state_handler_reset(struct mgos_zswitch *handle);
```
Reset the state handler previously set using `mgos_zswitch_state_handler_set(...)` function.

|Parameter||
|--|--|
|handle|Switch handle.|
### mgos_zswitch_state_set()
```c
bool mgos_zswitch_state_set(struct mgos_zswitch *handle, bool state);
```
Set switch state (ON or OFF). Returns `true` on success, `false` otherwise.  

|Parameter||
|--|--|
|handle|Switch handle.|
|state|Desired state (`true`=ON, `false`=OFF).|

**Example** - Turn the switch ON.
```c
if (mgos_zswitch_state_set(sw, true)) {
  LOG(LL_INFO, ("Switch '%s' successfully turned ON.", sw->id));
} else {
  LOG(LL_ERROR, ("Error tunring ON switch '%s'.", sw->id));
}
```
### mgos_zswitch_state_toggle()
```c
int mgos_zswitch_state_toggle(struct mgos_zswitch *handle);
```
Toggle switch state. Returns the new state (`true` or `false`), `MGOS_ZTHING_RESULT_ERROR` on error.  

|Parameter||
|--|--|
|handle|Switch handle.|

**Example** - Toggle the switch.
```c
int state = mgos_zswitch_state_toggle(sw);
if (state == MGOS_ZTHING_RESULT_ERROR) {
  LOG(LL_ERROR, ("Error toggling switch '%s'.", sw->id));
} else {
  LOG(LL_INFO, ("Switch '%s' successfully turned %s.", sw->id, (state == true ? "ON" : "OFF")));
}
```
### mgos_zswitch_state_get()
```c
int mgos_zswitch_state_get(struct mgos_zswitch *handle);
```
Get switch state. Returns `true` or `false` in case the switch is ON or OFF, `MGOS_ZTHING_RESULT_ERROR` on error.  

|Parameter||
|--|--|
|handle|Switch handle.|

**Example** - Toggle the switch.
```c
int state = mgos_zswitch_state_get(sw);
if (state == MGOS_ZTHING_RESULT_ERROR) {
  LOG(LL_ERROR, ("Error reading state of switch '%s'.", sw->id));
} else {
  LOG(LL_INFO, ("Switch '%s' is %s.", sw->id, (state == true ? "ON" : "OFF")));
}
```
