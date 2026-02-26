#include "internal.h"

#include <utility>

namespace vterm {

// --- Terminal ---

Terminal::Terminal(int32_t rows, int32_t cols)
    : impl_(std::make_unique<Impl>())
{
    impl_->rows = std::max(rows, 1);
    impl_->cols = std::max(cols, 1);

    impl_->parser.state = ParserState::Normal;
    impl_->parser.callbacks = nullptr;
    impl_->parser.emit_nul = false;

    static constexpr size_t default_outbuffer_size = 4096;
    impl_->outbuffer.resize(default_outbuffer_size);
    impl_->outbuffer_cur = 0;
}

Terminal::~Terminal() = default;

Terminal::Terminal(Terminal&& other) noexcept = default;

Terminal& Terminal::operator=(Terminal&& other) noexcept = default;

int32_t Terminal::rows() const { return impl_->rows; }
int32_t Terminal::cols() const { return impl_->cols; }

void Terminal::set_size(int32_t rows, int32_t cols) {
    if(rows < 1 || cols < 1)
        return;

    int32_t old_rows = impl_->rows;
    int32_t old_cols = impl_->cols;

    auto* sb = impl_->scrollback_impl.get();
    if(sb && sb->capacity > 0)
        sb->begin_resize();

    impl_->rows = rows;
    impl_->cols = cols;

    if(impl_->parser.callbacks && impl_->parser.callbacks->on_resize(rows, cols)) {
        // callback handled it
    }

    if(sb && sb->capacity > 0)
        sb->commit_resize(old_rows, rows, old_cols, cols);
}

bool Terminal::utf8() const { return impl_->mode.utf8; }
void Terminal::set_utf8(bool enabled) { impl_->mode.utf8 = enabled; }

size_t Terminal::write(std::span<const char> data) {
    return impl_->input_write(data);
}

void Terminal::set_output_callback(std::function<void(std::span<const char>)> cb) {
    impl_->outfunc = std::move(cb);
}

void Terminal::keyboard_unichar(uint32_t c, Modifier mod) {
    impl_->keyboard_unichar(c, mod);
}

void Terminal::keyboard_key(Key key, Modifier mod) {
    impl_->keyboard_key(key, mod);
}

void Terminal::keyboard_start_paste() {
    impl_->keyboard_start_paste();
}

void Terminal::keyboard_end_paste() {
    impl_->keyboard_end_paste();
}

void Terminal::mouse_move(int32_t row, int32_t col, Modifier mod) {
    impl_->mouse_move(row, col, mod);
}

void Terminal::mouse_button(int32_t button, bool pressed, Modifier mod) {
    impl_->mouse_button(button, pressed, mod);
}

State& Terminal::state() {
    if(!impl_->state)
        impl_->obtain_state();
    impl_->state_wrapper.impl_ = impl_->state.get();
    return impl_->state_wrapper;
}

Screen& Terminal::screen() {
    if(!impl_->screen)
        impl_->obtain_screen();
    impl_->screen_wrapper.impl_ = impl_->screen.get();
    return impl_->screen_wrapper;
}

Scrollback& Terminal::scrollback() {
    if(!impl_->scrollback_impl)
        impl_->scrollback_impl = std::make_unique<Scrollback::Impl>();
    impl_->scrollback_wrapper.impl_ = impl_->scrollback_impl.get();
    return impl_->scrollback_wrapper;
}

void Terminal::parser_set_callbacks(ParserCallbacks& cb) {
    impl_->parser.callbacks = &cb;
}

void Terminal::parser_clear_callbacks() {
    impl_->parser.callbacks = nullptr;
}

// --- Output helpers ---

void Terminal::Impl::push_output_bytes(std::span<const char> bytes) {
    if(outfunc) {
        outfunc(bytes);
        return;
    }

    assert(outbuffer_cur <= outbuffer.size());
    if(bytes.size() > outbuffer.size() - outbuffer_cur)
        return;

    std::copy(bytes.begin(), bytes.end(), outbuffer.begin() + outbuffer_cur);
    outbuffer_cur += bytes.size();
}

} // namespace vterm
