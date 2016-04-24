#include <pebble.h>
#include <inttypes.h>
#include "main.h"
#include "constants.h"
#include "mood.h"
#include "app_message.h"

static void init();
static void deinit();
static void window_load(Window *window);
static void update_image(uint32_t resource_id);
static void window_unload(Window *window);
static void click_config_provider(void *context);
static void up_click_handler(ClickRecognizerRef recognizer, void *context);
static void down_click_handler(ClickRecognizerRef recognizer, void *context);
static void select_click_handler(ClickRecognizerRef recognizer, void *context);

static Window *window;
static Layer *window_layer;
static TextLayer *text_layer;
static GBitmap *bitmap;
static BitmapLayer *bitmap_layer;

static void init()
{
    init_mood();
    
    window = window_create();
    window_set_click_config_provider(window, click_config_provider);
    window_set_window_handlers(window, (WindowHandlers) {
        .load = window_load,
        .unload = window_unload,
    });
    window_stack_push(window, true);
    
    init_app_message(text_layer);
    
    APP_LOG(
        APP_LOG_LEVEL_DEBUG,
        "Done initializing, pushed window: %p",
        window
    );
}

static void deinit()
{
    deinit_mood();
    
    deinit_app_message();
    
    window_destroy(window);
    
    APP_LOG(
        APP_LOG_LEVEL_DEBUG,
        "Done deinitializing"
    );
}

static void window_load(Window *window)
{
    window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(window_layer);
    
    #if defined(PBL_ROUND)
    
    // image is 120 by 120 and in centre, text is curved around image
    unsigned int indent_h = (bounds.size.h - 120) / 2;
    unsigned int indent_w = (bounds.size.w - 120) / 2;
    bitmap_layer = bitmap_layer_create(GRect(indent_w, indent_h, 120, 120));
    // TODO different layout for round: face in centre, curved text "How are you feeling?" around face
    text_layer = text_layer_create(GRect(0, 0, 0, 0));
    
    #else
    
    // image is 120 by 120, text is under image and 20 high
    unsigned int indent_h = (bounds.size.h - 140) / 2;
    unsigned int indent_w = (bounds.size.w - 120) / 2;
    bitmap_layer = bitmap_layer_create(GRect(indent_w, indent_h, 120, 120));
    text_layer = text_layer_create(GRect(indent_w, indent_h + 120, 120, 20));
    
    #endif
    
    // bitmap layer settings
    bitmap_layer_set_compositing_mode(bitmap_layer, GCompOpSet);
    layer_add_child(window_layer, bitmap_layer_get_layer(bitmap_layer));
    
    // text layer settings
    text_layer_set_text(text_layer, "How do you feel?");
    text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
    layer_add_child(window_layer, text_layer_get_layer(text_layer));
    
    update_image(get_mood_image_id());
}

static void update_image(uint32_t resource_id)
{
    gbitmap_destroy(bitmap);
    bitmap = gbitmap_create_with_resource(resource_id);
    bitmap_layer_set_bitmap(bitmap_layer, bitmap);
    layer_add_child(window_layer, bitmap_layer_get_layer(bitmap_layer));    
}

static void window_unload(Window *window)
{
    text_layer_destroy(text_layer);
    gbitmap_destroy(bitmap);
    bitmap_layer_destroy(bitmap_layer);
}

static void click_config_provider(void *context)
{
    window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
    
    window_single_repeating_click_subscribe(
        BUTTON_ID_UP,
        INTERVAL_MS_REPEAT,
        up_click_handler
    );
    
    window_single_repeating_click_subscribe(
        BUTTON_ID_DOWN,
        INTERVAL_MS_REPEAT,
        down_click_handler
    );
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context)
{
    increment_mood();
    update_image(get_mood_image_id());
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context)
{
    decrement_mood();
    update_image(get_mood_image_id());
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context)
{
    send_app_message(write_mood_dict);
}

int main()
{
    init();
    app_event_loop();
    deinit();
}
