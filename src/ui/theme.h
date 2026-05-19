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

#include <QApplication>
#include <QFont>

namespace Theme {
    void reset_dark_colors();
    void generate_template();
    void apply_theme(const QString& theme_name);
    QString current_theme();

    QFont mono_font(int size = 13);
    QFont ui_font(int size = 13);

    // Color constants for use in QPainter custom widgets
    namespace Color {
        inline QString BG          = "#1C1E26";
        inline QString PANEL       = "#22243A";
        inline QString HEADER      = "#2A2D45";
        inline QString ACTIVE      = "#353A5C";
        inline QString ACCENT      = "#4A9EFF";
        inline QString ACCENT_HOVER= "#6DB3FF";
        inline QString ACCENT_PRESSED = "#3A8EEF";
        inline QString TEXT        = "#C8C9D4";
        inline QString TEXT_MUTED  = "#6B6D80";
        inline QString TEXT_DIM    = "#44465A";
        inline QString BORDER      = "#353A5C";
        inline QString REG_CHANGED = "#4AFF8C";
        inline QString ERROR       = "#FF5A5A";
        inline QString WARNING     = "#FFB347";
        inline QString BREAKPOINT  = "#FF4F4F";
        inline QString EXEC_LINE   = "#2A2D1A";
        inline QString CURRENT_LINE= "#292B38";
        inline QString ALT_BG      = "#1E2030";
        inline QString EDITOR_BG   = "#141620";
        
        inline QString SYN_LABEL     = "#C984FF";
        inline QString SYN_DIRECTIVE = "#FF7E7E";
        inline QString SYN_MNEMONIC  = "#4A9EFF";
        inline QString SYN_REGISTER  = "#FF9E64";
        inline QString SYN_NUMBER    = "#B5EAD7";
        inline QString SYN_COMMENT   = "#6B7D6A";
        inline QString SYN_STRING    = "#F7C948";
        inline QString MEM_ZERO      = "#3A3C50";
        inline QString EDITOR_STATUS_BG = "#2A2D45";
        inline QString EDITOR_STATUS_TEXT = "#6B6D80";
    }

    struct LayoutConfig {
        int reg_w = 280, ed_w = 920;
        int var_w = 600, var_h = 320;
        int stack_h = 250;
        int ed_h = 550, mem_h = 250;
        int console_h = 250;
    };
    inline LayoutConfig layout;

    void load_config(const QString& path = "memu8086_config.json");
    void save_config(const QString& path = "memu8086_config.json");
}