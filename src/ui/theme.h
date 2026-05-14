#pragma once

#include <QApplication>
#include <QFont>

namespace Theme {
    enum class Mode { Dark, Light };

    void apply(Mode mode);
    Mode current_mode();

    void apply_dark(QApplication& app);
    void apply_light(QApplication& app);
    QFont mono_font(int size = 13);
    QFont ui_font(int size = 13);

    // Color constants for use in QPainter custom widgets
    namespace Color {
        inline constexpr auto BG          = "#1C1E26";
        inline constexpr auto PANEL       = "#22243A";
        inline constexpr auto HEADER      = "#2A2D45";
        inline constexpr auto ACTIVE      = "#353A5C";
        inline constexpr auto ACCENT      = "#4A9EFF";
        inline constexpr auto ACCENT_HOVER= "#6DB3FF";
        inline constexpr auto TEXT        = "#C8C9D4";
        inline constexpr auto TEXT_MUTED  = "#6B6D80";
        inline constexpr auto TEXT_DIM    = "#44465A";
        inline constexpr auto BORDER      = "#353A5C";
        inline constexpr auto REG_CHANGED = "#4AFF8C";
        inline constexpr auto ERROR       = "#FF5A5A";
        inline constexpr auto WARNING     = "#FFB347";
        inline constexpr auto BREAKPOINT  = "#FF4F4F";
        inline constexpr auto EXEC_LINE   = "#2A2D1A";
    }
}