#include "mgos.h"
#include "mgos_zswitch.h"

#ifdef MGOS_HAVE_MJS
#include "mjs.h"
#endif /* MGOS_HAVE_MJS */

struct mg_zswitch_state_handler {
  mgos_zswitch_state_handler_t invoke;
  void *user_data;
};

struct mg_zswitch {
  MGOS_ZSWITCH_BASE
  struct mgos_zswitch_cfg cfg;
  int inching_timer_id;
  struct mg_zswitch_state_handler state_handler;
};

#define MG_ZSWITCH_CAST(h) ((struct mg_zswitch *)h)

struct mgos_zswitch *mgos_zswitch_create(const char *id,
                                         struct mgos_zswitch_cfg *cfg) {
  if (id == NULL) return NULL;
  struct mg_zswitch *handle = calloc(1, sizeof(struct mg_zswitch));
  if (handle != NULL) {
    handle->id = strdup(id);
    handle->type = MGOS_ZTHING_SWITCH;

    handle->cfg.group_id = (cfg == NULL ? MGOS_ZSWITCH_NO_GROUP : (cfg->group_id <= 0 ?
      MGOS_ZSWITCH_NO_GROUP : cfg->group_id));
    handle->cfg.inching_timeout = (cfg == NULL ? MGOS_ZSWITCH_NO_INCHING : (cfg->inching_timeout <= 0 ?
      MGOS_ZSWITCH_NO_INCHING : cfg->inching_timeout));
    handle->cfg.inching_lock = (cfg == NULL ? false : cfg->inching_lock);
    handle->cfg.switching_time = (cfg == NULL ? MGOS_ZSWITCH_DEFAULT_SWITCHING_TIME : (cfg->switching_time < 0 ?
      MGOS_ZSWITCH_DEFAULT_SWITCHING_TIME : cfg->switching_time));
    
    handle->inching_timer_id = MGOS_INVALID_TIMER_ID;
    
    if (mgos_zthing_register(MGOS_ZTHING_CAST(handle))) {
      /* Trigger the CREATED event */
      mgos_event_trigger(MGOS_EV_ZTHING_CREATED, handle);
      return (struct mgos_zswitch *)handle;
    }
    free(handle->id);
    free(handle);
    LOG(LL_ERROR, ("Error creating '%s'. Handle registration failed.", id));
  } else {
    LOG(LL_ERROR, ("Error creating '%s'. Memory allocation failed.", id));
  }
  return NULL;
}

void mgos_zswitch_close(struct mgos_zswitch *handle) {
  struct mg_zswitch *h = MG_ZSWITCH_CAST(handle);
  mgos_zswitch_state_handler_reset(handle);
  mgos_zthing_close(MGOS_ZTHING_CAST(h));
}

bool mgos_zswitch_state_handler_set(struct mgos_zswitch *handle,
                                    mgos_zswitch_state_handler_t handler,
                                    void *user_data) {
  if (handle == NULL || handler == NULL) return false;
  struct mg_zswitch *h = MG_ZSWITCH_CAST(handle);
  if (h->state_handler.invoke != NULL) {
    LOG(LL_ERROR, ("Error setting state handler for '%s'. Handler already set.", handle->id));
    return false;
  }
  h->state_handler.invoke = handler;
  h->state_handler.user_data = user_data;
  return true;
}

void mgos_zswitch_state_handler_reset(struct mgos_zswitch *handle) {
  struct mg_zswitch *h = MG_ZSWITCH_CAST(handle);
  if (h != NULL) {
    h->state_handler.invoke = NULL;
    h->state_handler.user_data = NULL;
  }
}

bool mg_zswitch_inching_del(struct mg_zswitch *handle) {
  if (handle->inching_timer_id != MGOS_INVALID_TIMER_ID) {
    if (handle->cfg.inching_lock) {
      LOG(LL_ERROR, ("Inching of '%s' is locked and cannot be turned off.",
        handle->id));
      return false;
    }
    mgos_clear_timer(handle->inching_timer_id);
    handle->inching_timer_id = MGOS_INVALID_TIMER_ID;
  }
  return true;
}

bool mg_zswitch_state_set_off(struct mg_zswitch *handle) {
  /* Remove inching if configured and in progress */
  if (!mg_zswitch_inching_del(handle)) return false;  

  struct mgos_zswitch_state state = MGOS_ZSWITCH_STATE_OFF(
      MGOS_ZSWITCH_CAST(handle));

  /* Check if already OFF */
  int cur_state = mgos_zswitch_state_get(state.handle);
  if (cur_state == MGOS_ZTHING_RESULT_ERROR) return false;
  if (cur_state == false) return true;
 
  /* Switch OFF the switch */
  state.value = false;
  if (!handle->state_handler.invoke(MGOS_ZTHING_STATE_SET, &state,
    handle->state_handler.user_data)) {
    LOG(LL_ERROR, ("Error switching '%s' OFF becuase state-handler failure.",
        handle->id));
    return false;
  }

  /* Wait for switching-time if needed */
  if (handle->cfg.switching_time > 0) {
    mgos_msleep(handle->cfg.switching_time);
  }

  /* Trigger the STATE_UPDATED event */
  mgos_event_trigger(MGOS_EV_ZTHING_STATE_UPDATED, &state);

  return true;
}

static void mg_zswitch_inching_cb(void *arg) {
  struct mg_zswitch *handle = MG_ZSWITCH_CAST(arg);
  handle->inching_timer_id = MGOS_INVALID_TIMER_ID;
  mg_zswitch_state_set_off(handle);
}

