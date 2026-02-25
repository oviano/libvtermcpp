// test_13_state_edit.cpp -- state layer editing tests
// Ported from upstream libvterm t/13state_edit.test

#include "harness.h"


// ===== ICH =====
TEST(state_edit_ich)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);

    callbacks_clear();
    ASSERT_CURSOR(state, 0, 0);

    push(vt, "ACD");
    push(vt, "\e[2D");
    ASSERT_CURSOR(state, 0, 1);

    callbacks_clear();
    push(vt, "\e[@");
    ASSERT_EQ(g_cb.scrollrect_count, 1);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_row, 0);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_row, 1);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_col, 1);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_col, 80);
    ASSERT_EQ(g_cb.scrollrect[0].downward, 0);
    ASSERT_EQ(g_cb.scrollrect[0].rightward, -1);
    ASSERT_CURSOR(state, 0, 1);

    push(vt, "B");
    ASSERT_CURSOR(state, 0, 2);

    callbacks_clear();
    push(vt, "\e[3@");
    ASSERT_EQ(g_cb.scrollrect_count, 1);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_row, 0);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_row, 1);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_col, 2);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_col, 80);
    ASSERT_EQ(g_cb.scrollrect[0].downward, 0);
    ASSERT_EQ(g_cb.scrollrect[0].rightward, -3);
}

// ===== ICH with DECSLRM =====
TEST(state_edit_ich_decslrm)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);

    // Set up left/right margins
    push(vt, "\e[?69h");
    push(vt, "\e[;50s");

    callbacks_clear();
    push(vt, "\e[20G\e[@");
    ASSERT_EQ(g_cb.scrollrect_count, 1);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_row, 0);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_row, 1);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_col, 19);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_col, 50);
    ASSERT_EQ(g_cb.scrollrect[0].downward, 0);
    ASSERT_EQ(g_cb.scrollrect[0].rightward, -1);
}

// ===== ICH outside DECSLRM =====
TEST(state_edit_ich_outside_decslrm)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);

    push(vt, "\e[?69h");
    push(vt, "\e[;50s");

    callbacks_clear();
    push(vt, "\e[70G\e[@");
    // nothing happens
    ASSERT_EQ(g_cb.scrollrect_count, 0);
}

// ===== DCH =====
TEST(state_edit_dch)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);

    callbacks_clear();
    ASSERT_CURSOR(state, 0, 0);

    push(vt, "ABBC");
    push(vt, "\e[3D");
    ASSERT_CURSOR(state, 0, 1);

    callbacks_clear();
    push(vt, "\e[P");
    ASSERT_EQ(g_cb.scrollrect_count, 1);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_row, 0);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_row, 1);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_col, 1);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_col, 80);
    ASSERT_EQ(g_cb.scrollrect[0].downward, 0);
    ASSERT_EQ(g_cb.scrollrect[0].rightward, 1);
    ASSERT_CURSOR(state, 0, 1);

    callbacks_clear();
    push(vt, "\e[3P");
    ASSERT_EQ(g_cb.scrollrect_count, 1);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_row, 0);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_row, 1);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_col, 1);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_col, 80);
    ASSERT_EQ(g_cb.scrollrect[0].downward, 0);
    ASSERT_EQ(g_cb.scrollrect[0].rightward, 3);
    ASSERT_CURSOR(state, 0, 1);
}

// ===== DCH with DECSLRM =====
TEST(state_edit_dch_decslrm)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);

    push(vt, "\e[?69h");
    push(vt, "\e[;50s");

    callbacks_clear();
    push(vt, "\e[20G\e[P");
    ASSERT_EQ(g_cb.scrollrect_count, 1);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_row, 0);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_row, 1);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_col, 19);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_col, 50);
    ASSERT_EQ(g_cb.scrollrect[0].downward, 0);
    ASSERT_EQ(g_cb.scrollrect[0].rightward, 1);
}

// ===== DCH outside DECSLRM =====
TEST(state_edit_dch_outside_decslrm)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);

    push(vt, "\e[?69h");
    push(vt, "\e[;50s");

    callbacks_clear();
    push(vt, "\e[70G\e[P");
    // nothing happens
    ASSERT_EQ(g_cb.scrollrect_count, 0);
}

