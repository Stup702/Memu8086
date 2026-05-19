#include "editor_panel.h"
#include "theme.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPainter>
#include <QTextBlock>
#include <QPushButton>
#include <QShortcut>
#include <QToolTip>
#include <QHelpEvent>
#include <QWheelEvent>
#include <QKeyEvent>
#include <QSettings>
#include <QPaintEvent>

namespace memu8086::ui {

struct QPlainTextEditHack : public QPlainTextEdit {
    using QPlainTextEdit::setViewportMargins;
    using QPlainTextEdit::firstVisibleBlock;
    using QPlainTextEdit::blockBoundingGeometry;
    using QPlainTextEdit::contentOffset;
    using QPlainTextEdit::blockBoundingRect;
};
#define EDITOR_HACK ((QPlainTextEditHack*)editor)

EditorPanel::EditorPanel(QWidget* parent) : QWidget(parent) {
    setup_editor();
}

void EditorPanel::setup_editor() {
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    editor = new QPlainTextEdit(this);
    editor->setFont(Theme::mono_font(20));
    editor->setLineWrapMode(QPlainTextEdit::WidgetWidth);
    
    highlighter = new AsmHighlighter(editor->document());
    gutter = new LineNumberArea(this, editor);
    set_font_size(20); // Force initialize the fonts and inline stylesheet securely

    connect(editor, &QPlainTextEdit::blockCountChanged, this, &EditorPanel::update_gutter_width);
    connect(editor, &QPlainTextEdit::updateRequest, this, &EditorPanel::update_gutter);
    connect(editor, &QPlainTextEdit::cursorPositionChanged, this, &EditorPanel::highlight_current_line);
    connect(editor, &QPlainTextEdit::cursorPositionChanged, this, &EditorPanel::on_cursor_position_changed);
    connect(editor, &QPlainTextEdit::textChanged, this, &EditorPanel::on_text_changed);
    editor->installEventFilter(this);
    editor->viewport()->installEventFilter(this);
    gutter->installEventFilter(this);

    layout->addWidget(editor);

    // Find/Replace bar
    find_replace_widget = new QWidget(this);
    find_replace_widget->setStyleSheet(QStringLiteral("background: %1; border-top: 1px solid %2;").arg(Theme::Color::PANEL, Theme::Color::BORDER));
    QHBoxLayout* fr_layout = new QHBoxLayout(find_replace_widget);
    fr_layout->setContentsMargins(8, 4, 8, 4);
    find_input = new QLineEdit();
    find_input->setPlaceholderText("Find...");
    replace_input = new QLineEdit();
    replace_input->setPlaceholderText("Replace...");
    
    QPushButton* btn_prev = new QPushButton("◀");
    QPushButton* btn_next = new QPushButton("▶");
    QPushButton* btn_replace = new QPushButton("Replace");
    QPushButton* btn_close = new QPushButton("✕");
    btn_close->setFlat(true);
    
    fr_layout->addWidget(find_input);
    fr_layout->addWidget(btn_prev);
    fr_layout->addWidget(btn_next);
    fr_layout->addWidget(replace_input);
    fr_layout->addWidget(btn_replace);
    fr_layout->addStretch();
    fr_layout->addWidget(btn_close);
    
    find_replace_widget->hide();
    layout->addWidget(find_replace_widget);

    connect(btn_close, &QPushButton::clicked, find_replace_widget, &QWidget::hide);
    connect(btn_next, &QPushButton::clicked, this, &EditorPanel::find_next);
    
    QShortcut* shortcut_find = new QShortcut(QKeySequence("Ctrl+F"), this);
    connect(shortcut_find, &QShortcut::activated, this, &EditorPanel::show_find_replace);

    // Status bar
    lbl_status = new QLabel(this);
    lbl_status->setStyleSheet(QStringLiteral("background: %1; color: %2; padding: 2px 6px; border-top: 1px solid %3;")
                              .arg(Theme::Color::EDITOR_STATUS_BG, Theme::Color::EDITOR_STATUS_TEXT, Theme::Color::BORDER));
    lbl_status->setFont(Theme::ui_font(11));
    layout->addWidget(lbl_status);

    update_gutter_width();
    highlight_current_line();
    on_cursor_position_changed();
}

QString EditorPanel::get_source() const { return editor->toPlainText(); }
void EditorPanel::set_source(const QString& src) { editor->setPlainText(src); modified = false; on_cursor_position_changed(); }
bool EditorPanel::is_modified() const { return modified; }
void EditorPanel::mark_saved() { modified = false; on_cursor_position_changed(); }

void EditorPanel::set_errors(const std::vector<emu8086::assembler::AssemblerError>& errors) {
    errors_map.clear();
    for (const auto& err : errors) {
        errors_map[err.line] = QString::fromStdString(err.message);
    }
    highlight_current_line();
    gutter->update();
}

void EditorPanel::set_exec_line(int line) { 
    exec_line = line; 
    gutter->update(); 
    highlight_current_line(); 

    // Automatically scroll the editor to follow the execution line!
    if (line > 0) {
        QTextBlock block = editor->document()->findBlockByLineNumber(line - 1);
        if (block.isValid()) {
            editor->setTextCursor(QTextCursor(block));
            editor->ensureCursorVisible();
        }
    }
}
void EditorPanel::set_breakpoint_lines(const std::set<int>& lines) { breakpoint_lines = lines; gutter->update(); }

int EditorPanel::gutter_width() {
    int digits = 1;
    int max = qMax(1, editor->blockCount());
    while (max >= 10) { max /= 10; ++digits; }
    int space = 24 + editor->fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;
    return space;
}

void EditorPanel::update_gutter_width() {
    int gw = gutter_width();
    EDITOR_HACK->setViewportMargins(gw, 0, 0, 0);
    QRect cr = editor->contentsRect();
    gutter->setGeometry(QRect(cr.left(), cr.top(), gw, cr.height()));
}

void EditorPanel::update_gutter(const QRect& rect, int dy) {
    if (dy) gutter->scroll(0, dy);
    else gutter->update(0, rect.y(), gutter->width(), rect.height());
    if (rect.contains(editor->viewport()->rect())) update_gutter_width();
}

void EditorPanel::gutter_paint_event(QPaintEvent* event) {
    QPainter painter(gutter);
    painter.fillRect(event->rect(), QColor(Theme::Color::BG));
    painter.setFont(gutter->font());

    QTextBlock block = EDITOR_HACK->firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = (int) EDITOR_HACK->blockBoundingGeometry(block).translated(EDITOR_HACK->contentOffset()).top();
    int bottom = top + (int) EDITOR_HACK->blockBoundingRect(block).height();

    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            int line = blockNumber + 1;
            QString number = QString::number(line);
            painter.setPen(line == editor->textCursor().blockNumber() + 1 ? QColor(Theme::Color::TEXT) : QColor(Theme::Color::TEXT_MUTED));
            int block_height = (int)EDITOR_HACK->blockBoundingRect(block).height();
            painter.drawText(0, top, gutter->width() - 8, block_height, Qt::AlignRight | Qt::AlignVCenter, number);

            if (breakpoint_lines.count(line)) {
                painter.setBrush(QColor(Theme::Color::BREAKPOINT));
                painter.setPen(Qt::NoPen);
                painter.drawEllipse(QPoint(8, top + block_height / 2), 4, 4);
            }
            if (line == exec_line) {
                QPolygon poly;
                int y_c = top + block_height / 2;
                poly << QPoint(4, y_c - 4) << QPoint(12, y_c) << QPoint(4, y_c + 4);
                painter.setBrush(QColor(Theme::Color::WARNING));
                painter.setPen(Qt::NoPen);
                painter.drawPolygon(poly);
            }
            if (errors_map.contains(line)) {
                painter.setPen(QPen(QColor(Theme::Color::ERROR), 2));
                int y_c = top + block_height / 2;
                painter.drawLine(4, y_c - 3, 10, y_c + 3);
                painter.drawLine(10, y_c - 3, 4, y_c + 3);
            }
        }
        block = block.next();
        top = bottom;
        bottom = top + (int) EDITOR_HACK->blockBoundingRect(block).height();
        ++blockNumber;
    }
}

