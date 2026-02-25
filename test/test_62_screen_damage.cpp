// test_62_screen_damage.cpp — screen damage tests
// Ported from upstream libvterm t/62screen_damage.test

#include "harness.h"

// Screen callbacks without moverect — used for damage-only tests.
struct TestScreenCallbacksNoMoverect : ScreenCallbacks {
    bool on_damage(Rect rect) override {
        if(g_cb.damage_count < CALLBACK_LOG_MAX)
            g_cb.damage[g_cb.damage_count].rect = rect;
        g_cb.damage_count++;
        return true;
    }

    bool on_movecursor(Pos pos, Pos oldpos, bool visible) override {
        if(g_cb.movecursor_count < CALLBACK_LOG_MAX) {
            auto* r = &g_cb.movecursor[g_cb.movecursor_count];
            r->pos = pos;
            r->oldpos = oldpos;
            r->visible = visible;
        }
        g_cb.movecursor_count++;
        return true;
    }

    bool on_settermprop(Prop prop, const Value& val) override {
        if(g_cb.settermprop_count < CALLBACK_LOG_MAX) {
            auto* r = &g_cb.settermprop[g_cb.settermprop_count];
            r->prop = prop;
            r->val = val;
        }
        g_cb.settermprop_count++;
        return true;
    }

    bool on_bell() override {
        g_cb.bell_count++;
        return true;
    }

    bool on_sb_pushline(std::span<const ScreenCell> cells, bool continuation) override {
        if(g_cb.sb_pushline_count < CALLBACK_LOG_MAX) {
            auto* r = &g_cb.sb_pushline[g_cb.sb_pushline_count];
            r->cols = static_cast<int32_t>(cells.size());
            r->continuation = continuation ? 1 : 0;
            for(int32_t i = 0; i < static_cast<int32_t>(cells.size()) && i < 256; i++)
                r->chars[i] = cells[i].chars[0];
        }
        g_cb.sb_pushline_count++;
        return true;
    }

    bool on_sb_popline([[maybe_unused]] std::span<ScreenCell> cells, bool& continuation) override {
        continuation = false;
        g_cb.sb_popline_count++;
        return false;
    }
};

static TestScreenCallbacksNoMoverect screen_cbs_no_moverect;

// Helper macro: assert damage rect at index i
#define ASSERT_DAMAGE(i, sr, er, sc, ec)                                      \
    do {                                                                       \
        ASSERT_EQ(g_cb.damage[(i)].rect.start_row, (sr));                      \
        ASSERT_EQ(g_cb.damage[(i)].rect.end_row, (er));                        \
        ASSERT_EQ(g_cb.damage[(i)].rect.start_col, (sc));                      \
        ASSERT_EQ(g_cb.damage[(i)].rect.end_col, (ec));                        \
    } while (0)

// Helper macro: assert moverect at index i (dest <- src)
#define ASSERT_MOVERECT(i, dsr, der, dsc, dec, ssr, ser, ssc, sec)            \
    do {                                                                       \
        ASSERT_EQ(g_cb.moverect[(i)].dest.start_row, (dsr));                   \
        ASSERT_EQ(g_cb.moverect[(i)].dest.end_row, (der));                     \
        ASSERT_EQ(g_cb.moverect[(i)].dest.start_col, (dsc));                   \
        ASSERT_EQ(g_cb.moverect[(i)].dest.end_col, (dec));                     \
        ASSERT_EQ(g_cb.moverect[(i)].src.start_row, (ssr));                    \
        ASSERT_EQ(g_cb.moverect[(i)].src.end_row, (ser));                      \
        ASSERT_EQ(g_cb.moverect[(i)].src.start_col, (ssc));                    \
        ASSERT_EQ(g_cb.moverect[(i)].src.end_col, (sec));                      \
    } while (0)

// Helper macro: assert sb_pushline at index i
#define ASSERT_SB_PUSHLINE(i, expected_cols)                                   \
    do {                                                                       \
        ASSERT_EQ(g_cb.sb_pushline[(i)].cols, (expected_cols));                 \
    } while (0)

// Helper macro: assert sb_pushline at index i with first char
#define ASSERT_SB_PUSHLINE_CHAR(i, expected_cols, ch)                          \
    do {                                                                       \
        ASSERT_EQ(g_cb.sb_pushline[(i)].cols, (expected_cols));                 \
        ASSERT_EQ(g_cb.sb_pushline[(i)].chars[0], static_cast<uint32_t>(ch));  \
    } while (0)

