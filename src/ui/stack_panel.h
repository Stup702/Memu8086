#pragma once

#include <QWidget>
#include <QLabel>
#include <QTableWidget>
#include <QTimer>
#include "core/cpu.h"

namespace memu8086::ui {

class StackDepthBar : public QWidget {
    Q_OBJECT
public:
    explicit StackDepthBar(QWidget* parent = nullptr);
    void set_fill_ratio(float ratio);
protected:
    void paintEvent(QPaintEvent*) override;
private:
    float fill_ratio = 0.0f;
};

class StackPanel : public QWidget {
    Q_OBJECT
public:
    explicit StackPanel(QWidget* parent = nullptr);
    StackPanel(emu8086::core::CPU& cpu, QWidget* parent = nullptr); // Compatible with MainWindow
    
    void update(const emu8086::core::CPU& cpu);
    void refresh();

signals:
    void jump_to_memory(uint32_t addr);

private slots:
    void show_context_menu(const QPoint& pos);

private:
    QLabel* lbl_ss_sp;
    QTableWidget* table;
    StackDepthBar* depth_bar;
    
    const emu8086::core::CPU* m_cpu = nullptr;
    uint16_t prev_sp_val = 0;
    uint16_t prev_top_val = 0;
    bool flash_row0 = false;
};

} // namespace memu8086::ui