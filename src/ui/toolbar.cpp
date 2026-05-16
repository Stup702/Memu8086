#include "toolbar.h"
#include "theme.h"

#include <cmath>
#include <QIcon>
#include <QWidget>
#include <QHBoxLayout>

namespace memu8086::ui {

Toolbar::Toolbar(QWidget* parent) : QToolBar(parent) {
    setObjectName("main_toolbar");
    setMovable(false);
    setFloatable(false);

    btn_assemble  = make_tool_button("▶▶", "Assemble",  "F5",  "btn_assemble");
    QToolButton* btn_assemble_run = make_tool_button("⚙▶", "Assemble & Run", "F6", "btn_assemble_run");
    connect(btn_assemble_run, &QToolButton::clicked, this, [this]() {
        emit stop_clicked();
        emit assemble_clicked();
        emit run_clicked();
    });

    btn_run       = make_tool_button("▶",  "Run",       "F9",  "btn_run");
    btn_stop      = make_tool_button("⏸",  "Stop",      "Esc", "btn_stop");
    btn_step      = make_tool_button("↓",  "Step",      "F7",  "btn_step");
    btn_step_over = make_tool_button("↘",  "Step Over", "F8",  "btn_step_over");
    btn_reset     = make_tool_button("⟳",  "Reset",     "",    "btn_reset");

    connect(btn_assemble,  &QToolButton::clicked, this, &Toolbar::assemble_clicked);
    connect(btn_run,       &QToolButton::clicked, this, &Toolbar::run_clicked);
    connect(btn_stop,      &QToolButton::clicked, this, &Toolbar::stop_clicked);
    connect(btn_step,      &QToolButton::clicked, this, &Toolbar::step_clicked);
    connect(btn_step_over, &QToolButton::clicked, this, &Toolbar::step_over_clicked);
    connect(btn_reset,     &QToolButton::clicked, this, &Toolbar::reset_clicked);

    speed_slider = new QSlider(Qt::Horizontal, this);
    speed_slider->setRange(0, 100);
    speed_slider->setFixedWidth(150);
    connect(speed_slider, &QSlider::valueChanged, this, &Toolbar::on_slider_changed);

    lbl_speed = new QLabel(this);
    lbl_speed->setFixedWidth(70);
    lbl_speed->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    lbl_status = new QLabel("● Ready", this);
    lbl_ip = new QLabel("IP: 0000", this);
    lbl_cycles = new QLabel("Cycles: 0", this);

    lbl_ip->setFont(Theme::mono_font(12));
    lbl_cycles->setFont(Theme::mono_font(12));
    lbl_ip->setStyleSheet(QStringLiteral("color: %1;").arg(Theme::Color::TEXT_MUTED));
    lbl_cycles->setStyleSheet(QStringLiteral("color: %1;").arg(Theme::Color::TEXT_MUTED));

    addWidget(btn_assemble);
    addWidget(btn_assemble_run);
    addWidget(btn_run);
    addWidget(btn_stop);
    addWidget(btn_step);
    addWidget(btn_step_over);
    addWidget(btn_reset);
    addSeparator();
    addWidget(speed_slider);
    addWidget(lbl_speed);

    QWidget* spacer = new QWidget(this);
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    addWidget(spacer);

    addWidget(lbl_status);
    addWidget(new QLabel("  ", this));
    addWidget(lbl_ip);
    addWidget(new QLabel("  ", this));
    addWidget(lbl_cycles);
    addWidget(new QLabel("  ", this));

    anim_timer = new QTimer(this);
    connect(anim_timer, &QTimer::timeout, this, &Toolbar::animate_status);

    set_speed(1000.0f); // Default 1 kHz
    sync_state(emu8086::core::DebuggerState::IDLE);
}

QToolButton* Toolbar::make_tool_button(const QString& icon_text, const QString& text, const QString& shortcut, const QString& obj_name) {
    QToolButton* btn = new QToolButton(this);
    btn->setObjectName(obj_name);
    btn->setToolButtonStyle(Qt::ToolButtonTextOnly);
    
    btn->setText(icon_text + " " + text);
    
    if (!shortcut.isEmpty()) {
        btn->setToolTip(QStringLiteral("%1 (%2)").arg(text, shortcut));
    } else {
        btn->setToolTip(text);
    }
    return btn;
}

float Toolbar::slider_to_ips(int v) const { return std::pow(10.f, v / 100.f * 6.f); }
int Toolbar::ips_to_slider(float ips) const { return (int)(std::log10(ips) / 6.f * 100.f); }

void Toolbar::set_speed(float ips) {
    speed_slider->blockSignals(true);
    speed_slider->setValue(ips_to_slider(ips));
    speed_slider->blockSignals(false);
    
    if (ips < 1000.f) {
        lbl_speed->setText(QString::number((int)ips) + " Hz");
    } else if (ips < 1000000.f) {
        lbl_speed->setText(QString::number(ips / 1000.f, 'f', 1) + " kHz");
    } else {
        lbl_speed->setText(QString::number(ips / 1000000.f, 'f', 1) + " MHz");
    }
}

float Toolbar::get_speed() const { return slider_to_ips(speed_slider->value()); }

void Toolbar::on_slider_changed(int value) {
    float ips = slider_to_ips(value);
    set_speed(ips);
    emit speed_changed(ips);
}

void Toolbar::sync_state(emu8086::core::DebuggerState state) {
    set_state(state, false, true); // Fallback for simple status state passing
}

void Toolbar::set_state(emu8086::core::DebuggerState state, bool source_modified, bool assembled) {
    current_state = state;

    bool idle = (state == emu8086::core::DebuggerState::IDLE || state == emu8086::core::DebuggerState::PAUSED);
    bool running = (state == emu8086::core::DebuggerState::RUNNING);

    btn_run->setEnabled(idle);
    btn_stop->setEnabled(running);
    btn_step->setEnabled(idle);
    btn_step_over->setEnabled(idle);

    if (source_modified && !assembled) {
        btn_assemble->setStyleSheet(QStringLiteral("color: %1;").arg(Theme::Color::WARNING));
        if (auto* btn_ar = findChild<QToolButton*>("btn_assemble_run")) {
            btn_ar->setStyleSheet(QStringLiteral("color: %1;").arg(Theme::Color::WARNING));
        }
    } else {
        btn_assemble->setStyleSheet(""); // Normal / Reset
        if (auto* btn_ar = findChild<QToolButton*>("btn_assemble_run")) {
            btn_ar->setStyleSheet("");
        }
    }

    if (state != emu8086::core::DebuggerState::RUNNING) {
        anim_timer->stop();
        anim_frame = false;
        animate_status(); // Force correct static state layout
    } else if (!anim_timer->isActive()) {
        anim_timer->start(500);
        animate_status();
    }
}

void Toolbar::update_stats(uint16_t ip, uint64_t cycles, uint64_t instrs) {
    (void)instrs;
    lbl_ip->setText(QStringLiteral("IP: %1").arg(ip, 4, 16, QChar('0')).toUpper());
    lbl_cycles->setText(QStringLiteral("Cycles: %1").arg(cycles));
}

void Toolbar::animate_status() {
    QString status_text;
    QString color;

    switch (current_state) {
        case emu8086::core::DebuggerState::IDLE:
        case emu8086::core::DebuggerState::PAUSED:
            status_text = "● Ready"; color = Theme::Color::ACCENT;
            break;
        case emu8086::core::DebuggerState::RUNNING:
            anim_frame = !anim_frame;
            status_text = anim_frame ? "○ Running…" : "● Running…";
            color = Theme::Color::REG_CHANGED;
            break;
        case emu8086::core::DebuggerState::HALTED:
            status_text = "● Halted"; color = Theme::Color::WARNING;
            break;
        case emu8086::core::DebuggerState::ERROR:
            status_text = "✕ Error"; color = Theme::Color::ERROR;
            break;
    }

    lbl_status->setText(QStringLiteral("<span style='color: %1; font-weight: bold;'>%2</span>").arg(color, status_text));
}

} // namespace memu8086::ui