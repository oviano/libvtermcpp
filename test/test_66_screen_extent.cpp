// test_66_screen_extent.cpp — screen attribute extent tests
// Ported from upstream libvterm t/66screen_extent.test

#include "harness.h"

// Bold extent
TEST(screen_extent_bold)
{
    Terminal vt(25, 80);
    vt.set_utf8(false);
    Screen& screen = vt.screen();
    screen.set_callbacks(screen_cbs);
    screen.reset(true);

    // Write: A,B in default attrs; C,D in bold; E in default attrs
    push(vt, "AB\e[1mCD\e[mE");

    // Extent at (0,0): non-bold region cols 0-1, exclusive end = col 1
    {
        Pos pos = { .row = 0, .col = 0 };
        Rect rect = { .start_col = 0, .end_col = -1 };
        (void)screen.get_attrs_extent(rect, pos, AttrMask::All);
        ASSERT_EQ(rect.start_row, 0);
        ASSERT_EQ(rect.start_col, 0);
        ASSERT_EQ(rect.end_row, 1);
        ASSERT_EQ(rect.end_col, 2);
    }

    // Extent at (0,1): same non-bold region
    {
        Pos pos = { .row = 0, .col = 1 };
        Rect rect = { .start_col = 0, .end_col = -1 };
        (void)screen.get_attrs_extent(rect, pos, AttrMask::All);
        ASSERT_EQ(rect.start_row, 0);
        ASSERT_EQ(rect.start_col, 0);
        ASSERT_EQ(rect.end_row, 1);
        ASSERT_EQ(rect.end_col, 2);
    }

    // Extent at (0,2): bold region cols 2-3
    {
        Pos pos = { .row = 0, .col = 2 };
        Rect rect = { .start_col = 0, .end_col = -1 };
        (void)screen.get_attrs_extent(rect, pos, AttrMask::All);
        ASSERT_EQ(rect.start_row, 0);
        ASSERT_EQ(rect.start_col, 2);
        ASSERT_EQ(rect.end_row, 1);
        ASSERT_EQ(rect.end_col, 4);
    }

    // Extent at (0,3): same bold region
    {
        Pos pos = { .row = 0, .col = 3 };
        Rect rect = { .start_col = 0, .end_col = -1 };
        (void)screen.get_attrs_extent(rect, pos, AttrMask::All);
        ASSERT_EQ(rect.start_row, 0);
        ASSERT_EQ(rect.start_col, 2);
        ASSERT_EQ(rect.end_row, 1);
        ASSERT_EQ(rect.end_col, 4);
    }

    // Extent at (0,4): trailing non-bold region col 4 to end of line
    {
        Pos pos = { .row = 0, .col = 4 };
        Rect rect = { .start_col = 0, .end_col = -1 };
        (void)screen.get_attrs_extent(rect, pos, AttrMask::All);
        ASSERT_EQ(rect.start_row, 0);
        ASSERT_EQ(rect.start_col, 4);
        ASSERT_EQ(rect.end_row, 1);
        ASSERT_EQ(rect.end_col, 80);
    }
}
