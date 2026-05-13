#pragma once

#include <QWidget>
#include <QFont>
#include <QFontMetrics>
#include <QTimer>
#include <QList>
#include <QScrollBar>
#include <QLabel>
#include <array>

namespace emu8086::core {
    struct ConsoleState;
}

namespace memu8086::ui {

class ConsolePanelWidget : public QWidget {
    Q_OBJECT
public:
    explicit ConsolePanelWidget(emu8086::core::ConsoleState& state, QWidget* parent = nullptr);
    ~ConsolePanelWidget() override;

    void refresh(); // call every frame — repaints the screen

signals:
    void key_pressed(char c);

protected:
    void paintEvent(QPaintEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void focusInEvent(QFocusEvent* event) override;
    void focusOutEvent(QFocusEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

private:
    emu8086::core::ConsoleState& state;
    QFont mono;
    QFontMetrics* fm;
    int char_w, char_h;
    QTimer* cursor_blink_timer;
    bool cursor_visible_blink = true;

    // Scrollback
    struct ScrollbackRow { std::array<uint8_t, 80> chars; std::array<uint8_t, 80> attrs; };
    QList<ScrollbackRow> scrollback; // max 500 rows
    int scroll_offset = 0; // 0 = show live screen, >0 = show scrollback
    QScrollBar* vscrollbar;
    
    QLabel* lbl_status;
    QWidget* toolbar;

    void push_scrollback();
    void update_scrollbar();
};

} // namespace memu8086::ui