bool mg_zswitch_state_set_on(struct mg_zswitch *handle) {
  struct mgos_zswitch_state state = MGOS_ZSWITCH_STATE_OFF(
    MGOS_ZSWITCH_CAST(handle));

  /* Check if already ON */
  int cur_state = mgos_zswitch_state_get(state.handle);
  if (cur_state == MGOS_ZTHING_RESULT_ERROR) return false;
  if (cur_state == true) return true;

  /* Check if some other switch having inching_lock is still ON */
  MGOS_ZSWITCH_GROUP_FOREACH(handle, s, {
    if (MG_ZSWITCH_CAST(s)->cfg.inching_lock == true &&
        MG_ZSWITCH_CAST(s)->inching_timer_id != MGOS_INVALID_TIMER_ID) {
      LOG(LL_ERROR, ("Error switching '%s' ON. '%s' has inching-lock and it is still ON.",
        handle->id, s->id)); 
      return false;
    }
  });

  /* Remove inching if configured and in progress */
  if (!mg_zswitch_inching_del(handle)) {
    LOG(LL_ERROR, ("Error switching '%s' ON becuase failure on turning off inching.",
        handle->id));
    return false;
  }

  /* Switch ON the switch */
  state.value = true;
  if (!handle->state_handler.invoke(MGOS_ZTHING_STATE_SET, &state,
    handle->state_handler.user_data)) {
    LOG(LL_ERROR, ("Error switching '%s' ON becuase state-handler failure.",
        handle->id));
    return false;
  }
  
  /* Wait switching time if needed */
  if (handle->cfg.switching_time > 0) {
    mgos_msleep(handle->cfg.switching_time);
  }

  /* Trigger the STATE_UPDATED event */
  mgos_event_trigger(MGOS_EV_ZTHING_STATE_UPDATED, &state);

  /* Start inching count-down */
  if (handle->cfg.inching_timeout != MGOS_ZSWITCH_NO_INCHING) {
    handle->inching_timer_id = mgos_set_timer(handle->cfg.inching_timeout,
      0, mg_zswitch_inching_cb, handle);
  }

  return true;
}

bool mgos_zswitch_state_set(struct mgos_zswitch *handle,
                            bool state) {
  struct mg_zswitch *h = MG_ZSWITCH_CAST(handle);
  if (h == NULL || h->state_handler.invoke == NULL) return false;
  
  /* Switch OFF all switches in the same group */
  if (state == true) {
    MGOS_ZSWITCH_GROUP_FOREACH(handle, sibling, {
      if (!mg_zswitch_state_set_off(MG_ZSWITCH_CAST(sibling))) {
        LOG(LL_ERROR, ("Error switching '%s' ON becuase failure switching off siblings.",
          handle->id));
        return false;
      }
    });
  }

  return (state == false ? mg_zswitch_state_set_off(h) :
    mg_zswitch_state_set_on(h));
}

int mgos_zswitch_state_toggle(struct mgos_zswitch *handle) {
  int state = mgos_zswitch_state_get(handle);
  if (state == true) {
    if (mgos_zswitch_state_set(handle, false)) return false;
  } else if (state == false) {
    if (mgos_zswitch_state_set(handle, true)) return true;
  }
  return MGOS_ZTHING_RESULT_ERROR;
}

int mgos_zswitch_state_get(struct mgos_zswitch *handle) {
  struct mg_zswitch *h = MG_ZSWITCH_CAST(handle);
  if (h != NULL || h->state_handler.invoke != NULL) {
    struct mgos_zswitch_state state = MGOS_ZSWITCH_STATE_OFF(handle);
    if (h->state_handler.invoke(MGOS_ZTHING_STATE_GET, &state,
      h->state_handler.user_data)) return state.value;
    LOG(LL_ERROR, ("Error reading '%s' state. The state-handler had issue.",
      handle->id));
  }
  return MGOS_ZTHING_RESULT_ERROR;
}

#ifdef MGOS_HAVE_MJS

struct mgos_zswitch_cfg *mjs_zswitch_cfg_create(int group_id,
                                                int inching_timeout,
                                                bool inching_lock,
                                                int switching_time) {
  struct mgos_zswitch_cfg *cfg = calloc(1, sizeof(struct mgos_zswitch_cfg));
  if (cfg != NULL) {
    cfg->group_id = (group_id == -1 ? 
      MGOS_ZSWITCH_NO_GROUP : group_id);
    cfg->inching_timeout = (inching_timeout == -1 ?
      MGOS_ZSWITCH_NO_INCHING : inching_timeout);
    cfg->inching_lock = inching_lock;
    cfg->switching_time = (switching_time == -1 ?
      MGOS_ZSWITCH_DEFAULT_SWITCHING_TIME : switching_time);
  }
  return cfg;
}

void mjs_zswitch_state_set(struct mgos_zswitch_state *state,
                           bool val) {
  if (state != NULL) state->value = val;
}

static const struct mjs_c_struct_member mjs_zswitch_state_descr[] = {
  //{"handle", offsetof(struct mgos_zswitch_state, handle), MJS_STRUCT_FIELD_TYPE_STRUCT_PTR, mjs_zswitch_descr},
  {"handle", offsetof(struct mgos_zswitch_state, handle), MJS_STRUCT_FIELD_TYPE_VOID_PTR, NULL}, 
  {"value", offsetof(struct mgos_zswitch_state, value), MJS_STRUCT_FIELD_TYPE_BOOL, NULL},
  {NULL, 0, MJS_STRUCT_FIELD_TYPE_INVALID, NULL},
};

const struct mjs_c_struct_member *mjs_zswitch_state_descr_get(void) {
  return mjs_zswitch_state_descr;
};

#endif /* MGOS_HAVE_MJS */


bool mgos_zswitch_init() {
  return true;
}