// ===== ECH =====
TEST(state_edit_ech)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);

    callbacks_clear();
    ASSERT_CURSOR(state, 0, 0);

    push(vt, "ABC");
    push(vt, "\e[2D");
    ASSERT_CURSOR(state, 0, 1);

    callbacks_clear();
    push(vt, "\e[X");
    ASSERT_EQ(g_cb.erase_count, 1);
    ASSERT_EQ(g_cb.erase[0].rect.start_row, 0);
    ASSERT_EQ(g_cb.erase[0].rect.end_row, 1);
    ASSERT_EQ(g_cb.erase[0].rect.start_col, 1);
    ASSERT_EQ(g_cb.erase[0].rect.end_col, 2);
    ASSERT_CURSOR(state, 0, 1);

    callbacks_clear();
    push(vt, "\e[3X");
    ASSERT_EQ(g_cb.erase_count, 1);
    ASSERT_EQ(g_cb.erase[0].rect.start_row, 0);
    ASSERT_EQ(g_cb.erase[0].rect.end_row, 1);
    ASSERT_EQ(g_cb.erase[0].rect.start_col, 1);
    ASSERT_EQ(g_cb.erase[0].rect.end_col, 4);
    ASSERT_CURSOR(state, 0, 1);

    // ECH more columns than there are should be bounded
    callbacks_clear();
    push(vt, "\e[100X");
    ASSERT_EQ(g_cb.erase_count, 1);
    ASSERT_EQ(g_cb.erase[0].rect.start_row, 0);
    ASSERT_EQ(g_cb.erase[0].rect.end_row, 1);
    ASSERT_EQ(g_cb.erase[0].rect.start_col, 1);
    ASSERT_EQ(g_cb.erase[0].rect.end_col, 80);
}

// ===== IL =====
TEST(state_edit_il)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);

    callbacks_clear();
    ASSERT_CURSOR(state, 0, 0);

    push(vt, "A\r\nC");
    ASSERT_CURSOR(state, 1, 1);

    callbacks_clear();
    push(vt, "\e[L");
    ASSERT_EQ(g_cb.scrollrect_count, 1);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_row, 1);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_row, 25);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_col, 0);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_col, 80);
    ASSERT_EQ(g_cb.scrollrect[0].downward, -1);
    ASSERT_EQ(g_cb.scrollrect[0].rightward, 0);
    ASSERT_CURSOR(state, 1, 1);

    push(vt, "\rB");
    ASSERT_CURSOR(state, 1, 1);

    callbacks_clear();
    push(vt, "\e[3L");
    ASSERT_EQ(g_cb.scrollrect_count, 1);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_row, 1);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_row, 25);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_col, 0);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_col, 80);
    ASSERT_EQ(g_cb.scrollrect[0].downward, -3);
    ASSERT_EQ(g_cb.scrollrect[0].rightward, 0);
}

// ===== IL with DECSTBM =====
TEST(state_edit_il_decstbm)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);

    push(vt, "\e[5;15r");

    callbacks_clear();
    push(vt, "\e[5H\e[L");
    ASSERT_EQ(g_cb.scrollrect_count, 1);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_row, 4);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_row, 15);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_col, 0);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_col, 80);
    ASSERT_EQ(g_cb.scrollrect[0].downward, -1);
    ASSERT_EQ(g_cb.scrollrect[0].rightward, 0);
}

// ===== IL outside DECSTBM =====
TEST(state_edit_il_outside_decstbm)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);

    push(vt, "\e[5;15r");

    callbacks_clear();
    push(vt, "\e[20H\e[L");
    // nothing happens
    ASSERT_EQ(g_cb.scrollrect_count, 0);
}

