#pragma once

#include <QDialog>
#include <QTabWidget>
#include <QSpinBox>
#include <QComboBox>
#include <QCheckBox>
#include <QSlider>
#include <QButtonGroup>
#include <QTableWidget>

namespace memu8086::ui {

class SettingsDialog : public QDialog {
    Q_OBJECT
public:
    explicit SettingsDialog(QWidget* parent = nullptr);
    void load_settings();
    void save_settings();

private:
    QTabWidget* tabs;
    
    // Editor tab
    QSpinBox* font_size;
    QComboBox* tab_size;
    QCheckBox* auto_indent, *show_line_numbers, *highlight_line, *blink_cursor;
    // Emulator tab
    QSlider* default_speed;
    QSpinBox* max_instr_per_frame;
    QCheckBox* auto_reset_on_assemble, *break_on_undefined;
    // Theme tab
    QButtonGroup* theme_group;
    // Key bindings tab
    QTableWidget* keybindings_table;
};

} // namespace memu8086::ui