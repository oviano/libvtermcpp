// test_25_state_input.cpp — state input (keyboard/paste/focus) tests
// Ported from upstream libvterm t/25state_input.test

#include "harness.h"

// Unmodified ASCII
TEST(state_input_unmodified_ascii)
{
    Terminal vt(25, 80);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    output_init(vt);

    // INCHAR 0 41 -> output "A"
    output_clear();
    vt.keyboard_unichar(0x41, Modifier::None);
    ASSERT_OUTPUT_BYTES("A", 1);

    // INCHAR 0 61 -> output "a"
    output_clear();
    vt.keyboard_unichar(0x61, Modifier::None);
    ASSERT_OUTPUT_BYTES("a", 1);
}

// Ctrl modifier on ASCII letters
TEST(state_input_ctrl_ascii)
{
    Terminal vt(25, 80);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    output_init(vt);

    // INCHAR C 41 -> output "\e[65;5u"
    output_clear();
    vt.keyboard_unichar(0x41, Modifier::Ctrl);
    ASSERT_OUTPUT_BYTES("\e[65;5u", 7);

    // INCHAR C 61 -> output "\x01"
    output_clear();
    vt.keyboard_unichar(0x61, Modifier::Ctrl);
    ASSERT_OUTPUT_BYTES("\x01", 1);
}

// Alt modifier on ASCII letters
TEST(state_input_alt_ascii)
{
    Terminal vt(25, 80);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    output_init(vt);

    // INCHAR A 41 -> output "\eA"
    output_clear();
    vt.keyboard_unichar(0x41, Modifier::Alt);
    ASSERT_OUTPUT_BYTES("\eA", 2);

    // INCHAR A 61 -> output "\ea"
    output_clear();
    vt.keyboard_unichar(0x61, Modifier::Alt);
    ASSERT_OUTPUT_BYTES("\ea", 2);
}

// Ctrl-Alt modifier on ASCII letters
TEST(state_input_ctrl_alt_ascii)
{
    Terminal vt(25, 80);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    output_init(vt);

    // INCHAR CA 41 -> output "\e[65;7u"
    output_clear();
    vt.keyboard_unichar(0x41, Modifier::Ctrl | Modifier::Alt);
    ASSERT_OUTPUT_BYTES("\e[65;7u", 7);

    // INCHAR CA 61 -> output "\e\x01"
    output_clear();
    vt.keyboard_unichar(0x61, Modifier::Ctrl | Modifier::Alt);
    ASSERT_OUTPUT_BYTES("\e" "\x01", 2);
}

// Special handling of Ctrl-I
TEST(state_input_ctrl_i)
{
    Terminal vt(25, 80);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    output_init(vt);

    // INCHAR 0 49 -> output "I"
    output_clear();
    vt.keyboard_unichar(0x49, Modifier::None);
    ASSERT_OUTPUT_BYTES("I", 1);

    // INCHAR 0 69 -> output "i"
    output_clear();
    vt.keyboard_unichar(0x69, Modifier::None);
    ASSERT_OUTPUT_BYTES("i", 1);

    // INCHAR C 49 -> output "\e[73;5u"
    output_clear();
    vt.keyboard_unichar(0x49, Modifier::Ctrl);
    ASSERT_OUTPUT_BYTES("\e[73;5u", 7);

    // INCHAR C 69 -> output "\e[105;5u"
    output_clear();
    vt.keyboard_unichar(0x69, Modifier::Ctrl);
    ASSERT_OUTPUT_BYTES("\e[105;5u", 8);

    // INCHAR A 49 -> output "\eI"
    output_clear();
    vt.keyboard_unichar(0x49, Modifier::Alt);
    ASSERT_OUTPUT_BYTES("\eI", 2);

    // INCHAR A 69 -> output "\ei"
    output_clear();
    vt.keyboard_unichar(0x69, Modifier::Alt);
    ASSERT_OUTPUT_BYTES("\ei", 2);

    // INCHAR CA 49 -> output "\e[73;7u"
    output_clear();
    vt.keyboard_unichar(0x49, Modifier::Ctrl | Modifier::Alt);
    ASSERT_OUTPUT_BYTES("\e[73;7u", 7);

    // INCHAR CA 69 -> output "\e[105;7u"
    output_clear();
    vt.keyboard_unichar(0x69, Modifier::Ctrl | Modifier::Alt);
    ASSERT_OUTPUT_BYTES("\e[105;7u", 8);
}

