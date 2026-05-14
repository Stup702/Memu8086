#include "console_panel_widget.h"
#include "theme.h"
#include "../core/debugger.h"
#include <QKeyEvent>
#include <QVBoxLayout>

namespace memu8086::ui {

ConsolePanelWidget::ConsolePanelWidget(emu8086::core::ConsoleState& state, QWidget* parent)
    : QWidget(parent), state(state) {
    
    QVBoxLayout* main_layout = new QVBoxLayout(this);
    main_layout->setContentsMargins(0, 0, 0, 0);

    terminal = new QPlainTextEdit(this);
    terminal->setReadOnly(true);
    terminal->setFont(Theme::mono_font(13));
    terminal->setStyleSheet("QPlainTextEdit { background-color: #0C0C0C; color: #CCCCCC; border: none; }");
    terminal->setWordWrapMode(QTextOption::NoWrap);
    
    terminal->installEventFilter(this);
    
    main_layout->addWidget(terminal);
    
    resize(800, 600);
}

void ConsolePanelWidget::refresh() {
    QString text;
    for (int r = 0; r < 25; ++r) {
        QString line;
        for (int c = 0; c < 80; ++c) {
            char ch = state.screen[r][c];
            line += (ch >= 32 && ch < 127) ? QChar(ch) : QChar(' ');
        }
        text += line + "\n";
    }

    if (text != last_render) {
        terminal->setPlainText(text);
        last_render = text;
        
        QTextCursor cursor = terminal->textCursor();
        cursor.setPosition(0);
        cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, state.cursor_y);
        cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, state.cursor_x);
        terminal->setTextCursor(cursor);
    }
}

bool ConsolePanelWidget::eventFilter(QObject* obj, QEvent* event) {
    if (obj == terminal && event->type() == QEvent::KeyPress) {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
        int key = keyEvent->key();
        QString text = keyEvent->text();
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
        return true; // Consume event to prevent default QPlainTextEdit typing behavior
    }
    return QWidget::eventFilter(obj, event);
}

} // namespace memu8086::ui