// Putglyph
TEST(screen_damage_putglyph)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    Screen& screen = vt.screen();
    screen.enable_altscreen(true);
    screen.set_callbacks(screen_cbs_no_moverect);
    screen.reset(true);
    callbacks_clear();

    push(vt, "123");

    ASSERT_EQ(g_cb.damage_count, 3);
    ASSERT_DAMAGE(0, 0, 1, 0, 1);
    ASSERT_DAMAGE(1, 0, 1, 1, 2);
    ASSERT_DAMAGE(2, 0, 1, 2, 3);
}

// Erase
TEST(screen_damage_erase)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    Screen& screen = vt.screen();
    screen.enable_altscreen(true);
    screen.set_callbacks(screen_cbs_no_moverect);
    screen.reset(true);
    callbacks_clear();

    push(vt, "123");
    callbacks_clear();

    push(vt, "\x1b[H");
    callbacks_clear();

    push(vt, "\x1b[3X");
    ASSERT_EQ(g_cb.damage_count, 1);
    ASSERT_DAMAGE(0, 0, 1, 0, 3);
}

// Scroll damages entire line in two chunks
TEST(screen_damage_scroll_line_two_chunks)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    Screen& screen = vt.screen();
    screen.enable_altscreen(true);
    screen.set_callbacks(screen_cbs_no_moverect);
    screen.reset(true);
    callbacks_clear();

    push(vt, "\x1b[H");
    callbacks_clear();

    push(vt, "\x1b[5@");
    ASSERT_EQ(g_cb.damage_count, 2);
    ASSERT_DAMAGE(0, 0, 1, 5, 80);
    ASSERT_DAMAGE(1, 0, 1, 0, 5);
}

// Scroll down damages entire screen in two chunks
TEST(screen_damage_scroll_down_two_chunks)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    Screen& screen = vt.screen();
    screen.enable_altscreen(true);
    screen.set_callbacks(screen_cbs_no_moverect);
    screen.reset(true);
    callbacks_clear();

    push(vt, "\x1b[T");
    ASSERT_EQ(g_cb.damage_count, 2);
    ASSERT_DAMAGE(0, 1, 25, 0, 80);
    ASSERT_DAMAGE(1, 0, 1, 0, 80);
}

// Altscreen damages entire area
TEST(screen_damage_altscreen)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    Screen& screen = vt.screen();
    screen.enable_altscreen(true);
    screen.set_callbacks(screen_cbs_no_moverect);
    screen.reset(true);
    callbacks_clear();

    push(vt, "\x1b[?1049h");
    ASSERT_EQ(g_cb.damage_count, 1);
    ASSERT_DAMAGE(0, 0, 25, 0, 80);

    callbacks_clear();
    push(vt, "\x1b[?1049l");
    ASSERT_EQ(g_cb.damage_count, 1);
    ASSERT_DAMAGE(0, 0, 25, 0, 80);
}

// Scroll invokes moverect but not damage (with moverect callback)
TEST(screen_damage_scroll_with_moverect)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    Screen& screen = vt.screen();
    screen.enable_altscreen(true);
    screen.set_callbacks(screen_cbs);
    screen.reset(true);
    callbacks_clear();

    push(vt, "\x1b[5@");
    ASSERT_EQ(g_cb.moverect_count, 1);
    ASSERT_MOVERECT(0,  0, 1, 5, 80,  0, 1, 0, 75);
    ASSERT_EQ(g_cb.damage_count, 1);
    ASSERT_DAMAGE(0, 0, 1, 0, 5);
}

// Merge to cells
TEST(screen_damage_merge_cell)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    Screen& screen = vt.screen();
    screen.enable_altscreen(true);
    screen.set_callbacks(screen_cbs_no_moverect);
    screen.reset(true);
    callbacks_clear();

    screen.set_damage_merge(DamageSize::Cell);

    push(vt, "A");
    ASSERT_EQ(g_cb.damage_count, 1);
    ASSERT_DAMAGE(0, 0, 1, 0, 1);

    callbacks_clear();
    push(vt, "B");
    ASSERT_EQ(g_cb.damage_count, 1);
    ASSERT_DAMAGE(0, 0, 1, 1, 2);

    callbacks_clear();
    push(vt, "C");
    ASSERT_EQ(g_cb.damage_count, 1);
    ASSERT_DAMAGE(0, 0, 1, 2, 3);
}

