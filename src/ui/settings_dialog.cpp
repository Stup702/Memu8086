#include "settings_dialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QSettings>
#include <QFormLayout>
#include <QRadioButton>
#include <QApplication>
#include "theme.h"

namespace memu8086::ui {

SettingsDialog::SettingsDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle("Settings");
    setMinimumSize(400, 300);

    QVBoxLayout* main_layout = new QVBoxLayout(this);
    tabs = new QTabWidget(this);

    // Editor Tab
    QWidget* editor_tab = new QWidget();
    QFormLayout* editor_layout = new QFormLayout(editor_tab);
    font_size = new QSpinBox(); font_size->setRange(8, 36);
    tab_size = new QComboBox(); tab_size->addItems({"2", "4", "8"});
    auto_indent = new QCheckBox("Auto-indent");
    show_line_numbers = new QCheckBox("Show line numbers");
    highlight_line = new QCheckBox("Highlight current line");
    blink_cursor = new QCheckBox("Blink cursor");
    editor_layout->addRow("Font Size:", font_size);
    editor_layout->addRow("Tab Size:", tab_size);
    editor_layout->addRow(auto_indent);
    editor_layout->addRow(show_line_numbers);
    editor_layout->addRow(highlight_line);
    editor_layout->addRow(blink_cursor);
    tabs->addTab(editor_tab, "Editor");

    // Emulator Tab
    QWidget* emu_tab = new QWidget();
    QFormLayout* emu_layout = new QFormLayout(emu_tab);
    default_speed = new QSlider(Qt::Horizontal); default_speed->setRange(0, 100);
    max_instr_per_frame = new QSpinBox(); max_instr_per_frame->setRange(100, 100000);
    auto_reset_on_assemble = new QCheckBox("Auto-reset on assemble");
    break_on_undefined = new QCheckBox("Break on undefined instruction");
    emu_layout->addRow("Default Speed:", default_speed);
    emu_layout->addRow("Max Instr/Frame:", max_instr_per_frame);
    emu_layout->addRow(auto_reset_on_assemble);
    emu_layout->addRow(break_on_undefined);
    tabs->addTab(emu_tab, "Emulator");

    // Theme Tab
    QWidget* theme_tab = new QWidget();
    QVBoxLayout* theme_layout = new QVBoxLayout(theme_tab);
    theme_group = new QButtonGroup(this);
    QRadioButton* rb_dark = new QRadioButton("Dark Theme");
    QRadioButton* rb_light = new QRadioButton("Light Theme");
    theme_group->addButton(rb_dark, 0);
    theme_group->addButton(rb_light, 1);
    theme_layout->addWidget(rb_dark);
    theme_layout->addWidget(rb_light);
    theme_layout->addStretch();
    tabs->addTab(theme_tab, "Theme");

    // Key bindings Tab
    QWidget* keys_tab = new QWidget();
    QVBoxLayout* keys_layout = new QVBoxLayout(keys_tab);
    keybindings_table = new QTableWidget(0, 2);
    keybindings_table->setHorizontalHeaderLabels({"Action", "Shortcut"});
    keys_layout->addWidget(keybindings_table);
    tabs->addTab(keys_tab, "Key Bindings");

    main_layout->addWidget(tabs);

    QHBoxLayout* btn_layout = new QHBoxLayout();
    QPushButton* btn_ok = new QPushButton("OK");
    QPushButton* btn_cancel = new QPushButton("Cancel");
    QPushButton* btn_apply = new QPushButton("Apply");
    btn_layout->addStretch();
    btn_layout->addWidget(btn_ok);
    btn_layout->addWidget(btn_cancel);
    btn_layout->addWidget(btn_apply);
    main_layout->addLayout(btn_layout);

    connect(btn_ok, &QPushButton::clicked, this, [this]() { save_settings(); accept(); });
    connect(btn_cancel, &QPushButton::clicked, this, &QDialog::reject);
    connect(btn_apply, &QPushButton::clicked, this, &SettingsDialog::save_settings);

    load_settings();
}

void SettingsDialog::load_settings() {
    QSettings s("memu8086", "memu8086");

    // Editor settings
    font_size->setValue(s.value("editor/font_size", 13).toInt());
    highlight_line->setChecked(s.value("editor/highlight_line", true).toBool());
    auto_indent->setChecked(s.value("editor/auto_indent", true).toBool());

    // Theme
    int mode = s.value("theme_mode", 0).toInt(); // 0=dark, 1=light
    QAbstractButton* btn = theme_group->button(mode);
    if (btn) btn->setChecked(true);
}

void SettingsDialog::save_settings() {
    QSettings s("memu8086", "memu8086");

    // Editor settings
    s.setValue("editor/font_size", font_size->value());
    s.setValue("editor/highlight_line", highlight_line->isChecked());
    s.setValue("editor/auto_indent", auto_indent->isChecked());

    // Theme — apply immediately
    int mode_id = theme_group->checkedId(); // 0=dark, 1=light
    Theme::apply(mode_id == 0 ? Theme::Mode::Dark : Theme::Mode::Light);

    // Emit signal so MainWindow can react (update editor font size etc.)
    emit settings_applied();
}

} // namespace memu8086::ui