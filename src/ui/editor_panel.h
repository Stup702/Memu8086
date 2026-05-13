#pragma once

#include <QWidget>
#include <QPlainTextEdit>
#include <QLineEdit>
#include <QLabel>
#include <set>
#include <vector>
#include <QMap>
#include "assembler/assembler.h"
#include "highlighter.h"

namespace memu8086::ui {

class LineNumberArea;

class EditorPanel : public QWidget {
    Q_OBJECT
public:
    explicit EditorPanel(QWidget* parent = nullptr);
    QString get_source() const;
    void set_source(const QString& src);
    bool is_modified() const;
    void mark_saved();
    void set_errors(const std::vector<emu8086::assembler::AssemblerError>& errors);
    void set_exec_line(int line);
    void set_breakpoints(const std::set<uint16_t>& bps); // Offset based bps mapping requires logic in mainwindow, or mapping here. Assuming line-based for UI.
    void set_breakpoint_lines(const std::set<int>& lines);

    void gutter_paint_event(QPaintEvent* event);
    int gutter_width();

signals:
    void breakpoint_toggled(int line);
    void source_modified();

private slots:
    void update_gutter_width();
    void highlight_current_line();
    void update_gutter(const QRect& rect, int dy);
    void on_text_changed();
    void on_cursor_position_changed();
    void show_find_replace();
    void find_next();

private:
    QPlainTextEdit* editor;
    AsmHighlighter* highlighter;
    LineNumberArea* gutter;
    QWidget* find_replace_widget;
    QLineEdit* find_input;
    QLineEdit* replace_input;
    QLabel* lbl_status;

    bool modified = false;
    int exec_line = -1;
    std::set<int> breakpoint_lines;
    QMap<int, QString> errors_map;

    void setup_editor();
    void mark_error_lines();
    bool eventFilter(QObject* obj, QEvent* event) override;
};

class LineNumberArea : public QWidget {
public:
    LineNumberArea(EditorPanel* editor, QWidget* parent) : QWidget(parent), editor_panel(editor) {}
    QSize sizeHint() const override { return QSize(editor_panel->gutter_width(), 0); }
protected:
    void paintEvent(QPaintEvent* event) override { editor_panel->gutter_paint_event(event); }
    void mousePressEvent(QMouseEvent* event) override;
private:
    EditorPanel* editor_panel;
};

} // namespace memu8086::ui