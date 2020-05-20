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
### mgos_zswitch_close()
```c
void mgos_zswitch_close(struct mgos_zswitch *handle);
```
Close and destroy the switch instance.

|Parameter||
|--|--|
|handle|Switch handle|

**Example** - Create a switch using default configuration.
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
