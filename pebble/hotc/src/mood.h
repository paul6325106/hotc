#pragma once

void init_mood();
void deinit_mood();
void increment_mood();
void decrement_mood();
uint32_t get_mood_image_id();
void write_mood_dict(DictionaryIterator *iter);