// ===== IL with DECSTBM+DECSLRM =====
TEST(state_edit_il_decstbm_decslrm)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);

    push(vt, "\e[?69h");
    push(vt, "\e[10;50s");
    push(vt, "\e[5;15r");

    callbacks_clear();
    push(vt, "\e[5;10H\e[L");
    ASSERT_EQ(g_cb.scrollrect_count, 1);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_row, 4);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_row, 15);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_col, 9);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_col, 50);
    ASSERT_EQ(g_cb.scrollrect[0].downward, -1);
    ASSERT_EQ(g_cb.scrollrect[0].rightward, 0);
}

// ===== DL =====
TEST(state_edit_dl)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);

    callbacks_clear();
    ASSERT_CURSOR(state, 0, 0);

    push(vt, "A\r\nB\r\nB\r\nC");
    ASSERT_CURSOR(state, 3, 1);

    push(vt, "\e[2H");
    ASSERT_CURSOR(state, 1, 0);

    callbacks_clear();
    push(vt, "\e[M");
    ASSERT_EQ(g_cb.scrollrect_count, 1);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_row, 1);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_row, 25);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_col, 0);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_col, 80);
    ASSERT_EQ(g_cb.scrollrect[0].downward, 1);
    ASSERT_EQ(g_cb.scrollrect[0].rightward, 0);
    ASSERT_CURSOR(state, 1, 0);

    callbacks_clear();
    push(vt, "\e[3M");
    ASSERT_EQ(g_cb.scrollrect_count, 1);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_row, 1);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_row, 25);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_col, 0);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_col, 80);
    ASSERT_EQ(g_cb.scrollrect[0].downward, 3);
    ASSERT_EQ(g_cb.scrollrect[0].rightward, 0);
    ASSERT_CURSOR(state, 1, 0);
}

// ===== DL with DECSTBM =====
TEST(state_edit_dl_decstbm)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);

    push(vt, "\e[5;15r");

    callbacks_clear();
    push(vt, "\e[5H\e[M");
    ASSERT_EQ(g_cb.scrollrect_count, 1);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_row, 4);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_row, 15);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_col, 0);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_col, 80);
    ASSERT_EQ(g_cb.scrollrect[0].downward, 1);
    ASSERT_EQ(g_cb.scrollrect[0].rightward, 0);
}

// ===== DL outside DECSTBM =====
TEST(state_edit_dl_outside_decstbm)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);

    push(vt, "\e[5;15r");

    callbacks_clear();
    push(vt, "\e[20H\e[M");
    // nothing happens
    ASSERT_EQ(g_cb.scrollrect_count, 0);
}

// ===== DL with DECSTBM+DECSLRM =====
TEST(state_edit_dl_decstbm_decslrm)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);

    push(vt, "\e[?69h");
    push(vt, "\e[10;50s");
    push(vt, "\e[5;15r");

    callbacks_clear();
    push(vt, "\e[5;10H\e[M");
    ASSERT_EQ(g_cb.scrollrect_count, 1);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_row, 4);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_row, 15);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_col, 9);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_col, 50);
    ASSERT_EQ(g_cb.scrollrect[0].downward, 1);
    ASSERT_EQ(g_cb.scrollrect[0].rightward, 0);
}

// ===== DECIC =====
TEST(state_edit_decic)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);

    callbacks_clear();

    push(vt, "\e[20G\e[5'}");
    ASSERT_EQ(g_cb.scrollrect_count, 1);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_row, 0);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_row, 25);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_col, 19);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_col, 80);
    ASSERT_EQ(g_cb.scrollrect[0].downward, 0);
    ASSERT_EQ(g_cb.scrollrect[0].rightward, -5);
}

// ===== DECIC with DECSTBM+DECSLRM =====
TEST(state_edit_decic_decstbm_decslrm)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);

    push(vt, "\e[?69h");
    push(vt, "\e[4;20r\e[20;60s");

    callbacks_clear();
    push(vt, "\e[4;20H\e[3'}");
    ASSERT_EQ(g_cb.scrollrect_count, 1);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_row, 3);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_row, 20);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_col, 19);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_col, 60);
    ASSERT_EQ(g_cb.scrollrect[0].downward, 0);
    ASSERT_EQ(g_cb.scrollrect[0].rightward, -3);
}

