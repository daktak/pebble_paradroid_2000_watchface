#include <pebble.h>
#include "settings.h"
#include "main.h"

static ClaySettings settings;

#define SETTINGS_VERSION 1

static void prv_default_settings() {
  settings.version = SETTINGS_VERSION;
  settings.droid_change = 5;
  settings.droid_select = 0;
  settings.quiet_time = false;
  settings.quiet_start = 23;
  settings.quiet_stop = 6;
  settings.color_val = 0x00FFFFFF;
}

static void prv_save_settings() {
  persist_write_data(SETTINGS_KEY, &settings, sizeof(settings));
}

void prv_load_settings() {
  prv_default_settings();
  persist_read_data(SETTINGS_KEY, &settings, sizeof(settings));
  if (settings.version != SETTINGS_VERSION) {
    prv_default_settings();
    prv_save_settings();
  }
}

ClaySettings get_settings() {
  return settings;
}

void prv_inbox_received_handler(DictionaryIterator *iter, void *context) {
  prv_default_settings();

  Tuple *t = dict_find(iter, MESSAGE_KEY_COLOR_INDEX);
  if (t) settings.color_val = (uint32_t)t->value->int32;

  t = dict_find(iter, MESSAGE_KEY_DROID_SELECT);
  if (t) {
    int val = atoi(t->value->cstring);
    if (val < 0) val = 0;
    if (val > DROID_COUNT) val = DROID_COUNT;
    settings.droid_select = val;
  }

  t = dict_find(iter, MESSAGE_KEY_DROID_CHANGE);
  if (t) {
    int val = atoi(t->value->cstring);
    if (val < 0) val = 0;
    settings.droid_change = val;
  }

  t = dict_find(iter, MESSAGE_KEY_DROID_QUIET_TIME);
  if (t) settings.quiet_time = t->value->int32 == 1;

  t = dict_find(iter, MESSAGE_KEY_DROID_QUIET_START);
  if (t) {
    settings.quiet_start = (int)t->value->int32;
    if (settings.quiet_start < 0) settings.quiet_start = 0;
    if (settings.quiet_start > 23) settings.quiet_start = 23;
  }

  t = dict_find(iter, MESSAGE_KEY_DROID_QUIET_STOP);
  if (t) {
    settings.quiet_stop = (int)t->value->int32;
    if (settings.quiet_stop < 0) settings.quiet_stop = 0;
    if (settings.quiet_stop > 23) settings.quiet_stop = 23;
  }

  prv_save_settings();
  apply_color_index(settings.color_val);
  change_droid();
}
