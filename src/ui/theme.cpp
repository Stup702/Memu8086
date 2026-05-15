#include "theme.h"
#include <QFile>
#include <QTextStream>
#include <QSettings>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDir>
#include <QApplication>

namespace Theme {

static QString s_current_theme = "Dark";

QString current_theme() { return s_current_theme; }

void reset_dark_colors() {
    Color::BG = "#1C1E26"; Color::PANEL = "#22243A"; Color::HEADER = "#2A2D45";
    Color::ACTIVE = "#353A5C"; Color::ACCENT = "#4A9EFF"; Color::ACCENT_HOVER = "#6DB3FF";
    Color::ACCENT_PRESSED = "#3A8EEF"; Color::TEXT = "#C8C9D4"; Color::TEXT_MUTED = "#6B6D80";
    Color::TEXT_DIM = "#44465A"; Color::BORDER = "#353A5C"; Color::REG_CHANGED = "#4AFF8C";
    Color::ERROR = "#FF5A5A"; Color::WARNING = "#FFB347"; Color::BREAKPOINT = "#FF4F4F";
    Color::EXEC_LINE = "#2D5A2D"; Color::CURRENT_LINE = "#292B38"; Color::ALT_BG = "#1E2030";
    Color::EDITOR_BG = "#141620"; Color::SYN_LABEL = "#C984FF"; Color::SYN_DIRECTIVE = "#FF7E7E";
    Color::SYN_MNEMONIC = "#4A9EFF"; Color::SYN_REGISTER = "#FF9E64"; Color::SYN_NUMBER = "#B5EAD7";
    Color::SYN_COMMENT = "#6B7D6A"; Color::SYN_STRING = "#F7C948"; Color::MEM_ZERO = "#3A3C50";
    Color::EDITOR_STATUS_BG = "#2A2D45"; Color::EDITOR_STATUS_TEXT = "#6B6D80";
    layout = LayoutConfig();
}

void save_config(const QString& path) {
    QJsonObject colors;
    colors["BG"] = Color::BG; colors["PANEL"] = Color::PANEL; colors["HEADER"] = Color::HEADER;
    colors["ACTIVE"] = Color::ACTIVE; colors["ACCENT"] = Color::ACCENT; colors["ACCENT_HOVER"] = Color::ACCENT_HOVER;
    colors["ACCENT_PRESSED"] = Color::ACCENT_PRESSED; colors["TEXT"] = Color::TEXT;
    colors["TEXT_MUTED"] = Color::TEXT_MUTED; colors["TEXT_DIM"] = Color::TEXT_DIM;
    colors["BORDER"] = Color::BORDER; colors["REG_CHANGED"] = Color::REG_CHANGED;
    colors["ERROR"] = Color::ERROR; colors["WARNING"] = Color::WARNING;
    colors["BREAKPOINT"] = Color::BREAKPOINT; colors["EXEC_LINE"] = Color::EXEC_LINE;
    colors["CURRENT_LINE"] = Color::CURRENT_LINE; colors["ALT_BG"] = Color::ALT_BG;
    colors["EDITOR_BG"] = Color::EDITOR_BG; colors["SYN_LABEL"] = Color::SYN_LABEL;
    colors["SYN_DIRECTIVE"] = Color::SYN_DIRECTIVE; colors["SYN_MNEMONIC"] = Color::SYN_MNEMONIC;
    colors["SYN_REGISTER"] = Color::SYN_REGISTER; colors["SYN_NUMBER"] = Color::SYN_NUMBER;
    colors["SYN_COMMENT"] = Color::SYN_COMMENT; colors["SYN_STRING"] = Color::SYN_STRING;
    colors["MEM_ZERO"] = Color::MEM_ZERO;
    colors["EDITOR_STATUS_BG"] = Color::EDITOR_STATUS_BG;
    colors["EDITOR_STATUS_TEXT"] = Color::EDITOR_STATUS_TEXT;

    QJsonObject lay;
    lay["reg_w"] = layout.reg_w; lay["ed_w"] = layout.ed_w;
    lay["var_w"] = layout.var_w; lay["var_h"] = layout.var_h;
    lay["ed_h"] = layout.ed_h; lay["mem_h"] = layout.mem_h;
    lay["stack_h"] = layout.stack_h;
    lay["console_h"] = layout.console_h;

    QJsonObject root; root["colors"] = colors; root["layout"] = lay;
    QFile file(path);
    if (file.open(QIODevice::WriteOnly)) file.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
}

void load_config(const QString& path) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) return;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (doc.isObject()) {
        QJsonObject root = doc.object();
        if (root.contains("colors")) {
            QJsonObject c = root["colors"].toObject();
            if (c.contains("BG")) Color::BG = c["BG"].toString();
            if (c.contains("PANEL")) Color::PANEL = c["PANEL"].toString();
            if (c.contains("HEADER")) Color::HEADER = c["HEADER"].toString();
            if (c.contains("ACTIVE")) Color::ACTIVE = c["ACTIVE"].toString();
            if (c.contains("ACCENT")) Color::ACCENT = c["ACCENT"].toString();
            if (c.contains("ACCENT_HOVER")) Color::ACCENT_HOVER = c["ACCENT_HOVER"].toString();
            if (c.contains("ACCENT_PRESSED")) Color::ACCENT_PRESSED = c["ACCENT_PRESSED"].toString();
            if (c.contains("TEXT")) Color::TEXT = c["TEXT"].toString();
            if (c.contains("TEXT_MUTED")) Color::TEXT_MUTED = c["TEXT_MUTED"].toString();
            if (c.contains("TEXT_DIM")) Color::TEXT_DIM = c["TEXT_DIM"].toString();
            if (c.contains("BORDER")) Color::BORDER = c["BORDER"].toString();
            if (c.contains("REG_CHANGED")) Color::REG_CHANGED = c["REG_CHANGED"].toString();
            if (c.contains("ERROR")) Color::ERROR = c["ERROR"].toString();
            if (c.contains("WARNING")) Color::WARNING = c["WARNING"].toString();
            if (c.contains("BREAKPOINT")) Color::BREAKPOINT = c["BREAKPOINT"].toString();
            if (c.contains("EXEC_LINE")) Color::EXEC_LINE = c["EXEC_LINE"].toString();
            if (c.contains("CURRENT_LINE")) Color::CURRENT_LINE = c["CURRENT_LINE"].toString();
            if (c.contains("ALT_BG")) Color::ALT_BG = c["ALT_BG"].toString();
            if (c.contains("EDITOR_BG")) Color::EDITOR_BG = c["EDITOR_BG"].toString();
            if (c.contains("SYN_LABEL")) Color::SYN_LABEL = c["SYN_LABEL"].toString();
            if (c.contains("SYN_DIRECTIVE")) Color::SYN_DIRECTIVE = c["SYN_DIRECTIVE"].toString();
            if (c.contains("SYN_MNEMONIC")) Color::SYN_MNEMONIC = c["SYN_MNEMONIC"].toString();
            if (c.contains("SYN_REGISTER")) Color::SYN_REGISTER = c["SYN_REGISTER"].toString();
            if (c.contains("SYN_NUMBER")) Color::SYN_NUMBER = c["SYN_NUMBER"].toString();
            if (c.contains("SYN_COMMENT")) Color::SYN_COMMENT = c["SYN_COMMENT"].toString();
            if (c.contains("SYN_STRING")) Color::SYN_STRING = c["SYN_STRING"].toString();
            if (c.contains("MEM_ZERO")) Color::MEM_ZERO = c["MEM_ZERO"].toString();
            if (c.contains("EDITOR_STATUS_BG")) Color::EDITOR_STATUS_BG = c["EDITOR_STATUS_BG"].toString();
            if (c.contains("EDITOR_STATUS_TEXT")) Color::EDITOR_STATUS_TEXT = c["EDITOR_STATUS_TEXT"].toString();
        }
        if (root.contains("layout")) {
            QJsonObject l = root["layout"].toObject();
            if (l.contains("reg_w")) layout.reg_w = l["reg_w"].toInt();
            if (l.contains("ed_w")) layout.ed_w = l["ed_w"].toInt();
            if (l.contains("var_w")) layout.var_w = l["var_w"].toInt();
            if (l.contains("var_h")) layout.var_h = l["var_h"].toInt();
            if (l.contains("ed_h")) layout.ed_h = l["ed_h"].toInt();
            if (l.contains("mem_h")) layout.mem_h = l["mem_h"].toInt();
            if (l.contains("stack_h")) layout.stack_h = l["stack_h"].toInt();
                if (l.contains("console_h")) layout.console_h = l["console_h"].toInt();
        }
    }
}

