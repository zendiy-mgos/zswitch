load('api_zthing.js');

let ZenSwitch = {
  _crt: ffi('void *mgos_zswitch_create(char *, void *)'),
  _sset: ffi('bool mgos_zswitch_state_set(void *, bool)'),
  _stog: ffi('int mgos_zswitch_state_toggle(void *)'),
  _sget: ffi('int mgos_zswitch_state_get(void *)'),
  _shset: ffi('bool mgos_zswitch_state_handler_set(void *, bool (*)(int, void *, userdata), userdata)'),
  _shres: ffi('void mgos_zswitch_state_handler_reset(void *)'), 
  _cls: ffi('void mgos_zswitch_close(void *)'),
  _cfgc: ffi('void *mjs_zswitch_cfg_create(int, int, bool , int)'),
  _ss: ffi('void mjs_zswitch_state_set(void *, bool)'),

  THING_TYPE: (4 | ZenThing.TYPE_ACTUATOR),

  _scon: function(val) {
    if (val === 1) return true;
    if (val === 0) return false;
    return val;
  },

  _shset_cb: function(act, state, ud) {
    // convert strucy zswitch_state to js object {handle:<object>, value:<bool>, thing:<ZenSwitch>}
    let sd = ffi('void *mjs_zswitch_state_descr_get(void)')(); 
    let s = s2o(state, sd);
    s.thing = ZenThing._getTFromH(s.handle);
    // invoke callback
    let r = ud.h(act, s, ud.ud);
    
    if (act === ZenThing.ACT_STATE_GET) {
      ZenSwitch._ss(state, ZenSwitch._scon(s.value));  
    }
    return r;
  },
 
  // ## **`ZenSwitch.create(id, cfg)`**
  //
  // Example:
  // ```javascript
  // let sw = ZenSwitch.create('sw1', {
  //   groupId: 1, 
  //   inchingTimeout: 1000,
  //   inchingLock: false,
  //   switchingTime, 100
  // });
  // ```
  create: function(id, cfg) {
    let cfgo = null;
    if (cfg) {
      cfgo = ZenSwitch._cfgc(
        cfg.groupId || -1,
        cfg.inchingTimeout || -1,
        ((cfg.inchingLock === undefined || cfg.inchingLock === null) ? false : cfg.inchingLock),
        cfg.switchingTime || -1
      );
    }
    // create the handle
    let handle = ZenSwitch._crt(id, cfgo);
    ZenThing._free(cfgo);
    if (!handle) return null; 
    
    // create the JS instance
    let obj = Object.create(ZenSwitch._proto);
    obj.handle = handle;
    obj.id = id;
    obj.type = ZenSwitch.THING_TYPE;
    
    // raise on-create event
    ZenThing._onCreatePub(obj);

    return obj;
  },

  _proto: {
    handle: null,
    id: null,

    setState: function(state) {
      if (state === true || state === false) {
        return ZenSwitch._sset(this.handle, state);
      }
      return false;
    },
    
    getState: function() {
      return ZenSwitch._scon(
        ZenSwitch._sget(this.handle)); 
    },
    
    toggleState: function() {
      return ZenSwitch._scon(
        ZenSwitch._stog(this.handle));
    },
    
    // ## **`object.setStateHandler(handler, userdata)`**
    //
    // Example:
    // ```javascript
    // sw.setEventHandler(function(act, state, ud) {
    //   print('Switch ID', state.thing.id);
    //   if (act === ZenThing.ACT_STATE_SET) {
    //     if (state.value) {
    //       /* switch ON */
    //     } else {
    //       /* switch OFF */
    //     }
    //   } else if (act === ZenThing.ACT_STATE_GET) {
    //     state.value = /* current switch state */
    //   }
    //   return true;
    // }, null);
    // ```
    setStateHandler: function(h, ud) {
      return ZenSwitch._shset(this.handle, ZenSwitch._shset_cb, {
        h: h,
        ud: ud
      });
    },

    resetStateHandler: function() {
      ZenSwitch._shres(this.handle);
    },
    
    close: function() {
      ZenSwitch._cls(this.handle);
    },
  },
};