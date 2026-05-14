#include <QApplication>
#include <QFontDatabase>
#include "ui/mainwindow.h"
#include "ui/theme.h"
#include "core/cpu.h"
#include "core/debugger.h"
#include "assembler/assembler.h"
#include <QSettings>

using namespace emu8086::core;
using namespace emu8086::assembler;

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("memu8086");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("memu8086");

    // Load embedded monospace font
    QFontDatabase::addApplicationFont(":/fonts/JetBrainsMono-Regular.ttf");

    // Read and apply the saved theme
    QSettings s("memu8086", "memu8086");
    int saved_mode = s.value("theme_mode", 0).toInt();
    Theme::apply(saved_mode == 0 ? Theme::Mode::Dark : Theme::Mode::Light);

    // Instantiate core objects
    CPU cpu;
    Assembler assembler;
    ConsoleState console;
    Debugger debugger(cpu, cpu.mem, console);

    MainWindow window(cpu, assembler, debugger, console);
    window.show();
    return app.exec();
}
