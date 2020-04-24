/*
 * Copyright (c) 2020 ZenDIY
 * All rights reserved
 *
 * Licensed under the Apache License, Version 2.0 (the ""License"");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an ""AS IS"" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef MGOS_ZSWITCH_H_
#define MGOS_ZSWITCH_H_

#include <stdio.h>
#include <stdbool.h>
#include "mgos_zthing.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MGOS_ZTHING_SWITCH (4 | MGOS_ZTHING_ACTUATOR)

#ifdef MGOS_HAVE_MJS

#define MJS_ZSWITCH_DESCR \
  MJS_ZTHING_DESCR

static const struct mjs_c_struct_member mjs_zswitch_descr[] = {
  MJS_ZSWITCH_DESCR
  {NULL, 0, MJS_STRUCT_FIELD_TYPE_INVALID, NULL},
};

#endif /* MGOS_HAVE_MJS */

/* WARN: if you change MGOS_ZSWITCH_BASE below,
   you must update the above MJS_ZSWITCH_DESCR as well. */
   
#define MGOS_ZSWITCH_BASE \
  MGOS_ZTHING_BASE

/* WARN: if you change the struct 'mgos_zswitch' below,
   you must update the above MJS_ZSWITCH_DESCR as well. */

struct mgos_zswitch {
  MGOS_ZSWITCH_BASE
};

struct mg_zswitch;

#define MGOS_ZSWITCH_CAST(h) ((struct mgos_zswitch *)h)

#define MGOS_ZSWITCH_NO_GROUP 0
#define MGOS_ZSWITCH_NO_INCHING 0
#define MGOS_ZSWITCH_DEFAULT_SWITCHING_TIME 10

#define MGOS_ZSWITCH_CMD_ON "ON"
#define MGOS_ZSWITCH_CMD_OFF "OFF"
#define MGOS_ZSWITCH_CMD_TOGGLE "TOGGLE"

#define MGOS_ZSWITCH_CFG {            \
  MGOS_ZSWITCH_NO_GROUP,              \
  MGOS_ZSWITCH_NO_INCHING,            \
  false,                              \
  MGOS_ZSWITCH_DEFAULT_SWITCHING_TIME }

struct mgos_zswitch_cfg {
  int group_id;
  int inching_timeout;
  bool inching_lock;
  int switching_time;
};

#define MGOS_ZSWITCH_GROUP_FOREACH(item, sibling, lambda) {       \
    MGOS_ZTHING_FOREACH(sibling) {                                \
      if (((struct mg_zswitch *)(item))->cfg.group_id ==          \
        MGOS_ZSWITCH_NO_GROUP) break;                             \
      if ((sibling)->type != MGOS_ZTHING_SWITCH ||                \
          ((void *)(sibling)) == ((void *)(item))) {              \
        continue;                                                 \
      }                                                           \
      if (((struct mg_zswitch *)(sibling))->cfg.group_id ==       \
        MGOS_ZSWITCH_NO_GROUP) continue;                          \
      if (((struct mg_zswitch *)(sibling))->cfg.group_id ==       \
          ((struct mg_zswitch *)(item))->cfg.group_id) {          \
        (lambda);                                                 \
      }                                                           \
    }                                                             \
  }

#define MGOS_ZSWITCH_STATE_ON(h) {h, true}
#define MGOS_ZSWITCH_STATE_OFF(h) {h, false}
struct mgos_zswitch_state {
  struct mgos_zswitch *handle;
  bool value;
};

typedef bool (*mgos_zswitch_state_handler_t)(enum mgos_zthing_state_act act,
                                             struct mgos_zswitch_state *state,
                                             void *user_data);

struct mgos_zswitch *mgos_zswitch_create(const char *id,
                                         struct mgos_zswitch_cfg *cfg);

bool mgos_zswitch_state_set(struct mgos_zswitch *handle, bool state);

int mgos_zswitch_state_toggle(struct mgos_zswitch *handle);

int mgos_zswitch_state_get(struct mgos_zswitch *handle);

bool mgos_zswitch_state_handler_set(struct mgos_zswitch *handle,
                                    mgos_zswitch_state_handler_t handler,
                                    void *user_data);

void mgos_zswitch_state_handler_reset(struct mgos_zswitch *handle);

void mgos_zswitch_close(struct mgos_zswitch *handle);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* MGOS_ZSWITCH_H_ */