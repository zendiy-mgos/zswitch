# ZenSwitch
## Overview
A Mongoose OS library for Zen Switches ecosystem.
## C/C++ API Reference
### mgos_zswitch_create()
```c
struct mgos_zswitch *mgos_zswitch_create(const char *id, struct mgos_zswitch_cfg *cfg);
```
Create and initialize the switch instance. Return the instance handle, or `NULL` on error.

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
|handle|Switch handle|
### mgos_zswitch_state_handler_set()
```c
bool mgos_zswitch_state_handler_set(struct mgos_zswitch *handle,
                                    mgos_zswitch_state_handler_t handler,
                                    void *user_data);
```
Set the handler for manaing (get/set) switch state. Return `true` on success, `false` otherwise.  

|Parameter||
|--|--|
|handle|Switch handle|
|handler|State handler|
|user_data|Handler user-data|

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
