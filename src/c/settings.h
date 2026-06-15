#pragma once
#include <pebble.h>

#define SETTINGS_KEY 1

typedef struct {
  uint32_t version;
  int droid_change;
  int droid_select;
  bool quiet_time;
  int quiet_start;
  int quiet_stop;
  uint32_t color_val;
} ClaySettings;

void prv_load_settings();
ClaySettings get_settings();
void prv_inbox_received_handler(DictionaryIterator *iter, void *context);
