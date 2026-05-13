#include <QApplication>
#include <QFontDatabase>
#include "ui/mainwindow.h"
#include "ui/theme.h"
#include "core/cpu.h"
#include "core/debugger.h"
#include "assembler/assembler.h"

using namespace emu8086::core;
using namespace emu8086::assembler;

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("memu8086");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("memu8086");

    // Load embedded monospace font
    QFontDatabase::addApplicationFont(":/fonts/JetBrainsMono-Regular.ttf");

    // Apply dark theme stylesheet
    Theme::apply_dark(app);

    // Instantiate core objects
    CPU cpu;
    Assembler assembler;
    ConsoleState console;
    Debugger debugger(cpu, cpu.mem, console);

    MainWindow window(cpu, assembler, debugger, console);
    window.show();
    return app.exec();
}
