#pragma once

#include <QWidget>
#include <QMap>
#include <QLabel>
#include <QTimer>
#include <QTableWidgetItem>
#include "core/cpu.h"
#include "core/emulator.h"

namespace memu8086::ui {

class FlagWidget : public QWidget {
    Q_OBJECT
public:
    FlagWidget(const QString& name, const QString& tooltip_text, QWidget* p);
    void set_value(bool val, bool changed);
protected:
    void paintEvent(QPaintEvent*) override;
    void enterEvent(QEnterEvent*) override;
    void leaveEvent(QEvent*) override;
private:
    QString flag_name, tip_text;
    bool value = false;
    bool flashing = false;
    QTimer* flash_timer;
};

class RegistersPanel : public QWidget {
    Q_OBJECT
public:
    explicit RegistersPanel(emu8086::core::CPU& cpu, QWidget* parent = nullptr);
    void refresh();
private:
    emu8086::core::CPU& cpu;
    emu8086::core::Registers prev_regs;
    
    QMap<QString, QLabel*> reg_labels;
    QMap<QString, QTableWidgetItem*> reg_hex_items;
    QMap<QString, QTableWidgetItem*> reg_dec_items;
    QMap<QString, QTableWidgetItem*> reg_bin_items;
    
    QMap<QString, FlagWidget*> flag_widgets;
    QLabel* lbl_cs_ip;
    QLabel* lbl_flags_hex;
    QLabel* lbl_cycles;
    
    void setup_ui();
    void update_reg_row(int row, const QString& name, uint16_t val, uint16_t prev_val);
};

} // namespace memu8086::ui