// Special handling of Space
TEST(state_input_space)
{
    Terminal vt(25, 80);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    output_init(vt);

    // INCHAR 0 20 -> output " "
    output_clear();
    vt.keyboard_unichar(0x20, Modifier::None);
    ASSERT_OUTPUT_BYTES(" ", 1);

    // INCHAR S 20 -> output "\e[32;2u"
    output_clear();
    vt.keyboard_unichar(0x20, Modifier::Shift);
    ASSERT_OUTPUT_BYTES("\e[32;2u", 7);

    // INCHAR C 20 -> output "\0"
    output_clear();
    vt.keyboard_unichar(0x20, Modifier::Ctrl);
    ASSERT_OUTPUT_BYTES("\0", 1);

    // INCHAR SC 20 -> output "\e[32;6u"
    output_clear();
    vt.keyboard_unichar(0x20, Modifier::Shift | Modifier::Ctrl);
    ASSERT_OUTPUT_BYTES("\e[32;6u", 7);

    // INCHAR A 20 -> output "\e "
    output_clear();
    vt.keyboard_unichar(0x20, Modifier::Alt);
    ASSERT_OUTPUT_BYTES("\e ", 2);

    // INCHAR SA 20 -> output "\e[32;4u"
    output_clear();
    vt.keyboard_unichar(0x20, Modifier::Shift | Modifier::Alt);
    ASSERT_OUTPUT_BYTES("\e[32;4u", 7);

    // INCHAR CA 20 -> output "\e\0"
    output_clear();
    vt.keyboard_unichar(0x20, Modifier::Ctrl | Modifier::Alt);
    ASSERT_OUTPUT_BYTES("\e" "\0", 2);

    // INCHAR SCA 20 -> output "\e[32;8u"
    output_clear();
    vt.keyboard_unichar(0x20, Modifier::Shift | Modifier::Ctrl | Modifier::Alt);
    ASSERT_OUTPUT_BYTES("\e[32;8u", 7);
}

// Cursor keys in reset (cursor) mode
TEST(state_input_cursor_keys_reset)
{
    Terminal vt(25, 80);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    output_init(vt);

    // INKEY 0 Up -> output "\e[A"
    output_clear();
    vt.keyboard_key(Key::Up, Modifier::None);
    ASSERT_OUTPUT_BYTES("\e[A", 3);

    // INKEY S Up -> output "\e[1;2A"
    output_clear();
    vt.keyboard_key(Key::Up, Modifier::Shift);
    ASSERT_OUTPUT_BYTES("\e[1;2A", 6);

    // INKEY C Up -> output "\e[1;5A"
    output_clear();
    vt.keyboard_key(Key::Up, Modifier::Ctrl);
    ASSERT_OUTPUT_BYTES("\e[1;5A", 6);

    // INKEY SC Up -> output "\e[1;6A"
    output_clear();
    vt.keyboard_key(Key::Up, Modifier::Shift | Modifier::Ctrl);
    ASSERT_OUTPUT_BYTES("\e[1;6A", 6);

    // INKEY A Up -> output "\e[1;3A"
    output_clear();
    vt.keyboard_key(Key::Up, Modifier::Alt);
    ASSERT_OUTPUT_BYTES("\e[1;3A", 6);

    // INKEY SA Up -> output "\e[1;4A"
    output_clear();
    vt.keyboard_key(Key::Up, Modifier::Shift | Modifier::Alt);
    ASSERT_OUTPUT_BYTES("\e[1;4A", 6);

    // INKEY CA Up -> output "\e[1;7A"
    output_clear();
    vt.keyboard_key(Key::Up, Modifier::Ctrl | Modifier::Alt);
    ASSERT_OUTPUT_BYTES("\e[1;7A", 6);

    // INKEY SCA Up -> output "\e[1;8A"
    output_clear();
    vt.keyboard_key(Key::Up, Modifier::Shift | Modifier::Ctrl | Modifier::Alt);
    ASSERT_OUTPUT_BYTES("\e[1;8A", 6);
}