void EditorPanel::highlight_current_line() {
    QList<QTextEdit::ExtraSelection> extraSelections;
    
    // 1. Highlight execution line
    if (exec_line > 0) {
        QTextBlock block = editor->document()->findBlockByLineNumber(exec_line - 1);
        if (block.isValid()) {
            QTextEdit::ExtraSelection exec_sel;
            exec_sel.format.setBackground(QColor(Theme::Color::EXEC_LINE));
            exec_sel.format.setProperty(QTextFormat::FullWidthSelection, true);
            exec_sel.cursor = QTextCursor(block);
            exec_sel.cursor.clearSelection();
            extraSelections.append(exec_sel);
        }
    }

    // 2. Highlight text cursor (skip if currently on the execution line so it doesn't overlap)
    if (!editor->isReadOnly()) {
        QTextEdit::ExtraSelection selection;
        selection.format.setBackground(QColor(Theme::Color::CURRENT_LINE));
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = editor->textCursor();
        selection.cursor.clearSelection();
        if (selection.cursor.blockNumber() + 1 != exec_line) extraSelections.append(selection);
    }
    for (auto it = errors_map.begin(); it != errors_map.end(); ++it) {
        int line = it.key();
        QTextBlock block = editor->document()->findBlockByLineNumber(line - 1);
        if (block.isValid()) {
            QTextEdit::ExtraSelection err_sel;
            err_sel.format.setUnderlineColor(QColor(Theme::Color::ERROR));
            err_sel.format.setUnderlineStyle(QTextCharFormat::WaveUnderline);
            err_sel.cursor = QTextCursor(block);
            err_sel.cursor.clearSelection();
            extraSelections.append(err_sel);
        }
    }

    editor->setExtraSelections(extraSelections);
}

