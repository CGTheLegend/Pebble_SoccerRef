#include <pebble.h>
#include "checkbox_window.h"

/*
 * Example implementation of the checkbox list UI pattern.
 */

static Window *main_window;
static MenuLayer *menu_layer;

static GBitmap *tick_black_bitmap, *tick_white_bitmap;
static bool s_selections[CHECKBOX_WINDOW_NUM_ROWS];

static uint16_t get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *context) {
  return CHECKBOX_WINDOW_NUM_ROWS + 1;
}

static void draw_row_callback(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *context) {
  if(cell_index->row == CHECKBOX_WINDOW_NUM_ROWS) {
    // Submit item
    menu_cell_basic_draw(ctx, cell_layer, "Submit", NULL, NULL);
  } else {
    // Choice item
    static char s_buff[16];
    snprintf(s_buff, sizeof(s_buff), "Choice %d", (int)cell_index->row);
    menu_cell_basic_draw(ctx, cell_layer, s_buff, NULL, NULL);

    // Selected?
    GBitmap *ptr = tick_black_bitmap;
    if(menu_cell_layer_is_highlighted(cell_layer)) {
      graphics_context_set_stroke_color(ctx, GColorWhite);
      ptr = tick_white_bitmap;
    }

    GRect bounds = layer_get_bounds(cell_layer);
    GRect bitmap_bounds = gbitmap_get_bounds(ptr);

    // Draw checkbox
    GRect r = GRect(
      bounds.size.w - (2 * CHECKBOX_WINDOW_BOX_SIZE),
      (bounds.size.h / 2) - (CHECKBOX_WINDOW_BOX_SIZE / 2),
      CHECKBOX_WINDOW_BOX_SIZE, CHECKBOX_WINDOW_BOX_SIZE);
    graphics_draw_rect(ctx, r);
    if(s_selections[cell_index->row]) {
      graphics_context_set_compositing_mode(ctx, GCompOpSet);
      graphics_draw_bitmap_in_rect(ctx, ptr, GRect(r.origin.x, r.origin.y - 3, bitmap_bounds.size.w, bitmap_bounds.size.h));
    }
  }
}

static int16_t get_cell_height_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *context) {
  return PBL_IF_ROUND_ELSE(
    menu_layer_is_index_selected(menu_layer, cell_index) ?
      MENU_CELL_ROUND_FOCUSED_SHORT_CELL_HEIGHT : MENU_CELL_ROUND_UNFOCUSED_TALL_CELL_HEIGHT,
    CHECKBOX_WINDOW_CELL_HEIGHT);
}

static void select_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context) {
  if(cell_index->row == CHECKBOX_WINDOW_NUM_ROWS) {
    // Do something with choices made
    for(int i = 0; i < CHECKBOX_WINDOW_NUM_ROWS; i++) {
      APP_LOG(APP_LOG_LEVEL_INFO, "Option %d was %s", i, (s_selections[i] ? "selected" : "not selected"));
    }
    window_stack_pop(true);
  } else {
    // Check/uncheck
    int row = cell_index->row;
    s_selections[row] = !s_selections[row];
    menu_layer_reload_data(menu_layer);
  }
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  tick_black_bitmap = gbitmap_create_with_resource(RESOURCE_ID_TICK_BLACK);
  tick_white_bitmap = gbitmap_create_with_resource(RESOURCE_ID_TICK_WHITE);

  menu_layer = menu_layer_create(bounds);
  menu_layer_set_click_config_onto_window(menu_layer, window);
  menu_layer_set_callbacks(menu_layer, NULL, (MenuLayerCallbacks) {
      .get_num_rows = get_num_rows_callback,
      .draw_row = draw_row_callback,
      .get_cell_height = get_cell_height_callback,
      .select_click = select_callback,
  });
  layer_add_child(window_layer, menu_layer_get_layer(menu_layer));
}

static void window_unload(Window *window) {
  menu_layer_destroy(menu_layer);

  gbitmap_destroy(tick_black_bitmap);
  gbitmap_destroy(tick_white_bitmap);

  window_destroy(window);
  main_window = NULL;
}

void checkbox_window_push() {
  if(!main_window) {
    main_window = window_create();
    window_set_window_handlers(main_window, (WindowHandlers) {
        .load = window_load,
        .unload = window_unload,
    });
  }
  window_stack_push(main_window, true);
}