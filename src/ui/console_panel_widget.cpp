#include "console_panel_widget.h"
#include "theme.h"
#include "../core/debugger.h"
#include <QKeyEvent>
#include <QPainter>
#include <QDockWidget>
#include <QVBoxLayout>
#include <QTimer>

namespace memu8086::ui {

class ConsoleRenderWidget : public QPlainTextEdit {
public:
    ConsoleRenderWidget(emu8086::core::ConsoleState& state, ConsolePanelWidget* panel) 
        : QPlainTextEdit(panel), state(state), panel(panel) {
        setFocusPolicy(Qt::StrongFocus);
        setCursor(Qt::IBeamCursor);
        setReadOnly(true); // Disable native editing completely
        setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        
        QTimer* blink_timer = new QTimer(this);
        connect(blink_timer, &QTimer::timeout, this, [this]() {
            cursor_blink = !cursor_blink;
            viewport()->update();
        });
        blink_timer->start(500);
    }
    
protected:
    void paintEvent(QPaintEvent*) override {
        QPainter p(viewport());
        p.fillRect(viewport()->rect(), QColor("#0C0C0C"));
        
        QFont font = Theme::mono_font(14);
        p.setFont(font);
        
        QFontMetrics fm(font);
        int char_h = fm.height();
        int ascent = fm.ascent();
        
        p.setPen(QColor("#CCCCCC"));
        for (int r = 0; r < state.rows; ++r) {
            QString line;
            for (int c = 0; c < state.cols; ++c) {
                char ch = state.screen[r][c];
                line += (ch >= 32 && ch < 127) ? QChar(ch) : QChar(' ');
            }
            p.drawText(4, r * char_h + ascent + 4, line);
        }
        
        if (hasFocus() && cursor_blink) {
            int cx = state.cursor_x;
            int cy = state.cursor_y;
            if (cx >= 0 && cx <= state.cols && cy >= 0 && cy < state.rows) {
                // Mathematically calculate the exact string width up to the cursor to guarantee perfect alignment!
                QString text_before;
                int calc_cx = qMin(cx, state.cols);
                for (int c = 0; c < calc_cx; ++c) {
                    char ch = state.screen[cy][c];
                    text_before += (ch >= 32 && ch < 127) ? QChar(ch) : QChar(' ');
                }
                int cursor_px = 4 + fm.horizontalAdvance(text_before);
                
                char under = (cx < state.cols) ? state.screen[cy][cx] : ' ';
                QChar under_char = (under >= 32 && under < 127) ? QChar(under) : QChar(' ');
                int cursor_w = fm.horizontalAdvance(under_char);
                if (cursor_w == 0) cursor_w = fm.horizontalAdvance(QLatin1Char('0'));

                p.fillRect(cursor_px, cy * char_h + 4, cursor_w, char_h, QColor("#CCCCCC"));
                if (under_char != ' ') {
                    p.setPen(QColor("#0C0C0C"));
                    p.drawText(cursor_px, cy * char_h + ascent + 4, QString(under_char));
                }
            }
        }
    }
    
    void keyPressEvent(QKeyEvent* event) override {
        int key = event->key();
        QString text = event->text();
        char ch = 0;

        if (key == Qt::Key_Return || key == Qt::Key_Enter) ch = '\r';
        else if (key == Qt::Key_Backspace) ch = 0x08;
        else if (key == Qt::Key_Escape) ch = 0x1B;
        else if (key == Qt::Key_Tab) ch = '\t';
        else if (!text.isEmpty()) {
            ch = static_cast<char>(text[0].unicode() & 0xFF);
        }

        if (ch != 0) {
            emit panel->key_pressed(ch);
        } else {
            char scan1 = 0, scan2 = 0;
            if (key == Qt::Key_Up) { scan1 = 0xE0; scan2 = 0x48; }
            else if (key == Qt::Key_Down) { scan1 = 0xE0; scan2 = 0x50; }
            else if (key == Qt::Key_Left) { scan1 = 0xE0; scan2 = 0x4B; }
            else if (key == Qt::Key_Right) { scan1 = 0xE0; scan2 = 0x4D; }
            else if (key == Qt::Key_Insert) { scan1 = 0xE0; scan2 = 0x52; }
            else if (key == Qt::Key_Delete) { scan1 = 0xE0; scan2 = 0x53; }
            else if (key == Qt::Key_Home) { scan1 = 0xE0; scan2 = 0x47; }
            else if (key == Qt::Key_End) { scan1 = 0xE0; scan2 = 0x4F; }
            else if (key == Qt::Key_PageUp) { scan1 = 0xE0; scan2 = 0x49; }
            else if (key == Qt::Key_PageDown) { scan1 = 0xE0; scan2 = 0x51; }
            else if (key >= Qt::Key_F1 && key <= Qt::Key_F10) { scan1 = 0x00; scan2 = 0x3B + (key - Qt::Key_F1); }
            else if (key == Qt::Key_F11) { scan1 = 0x00; scan2 = static_cast<char>(0x85); }
            else if (key == Qt::Key_F12) { scan1 = 0x00; scan2 = static_cast<char>(0x86); }

            if (scan1 != 0 || scan2 != 0) {
                emit panel->key_pressed(scan1);
                emit panel->key_pressed(scan2);
            }
        }
        
        cursor_blink = true;
        viewport()->update();
    }
    
    void mousePressEvent(QMouseEvent*) override {
        setFocus();
    }
    
    void resizeEvent(QResizeEvent* event) override {
        QPlainTextEdit::resizeEvent(event);
        QFontMetrics fm(Theme::mono_font(14));
        int new_cols = (viewport()->width() - 8) / fm.horizontalAdvance('0');
        int new_rows = (viewport()->height() - 8) / fm.height();
        state.resize(new_cols, new_rows);
    }
    
private:
    emu8086::core::ConsoleState& state;
    ConsolePanelWidget* panel;
    bool cursor_blink = true;
};

ConsolePanelWidget::ConsolePanelWidget(emu8086::core::ConsoleState& state, QWidget* parent)
    : QWidget(parent), state(state) {
    
    QVBoxLayout* main_layout = new QVBoxLayout(this);
    main_layout->setContentsMargins(0, 0, 0, 0);

    terminal = new ConsoleRenderWidget(state, this);
    main_layout->addWidget(terminal);
}

QSize ConsolePanelWidget::sizeHint() const {
    QFontMetrics fm(Theme::mono_font(14));
    return QSize(80 * fm.horizontalAdvance('0') + 20, 25 * fm.height() + 20);
}

void ConsolePanelWidget::refresh() {
    QString text;
    for (int r = 0; r < state.rows; ++r) {
        QString line;
        for (int c = 0; c < state.cols; ++c) {
            char ch = state.screen[r][c];
            line += (ch >= 32 && ch < 127) ? QChar(ch) : QChar(' ');
        }
        text += line + "\n";
    }

    bool text_changed = (text != last_render);
    if (text_changed) {
        bool initial_render = last_render.isEmpty();
        last_render = text;
        
        if (!initial_render) {
            QWidget* p = this;
            while (p && !qobject_cast<QDockWidget*>(p)) {
                p = p->parentWidget();
            }
            if (p && !p->isVisible()) {
                p->show();
                p->raise();
            }
        }
    }
    
    terminal->viewport()->update();
}

bool ConsolePanelWidget::eventFilter(QObject* obj, QEvent* event) {
    return QWidget::eventFilter(obj, event);
}

} // namespace memu8086::ui