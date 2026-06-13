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

#include <QApplication>
#include <QFontDatabase>
#include <QIcon>
#include <memory>
#include "ui/mainwindow.h"
#include "core/cpu.h"
#include "core/debugger.h"
#include "assembler/assembler.h"
#include <QSettings>

using namespace emu8086::core;
using namespace emu8086::assembler;

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("memu8086");
    app.setApplicationVersion("1.0.3");
    app.setOrganizationName("memu8086");
    app.setWindowIcon(QIcon(":/appicon.ico"));

    // Load embedded monospace font
    QFontDatabase::addApplicationFont(":/fonts/JetBrainsMono-Regular.ttf");

    // Read and apply the saved theme
    QSettings s("memu8086", "memu8086");
    
    // Instantiate core objects on the heap to avoid stack overflow (CPU contains 1MB array)
    auto cpu = std::make_unique<CPU>();
    auto assembler = std::make_unique<Assembler>();
    auto console = std::make_unique<ConsoleState>();
    auto debugger = std::make_unique<Debugger>(*cpu, cpu->mem, *console);

    MainWindow window(*cpu, *assembler, *debugger, *console);
    window.show();
    return app.exec();
}
