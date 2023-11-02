#ifndef BONGO_CAT_INPUT_HPP
#define BONGO_CAT_INPUT_HPP
#include <global.hpp>

#define TOTAl_INPUT_TABLE_SIZE 256
extern int INPUT_KEY_TABLE[TOTAl_INPUT_TABLE_SIZE];

sf::Keyboard::Key ascii_to_key(int key_code);
bool is_pressed_fallback(int key_code);

namespace BongoInput {
//============================================================================
// INIT
//============================================================================
void init();
//============================================================================
// IS PRESSED
//============================================================================
bool is_pressed(int key_code);
bool is_pressed(char c);
//============================================================================
//  BIND TO LUA
//============================================================================
void bindToLua();
} // namespace BongoInput
#endif