QString process_qss(QString qss) {
    qss.replace("#4A9EFF22", Color::ACCENT + "22");
    qss.replace("#4A9EFF55", Color::ACCENT + "55");
    qss.replace("#4A9EFF44", Color::ACCENT + "44");
    qss.replace("#4A9EFF33", Color::ACCENT + "33");
    qss.replace("#4A9EFF18", Color::ACCENT + "18");
    qss.replace("#FF5A5A18", Color::ERROR + "18");
    qss.replace("#FF5A5A44", Color::ERROR + "44");
    qss.replace("#FF5A5A22", Color::ERROR + "22");
    qss.replace("#FF5A5A55", Color::ERROR + "55");
    qss.replace("#353A5C33", Color::BORDER + "33");
    qss.replace("#4AFF8C18", Color::REG_CHANGED + "18");
    qss.replace("#1C1E26", Color::BG); qss.replace("#22243A", Color::PANEL);
    qss.replace("#2A2D45", Color::HEADER); qss.replace("#353A5C", Color::BORDER);
    qss.replace("#4A9EFF", Color::ACCENT); qss.replace("#6DB3FF", Color::ACCENT_HOVER);
    qss.replace("#3A8EEF", Color::ACCENT_PRESSED); qss.replace("#C8C9D4", Color::TEXT);
    qss.replace("#6B6D80", Color::TEXT_MUTED); qss.replace("#44465A", Color::TEXT_DIM);
    qss.replace("#4AFF8C", Color::REG_CHANGED); qss.replace("#FF5A5A", Color::ERROR);
    qss.replace("#FFB347", Color::WARNING); qss.replace("#1E2030", Color::ALT_BG);
    qss.replace("#141620", Color::EDITOR_BG);
    return qss;
}

void generate_template() {
    QDir dir;
    if (!dir.exists("themes")) dir.mkdir("themes");
    reset_dark_colors();
    save_config("themes/default_theme.json");
}

void apply_theme(const QString& theme_name) {
    s_current_theme = theme_name;
    QSettings("memu8086", "memu8086").setValue("theme_name", theme_name);

    if (theme_name == "Light") {
        reset_dark_colors();
        QFile file(":/assets/styles/light.qss");
        if (file.open(QFile::ReadOnly | QFile::Text)) qApp->setStyleSheet(QTextStream(&file).readAll());
        return;
    }

    reset_dark_colors();
    if (theme_name != "Dark") {
        load_config(theme_name);
    }

    QFile file(":/assets/styles/dark.qss");
    if (file.open(QFile::ReadOnly | QFile::Text)) {
        QString qss = QTextStream(&file).readAll();
        qss = process_qss(qss);
        qApp->setStyleSheet(qss);
    }
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