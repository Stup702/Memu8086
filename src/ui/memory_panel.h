#pragma once

#include <QWidget>
#include <QAbstractScrollArea>
#include <QLineEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QPushButton>
#include <QToolBar>
#include <QLabel>
#include <QMap>
#include <QFont>
#include <QFontMetrics>
#include <QTimer>
#include <vector>

#include "core/cpu.h"
#include "core/emulator.h" // Needed for MEMORY_SIZE if applicable

namespace memu8086::ui {

class HexView : public QAbstractScrollArea {
    Q_OBJECT
public:
    explicit HexView(QWidget* parent = nullptr);
    ~HexView() override;

    void set_data(const uint8_t* data, uint32_t size);
    void set_cpu(const emu8086::core::CPU* cpu);
    void set_bytes_per_row(int n);
    void jump_to(uint32_t addr);
    void set_selection(uint32_t start, uint32_t end);
    void mark_written(uint32_t addr, int count);
    void set_show_ascii(bool show);
    
    void set_search_results(const std::vector<uint32_t>& results, int match_len);
    void set_active_match(int index);

    const uint8_t* get_data() const { return data; }
    uint32_t get_data_size() const { return data_size; }

signals:
    void jump_to_disasm(uint32_t addr);
    void add_watch(uint32_t addr);
    void address_selected(uint32_t addr);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void contextMenuEvent(QContextMenuEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    const uint8_t* data = nullptr;
    uint32_t data_size = 0;
    const emu8086::core::CPU* cpu = nullptr;
    int bytes_per_row = 16;
    int row_height = 0;
    int char_width = 0;
    uint32_t sel_start = UINT32_MAX, sel_end = UINT32_MAX;
    QMap<uint32_t, qint64> written_timestamps;
    QFont mono;
    QFontMetrics* fm = nullptr;
    bool show_ascii = true;
    QTimer* anim_timer;

    std::vector<uint32_t> search_results;
    int search_match_len = 0;
    int active_match_idx = -1;

    uint32_t addr_at_pos(QPoint p) const;
    void draw_row(QPainter& p, int y, uint32_t addr, qint64 now, bool& has_animations);
    QString copy_as_hex() const;
    QString copy_as_c_array() const;
    QString copy_as_string() const;
    void update_scrollbars();
};

class MemoryPanel : public QWidget {
    Q_OBJECT
public:
    explicit MemoryPanel(QWidget* parent = nullptr);
    void update(const emu8086::core::Memory& mem, const emu8086::core::CPU& cpu);
    void jump_to(uint32_t linear_addr);

signals:
    void address_selected(uint32_t addr);
    void jump_to_disasm(uint32_t addr);
    void add_watch(uint32_t addr);

private slots:
    void on_go_clicked();
    void on_search_changed();
    void on_search_next();
    void on_search_prev();
    void on_ascii_toggled(bool checked);
    void on_width_changed(int index);

private:
    HexView* hex_view;
    QLineEdit* addr_input;
    QComboBox* width_combo;
    QCheckBox* ascii_check;
    QCheckBox* follow_ip;
    QCheckBox* follow_sp;
    QPushButton* btn_go;
    QToolBar* toolbar;

    QWidget* search_bar;
    QLineEdit* search_hex_input;
    QLineEdit* search_ascii_input;
    QLabel* search_status;
    QPushButton* btn_find_next;
    QPushButton* btn_find_prev;

    bool is_updating_search = false;
    int current_match_idx = -1;
    std::vector<uint32_t> current_matches;

    void perform_search();
};

} // namespace memu8086::ui