#include <pebble.h>
#include <inttypes.h>
#include "main.h"

#define NUM_KEY_MOOD 1
#define NUM_DEFAULT_MOOD 0

static Window *window;
Layer *window_layer;

static TextLayer *text_layer;
static GBitmap *bitmap;
static BitmapLayer *bitmap_layer;

static const uint32_t IMAGE_IDS[] = {
    RESOURCE_ID_IMAGE_EMOTION_01,
    RESOURCE_ID_IMAGE_EMOTION_02,
    RESOURCE_ID_IMAGE_EMOTION_03,
    RESOURCE_ID_IMAGE_EMOTION_04,
    RESOURCE_ID_IMAGE_EMOTION_05,
    RESOURCE_ID_IMAGE_EMOTION_06,
    RESOURCE_ID_IMAGE_EMOTION_07,
    RESOURCE_ID_IMAGE_EMOTION_08,
    RESOURCE_ID_IMAGE_EMOTION_09,
    RESOURCE_ID_IMAGE_EMOTION_10
};

static const int IMAGE_MAX = 9;

static int mood = NUM_DEFAULT_MOOD;

static void set_mood(int delta)
{
    // modulo operation on mood value
    int shifted = mood + delta;
    if (shifted < 0)
    {
        mood = IMAGE_MAX;
    }
    else if (shifted > IMAGE_MAX)
    {
        mood = 0;
    }
    else
    {
        mood = shifted;
    }
    
    // deinit old image
    gbitmap_destroy(bitmap);
    bitmap_layer_destroy(bitmap_layer);
    
    // init new image
    bitmap = gbitmap_create_with_resource(IMAGE_IDS[mood]);
    bitmap_layer = bitmap_layer_create(GRect(12, 0, 120, 120));
    bitmap_layer_set_compositing_mode(bitmap_layer, GCompOpAssign);
    bitmap_layer_set_bitmap(bitmap_layer, bitmap);
    layer_add_child(window_layer, bitmap_layer_get_layer(bitmap_layer));
}

int main(void)
{
    init();
    
    APP_LOG(
        APP_LOG_LEVEL_DEBUG,
        "Done initializing, pushed window: %p",
        window
    );
    
    app_event_loop();
    deinit();
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context)
{
    unsigned int epoch = time(NULL);
    APP_LOG(
        APP_LOG_LEVEL_DEBUG,
        "mood id: %d, epoch: %u",
        mood, epoch
    );
    // TODO send data to companion app
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context)
{
    set_mood(1);
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context)
{
    set_mood(-1);
}

static void click_config_provider(void *context)
{
    window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
    window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
    window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
}

static void window_load(Window *window)
{
    window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(window_layer);
    
    text_layer = text_layer_create(GRect(0, 120, bounds.size.w, 20));
    text_layer_set_text(text_layer, "How are you feeling?");
    text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
    layer_add_child(window_layer, text_layer_get_layer(text_layer));
    
    set_mood(0);
}

static void window_unload(Window *window)
{
    text_layer_destroy(text_layer);
    gbitmap_destroy(bitmap);
    bitmap_layer_destroy(bitmap_layer);
}

static void init(void)
{
    mood = persist_exists(NUM_KEY_MOOD)
        ? persist_read_int(NUM_KEY_MOOD)
        : NUM_DEFAULT_MOOD;
    
    window = window_create();
    window_set_click_config_provider(window, click_config_provider);
    window_set_window_handlers(window, (WindowHandlers) {
        .load = window_load,
        .unload = window_unload,
    });
    const bool animated = true;
    window_stack_push(window, animated);
}

static void deinit(void)
{
    persist_write_int(NUM_KEY_MOOD, mood);
    window_destroy(window);
}