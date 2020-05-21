# ZenSwitch
## Overview
A Mongoose OS library for Zen Switches ecosystem.
## GET STARTED
Build up your own device in few minutes just starting from one of the following samples.

|Sample|Notes|
|--|--|
|[zswitch-demo](https://github.com/zendiy-mgos/zswitch-demo)|Mongoose OS demo firmware for using ZenSwitches.|
|[zswitch-gpio-demo](https://github.com/zendiy-mgos/zswitch-gpio-demo)|Mongoose OS demo firmware for using GPIO-enabled ZenSwitches.|
|[zswitch-mqtt-demo](https://github.com/zendiy-mgos/zswitch-mqtt-demo)|Mongoose OS demo firmware for using MQTT to drive ZenSwitches.|
## Usage
Include the library into your `mos.yml` file.
```yaml
libs:
  - origin: https://github.com/zendiy-mgos/zswitch
```
If you are developing a JavaScript firmware, load `api_hcsr04.js` in your `init.js` file.
```js
load('api_zswitch.js');
```
## C/C++ API Reference
### mgos_zswitch
```c
struct mgos_zswitch {
  char *id;
  int type;
};
```
Switch handle. You can get a valid handle using `gos_zswitch_create()`.

|Fields||
|--|--|
|id|Switch unique ID|
|type|Fixed value: `MGOS_ZTHING_SWITCH`.|

**Example** - Use handle fields.
```c
struct mgos_zswitch *handle = mgos_zswitch_create("sw-1", NULL);
LOG(LL_INFO, ("ID '%s' detected.", handle->id));
if ((handle->type & MGOS_ZTHING_SENSOR) == MGOS_ZTHING_SENSOR) {
  LOG(LL_INFO, ("Sensor's handle detected."));
}
if ((handle->type & MGOS_ZTHING_ACTUATOR) == MGOS_ZTHING_ACTUATOR) {
  LOG(LL_INFO, ("Actuator's handle detected."));
}
if (handle->type == MGOS_ZTHING_SWITCH) {
  LOG(LL_INFO, ("Switch's handle detected."));
}
```
The console output:
```console
ID 'sw-1' detected.
Sensor's handle detected.
Actuator's handle detected.
Switch's handle detected.
```
### mgos_zswitch_cfg
```c
struct mgos_zswitch_cfg {
  int group_id;
  int inching_timeout;
  bool inching_lock;
  int switching_time;
};
```
Switch configuration values for `gos_zswitch_create()`.

|Fields||
|--|--|
|group_id|The group to which the switch belongs. Switches in the same group run in interlock mode. Set to `MGOS_ZSWITCH_NO_GROUP` to disable gouping and interlock mode.|
|inching_timeout|The inching timeout, in milliseconds. Set to `MGOS_ZSWITCH_NO_INCHING` to disable inching mode.|
|inching_lock|If `true`, this flag prevents a switch to be turned OFF before its inching timeout. Set to `false` to disable the lock.|
|switching_time|The time, in milliseconds, the physical switch soldered on the circuit board (like a relay) may require to change its state. Set to `MGOS_ZSWITCH_DEFAULT_SWITCHING_TIME ` to use the default 10ms timeout ot set to `0` to disable it.|

**Example** - Create and initialize configuration settings.
```c
// default cfg {MGOS_ZSWITCH_NO_GROUP, MGOS_ZSWITCH_NO_INCHING, false, MGOS_ZSWITCH_DEFAULT_SWITCHING_TIME}
struct mgos_zswitch_cfg cfg = MGOS_ZSWITCH_CFG;
```
### mgos_zswitch_create()
```c
struct mgos_zswitch *mgos_zswitch_create(const char *id, struct mgos_zswitch_cfg *cfg);
```
Create and initialize the switch instance. Returns the instance handle, or `NULL` on error.

|Parameter||
|--|--|
|id|Unique ZenThing ID.|
|cfg|Optional. Switch configuration. If `NULL`, following default configuration values are used: group_id=`MGOS_ZSWITCH_NO_GROUP`, inching_timeout=`MGOS_ZSWITCH_NO_INCHING`, inching_lock=`false`, switching_time=`MGOS_ZSWITCH_DEFAULT_SWITCHING_TIME`.|

**Example 1** - Create a switch using default configuration values.
```c
// group_id = MGOS_ZSWITCH_NO_GROUP
// inching_timeout = MGOS_ZSWITCH_NO_INCHING
// inching_lock= false
// switching_time = MGOS_ZSWITCH_DEFAULT_SWITCHING_TIME

struct mgos_zswitch *sw = mgos_zswitch_create("sw-1", NULL);
```
**Example 2** - Create two switches and put them in the same group (interlock mode).
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
Reset the state handler previously set using `mgos_zswitch_state_handler_set()`.

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

**Example** - Read and log switch state.
```c
int state = mgos_zswitch_state_get(sw);
if (state == MGOS_ZTHING_RESULT_ERROR) {
  LOG(LL_ERROR, ("Error reading state of switch '%s'.", sw->id));
} else {
  LOG(LL_INFO, ("Switch '%s' is %s.", sw->id, (state == true ? "ON" : "OFF")));
}
```
## JS API Reference
### ZenSwitch.create()
```js
ZenSwitch.create(id, cfg);
```
Create and initialize the switch instance. Returns the instance, or `null` on error.

|Parameter||
|--|--|
|id|Unique ZenThing ID|
|cfg|Optional. Switch configuration. If missing, default configuration values are used.|

|Configuration property||
|--|--|
|groupId|Optional. The group to which the switch belongs. Switches in the same group run in interlock mode.|
|inchingTimeout|Optional. The inching timeout, in milliseconds.|
|inchingLock|Optional. If `true`, this flag prevents a switch to be turned OFF before its inching timeout.|
|switchingTime|Optional. The time, in milliseconds, the physical switch soldered on the circuit board (like a relay) may require to change its state.|

**Example 1** - Create a switch using default configuration values.
```js
let sw = ZenSwitch.create('sw-1');
```
**Example 2** - Create two switches and put them in the same group (interlock mode).
```js
let cfg = {groupId: 1};
let sw1 = ZenSwitch.create('sw-1', cfg);
let sw2 = ZenSwitch.create('sw-2', cfg);
```
### <sensor_instance>.getState()

## Additional resources
Take a look to some other demo samples.

|Sample|Notes|
|--|--|
|[zswitch-gpio-demo](https://github.com/zendiy-mgos/zswitch-gpio-demo)|Mongoose OS demo firmware for using GPIO-enabled ZenSwitches.|
|[zswitch-mqtt-demo](https://github.com/zendiy-mgos/zswitch-mqtt-demo)|Mongoose OS demo firmware for using MQTT to drive ZenSwitches.|
