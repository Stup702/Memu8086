#pragma once

#include <QToolBar>
#include <QToolButton>
#include <QSlider>
#include <QLabel>
#include <QTimer>
#include "core/debugger.h"

namespace memu8086::ui {

class Toolbar : public QToolBar {
    Q_OBJECT
public:
    explicit Toolbar(QWidget* parent = nullptr);

    void set_state(emu8086::core::DebuggerState state, bool source_modified, bool assembled);
    void sync_state(emu8086::core::DebuggerState state); // Compatibility helper
    
    void set_speed(float ips);
    float get_speed() const;
    
    void update_stats(uint16_t ip, uint64_t cycles, uint64_t instrs);

signals:
    void assemble_clicked();
    void run_clicked();
    void stop_clicked();
    void step_clicked();
    void step_over_clicked();
    void reset_clicked();
    void speed_changed(float ips);

private slots:
    void on_slider_changed(int value);
    void animate_status();

private:
    QToolButton* btn_assemble;
    QToolButton* btn_run;
    QToolButton* btn_stop;
    QToolButton* btn_step;
    QToolButton* btn_step_over;
    QToolButton* btn_reset;
    QSlider* speed_slider;
    QLabel* lbl_speed;
    QLabel* lbl_status;
    QLabel* lbl_ip;
    QLabel* lbl_cycles;

    QTimer* anim_timer;
    bool anim_frame = false;
    emu8086::core::DebuggerState current_state = emu8086::core::DebuggerState::IDLE;

    QToolButton* make_tool_button(const QString& icon_text, const QString& text, const QString& shortcut, const QString& obj_name);
    float slider_to_ips(int v) const;
    int ips_to_slider(float ips) const;
};

} // namespace memu8086::ui