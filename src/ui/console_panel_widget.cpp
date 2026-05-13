#include "console_panel_widget.h"
#include "theme.h"
#include "../core/debugger.h"
#include <QPainter>
#include <QKeyEvent>
#include <QWheelEvent>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QColor>
#include <QString>

namespace memu8086::ui {

static const QColor DOS_COLORS[16] = {
    {0x0C,0x0C,0x0C}, {0x00,0x37,0xDA}, {0x13,0xA1,0x0E}, {0x3A,0x96,0xDD},
    {0xC5,0x0F,0x1F}, {0x88,0x17,0x98}, {0xC1,0x9C,0x00}, {0xCC,0xCC,0xCC},
    {0x76,0x76,0x76}, {0x3B,0x78,0xFF}, {0x16,0xC6,0x0C}, {0x61,0xD6,0xD6},
    {0xE7,0x48,0x56}, {0xB4,0x00,0x9E}, {0xF9,0xF1,0xA5}, {0xF2,0xF2,0xF2}
};

ConsolePanelWidget::ConsolePanelWidget(emu8086::core::ConsoleState& state, QWidget* parent)
    : QWidget(parent), state(state) {
    
    setFocusPolicy(Qt::StrongFocus);

    mono = Theme::mono_font(13);
    fm = new QFontMetrics(mono);
    char_w = fm->horizontalAdvance('W');
    char_h = fm->height() + 2;

    QVBoxLayout* main_layout = new QVBoxLayout(this);
    main_layout->setContentsMargins(8, 8, 8, 8);
    main_layout->setSpacing(4);

    // Toolbar Setup
    toolbar = new QWidget(this);
    QHBoxLayout* tb_layout = new QHBoxLayout(toolbar);
    tb_layout->setContentsMargins(0, 0, 0, 0);

    QPushButton* btn_clear = new QPushButton("Clear", this);
    QPushButton* btn_copy = new QPushButton("Copy Screen", this);
    QPushButton* btn_paste = new QPushButton("Paste", this);
    lbl_status = new QLabel("80×25 | CGA | Cursor: (0,0)", this);
    lbl_status->setStyleSheet(QStringLiteral("color: %1;").arg(Theme::Color::TEXT_MUTED));

    tb_layout->addWidget(btn_clear);
    tb_layout->addWidget(btn_copy);
    tb_layout->addWidget(btn_paste);
    tb_layout->addStretch();
    tb_layout->addWidget(lbl_status);
    main_layout->addWidget(toolbar);

    // Screen & Scrollbar Layout
    QHBoxLayout* screen_layout = new QHBoxLayout();
    screen_layout->addStretch();
    vscrollbar = new QScrollBar(Qt::Vertical, this);
    vscrollbar->setMinimum(0);
    vscrollbar->setMaximum(0);
    screen_layout->addWidget(vscrollbar);
    main_layout->addLayout(screen_layout);

    cursor_blink_timer = new QTimer(this);
    connect(cursor_blink_timer, &QTimer::timeout, this, [this]{
        cursor_visible_blink = !cursor_visible_blink;
        update();
    });
    cursor_blink_timer->start(500);

    connect(vscrollbar, &QScrollBar::valueChanged, this, [this](int value) {
        scroll_offset = vscrollbar->maximum() - value;
        update();
    });

    // Hook ConsoleState scroll_up to push into our scrollback buffers
    if (state.on_scroll_up) {
        auto old_hook = state.on_scroll_up;
        state.on_scroll_up = [this, old_hook]() {
            if (old_hook) old_hook();
            push_scrollback();
        };
    } else {
        state.on_scroll_up = [this]() { push_scrollback(); };
    }
}

ConsolePanelWidget::~ConsolePanelWidget() {
    delete fm;
}

void ConsolePanelWidget::push_scrollback() {
    ScrollbackRow row;
    for (int c = 0; c < 80; ++c) {
        row.chars[c] = state.screen[0][c];
        row.attrs[c] = state.color[0][c];
    }
    scrollback.append(row);
    if (scrollback.size() > 500) scrollback.removeFirst();
    if (scroll_offset > 0) scroll_offset = qMin(scroll_offset + 1, (int)scrollback.size());
    update_scrollbar();
}

void ConsolePanelWidget::update_scrollbar() {
    vscrollbar->setMaximum(scrollback.size());
    vscrollbar->setValue(scrollback.size() - scroll_offset);
}

void ConsolePanelWidget::refresh() {
    lbl_status->setText(QString("80×25 | CGA | Cursor: (%1,%2)").arg(state.cursor_x).arg(state.cursor_y));
    update();
}

void ConsolePanelWidget::paintEvent(QPaintEvent* event) {
    QPainter painter(this);
    painter.fillRect(event->rect(), QColor(Theme::Color::BG));

    int tb_h = toolbar->height() + 4;
    int avail_w = width() - vscrollbar->width() - 16;
    int avail_h = height() - tb_h - 16;
    if (avail_w <= 0 || avail_h <= 0) return;

    int cga_w = char_w * 80;
    int cga_h = char_h * 25;
    float scale = qMin((float)avail_w / cga_w, (float)avail_h / cga_h);

    // Maintain aspect ratio, letterbox and center
    int dx = 8 + (avail_w - cga_w * scale) / 2;
    int dy = tb_h + 8 + (avail_h - cga_h * scale) / 2;

    painter.translate(dx, dy);
    painter.scale(scale, scale);
    painter.setClipRect(0, 0, cga_w, cga_h);
    painter.fillRect(0, 0, cga_w, cga_h, DOS_COLORS[0]);
    painter.setFont(mono);

    for (int r = 0; r < 25; ++r) {
        int abs_r = scrollback.size() + r - scroll_offset;
        
        // Note: state.screen and state.color are raw C-arrays, so they don't use .data()
        const uint8_t* chars = (abs_r < scrollback.size()) ? scrollback[abs_r].chars.data() : state.screen[abs_r - scrollback.size()];
        const uint8_t* attrs = (abs_r < scrollback.size()) ? scrollback[abs_r].attrs.data() : state.color[abs_r - scrollback.size()];

        for (int c = 0; c < 80; ++c) {
            QColor bg = DOS_COLORS[(attrs[c] >> 4) & 0x0F];
            QColor fg = DOS_COLORS[attrs[c] & 0x0F];
            QRectF cellRect(c * char_w, r * char_h, char_w, char_h);
            
            if ((attrs[c] >> 4) & 0x0F) painter.fillRect(cellRect, bg);
            if (chars[c] != 0 && chars[c] != ' ') {
                painter.setPen(fg);
                painter.drawText(cellRect.left(), cellRect.top() + fm->ascent(), QChar(chars[c]));
            }

            // Paint blinking cursor
            if (scroll_offset == 0 && r == state.cursor_y && c == state.cursor_x && cursor_visible_blink) {
                painter.fillRect(cellRect.left(), cellRect.bottom() - 2, char_w, 2, fg);
            }
        }
    }

    if (scroll_offset > 0) {
        painter.fillRect(0, 0, cga_w, char_h, QColor(28, 30, 38, 200)); // #1C1E26 with alpha
        painter.setPen(QColor(Theme::Color::TEXT));
        painter.drawText(0, fm->ascent(), QString(" ↑ Scrollback (%1 lines above)").arg(scroll_offset));
    }

    if (state.waiting_for_input) {
        painter.fillRect(0, 0, cga_w, cga_h, QColor(28, 30, 38, 153)); // #1C1E2699
        painter.setPen(QColor(Theme::Color::REG_CHANGED));             // #4AFF8C
        QString msg = "▌ Waiting for input...";
        painter.drawText((cga_w - fm->horizontalAdvance(msg)) / 2, cga_h / 2, msg);
    }

    if (hasFocus()) {
        painter.setPen(QPen(QColor(Theme::Color::ACCENT), 1.0f / scale));
        painter.drawRect(0, 0, cga_w, cga_h);
    }
}

void ConsolePanelWidget::keyPressEvent(QKeyEvent* event) {
    int key = event->key();
    QString text = event->text();
    char ch = 0;

    if (key == Qt::Key_Return || key == Qt::Key_Enter) ch = '\r';
    else if (key == Qt::Key_Backspace) ch = 0x08;
    else if (key == Qt::Key_Escape) ch = 0x1B;
    else if (!text.isEmpty() && text[0].unicode() >= 32 && text[0].unicode() < 127) ch = text[0].toLatin1();

    if (ch != 0) {
        if (!state.waiting_for_input) state.input_buffer += ch;
        if (state.on_key) state.on_key(ch);
        emit key_pressed(ch);
    } else {
        char scan1 = 0, scan2 = 0;
        if (key == Qt::Key_Up) { scan1 = 0xE0; scan2 = 0x48; }
        else if (key == Qt::Key_Down) { scan1 = 0xE0; scan2 = 0x50; }
        else if (key == Qt::Key_Left) { scan1 = 0xE0; scan2 = 0x4B; }
        else if (key == Qt::Key_Right) { scan1 = 0xE0; scan2 = 0x4D; }

        if (scan1 != 0 && state.on_key) {
            state.on_key(scan1);
            state.on_key(scan2);
        }
    }
}

void ConsolePanelWidget::wheelEvent(QWheelEvent* event) {
    if (event->angleDelta().y() > 0) scroll_offset = qMin(scroll_offset + 3, (int)scrollback.size());
    else scroll_offset = qMax(scroll_offset - 3, 0);
    
    update_scrollbar();
    update();
}

void ConsolePanelWidget::focusInEvent(QFocusEvent* event) { QWidget::focusInEvent(event); update(); }
void ConsolePanelWidget::focusOutEvent(QFocusEvent* event) { QWidget::focusOutEvent(event); update(); }

} // namespace memu8086::ui