#include <pebble.h>
#include "main.h"
#include "settings.h"

#define TIME_FONT FONT_KEY_LECO_42_NUMBERS
#define ROW_H 50

static const uint32_t DROID_RESOURCES[] = {
  RESOURCE_ID_DROID_001, RESOURCE_ID_DROID_123, RESOURCE_ID_DROID_139,
  RESOURCE_ID_DROID_247, RESOURCE_ID_DROID_249, RESOURCE_ID_DROID_296,
  RESOURCE_ID_DROID_302, RESOURCE_ID_DROID_329, RESOURCE_ID_DROID_420,
  RESOURCE_ID_DROID_476, RESOURCE_ID_DROID_493, RESOURCE_ID_DROID_516,
  RESOURCE_ID_DROID_571, RESOURCE_ID_DROID_598, RESOURCE_ID_DROID_614,
  RESOURCE_ID_DROID_615, RESOURCE_ID_DROID_629, RESOURCE_ID_DROID_711,
  RESOURCE_ID_DROID_742, RESOURCE_ID_DROID_751, RESOURCE_ID_DROID_821,
  RESOURCE_ID_DROID_834, RESOURCE_ID_DROID_883, RESOURCE_ID_DROID_999
};

static Window *s_window;
static GBitmap *s_droid_bitmap;
static BitmapLayer *s_droid_layer;
static TextLayer *s_hh_layer;
static TextLayer *s_mm_layer;
static char s_hh_buf[4];
static char s_mm_buf[4];

static GColor get_time_color(int index) {
  switch (index) {
    case 1:  return GColorGreen;
    case 2:  return GColorYellow;
    case 3:  return GColorCyan;
    case 4:  return GColorOrange;
    default: return GColorWhite;
  }
}

void apply_color_index(int index) {
  GColor c = get_time_color(index);
  text_layer_set_text_color(s_hh_layer, c);
  text_layer_set_text_color(s_mm_layer, c);
}

void change_droid() {
  ClaySettings s = get_settings();
  int idx = s.droid_select > 0 ? s.droid_select - 1 : rand() % DROID_COUNT;
  if (s_droid_bitmap) gbitmap_destroy(s_droid_bitmap);
  s_droid_bitmap = gbitmap_create_with_resource(DROID_RESOURCES[idx]);
  bitmap_layer_set_bitmap(s_droid_layer, s_droid_bitmap);
}

static bool is_quiet_time(struct tm *tick_time, ClaySettings *s) {
  if (!s->quiet_time) return false;
  if (s->quiet_start < s->quiet_stop)
    return tick_time->tm_hour >= s->quiet_start && tick_time->tm_hour < s->quiet_stop;
  return tick_time->tm_hour >= s->quiet_start || tick_time->tm_hour < s->quiet_stop;
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  strftime(s_hh_buf, sizeof(s_hh_buf),
           clock_is_24h_style() ? "%H" : "%I", tick_time);
  strftime(s_mm_buf, sizeof(s_mm_buf), "%M", tick_time);
  text_layer_set_text(s_hh_layer, s_hh_buf);
  text_layer_set_text(s_mm_layer, s_mm_buf);

  ClaySettings s = get_settings();
  if (is_quiet_time(tick_time, &s)) return;

  if (s.droid_select == 0 && s.droid_change > 0 &&
      tick_time->tm_min % s.droid_change == 0) {
    change_droid();
  }
}

static void window_load(Window *window) {
  Layer *root = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(root);
  ClaySettings s = get_settings();

  s_droid_layer = bitmap_layer_create(GRect(0, 0, bounds.size.w, bounds.size.h));
  bitmap_layer_set_compositing_mode(s_droid_layer, GCompOpAssign);
  layer_add_child(root, bitmap_layer_get_layer(s_droid_layer));

  int idx = s.droid_select > 0 ? s.droid_select - 1 : rand() % DROID_COUNT;
  s_droid_bitmap = gbitmap_create_with_resource(DROID_RESOURCES[idx]);
  bitmap_layer_set_bitmap(s_droid_layer, s_droid_bitmap);

  GSize img_sz = gbitmap_get_bounds(s_droid_bitmap).size;
  int droid_w = img_sz.w;
  int avail_w = bounds.size.w - droid_w;
  int gap = 2;
  int total_h = ROW_H * 2 + gap;
  int y0 = (bounds.size.h - total_h) / 2;

  layer_set_frame(bitmap_layer_get_layer(s_droid_layer),
                  GRect(0, 0, droid_w, bounds.size.h));

  GColor tc = get_time_color(s.color_index);

  s_hh_layer = text_layer_create(GRect(droid_w, y0, avail_w, ROW_H));
  text_layer_set_background_color(s_hh_layer, GColorClear);
  text_layer_set_text_color(s_hh_layer, tc);
  text_layer_set_font(s_hh_layer, fonts_get_system_font(TIME_FONT));
  text_layer_set_text_alignment(s_hh_layer, GTextAlignmentCenter);
  layer_add_child(root, text_layer_get_layer(s_hh_layer));

  s_mm_layer = text_layer_create(GRect(droid_w, y0 + ROW_H + gap, avail_w, ROW_H));
  text_layer_set_background_color(s_mm_layer, GColorClear);
  text_layer_set_text_color(s_mm_layer, tc);
  text_layer_set_font(s_mm_layer, fonts_get_system_font(TIME_FONT));
  text_layer_set_text_alignment(s_mm_layer, GTextAlignmentCenter);
  layer_add_child(root, text_layer_get_layer(s_mm_layer));

  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  tick_handler(t, MINUTE_UNIT);

  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
}

static void window_unload(Window *window) {
  tick_timer_service_unsubscribe();
  gbitmap_destroy(s_droid_bitmap);
  bitmap_layer_destroy(s_droid_layer);
  text_layer_destroy(s_hh_layer);
  text_layer_destroy(s_mm_layer);
}

static void init(void) {
  srand(time(NULL));
  prv_load_settings();
  app_message_register_inbox_received(prv_inbox_received_handler);
  app_message_open(256, 256);

  s_window = window_create();
  window_set_background_color(s_window, GColorBlack);
  window_set_window_handlers(s_window, (WindowHandlers){
    .load = window_load,
    .unload = window_unload,
  });
  window_stack_push(s_window, true);
}

static void deinit(void) {
  window_destroy(s_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