// ===== DECIC outside DECSLRM =====
TEST(state_edit_decic_outside_decslrm)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);

    push(vt, "\e[?69h");
    push(vt, "\e[4;20r\e[20;60s");

    callbacks_clear();
    push(vt, "\e[70G\e['}");
    // nothing happens
    ASSERT_EQ(g_cb.scrollrect_count, 0);
}

// ===== DECDC =====
TEST(state_edit_decdc)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);

    callbacks_clear();

    push(vt, "\e[20G\e[5'~");
    ASSERT_EQ(g_cb.scrollrect_count, 1);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_row, 0);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_row, 25);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_col, 19);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_col, 80);
    ASSERT_EQ(g_cb.scrollrect[0].downward, 0);
    ASSERT_EQ(g_cb.scrollrect[0].rightward, 5);
}

// ===== DECDC with DECSTBM+DECSLRM =====
TEST(state_edit_decdc_decstbm_decslrm)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);

    push(vt, "\e[?69h");
    push(vt, "\e[4;20r\e[20;60s");

    callbacks_clear();
    push(vt, "\e[4;20H\e[3'~");
    ASSERT_EQ(g_cb.scrollrect_count, 1);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_row, 3);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_row, 20);
    ASSERT_EQ(g_cb.scrollrect[0].rect.start_col, 19);
    ASSERT_EQ(g_cb.scrollrect[0].rect.end_col, 60);
    ASSERT_EQ(g_cb.scrollrect[0].downward, 0);
    ASSERT_EQ(g_cb.scrollrect[0].rightward, 3);
}

// ===== DECDC outside DECSLRM =====
TEST(state_edit_decdc_outside_decslrm)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);

    push(vt, "\e[?69h");
    push(vt, "\e[4;20r\e[20;60s");

    callbacks_clear();
    push(vt, "\e[70G\e['~");
    // nothing happens
    ASSERT_EQ(g_cb.scrollrect_count, 0);
}

// ===== EL 0 =====
TEST(state_edit_el_0)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);

    callbacks_clear();
    ASSERT_CURSOR(state, 0, 0);

    push(vt, "ABCDE");
    push(vt, "\e[3D");
    ASSERT_CURSOR(state, 0, 2);

    callbacks_clear();
    push(vt, "\e[0K");
    ASSERT_EQ(g_cb.erase_count, 1);
    ASSERT_EQ(g_cb.erase[0].rect.start_row, 0);
    ASSERT_EQ(g_cb.erase[0].rect.end_row, 1);
    ASSERT_EQ(g_cb.erase[0].rect.start_col, 2);
    ASSERT_EQ(g_cb.erase[0].rect.end_col, 80);
    ASSERT_CURSOR(state, 0, 2);
}

// ===== EL 1 =====
TEST(state_edit_el_1)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);

    callbacks_clear();
    ASSERT_CURSOR(state, 0, 0);

    push(vt, "ABCDE");
    push(vt, "\e[3D");
    ASSERT_CURSOR(state, 0, 2);

    callbacks_clear();
    push(vt, "\e[1K");
    ASSERT_EQ(g_cb.erase_count, 1);
    ASSERT_EQ(g_cb.erase[0].rect.start_row, 0);
    ASSERT_EQ(g_cb.erase[0].rect.end_row, 1);
    ASSERT_EQ(g_cb.erase[0].rect.start_col, 0);
    ASSERT_EQ(g_cb.erase[0].rect.end_col, 3);
    ASSERT_CURSOR(state, 0, 2);
}

// ===== EL 2 =====
TEST(state_edit_el_2)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);

    callbacks_clear();
    ASSERT_CURSOR(state, 0, 0);

    push(vt, "ABCDE");
    push(vt, "\e[3D");
    ASSERT_CURSOR(state, 0, 2);

    callbacks_clear();
    push(vt, "\e[2K");
    ASSERT_EQ(g_cb.erase_count, 1);
    ASSERT_EQ(g_cb.erase[0].rect.start_row, 0);
    ASSERT_EQ(g_cb.erase[0].rect.end_row, 1);
    ASSERT_EQ(g_cb.erase[0].rect.start_col, 0);
    ASSERT_EQ(g_cb.erase[0].rect.end_col, 80);
    ASSERT_CURSOR(state, 0, 2);
}

