#include "theme.h"
#include <QFile>
#include <QTextStream>
#include <QSettings>

namespace Theme {

static Theme::Mode s_current_mode = Theme::Mode::Dark;

void apply(Theme::Mode mode) {
    s_current_mode = mode;
    QString path = (mode == Mode::Dark)
        ? QStringLiteral(":/assets/styles/dark.qss")
        : QStringLiteral(":/assets/styles/light.qss");

    QFile file(path);
    if (file.open(QFile::ReadOnly | QFile::Text)) {
        qApp->setStyleSheet(QTextStream(&file).readAll());
    }
    // Persist to settings
    QSettings("memu8086", "memu8086").setValue("theme_mode", (int)mode);
}

Theme::Mode current_mode() { return s_current_mode; }

void apply_dark(QApplication&) { apply(Mode::Dark); }

void apply_light(QApplication&) { apply(Mode::Light); }

QFont mono_font(int size) {
    QFont font("JetBrains Mono", size);
    font.setStyleHint(QFont::Monospace);
    font.setFixedPitch(true);
    return font;
}

QFont ui_font(int size) {
    QFont font("Inter", size);
    font.setFamilies({"Inter", "Segoe UI", "sans-serif"});
    return font;
}

} // namespace Theme