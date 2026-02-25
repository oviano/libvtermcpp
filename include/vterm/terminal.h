#ifndef VTERM_TERMINAL_H
#define VTERM_TERMINAL_H

#include "types.h"
#include "callbacks.h"
#include <functional>
#include <memory>
#include <span>

namespace vterm {

class State;
class Screen;
class Scrollback;

class Terminal {
public:
    Terminal(int32_t rows, int32_t cols);
    ~Terminal();

    Terminal(const Terminal&) = delete;
    Terminal& operator=(const Terminal&) = delete;
    Terminal(Terminal&&) noexcept;
    Terminal& operator=(Terminal&&) noexcept;

    [[nodiscard]] int32_t rows() const;
    [[nodiscard]] int32_t cols() const;
    void set_size(int32_t rows, int32_t cols);

    [[nodiscard]] bool utf8() const;
    void set_utf8(bool enabled);

    [[nodiscard]] size_t write(std::span<const char> data);

    void set_output_callback(std::function<void(std::span<const char>)> cb);

    void keyboard_unichar(uint32_t c, Modifier mod);
    void keyboard_key(Key key, Modifier mod);
    void keyboard_start_paste();
    void keyboard_end_paste();

    void mouse_move(int32_t row, int32_t col, Modifier mod);
    void mouse_button(int32_t button, bool pressed, Modifier mod);

    State& state();
    Screen& screen();
    Scrollback& scrollback();

    void parser_set_callbacks(ParserCallbacks& cb);
    void parser_clear_callbacks();

    struct Impl;

private:
    Impl* impl() { return impl_.get(); }
    const Impl* impl() const { return impl_.get(); }

    std::unique_ptr<Impl> impl_;
};

} // namespace vterm

#endif // VTERM_TERMINAL_H