// ===== SEL (Selective Erase in Line) =====
TEST(state_edit_sel)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);

    callbacks_clear();
    ASSERT_CURSOR(state, 0, 0);

    push(vt, "\e[11G");
    ASSERT_CURSOR(state, 0, 10);

    callbacks_clear();
    push(vt, "\e[?0K");
    ASSERT_EQ(g_cb.erase_count, 1);
    ASSERT_EQ(g_cb.erase[0].rect.start_row, 0);
    ASSERT_EQ(g_cb.erase[0].rect.end_row, 1);
    ASSERT_EQ(g_cb.erase[0].rect.start_col, 10);
    ASSERT_EQ(g_cb.erase[0].rect.end_col, 80);
    ASSERT_EQ(g_cb.erase[0].selective, true);
    ASSERT_CURSOR(state, 0, 10);

    callbacks_clear();
    push(vt, "\e[?1K");
    ASSERT_EQ(g_cb.erase_count, 1);
    ASSERT_EQ(g_cb.erase[0].rect.start_row, 0);
    ASSERT_EQ(g_cb.erase[0].rect.end_row, 1);
    ASSERT_EQ(g_cb.erase[0].rect.start_col, 0);
    ASSERT_EQ(g_cb.erase[0].rect.end_col, 11);
    ASSERT_EQ(g_cb.erase[0].selective, true);
    ASSERT_CURSOR(state, 0, 10);

    callbacks_clear();
    push(vt, "\e[?2K");
    ASSERT_EQ(g_cb.erase_count, 1);
    ASSERT_EQ(g_cb.erase[0].rect.start_row, 0);
    ASSERT_EQ(g_cb.erase[0].rect.end_row, 1);
    ASSERT_EQ(g_cb.erase[0].rect.start_col, 0);
    ASSERT_EQ(g_cb.erase[0].rect.end_col, 80);
    ASSERT_EQ(g_cb.erase[0].selective, true);
    ASSERT_CURSOR(state, 0, 10);
}

// ===== ED 0 =====
TEST(state_edit_ed_0)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);

    callbacks_clear();
    ASSERT_CURSOR(state, 0, 0);

    push(vt, "\e[2;2H");
    ASSERT_CURSOR(state, 1, 1);

    callbacks_clear();
    push(vt, "\e[0J");
    ASSERT_EQ(g_cb.erase_count, 2);
    ASSERT_EQ(g_cb.erase[0].rect.start_row, 1);
    ASSERT_EQ(g_cb.erase[0].rect.end_row, 2);
    ASSERT_EQ(g_cb.erase[0].rect.start_col, 1);
    ASSERT_EQ(g_cb.erase[0].rect.end_col, 80);
    ASSERT_EQ(g_cb.erase[1].rect.start_row, 2);
    ASSERT_EQ(g_cb.erase[1].rect.end_row, 25);
    ASSERT_EQ(g_cb.erase[1].rect.start_col, 0);
    ASSERT_EQ(g_cb.erase[1].rect.end_col, 80);
    ASSERT_CURSOR(state, 1, 1);
}

// ===== ED 1 =====
TEST(state_edit_ed_1)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);

    callbacks_clear();
    ASSERT_CURSOR(state, 0, 0);

    push(vt, "\e[2;2H");
    ASSERT_CURSOR(state, 1, 1);

    callbacks_clear();
    push(vt, "\e[1J");
    ASSERT_EQ(g_cb.erase_count, 2);
    ASSERT_EQ(g_cb.erase[0].rect.start_row, 0);
    ASSERT_EQ(g_cb.erase[0].rect.end_row, 1);
    ASSERT_EQ(g_cb.erase[0].rect.start_col, 0);
    ASSERT_EQ(g_cb.erase[0].rect.end_col, 80);
    ASSERT_EQ(g_cb.erase[1].rect.start_row, 1);
    ASSERT_EQ(g_cb.erase[1].rect.end_row, 2);
    ASSERT_EQ(g_cb.erase[1].rect.start_col, 0);
    ASSERT_EQ(g_cb.erase[1].rect.end_col, 2);
    ASSERT_CURSOR(state, 1, 1);
}