// Cursor keys in application mode
TEST(state_input_cursor_keys_app)
{
    Terminal vt(25, 80);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    output_init(vt);

    // PUSH "\e[?1h" - enable application cursor mode
    push(vt, "\e[?1h");

    // Plain "Up" should be SS3 A now
    // INKEY 0 Up -> output "\eOA"
    output_clear();
    vt.keyboard_key(Key::Up, Modifier::None);
    ASSERT_OUTPUT_BYTES("\eOA", 3);

    // Modified keys should still use CSI
    // INKEY S Up -> output "\e[1;2A"
    output_clear();
    vt.keyboard_key(Key::Up, Modifier::Shift);
    ASSERT_OUTPUT_BYTES("\e[1;2A", 6);

    // INKEY C Up -> output "\e[1;5A"
    output_clear();
    vt.keyboard_key(Key::Up, Modifier::Ctrl);
    ASSERT_OUTPUT_BYTES("\e[1;5A", 6);
}

// Shift-Tab should be different
TEST(state_input_tab)
{
    Terminal vt(25, 80);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    output_init(vt);

    // INKEY 0 Tab -> output "\x09"
    output_clear();
    vt.keyboard_key(Key::Tab, Modifier::None);
    ASSERT_OUTPUT_BYTES("\x09", 1);

    // INKEY S Tab -> output "\e[Z"
    output_clear();
    vt.keyboard_key(Key::Tab, Modifier::Shift);
    ASSERT_OUTPUT_BYTES("\e[Z", 3);

    // INKEY C Tab -> output "\e[9;5u"
    output_clear();
    vt.keyboard_key(Key::Tab, Modifier::Ctrl);
    ASSERT_OUTPUT_BYTES("\e[9;5u", 6);

    // INKEY A Tab -> output "\e\x09"
    output_clear();
    vt.keyboard_key(Key::Tab, Modifier::Alt);
    ASSERT_OUTPUT_BYTES("\e" "\x09", 2);

    // INKEY CA Tab -> output "\e[9;7u"
    output_clear();
    vt.keyboard_key(Key::Tab, Modifier::Ctrl | Modifier::Alt);
    ASSERT_OUTPUT_BYTES("\e[9;7u", 6);
}

// Enter in linefeed mode
TEST(state_input_enter_lf)
{
    Terminal vt(25, 80);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    output_init(vt);

    // INKEY 0 Enter -> output "\x0d"
    output_clear();
    vt.keyboard_key(Key::Enter, Modifier::None);
    ASSERT_OUTPUT_BYTES("\x0d", 1);
}

// Enter in newline mode
TEST(state_input_enter_nl)
{
    Terminal vt(25, 80);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    output_init(vt);

    // PUSH "\e[20h" - enable newline mode
    push(vt, "\e[20h");

    // INKEY 0 Enter -> output "\x0d\x0a"
    output_clear();
    vt.keyboard_key(Key::Enter, Modifier::None);
    ASSERT_OUTPUT_BYTES("\x0d" "\x0a", 2);
}

// Unmodified F1 is SS3 P
TEST(state_input_f1_unmodified)
{
    Terminal vt(25, 80);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    output_init(vt);

    // INKEY 0 F1 -> output "\eOP"
    output_clear();
    vt.keyboard_key(static_cast<Key>(static_cast<int32_t>(Key::Function0) + 1), Modifier::None);
    ASSERT_OUTPUT_BYTES("\eOP", 3);
}

