#pragma once

void init_app_message(TextLayer *output_text_layer);
void deinit_app_message();
bool send_app_message(void (*write_dict)(DictionaryIterator*));