// ===== ED 2 =====
TEST(state_edit_ed_2)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);

    callbacks_clear();
    ASSERT_CURSOR(state, 0, 0);

    push(vt, "\e[2;2H");
    ASSERT_CURSOR(state, 1, 1);

    callbacks_clear();
    push(vt, "\e[2J");
    ASSERT_EQ(g_cb.erase_count, 1);
    ASSERT_EQ(g_cb.erase[0].rect.start_row, 0);
    ASSERT_EQ(g_cb.erase[0].rect.end_row, 25);
    ASSERT_EQ(g_cb.erase[0].rect.start_col, 0);
    ASSERT_EQ(g_cb.erase[0].rect.end_col, 80);
    ASSERT_CURSOR(state, 1, 1);
}

// ===== ED 3 =====
TEST(state_edit_ed_3)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);

    callbacks_clear();
    push(vt, "\e[3J");
    ASSERT_EQ(g_cb.sb_clear_count, 1);
}

// ===== SED (Selective Erase in Display) =====
TEST(state_edit_sed)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);

    callbacks_clear();

    push(vt, "\e[5;5H");
    ASSERT_CURSOR(state, 4, 4);

    callbacks_clear();
    push(vt, "\e[?0J");
    ASSERT_EQ(g_cb.erase_count, 2);
    ASSERT_EQ(g_cb.erase[0].rect.start_row, 4);
    ASSERT_EQ(g_cb.erase[0].rect.end_row, 5);
    ASSERT_EQ(g_cb.erase[0].rect.start_col, 4);
    ASSERT_EQ(g_cb.erase[0].rect.end_col, 80);
    ASSERT_EQ(g_cb.erase[0].selective, true);
    ASSERT_EQ(g_cb.erase[1].rect.start_row, 5);
    ASSERT_EQ(g_cb.erase[1].rect.end_row, 25);
    ASSERT_EQ(g_cb.erase[1].rect.start_col, 0);
    ASSERT_EQ(g_cb.erase[1].rect.end_col, 80);
    ASSERT_EQ(g_cb.erase[1].selective, true);
    ASSERT_CURSOR(state, 4, 4);

    callbacks_clear();
    push(vt, "\e[?1J");
    ASSERT_EQ(g_cb.erase_count, 2);
    ASSERT_EQ(g_cb.erase[0].rect.start_row, 0);
    ASSERT_EQ(g_cb.erase[0].rect.end_row, 4);
    ASSERT_EQ(g_cb.erase[0].rect.start_col, 0);
    ASSERT_EQ(g_cb.erase[0].rect.end_col, 80);
    ASSERT_EQ(g_cb.erase[0].selective, true);
    ASSERT_EQ(g_cb.erase[1].rect.start_row, 4);
    ASSERT_EQ(g_cb.erase[1].rect.end_row, 5);
    ASSERT_EQ(g_cb.erase[1].rect.start_col, 0);
    ASSERT_EQ(g_cb.erase[1].rect.end_col, 5);
    ASSERT_EQ(g_cb.erase[1].selective, true);
    ASSERT_CURSOR(state, 4, 4);

    callbacks_clear();
    push(vt, "\e[?2J");
    ASSERT_EQ(g_cb.erase_count, 1);
    ASSERT_EQ(g_cb.erase[0].rect.start_row, 0);
    ASSERT_EQ(g_cb.erase[0].rect.end_row, 25);
    ASSERT_EQ(g_cb.erase[0].rect.start_col, 0);
    ASSERT_EQ(g_cb.erase[0].rect.end_col, 80);
    ASSERT_EQ(g_cb.erase[0].selective, true);
    ASSERT_CURSOR(state, 4, 4);
}