// Modified F1 is CSI P
TEST(state_input_f1_modified)
{
    Terminal vt(25, 80);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    output_init(vt);

    // INKEY S F1 -> output "\e[1;2P"
    output_clear();
    vt.keyboard_key(static_cast<Key>(static_cast<int32_t>(Key::Function0) + 1), Modifier::Shift);
    ASSERT_OUTPUT_BYTES("\e[1;2P", 6);

    // INKEY A F1 -> output "\e[1;3P"
    output_clear();
    vt.keyboard_key(static_cast<Key>(static_cast<int32_t>(Key::Function0) + 1), Modifier::Alt);
    ASSERT_OUTPUT_BYTES("\e[1;3P", 6);

    // INKEY C F1 -> output "\e[1;5P"
    output_clear();
    vt.keyboard_key(static_cast<Key>(static_cast<int32_t>(Key::Function0) + 1), Modifier::Ctrl);
    ASSERT_OUTPUT_BYTES("\e[1;5P", 6);
}

// Keypad in DECKPNM
TEST(state_input_keypad_numeric)
{
    Terminal vt(25, 80);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    output_init(vt);

    // INKEY 0 KP0 -> output "0"
    output_clear();
    vt.keyboard_key(Key::KP0, Modifier::None);
    ASSERT_OUTPUT_BYTES("0", 1);
}

// Keypad in DECKPAM
TEST(state_input_keypad_app)
{
    Terminal vt(25, 80);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    output_init(vt);

    // PUSH "\e=" - enable application keypad mode
    push(vt, "\e=");

    // INKEY 0 KP0 -> output "\eOp"
    output_clear();
    vt.keyboard_key(Key::KP0, Modifier::None);
    ASSERT_OUTPUT_BYTES("\eOp", 3);
}

// Bracketed paste mode off
TEST(state_input_paste_off)
{
    Terminal vt(25, 80);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    output_init(vt);

    // PASTE START - no output expected
    output_clear();
    vt.keyboard_start_paste();
    ASSERT_EQ(g_output_len, 0);

    // PASTE END - no output expected
    output_clear();
    vt.keyboard_end_paste();
    ASSERT_EQ(g_output_len, 0);
}

// Bracketed paste mode on
TEST(state_input_paste_on)
{
    Terminal vt(25, 80);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    output_init(vt);

    // PUSH "\e[?2004h" - enable bracketed paste mode
    push(vt, "\e[?2004h");

    // PASTE START -> output "\e[200~"
    output_clear();
    vt.keyboard_start_paste();
    ASSERT_OUTPUT_BYTES("\e[200~", 6);

    // PASTE END -> output "\e[201~"
    output_clear();
    vt.keyboard_end_paste();
    ASSERT_OUTPUT_BYTES("\e[201~", 6);
}

// Focus reporting disabled
TEST(state_input_focus_disabled)
{
    Terminal vt(25, 80);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    output_init(vt);

    // FOCUS IN - no output expected
    output_clear();
    state.focus_in();
    ASSERT_EQ(g_output_len, 0);

    // FOCUS OUT - no output expected
    output_clear();
    state.focus_out();
    ASSERT_EQ(g_output_len, 0);
}

// Focus reporting enabled
TEST(state_input_focus_enabled)
{
    Terminal vt(25, 80);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    output_init(vt);

    // PUSH "\e[?1004h" - enable focus reporting
    callbacks_clear();
    push(vt, "\e[?1004h");
    // settermprop 9 true -> Prop::FocusReport = true
    ASSERT_EQ(g_cb.settermprop_count, 1);
    ASSERT_EQ(g_cb.settermprop[0].prop, Prop::FocusReport);
    ASSERT_EQ(g_cb.settermprop[0].val.boolean, true);

    // FOCUS IN -> output "\e[I"
    output_clear();
    state.focus_in();
    ASSERT_OUTPUT_BYTES("\e[I", 3);

    // FOCUS OUT -> output "\e[O"
    output_clear();
    state.focus_out();
    ASSERT_OUTPUT_BYTES("\e[O", 3);
}
