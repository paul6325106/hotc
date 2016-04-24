#include <pebble.h>
#include <inttypes.h>
#include "constants.h"
#include "app_message.h"

static void inbox_dropped_handler(AppMessageResult reason, void *context);
static void inbox_received_handler(DictionaryIterator *iter, void *context);
static void outbox_sent_handler(DictionaryIterator *iter, void *context);
static void outbox_failed_handler(DictionaryIterator *iter, AppMessageResult reason, void *context);
static void recreate_message_timer();
static void clear_message_timer();
static void recreate_timeout_timer();
static void clear_timeout_timer();
static void message_timer_callback(void *message);
static void timeout_timer_callback(void *data);

static AppTimer *timeout_timer;
static AppTimer *message_timer;
static bool message_locked = false;
static TextLayer *text_layer;

void init_app_message(TextLayer *output_text_layer)
{
    text_layer = output_text_layer;
    
    app_message_register_inbox_received(inbox_received_handler);
	app_message_register_inbox_dropped(inbox_dropped_handler); 
    app_message_register_outbox_sent(outbox_sent_handler);
    app_message_register_outbox_failed(outbox_failed_handler);
    app_message_open(40, 40);
}

void deinit_app_message()
{
    app_message_deregister_callbacks();
}

bool send_app_message(void (*write_dict)(DictionaryIterator*))
{
    if (!message_locked)
    {
        message_locked = true;
        
        APP_LOG(APP_LOG_LEVEL_DEBUG, "Sending app message");
        text_layer_set_text(text_layer, "Sending mood...");
        
        DictionaryIterator *iter;
        app_message_outbox_begin(&iter);
        write_dict(iter);
        app_message_outbox_send();
        
        recreate_timeout_timer();
        
        return true;
    }
    else
    {
        APP_LOG(APP_LOG_LEVEL_DEBUG, "Message locked, did not send new message");
        return false;
    }
}

static void inbox_dropped_handler(AppMessageResult reason, void *context)
{
    APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped. Reason: %d", (int) reason);
    text_layer_set_text(text_layer, "Message dropped");
}

static void inbox_received_handler(DictionaryIterator *iter, void *context)
{
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Message received");
    
    Tuple *result_tuple = dict_find(iter, KEY_RESULT);
    if (result_tuple)
    {
        text_layer_set_text(text_layer, "Message received");
    }
}

static void outbox_sent_handler(DictionaryIterator *iter, void *context)
{
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Message sent");
    clear_timeout_timer();
    text_layer_set_text(text_layer, "Mood sent");
    recreate_message_timer();
    message_locked = false;
}

static void outbox_failed_handler(DictionaryIterator *iter, AppMessageResult reason, void *context)
{
    APP_LOG(APP_LOG_LEVEL_ERROR, "Message failed, reason: %d", (int) reason);
    clear_timeout_timer();
    text_layer_set_text(text_layer, "Mood send failed");
    recreate_message_timer();
    message_locked = false;
}

static void recreate_message_timer()
{
    if (message_timer != NULL) app_timer_cancel(message_timer);
    message_timer = app_timer_register(INTERVAL_MS_DELAY, message_timer_callback, "How do you feel?");
}

static void clear_message_timer()
{
    if (message_timer != NULL) app_timer_cancel(message_timer);
    message_timer = NULL;
}

static void recreate_timeout_timer()
{
    if (timeout_timer != NULL) app_timer_cancel(timeout_timer);
    timeout_timer = app_timer_register(INTERVAL_MS_TIMEOUT, timeout_timer_callback, "Timed out");
}

static void clear_timeout_timer()
{
    if (timeout_timer != NULL) app_timer_cancel(timeout_timer);
    timeout_timer = NULL;
}

static void message_timer_callback(void *message)
{
    text_layer_set_text(text_layer, message);
    message_timer = NULL;
}

static void timeout_timer_callback(void *data)
{
    APP_LOG(APP_LOG_LEVEL_ERROR, "Timed out when attempting to send");
    timeout_timer = NULL;
    text_layer_set_text(text_layer, "Timed out");
    recreate_message_timer();
    message_locked = false;
}
