#include "theme.h"
#include <QFile>
#include <QTextStream>

namespace Theme {

static void load_stylesheet(QApplication& app, const QString& path) {
    QFile file(path);
    if (file.open(QFile::ReadOnly | QFile::Text)) {
        QTextStream stream(&file);
        app.setStyleSheet(stream.readAll());
        file.close();
    }
}

void apply_dark(QApplication& app) {
    load_stylesheet(app, ":/assets/styles/dark.qss");
}

void apply_light(QApplication& app) {
    load_stylesheet(app, ":/assets/styles/light.qss");
}

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