// ===== DECRQSS on DECSCA =====
TEST(state_edit_decrqss_decsca)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs);
    state.reset(true);
    output_init(vt);

    push(vt, "\e[2\"q");
    push(vt, "\eP$q\"q\e\\");

    ASSERT_OUTPUT_BYTES("\eP1$r2\"q\e\\", 10);
}

// ===== ICH move+erase emulation =====
TEST(state_edit_ich_moverect)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs_no_scrollrect);
    state.enable_premove();
    state.reset(true);

    callbacks_clear();
    ASSERT_CURSOR(state, 0, 0);

    push(vt, "ACD");
    push(vt, "\e[2D");
    ASSERT_CURSOR(state, 0, 1);

    callbacks_clear();
    push(vt, "\e[@");
    // premove 0..1,79..80
    ASSERT_EQ(g_cb.premove_count, 1);
    ASSERT_EQ(g_cb.premove[0].rect.start_row, 0);
    ASSERT_EQ(g_cb.premove[0].rect.end_row, 1);
    ASSERT_EQ(g_cb.premove[0].rect.start_col, 79);
    ASSERT_EQ(g_cb.premove[0].rect.end_col, 80);
    // moverect 0..1,1..79 -> 0..1,2..80
    ASSERT_EQ(g_cb.moverect_count, 1);
    ASSERT_EQ(g_cb.moverect[0].src.start_row, 0);
    ASSERT_EQ(g_cb.moverect[0].src.end_row, 1);
    ASSERT_EQ(g_cb.moverect[0].src.start_col, 1);
    ASSERT_EQ(g_cb.moverect[0].src.end_col, 79);
    ASSERT_EQ(g_cb.moverect[0].dest.start_row, 0);
    ASSERT_EQ(g_cb.moverect[0].dest.end_row, 1);
    ASSERT_EQ(g_cb.moverect[0].dest.start_col, 2);
    ASSERT_EQ(g_cb.moverect[0].dest.end_col, 80);
    // erase 0..1,1..2
    ASSERT_EQ(g_cb.erase_count, 1);
    ASSERT_EQ(g_cb.erase[0].rect.start_row, 0);
    ASSERT_EQ(g_cb.erase[0].rect.end_row, 1);
    ASSERT_EQ(g_cb.erase[0].rect.start_col, 1);
    ASSERT_EQ(g_cb.erase[0].rect.end_col, 2);
    ASSERT_CURSOR(state, 0, 1);

    push(vt, "B");
    ASSERT_CURSOR(state, 0, 2);

    callbacks_clear();
    push(vt, "\e[3@");
    // premove 0..1,77..80
    ASSERT_EQ(g_cb.premove_count, 1);
    ASSERT_EQ(g_cb.premove[0].rect.start_row, 0);
    ASSERT_EQ(g_cb.premove[0].rect.end_row, 1);
    ASSERT_EQ(g_cb.premove[0].rect.start_col, 77);
    ASSERT_EQ(g_cb.premove[0].rect.end_col, 80);
    // moverect 0..1,2..77 -> 0..1,5..80
    ASSERT_EQ(g_cb.moverect_count, 1);
    ASSERT_EQ(g_cb.moverect[0].src.start_row, 0);
    ASSERT_EQ(g_cb.moverect[0].src.end_row, 1);
    ASSERT_EQ(g_cb.moverect[0].src.start_col, 2);
    ASSERT_EQ(g_cb.moverect[0].src.end_col, 77);
    ASSERT_EQ(g_cb.moverect[0].dest.start_row, 0);
    ASSERT_EQ(g_cb.moverect[0].dest.end_row, 1);
    ASSERT_EQ(g_cb.moverect[0].dest.start_col, 5);
    ASSERT_EQ(g_cb.moverect[0].dest.end_col, 80);
    // erase 0..1,2..5
    ASSERT_EQ(g_cb.erase_count, 1);
    ASSERT_EQ(g_cb.erase[0].rect.start_row, 0);
    ASSERT_EQ(g_cb.erase[0].rect.end_row, 1);
    ASSERT_EQ(g_cb.erase[0].rect.start_col, 2);
    ASSERT_EQ(g_cb.erase[0].rect.end_col, 5);
}

