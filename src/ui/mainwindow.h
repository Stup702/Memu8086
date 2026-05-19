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

#include <QMainWindow>
#include <QTimer>
#include <QLabel>
#include <QDockWidget>
#include <QCloseEvent>
#include <QStringList>
#include <QMenu>

#include "../core/cpu.h"
#include "../core/debugger.h"
#include "../assembler/assembler.h"

namespace memu8086::ui {
class EditorPanel;
class RegistersPanel;
class MemoryPanel;
class StackPanel;
class VariablesPanel;
class ConsolePanelWidget;
class Toolbar;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(emu8086::core::CPU& cpu, emu8086::assembler::Assembler& asm_,
               emu8086::core::Debugger& dbg, 
               emu8086::core::ConsoleState& console,
               QWidget* parent = nullptr);
    
protected:
    void closeEvent(QCloseEvent* event) override;

private slots:
    void on_assemble();
    void on_run();
    void on_stop();
    void on_step();
    void on_step_over();
    void on_reset();
    void on_open_file();
    void on_save_file();
    void on_save_as();
    void on_new_file();
    void on_debugger_tick();   // called by QTimer every ~16ms
    void on_load_example(const QString& source);
    void show_about();
    void show_settings();
    void show_examples();

private:
    void setup_menu_bar();
    void setup_toolbar();
    void setup_dock_panels();
    void reset_dock_layout();
    void setup_status_bar();
    void setup_shortcuts();
    void update_ui_state();     // sync buttons/status to debugger state
    void refresh_panels();      // update all panels with current CPU/mem state
    void update_recent_files_menu();

    emu8086::core::CPU& cpu; 
    emu8086::assembler::Assembler& asm_; 
    emu8086::core::Debugger& dbg; 
    emu8086::core::ConsoleState& console;

    // Panels
    EditorPanel*        editor;
    RegistersPanel*     registers_panel;
    MemoryPanel*        memory_panel;
    StackPanel*         stack_panel;
    VariablesPanel*     variables_panel;
    ConsolePanelWidget* console_panel;
    Toolbar*            toolbar_widget;

    // Dock widgets
    QDockWidget* dock_editor;
    QDockWidget* dock_registers;
    QDockWidget* dock_memory;
    QDockWidget* dock_stack;
    QDockWidget* dock_variables;
    QDockWidget* dock_console;

    // Status bar widgets
    QLabel* status_state;
    QLabel* status_ip;
    QLabel* status_cycles;
    QLabel* status_file;

    QMenu* recent_files_menu;

    QTimer* tick_timer;
    QString current_file;
    
    emu8086::assembler::AssemblyResult last_output;
    bool assembled = false;
    emu8086::core::CPUSnapshot prev_snapshot;
};

} // namespace memu8086::ui

using namespace memu8086::ui;
using namespace emu8086::core;
using namespace emu8086::assembler;