void EditorPanel::mark_error_lines() {} // Implemented via highlighter or block formats ideally, simplified for brevity here
bool EditorPanel::eventFilter(QObject* obj, QEvent* event) {
    if (obj == editor && event->type() == QEvent::Resize) {
        update_gutter_width();
    }
    else if ((obj == editor || obj == editor->viewport()) && event->type() == QEvent::Wheel) {
        QWheelEvent* wheelEvent = static_cast<QWheelEvent*>(event);
        if (wheelEvent->modifiers() & Qt::ControlModifier) {
            zoom(wheelEvent->angleDelta().y() > 0 ? 1 : -1);
            return true;
        }
    }
    else if (obj == editor && event->type() == QEvent::KeyPress) {
        auto* ke = static_cast<QKeyEvent*>(event);
        if (ke->modifiers() & Qt::ControlModifier) {
            if (ke->key() == Qt::Key_Equal || ke->key() == Qt::Key_Plus) {
                zoom(1);
                return true; // consume — don't insert character
            }
            if (ke->key() == Qt::Key_Minus) {
                zoom(-1);
                return true;
            }
            if (ke->key() == Qt::Key_0) {
                set_font_size(20);
                return true;
            }
        }
        else if (ke->key() == Qt::Key_Return || ke->key() == Qt::Key_Enter) {
            QSettings s("memu8086", "memu8086");
            if (s.value("editor/auto_indent", true).toBool()) {
                QTextCursor cursor = editor->textCursor();
                QString line = cursor.block().text();
                QString whitespace;
                for (QChar c : line) {
                    if (c == ' ' || c == '\t') whitespace += c;
                    else break;
                }
                cursor.insertText("\n" + whitespace);
                editor->setTextCursor(cursor);
                return true;
            }
        }
    }
    else if (event->type() == QEvent::ToolTip) {
        QHelpEvent* helpEvent = static_cast<QHelpEvent*>(event);
        int line = -1;
        
        if (obj == gutter) {
            QTextBlock block = EDITOR_HACK->firstVisibleBlock();
            int top = (int)EDITOR_HACK->blockBoundingGeometry(block).translated(EDITOR_HACK->contentOffset()).top();
            while (block.isValid() && top <= gutter->height()) {
                int bottom = top + (int)EDITOR_HACK->blockBoundingRect(block).height();
                if (block.isVisible() && helpEvent->pos().y() >= top && helpEvent->pos().y() <= bottom) {
                    line = block.blockNumber() + 1;
                    break;
                }
                block = block.next();
                top = bottom;
            }
        } else if (obj == editor->viewport()) {
            line = editor->cursorForPosition(helpEvent->pos()).blockNumber() + 1;
        }
        
        if (line != -1 && errors_map.contains(line)) {
            QToolTip::showText(helpEvent->globalPos(), "<b>Error:</b> " + errors_map[line].toHtmlEscaped(), static_cast<QWidget*>(obj));
            return true;
        } else {
            QToolTip::hideText();
        }
    }
    return QWidget::eventFilter(obj, event);
}

void EditorPanel::zoom(int delta) {
    QFont f = editor->font();
    int current_size = f.pixelSize() > 0 ? f.pixelSize() : (f.pointSize() > 0 ? f.pointSize() : 13);
    set_font_size(current_size + delta);
}

void EditorPanel::set_font_size(int size) {
    int new_size = qBound(8, size, 48);
    
    // Qt Style Sheets forcefully override QWidget::setFont, so we MUST update the inline stylesheet to break free!
    editor->setStyleSheet(QStringLiteral("QPlainTextEdit { selection-background-color: %1; selection-color: #FFFFFF; font-size: %2px; }")
                          .arg(Theme::Color::ACCENT).arg(new_size));
    
    QFont f = editor->font();
    f.setPixelSize(new_size);
    editor->setFont(f);
    gutter->setFont(f);
    
    QFontMetrics m(f);
    editor->setTabStopDistance(4 * m.horizontalAdvance(' '));
    update_gutter_width();
}

void EditorPanel::on_text_changed() { modified = true; emit source_modified(); on_cursor_position_changed(); }
void EditorPanel::show_find_replace() { find_replace_widget->show(); find_input->setFocus(); }

void EditorPanel::find_next() { editor->find(find_input->text()); }

void EditorPanel::on_cursor_position_changed() {
    QTextCursor cursor = editor->textCursor();
    lbl_status->setText(QStringLiteral("Ln %1, Col %2  |  %3 lines  |  %4")
        .arg(cursor.blockNumber() + 1).arg(cursor.columnNumber() + 1).arg(editor->blockCount()).arg(modified ? "●" : ""));
}

void LineNumberArea::mousePressEvent(QMouseEvent* event) {
    QTextBlock block = editor_panel->findChild<QPlainTextEdit*>()->cursorForPosition(QPoint(0, event->pos().y())).block();
    emit editor_panel->breakpoint_toggled(block.blockNumber() + 1);
}

} // namespace memu8086::ui