#ifndef VTERM_SCREEN_H
#define VTERM_SCREEN_H

#include "types.h"
#include "callbacks.h"

#include <span>

namespace vterm {

class Terminal;

class Screen {
public:
    void set_callbacks(ScreenCallbacks& cb);
    void clear_callbacks();
    void set_fallbacks(StateFallbacks& fb);
    void clear_fallbacks();
    void enable_altscreen(bool enabled);
    void enable_reflow(bool enabled);
    void set_damage_merge(DamageSize size);
    void flush_damage();
    void reset(bool hard);

    [[nodiscard]] bool get_cell(Pos pos, ScreenCell& cell) const;
    [[nodiscard]] size_t get_chars(std::span<uint32_t> chars, Rect rect) const;
    [[nodiscard]] size_t get_text(std::span<char> str, Rect rect) const;
    [[nodiscard]] bool get_attrs_extent(Rect& extent, Pos pos, AttrMask attrs) const;
    [[nodiscard]] bool is_eol(Pos pos) const;

    void convert_color_to_rgb(Color& col) const;
    void set_default_colors(const Color& fg, const Color& bg);

    struct Impl;

private:
    friend class Terminal;
    Screen() = default;
    ~Screen() = default;
    Screen(const Screen&) = delete;
    Screen& operator=(const Screen&) = delete;

    Impl* impl() { return impl_; }
    const Impl* impl() const { return impl_; }

    Impl* impl_ = nullptr;  // non-owning; lifetime managed by Terminal
};

} // namespace vterm

#endif // VTERM_SCREEN_H
