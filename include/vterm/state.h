#ifndef VTERM_STATE_H
#define VTERM_STATE_H

#include "types.h"
#include "callbacks.h"

namespace vterm {

class Terminal;

class State {
public:
    void set_callbacks(StateCallbacks& cb);
    void clear_callbacks();
    void set_fallbacks(StateFallbacks& fb);
    void clear_fallbacks();
    void enable_premove();

    void reset(bool hard);

    [[nodiscard]] Pos cursor_pos() const;
    [[nodiscard]] bool get_penattr(Attr attr, Value& val) const;

    [[nodiscard]] const LineInfo& get_lineinfo(int32_t row) const;

    struct ColorPair { Color fg, bg; };
    [[nodiscard]] ColorPair get_default_colors() const;
    void set_default_colors(const Color& fg, const Color& bg);
    [[nodiscard]] Color get_palette_color(int32_t index) const;
    void set_palette_color(int32_t index, const Color& col);
    void set_bold_highbright(bool enabled);

    void convert_color_to_rgb(Color& col) const;

    [[nodiscard]] bool set_termprop(Prop prop, Value& val);

    void focus_in();
    void focus_out();

    void set_selection_callbacks(SelectionCallbacks& cb, size_t buflen);
    void clear_selection_callbacks();
    void send_selection(SelectionMask mask, StringFragment frag);

    struct Impl;

private:
    friend class Terminal;
    State() = default;
    ~State() = default;
    State(const State&) = delete;
    State& operator=(const State&) = delete;

    Impl* impl() { return impl_; }
    const Impl* impl() const { return impl_; }

    Impl* impl_ = nullptr;  // non-owning; lifetime managed by Terminal
};

} // namespace vterm

#endif // VTERM_STATE_H
