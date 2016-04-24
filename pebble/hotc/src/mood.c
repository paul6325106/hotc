#include <pebble.h>
#include <inttypes.h>
#include "constants.h"
#include "mood.h"

#define MOOD_NEXT 1
#define MOOD_PREVIOUS -1

#define NUM_DEFAULT_MOOD 0

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

static unsigned int current_mood;

void init_mood()
{
    current_mood = persist_exists(KEY_MOOD)
        ? persist_read_int(KEY_MOOD)
        : NUM_DEFAULT_MOOD;
}

void deinit_mood()
{
    persist_write_int(KEY_MOOD, current_mood);
}

uint32_t get_mood_image_id()
{
    return IMAGE_IDS[current_mood];
}

static unsigned int shift_mood(unsigned int mood, int delta)
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
    
    return (unsigned int) mood;
}

void increment_mood()
{
    current_mood = shift_mood(current_mood, MOOD_NEXT);
}

void decrement_mood()
{
    current_mood = shift_mood(current_mood, MOOD_PREVIOUS);
}

void write_mood_dict(DictionaryIterator *iter)
{
    unsigned int epoch = time(NULL);
    
    dict_write_int(iter, KEY_MOOD, &current_mood, sizeof(current_mood), true);
    dict_write_int(iter, KEY_EPOCH, &epoch, sizeof(epoch), true);
    
    APP_LOG(
        APP_LOG_LEVEL_INFO,
        "epoch: %u, mood id: %d",
        epoch, current_mood
    );
}