// ===== DCH move+erase emulation =====
TEST(state_edit_dch_moverect)
{
    Terminal vt(25, 80);
    vt.set_utf8(true);
    State& state = vt.state();
    state.set_callbacks(state_cbs_no_scrollrect);
    state.enable_premove();
    state.reset(true);

    callbacks_clear();
    ASSERT_CURSOR(state, 0, 0);

    push(vt, "ABBC");
    push(vt, "\e[3D");
    ASSERT_CURSOR(state, 0, 1);

    callbacks_clear();
    push(vt, "\e[P");
    // premove 0..1,1..2
    ASSERT_EQ(g_cb.premove_count, 1);
    ASSERT_EQ(g_cb.premove[0].rect.start_row, 0);
    ASSERT_EQ(g_cb.premove[0].rect.end_row, 1);
    ASSERT_EQ(g_cb.premove[0].rect.start_col, 1);
    ASSERT_EQ(g_cb.premove[0].rect.end_col, 2);
    // moverect 0..1,2..80 -> 0..1,1..79
    ASSERT_EQ(g_cb.moverect_count, 1);
    ASSERT_EQ(g_cb.moverect[0].src.start_row, 0);
    ASSERT_EQ(g_cb.moverect[0].src.end_row, 1);
    ASSERT_EQ(g_cb.moverect[0].src.start_col, 2);
    ASSERT_EQ(g_cb.moverect[0].src.end_col, 80);
    ASSERT_EQ(g_cb.moverect[0].dest.start_row, 0);
    ASSERT_EQ(g_cb.moverect[0].dest.end_row, 1);
    ASSERT_EQ(g_cb.moverect[0].dest.start_col, 1);
    ASSERT_EQ(g_cb.moverect[0].dest.end_col, 79);
    // erase 0..1,79..80
    ASSERT_EQ(g_cb.erase_count, 1);
    ASSERT_EQ(g_cb.erase[0].rect.start_row, 0);
    ASSERT_EQ(g_cb.erase[0].rect.end_row, 1);
    ASSERT_EQ(g_cb.erase[0].rect.start_col, 79);
    ASSERT_EQ(g_cb.erase[0].rect.end_col, 80);
    ASSERT_CURSOR(state, 0, 1);

    callbacks_clear();
    push(vt, "\e[3P");
    // premove 0..1,1..4
    ASSERT_EQ(g_cb.premove_count, 1);
    ASSERT_EQ(g_cb.premove[0].rect.start_row, 0);
    ASSERT_EQ(g_cb.premove[0].rect.end_row, 1);
    ASSERT_EQ(g_cb.premove[0].rect.start_col, 1);
    ASSERT_EQ(g_cb.premove[0].rect.end_col, 4);
    // moverect 0..1,4..80 -> 0..1,1..77
    ASSERT_EQ(g_cb.moverect_count, 1);
    ASSERT_EQ(g_cb.moverect[0].src.start_row, 0);
    ASSERT_EQ(g_cb.moverect[0].src.end_row, 1);
    ASSERT_EQ(g_cb.moverect[0].src.start_col, 4);
    ASSERT_EQ(g_cb.moverect[0].src.end_col, 80);
    ASSERT_EQ(g_cb.moverect[0].dest.start_row, 0);
    ASSERT_EQ(g_cb.moverect[0].dest.end_row, 1);
    ASSERT_EQ(g_cb.moverect[0].dest.start_col, 1);
    ASSERT_EQ(g_cb.moverect[0].dest.end_col, 77);
    // erase 0..1,77..80
    ASSERT_EQ(g_cb.erase_count, 1);
    ASSERT_EQ(g_cb.erase[0].rect.start_row, 0);
    ASSERT_EQ(g_cb.erase[0].rect.end_row, 1);
    ASSERT_EQ(g_cb.erase[0].rect.start_col, 77);
    ASSERT_EQ(g_cb.erase[0].rect.end_col, 80);
    ASSERT_CURSOR(state, 0, 1);
}
