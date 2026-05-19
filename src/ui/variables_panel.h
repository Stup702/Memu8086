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
#include <QTableWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QList>
#include <QMap>
#include <map>
#include <string>
#include "../core/cpu.h"
#include "../assembler/assembler.h"

namespace memu8086::ui {

class VariablesPanel : public QWidget {
    Q_OBJECT
public:
    explicit VariablesPanel(QWidget* parent = nullptr);

    void set_symbols(const std::map<std::string, uint16_t>& syms);
    void update(const emu8086::core::CPU& cpu);

    // Aliases to seamlessly attach to MainWindow
    void refresh(const emu8086::assembler::AssemblyResult& res);
    void refresh();

signals:
    void show_in_memory(uint32_t addr);

private slots:
    void on_add_watch();
    void remove_watch(const QString& name);
    void show_context_menu(const QPoint& pos);

private:
    QTableWidget* table;
    QLineEdit* watch_name;
    QLineEdit* watch_addr;
    QComboBox* watch_size;
    QPushButton* btn_add;

    struct WatchEntry { QString name; uint32_t addr; int size; };
    QList<WatchEntry> watches;
    std::map<std::string, uint16_t> symbols;
    QMap<QString, uint16_t> prev_values;

    void rebuild_table(const emu8086::core::CPU& cpu);
};

} // namespace memu8086::ui