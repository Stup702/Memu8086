// memu8086
// Copyright (C) 2026 Animesh Barua Mugdha
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <QWidget>
#include <QMap>
#include <QLabel>
#include <QTimer>
#include <QTableWidgetItem>
#include "../core/cpu.h"

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