// Merge entire rows
TEST(screen_damage_merge_row)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    Screen& screen = vt.screen();
    screen.enable_altscreen(true);
    screen.set_callbacks(screen_cbs_no_moverect);
    screen.reset(true);
    callbacks_clear();

    screen.set_damage_merge(DamageSize::Row);

    push(vt, "ABCDE\r\nEFGH");
    ASSERT_EQ(g_cb.damage_count, 1);
    ASSERT_DAMAGE(0, 0, 1, 0, 5);

    screen.flush_damage();
    ASSERT_EQ(g_cb.damage_count, 2);
    ASSERT_DAMAGE(1, 1, 2, 0, 4);

    callbacks_clear();

    push(vt, "\x1b[3;6r\x1b[6H\x1b" "D");
    ASSERT_EQ(g_cb.damage_count, 1);
    ASSERT_DAMAGE(0, 2, 5, 0, 80);

    screen.flush_damage();
    ASSERT_EQ(g_cb.damage_count, 2);
    ASSERT_DAMAGE(1, 5, 6, 0, 80);
}

// Merge entire screen
TEST(screen_damage_merge_screen)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    Screen& screen = vt.screen();
    screen.enable_altscreen(true);
    screen.set_callbacks(screen_cbs_no_moverect);
    screen.reset(true);
    callbacks_clear();

    screen.set_damage_merge(DamageSize::Screen);

    push(vt, "ABCDE\r\nEFGH");
    ASSERT_EQ(g_cb.damage_count, 0);

    screen.flush_damage();
    ASSERT_EQ(g_cb.damage_count, 1);
    ASSERT_DAMAGE(0, 0, 2, 0, 5);

    callbacks_clear();

    push(vt, "\x1b[3;6r\x1b[6H\x1b" "D");
    ASSERT_EQ(g_cb.damage_count, 0);

    screen.flush_damage();
    ASSERT_EQ(g_cb.damage_count, 1);
    ASSERT_DAMAGE(0, 2, 6, 0, 80);
}

// Merge entire screen with moverect
TEST(screen_damage_merge_screen_moverect)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    Screen& screen = vt.screen();
    screen.enable_altscreen(true);
    screen.set_callbacks(screen_cbs);
    screen.reset(true);
    callbacks_clear();

    screen.set_damage_merge(DamageSize::Screen);

    push(vt, "ABCDE\r\nEFGH");
    push(vt, "\x1b[3;6r\x1b[6H\x1b" "D");
    ASSERT_EQ(g_cb.damage_count, 1);
    ASSERT_DAMAGE(0, 0, 2, 0, 5);
    ASSERT_EQ(g_cb.moverect_count, 1);
    ASSERT_MOVERECT(0,  2, 5, 0, 80,  3, 6, 0, 80);

    screen.flush_damage();
    ASSERT_EQ(g_cb.damage_count, 2);
    ASSERT_DAMAGE(1, 5, 6, 0, 80);
}

// Merge scroll
TEST(screen_damage_merge_scroll)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    Screen& screen = vt.screen();
    screen.enable_altscreen(true);
    screen.set_callbacks(screen_cbs);
    screen.reset(true);
    callbacks_clear();

    screen.set_damage_merge(DamageSize::Scroll);

    push(vt, "\x1b[H" "1\r\n" "2\r\n" "3");
    push(vt, "\x1b[25H\n\n\n");
    ASSERT_EQ(g_cb.sb_pushline_count, 3);
    ASSERT_SB_PUSHLINE_CHAR(0, 80, 0x31);
    ASSERT_SB_PUSHLINE_CHAR(1, 80, 0x32);
    ASSERT_SB_PUSHLINE_CHAR(2, 80, 0x33);

    screen.flush_damage();
    ASSERT_EQ(g_cb.moverect_count, 1);
    ASSERT_MOVERECT(0,  0, 22, 0, 80,  3, 25, 0, 80);
    ASSERT_TRUE(g_cb.damage_count >= 1);
    ASSERT_DAMAGE(g_cb.damage_count - 1, 0, 25, 0, 80);
}

// Merge scroll with damage
TEST(screen_damage_merge_scroll_with_damage)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    Screen& screen = vt.screen();
    screen.enable_altscreen(true);
    screen.set_callbacks(screen_cbs);
    screen.reset(true);
    callbacks_clear();

    screen.set_damage_merge(DamageSize::Scroll);

    push(vt, "\x1b[H" "1\r\n" "2\r\n" "3");
    push(vt, "\x1b[25H\n\n\n");
    screen.flush_damage();
    callbacks_clear();

    push(vt, "\x1b[25H");
    push(vt, "ABCDE\r\nEFGH\r\n");
    ASSERT_EQ(g_cb.sb_pushline_count, 2);
    ASSERT_SB_PUSHLINE(0, 80);
    ASSERT_SB_PUSHLINE(1, 80);

    screen.flush_damage();
    ASSERT_EQ(g_cb.moverect_count, 1);
    ASSERT_MOVERECT(0,  0, 23, 0, 80,  2, 25, 0, 80);
    ASSERT_TRUE(g_cb.damage_count >= 1);
    {
        int32_t last= g_cb.damage_count - 1;
        ASSERT_DAMAGE(last, 22, 25, 0, 80);
    }
}

// Merge scroll with damage past region
TEST(screen_damage_merge_scroll_past_region)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    Screen& screen = vt.screen();
    screen.enable_altscreen(true);
    screen.set_callbacks(screen_cbs);
    screen.reset(true);
    callbacks_clear();

    screen.set_damage_merge(DamageSize::Scroll);

    push(vt, "\x1b[3;6r\x1b[6H" "1\r\n" "2\r\n" "3\r\n" "4\r\n" "5");

    screen.flush_damage();
    ASSERT_TRUE(g_cb.damage_count >= 1);
    {
        int32_t last= g_cb.damage_count - 1;
        ASSERT_DAMAGE(last, 2, 6, 0, 80);
    }
}

// Damage entirely outside scroll region
TEST(screen_damage_outside_scroll_region)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    Screen& screen = vt.screen();
    screen.enable_altscreen(true);
    screen.set_callbacks(screen_cbs);
    screen.reset(true);
    callbacks_clear();

    screen.set_damage_merge(DamageSize::Scroll);

    push(vt, "\x1b[HABC\x1b[3;6r\x1b[6H\r\n" "6");
    ASSERT_EQ(g_cb.damage_count, 1);
    ASSERT_DAMAGE(0, 0, 1, 0, 3);

    screen.flush_damage();
    ASSERT_EQ(g_cb.moverect_count, 1);
    ASSERT_MOVERECT(0,  2, 5, 0, 80,  3, 6, 0, 80);
    {
        int32_t last= g_cb.damage_count - 1;
        ASSERT_DAMAGE(last, 5, 6, 0, 80);
    }
}

// Damage overlapping scroll region
TEST(screen_damage_overlapping_scroll_region)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    Screen& screen = vt.screen();
    screen.enable_altscreen(true);
    screen.set_callbacks(screen_cbs);
    screen.reset(true);
    callbacks_clear();

    screen.set_damage_merge(DamageSize::Scroll);

    push(vt, "\x1b[H\x1b[2J");
    screen.flush_damage();
    ASSERT_TRUE(g_cb.damage_count >= 1);
    {
        int32_t last= g_cb.damage_count - 1;
        ASSERT_DAMAGE(last, 0, 25, 0, 80);
    }
    callbacks_clear();

    push(vt, "\x1b[HABCD\r\nEFGH\r\nIJKL\x1b[2;5r\x1b[5H\r\nMNOP");

    screen.flush_damage();
    ASSERT_EQ(g_cb.moverect_count, 1);
    ASSERT_MOVERECT(0,  1, 4, 0, 80,  2, 5, 0, 80);
    {
        int32_t last= g_cb.damage_count - 1;
        ASSERT_DAMAGE(last, 0, 5, 0, 80);
    }
}

// Merge scroll*2 with damage
TEST(screen_damage_merge_scroll2_with_damage)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    Screen& screen = vt.screen();
    screen.enable_altscreen(true);
    screen.set_callbacks(screen_cbs);
    screen.reset(true);
    callbacks_clear();

    screen.set_damage_merge(DamageSize::Scroll);

    push(vt, "\x1b[25H\r\nABCDE\x08\x08\x08\x1b[2P\r\n");

    ASSERT_EQ(g_cb.sb_pushline_count, 2);
    ASSERT_SB_PUSHLINE(0, 80);
    ASSERT_SB_PUSHLINE(1, 80);

    ASSERT_EQ(g_cb.moverect_count, 2);
    ASSERT_MOVERECT(0,  0, 24, 0, 80,  1, 25, 0, 80);
    ASSERT_MOVERECT(1,  24, 25, 2, 78,  24, 25, 4, 80);

    ASSERT_EQ(g_cb.damage_count, 2);
    ASSERT_DAMAGE(0, 24, 25, 0, 80);
    ASSERT_DAMAGE(1, 24, 25, 78, 80);

    screen.flush_damage();
    {
        int32_t last_mr = g_cb.moverect_count - 1;
        ASSERT_MOVERECT(last_mr,  0, 24, 0, 80,  1, 25, 0, 80);
    }
    {
        int32_t last_d = g_cb.damage_count - 1;
        ASSERT_DAMAGE(last_d, 24, 25, 0, 80);
    }

    // ?screen_row 23 = "ABE"
    ASSERT_SCREEN_ROW(vt, screen, 23, "ABE");
}
