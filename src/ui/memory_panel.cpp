#include "memory_panel.h"
#include "theme.h"

#include <QPainter>
#include <QScrollBar>
#include <QMouseEvent>
#include <QMenu>
#include <QApplication>
#include <QClipboard>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QShortcut>
#include <QDateTime>

namespace memu8086::ui {

// --- HexView Implementation ---

HexView::HexView(QWidget* parent) : QAbstractScrollArea(parent) {
    mono = Theme::mono_font(13);
    fm = new QFontMetrics(mono);
    char_width = fm->horizontalAdvance('0');
    row_height = fm->height() + 4;

    anim_timer = new QTimer(this);
    connect(anim_timer, &QTimer::timeout, this, [this]() { viewport()->update(); });
    
    setFocusPolicy(Qt::StrongFocus);
}

HexView::~HexView() {
    delete fm;
}

void HexView::set_data(const uint8_t* in_data, uint32_t size) {
    data = in_data;
    data_size = size;
    update_scrollbars();
}

void HexView::set_cpu(const emu8086::core::CPU* in_cpu) {
    cpu = in_cpu;
}

void HexView::set_bytes_per_row(int n) {
    bytes_per_row = n;
    update_scrollbars();
    viewport()->update();
}

void HexView::set_show_ascii(bool show) {
    show_ascii = show;
    viewport()->update();
}

void HexView::jump_to(uint32_t addr) {
    if (addr >= data_size) addr = data_size > 0 ? data_size - 1 : 0;
    int target_row = addr / bytes_per_row;
    verticalScrollBar()->setValue(target_row);
    set_selection(addr, addr);
}

void HexView::set_selection(uint32_t start, uint32_t end) {
    sel_start = qMin(start, data_size - 1);
    sel_end = qMin(end, data_size - 1);
    viewport()->update();
    if (sel_start == sel_end) emit address_selected(sel_start);
}

void HexView::mark_written(uint32_t addr, int count) {
    qint64 now = QDateTime::currentMSecsSinceEpoch();
    for (int i = 0; i < count; ++i) {
        if (addr + i < data_size) written_timestamps[addr + i] = now;
    }
    viewport()->update();
}

void HexView::set_search_results(const std::vector<uint32_t>& results, int match_len) {
    search_results = results;
    search_match_len = match_len;
    active_match_idx = -1;
    viewport()->update();
}

void HexView::set_active_match(int index) {
    active_match_idx = index;
    if (index >= 0 && index < (int)search_results.size()) {
        jump_to(search_results[index]);
    }
    viewport()->update();
}

void HexView::update_scrollbars() {
    if (data_size == 0) return;
    int total_rows = (data_size + bytes_per_row - 1) / bytes_per_row;
    int visible_rows = viewport()->height() / row_height;
    verticalScrollBar()->setRange(0, qMax(0, total_rows - visible_rows));
    verticalScrollBar()->setPageStep(visible_rows);
    verticalScrollBar()->setSingleStep(1);
}

void HexView::resizeEvent(QResizeEvent* event) {
    QAbstractScrollArea::resizeEvent(event);
    update_scrollbars();
}

void HexView::paintEvent(QPaintEvent*) {
    QPainter p(viewport());
    p.setFont(mono);
    p.fillRect(viewport()->rect(), QColor(Theme::Color::BG));

    if (!data || data_size == 0) return;

    int first_row = verticalScrollBar()->value();
    int visible_rows = viewport()->height() / row_height + 1;
    int last_row = qMin(first_row + visible_rows, (int)((data_size + bytes_per_row - 1) / bytes_per_row));

    qint64 now = QDateTime::currentMSecsSinceEpoch();
    bool has_animations = false;

    for (int r = first_row; r <= last_row; ++r) {
        int y = (r - first_row) * row_height;
        uint32_t addr = r * bytes_per_row;
        draw_row(p, y, addr, now, has_animations);
    }

    if (has_animations && !anim_timer->isActive()) anim_timer->start(16);
    else if (!has_animations && anim_timer->isActive()) anim_timer->stop();
}

void HexView::draw_row(QPainter& p, int y, uint32_t addr, qint64 now, bool& has_animations) {
    // 1. Segment Gutter (4px wide, split into 4 1px strips for ES, CS, SS, DS overlapping)
    if (cpu) {
        uint32_t ds_base = cpu->regs.DS << 4;
        uint32_t cs_base = cpu->regs.CS << 4;
        uint32_t ss_base = cpu->regs.SS << 4;
        uint32_t es_base = cpu->regs.ES << 4;
        
        if (addr + bytes_per_row > ds_base && addr < ds_base + 0x10000) p.fillRect(QRect(0, y, 1, row_height), QColor(Theme::Color::ACCENT));
        if (addr + bytes_per_row > cs_base && addr < cs_base + 0x10000) p.fillRect(QRect(1, y, 1, row_height), QColor(Theme::Color::SYN_STRING));
        if (addr + bytes_per_row > ss_base && addr < ss_base + 0x10000) p.fillRect(QRect(2, y, 1, row_height), QColor(Theme::Color::REG_CHANGED));
        if (addr + bytes_per_row > es_base && addr < es_base + 0x10000) p.fillRect(QRect(3, y, 1, row_height), QColor(Theme::Color::SYN_LABEL));
    }

    // 2. Address
    p.setPen(QColor(Theme::Color::TEXT_MUTED));
    QString addr_str = QString("%1:%2").arg(addr >> 4, 4, 16, QLatin1Char('0')).arg(addr & 0xF, 4, 16, QLatin1Char('0')).toUpper();
    int x = 12; // Start after gutter
    p.drawText(x, y + fm->ascent() + 2, addr_str);
    x += 10 * char_width; // Length of XXXX:XXXX + 1 space

    // 3. Hex & ASCII bounds
    uint32_t ip_addr = cpu ? ((cpu->regs.CS << 4) + cpu->regs.IP) : UINT32_MAX;
    uint32_t sp_addr = cpu ? ((cpu->regs.SS << 4) + cpu->regs.SP) : UINT32_MAX;
    int ascii_x = x + (bytes_per_row * 3 * char_width) + (bytes_per_row / 8) * char_width + char_width;

    // ASCII Separator
    if (show_ascii) {
        p.setPen(QColor(Theme::Color::BORDER));
        p.drawText(ascii_x - char_width - 2, y + fm->ascent() + 2, "│");
    }

    uint32_t s_min = qMin(sel_start, sel_end);
    uint32_t s_max = qMax(sel_start, sel_end);

    QColor accent_a = QColor(Theme::Color::ACCENT); accent_a.setAlpha(48);
    QColor warn_a = QColor(Theme::Color::WARNING); warn_a.setAlpha(64);
    QColor accent_a2 = QColor(Theme::Color::ACCENT); accent_a2.setAlpha(64);
    QColor match_act = QColor(Theme::Color::SYN_STRING); match_act.setAlpha(120);
    QColor match_inact = QColor(Theme::Color::SYN_STRING); match_inact.setAlpha(48);

    for (int i = 0; i < bytes_per_row; ++i) {
        uint32_t curr = addr + i;
        if (curr >= data_size) break;
        uint8_t val = data[curr];

        int byte_x = x + i * 3 * char_width + (i / 8) * char_width;
        QRect byte_rect(byte_x, y, char_width * 2 + 2, row_height);

        // Highlights
        if (curr >= s_min && curr <= s_max && sel_start != UINT32_MAX) p.fillRect(byte_rect, accent_a);
        if (curr == ip_addr) p.fillRect(byte_rect, warn_a);
        if (curr == sp_addr) p.fillRect(byte_rect, accent_a2);

        // Search Highlight
        bool is_search_match = false;
        bool is_active_match = false;
        for (size_t res_idx = 0; res_idx < search_results.size(); ++res_idx) {
            uint32_t match_start = search_results[res_idx];
            if (curr >= match_start && curr < match_start + search_match_len) {
                is_search_match = true;
                if ((int)res_idx == active_match_idx) is_active_match = true;
                break;
            }
        }
        if (is_active_match) p.fillRect(byte_rect, match_act);
        else if (is_search_match) p.fillRect(byte_rect, match_inact);

        // Text Color & Write Animation
        QColor text_color = (val == 0) ? QColor(Theme::Color::MEM_ZERO) : QColor(Theme::Color::TEXT);
        
        if (written_timestamps.contains(curr)) {
            qint64 elapsed = now - written_timestamps[curr];
            if (elapsed < 500) {
                float t = elapsed / 500.0f;
                int r = 0x4A + t * (text_color.red() - 0x4A);
                int g = 0xFF + t * (text_color.green() - 0xFF);
                int b = 0x8C + t * (text_color.blue() - 0x8C);
                text_color = QColor(r, g, b);
                has_animations = true;
            } else {
                written_timestamps.remove(curr);
            }
        }
        
        p.setPen(text_color);
        QString hex_str = QString("%1").arg(val, 2, 16, QLatin1Char('0')).toUpper();
        p.drawText(byte_rect, Qt::AlignCenter, hex_str);

        if (show_ascii) {
            char c = (val >= 32 && val <= 126) ? (char)val : '.';
            QRect ascii_rect(ascii_x + i * char_width, y, char_width, row_height);
            if (curr >= s_min && curr <= s_max && sel_start != UINT32_MAX) p.fillRect(ascii_rect, accent_a);
            p.drawText(ascii_rect, Qt::AlignCenter, QString(c));
        }
    }
}

uint32_t HexView::addr_at_pos(QPoint p) const {
    int row = p.y() / row_height + verticalScrollBar()->value();
    if (row < 0) return 0;
    uint32_t row_addr = row * bytes_per_row;

    int start_x = 12 + 10 * char_width;
    int px = p.x() - start_x;
    
    if (px < 0) return row_addr;

    int byte_idx = 0;
    int cur_x = 0;
    for (int i = 0; i < bytes_per_row; ++i) {
        int w = char_width * 3 + ((i % 8 == 7) ? char_width : 0);
        if (px >= cur_x && px < cur_x + w) {
            byte_idx = i;
            break;
        }
        cur_x += w;
        byte_idx = i + 1;
    }
    
    byte_idx = qMin(byte_idx, bytes_per_row - 1);
    return qMin(row_addr + byte_idx, data_size > 0 ? data_size - 1 : 0);
}

void HexView::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        uint32_t addr = addr_at_pos(event->pos());
        if (event->modifiers() & Qt::ShiftModifier) {
            set_selection(sel_start, addr);
        } else {
            set_selection(addr, addr);
        }
    }
}

void HexView::mouseMoveEvent(QMouseEvent* event) {
    if (event->buttons() & Qt::LeftButton) {
        uint32_t addr = addr_at_pos(event->pos());
        set_selection(sel_start, addr);
    }
}

void HexView::mouseReleaseEvent(QMouseEvent*) {}

void HexView::keyPressEvent(QKeyEvent* event) {
    if (sel_end == UINT32_MAX) return QAbstractScrollArea::keyPressEvent(event);
    
    uint32_t next_addr = sel_end;
    if (event->key() == Qt::Key_Left && next_addr > 0) next_addr--;
    else if (event->key() == Qt::Key_Right && next_addr < data_size - 1) next_addr++;
    else if (event->key() == Qt::Key_Up && next_addr >= (uint32_t)bytes_per_row) next_addr -= bytes_per_row;
    else if (event->key() == Qt::Key_Down && next_addr + bytes_per_row < data_size) next_addr += bytes_per_row;
    else return QAbstractScrollArea::keyPressEvent(event);

    if (event->modifiers() & Qt::ShiftModifier) set_selection(sel_start, next_addr);
    else set_selection(next_addr, next_addr);
    
    int target_row = next_addr / bytes_per_row;
    if (target_row < verticalScrollBar()->value()) verticalScrollBar()->setValue(target_row);
    else if (target_row >= verticalScrollBar()->value() + viewport()->height() / row_height) 
        verticalScrollBar()->setValue(target_row - viewport()->height() / row_height + 1);
}

void HexView::wheelEvent(QWheelEvent* event) {
    QAbstractScrollArea::wheelEvent(event);
}

void HexView::contextMenuEvent(QContextMenuEvent* event) {
    QMenu menu(this);
    auto act_hex = menu.addAction("Copy as hex bytes");
    auto act_c = menu.addAction("Copy as C array");
    auto act_str = menu.addAction("Copy as string");
    menu.addSeparator();
    auto act_disasm = menu.addAction("Jump to address in disassembler");
    auto act_watch = menu.addAction("Add watch");

    QAction* res = menu.exec(event->globalPos());
    if (res == act_hex) QApplication::clipboard()->setText(copy_as_hex());
    else if (res == act_c) QApplication::clipboard()->setText(copy_as_c_array());
    else if (res == act_str) QApplication::clipboard()->setText(copy_as_string());
    else if (res == act_disasm) emit jump_to_disasm(qMin(sel_start, sel_end));
    else if (res == act_watch) emit add_watch(qMin(sel_start, sel_end));
}

QString HexView::copy_as_hex() const {
    QString res;
    uint32_t start = qMin(sel_start, sel_end);
    uint32_t end = qMax(sel_start, sel_end);
    for (uint32_t i = start; i <= end && i < data_size; ++i) {
        res += QString("%1 ").arg(data[i], 2, 16, QLatin1Char('0')).toUpper();
    }
    return res.trimmed();
}

QString HexView::copy_as_c_array() const {
    QString res = "{ ";
    uint32_t start = qMin(sel_start, sel_end);
    uint32_t end = qMax(sel_start, sel_end);
    for (uint32_t i = start; i <= end && i < data_size; ++i) {
        res += QString("0x%1").arg(data[i], 2, 16, QLatin1Char('0')).toUpper();
        if (i != end) res += ", ";
    }
    res += " }";
    return res;
}

QString HexView::copy_as_string() const {
    QString res;
    uint32_t start = qMin(sel_start, sel_end);
    uint32_t end = qMax(sel_start, sel_end);
    for (uint32_t i = start; i <= end && i < data_size; ++i) {
        char c = (data[i] >= 32 && data[i] <= 126) ? (char)data[i] : '.';
        res += c;
    }
    return res;
}

// --- MemoryPanel Implementation ---

MemoryPanel::MemoryPanel(QWidget* parent) : QWidget(parent) {
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    // Toolbar
    toolbar = new QToolBar(this);
    addr_input = new QLineEdit(this);
    addr_input->setPlaceholderText("Address...");
    addr_input->setMaximumWidth(120);
    
    btn_go = new QPushButton("Go", this);
    width_combo = new QComboBox(this);
    width_combo->addItems({"8", "16", "24", "32"});
    width_combo->setCurrentIndex(1);
    
    ascii_check = new QCheckBox("ASCII", this);
    ascii_check->setChecked(true);
    follow_ip = new QCheckBox("Follow IP", this);
    follow_sp = new QCheckBox("Follow SP", this);

    toolbar->addWidget(addr_input);
    toolbar->addWidget(btn_go);
    toolbar->addSeparator();
    toolbar->addWidget(new QLabel(" Cols: "));
    toolbar->addWidget(width_combo);
    toolbar->addSeparator();
    toolbar->addWidget(ascii_check);
    toolbar->addWidget(follow_ip);
    toolbar->addWidget(follow_sp);
    layout->addWidget(toolbar);

    // Hex View
    hex_view = new HexView(this);
    layout->addWidget(hex_view);

    // Search Bar
    search_bar = new QWidget(this);
    QHBoxLayout* search_layout = new QHBoxLayout(search_bar);
    search_layout->setContentsMargins(4, 4, 4, 4);
    
    search_hex_input = new QLineEdit(this);
    search_hex_input->setPlaceholderText("Find Hex (e.g. 41 42)");
    search_ascii_input = new QLineEdit(this);
    search_ascii_input->setPlaceholderText("Find ASCII");
    
    btn_find_prev = new QPushButton("Prev", this);
    btn_find_next = new QPushButton("Next", this);
    search_status = new QLabel("0 matches", this);
    
    search_layout->addWidget(search_hex_input);
    search_layout->addWidget(search_ascii_input);
    search_layout->addWidget(btn_find_prev);
    search_layout->addWidget(btn_find_next);
    search_layout->addWidget(search_status);
    search_bar->hide();
    layout->addWidget(search_bar);

    // Connections
    connect(btn_go, &QPushButton::clicked, this, &MemoryPanel::on_go_clicked);
    connect(addr_input, &QLineEdit::returnPressed, this, &MemoryPanel::on_go_clicked);
    connect(width_combo, &QComboBox::currentIndexChanged, this, &MemoryPanel::on_width_changed);
    connect(ascii_check, &QCheckBox::toggled, this, &MemoryPanel::on_ascii_toggled);
    
    connect(search_hex_input, &QLineEdit::textEdited, this, &MemoryPanel::on_search_changed);
    connect(search_ascii_input, &QLineEdit::textEdited, this, &MemoryPanel::on_search_changed);
    connect(btn_find_next, &QPushButton::clicked, this, &MemoryPanel::on_search_next);
    connect(btn_find_prev, &QPushButton::clicked, this, &MemoryPanel::on_search_prev);
    
    connect(hex_view, &HexView::address_selected, this, &MemoryPanel::address_selected);
    connect(hex_view, &HexView::jump_to_disasm, this, &MemoryPanel::jump_to_disasm);
    connect(hex_view, &HexView::add_watch, this, &MemoryPanel::add_watch);

    QShortcut* shortcut_find = new QShortcut(QKeySequence("Ctrl+F"), this);
    connect(shortcut_find, &QShortcut::activated, this, [this]() {
        search_bar->setVisible(!search_bar->isVisible());
        if (search_bar->isVisible()) search_hex_input->setFocus();
    });
    
    QShortcut* shortcut_goto = new QShortcut(QKeySequence("Ctrl+G"), this);
    connect(shortcut_goto, &QShortcut::activated, this, [this]() {
        addr_input->setFocus();
        addr_input->selectAll();
    });
}

void MemoryPanel::update(const emu8086::core::Memory& mem, const emu8086::core::CPU& cpu) {
    hex_view->set_data(mem.get_data().data(), emu8086::core::MEMORY_SIZE);
    hex_view->set_cpu(&cpu);
    if (follow_ip->isChecked()) jump_to((cpu.regs.CS << 4) + cpu.regs.IP);
    else if (follow_sp->isChecked()) jump_to((cpu.regs.SS << 4) + cpu.regs.SP);
    hex_view->viewport()->update();
}

void MemoryPanel::jump_to(uint32_t linear_addr) {
    hex_view->jump_to(linear_addr);
}

void MemoryPanel::on_go_clicked() {
    QString s = addr_input->text().trimmed();
    uint32_t addr = 0;
    if (s.contains(':')) {
        auto parts = s.split(':');
        if (parts.size() == 2) addr = (parts[0].toUInt(nullptr, 16) << 4) + parts[1].toUInt(nullptr, 16);
    } else if (s.startsWith("0x", Qt::CaseInsensitive)) {
        addr = s.mid(2).toUInt(nullptr, 16);
    } else {
        bool ok = false;
        addr = s.toUInt(&ok, 16); // Default to hex interpretation like standard memory viewers
        if (!ok) addr = s.toUInt(nullptr, 10);
    }
    jump_to(addr);
}

void MemoryPanel::on_width_changed(int index) {
    hex_view->set_bytes_per_row((index + 1) * 8);
}

void MemoryPanel::on_ascii_toggled(bool checked) {
    hex_view->set_show_ascii(checked);
}

void MemoryPanel::on_search_changed() {
    if (is_updating_search) return;
    is_updating_search = true;
    
    QObject* sender_obj = sender();
    std::vector<uint8_t> pattern;
    
    if (sender_obj == search_hex_input) {
        QString hex_str = search_hex_input->text().remove(' ');
        for (int i = 0; i + 1 < hex_str.length(); i += 2) {
            pattern.push_back((uint8_t)hex_str.mid(i, 2).toUInt(nullptr, 16));
        }
        QString ascii_str;
        for (uint8_t b : pattern) ascii_str += (b >= 32 && b <= 126) ? (char)b : '.';
        search_ascii_input->setText(ascii_str);
    } else {
        QByteArray utf8 = search_ascii_input->text().toUtf8();
        for (char c : utf8) pattern.push_back((uint8_t)c);
        
        QString hex_str;
        for (uint8_t b : pattern) hex_str += QString("%1 ").arg(b, 2, 16, QLatin1Char('0')).toUpper();
        search_hex_input->setText(hex_str.trimmed());
    }
    
    is_updating_search = false;
    
    current_matches.clear();
    current_match_idx = -1;
    
    if (!pattern.empty()) {
        const uint8_t* data = hex_view->get_data();
        uint32_t size = hex_view->get_data_size();
        if (data && size >= pattern.size()) {
            for (uint32_t i = 0; i <= size - pattern.size(); ++i) {
                bool match = true;
                for (size_t j = 0; j < pattern.size(); ++j) {
                    if (data[i + j] != pattern[j]) {
                        match = false;
                        break;
                    }
                }
                if (match) current_matches.push_back(i);
            }
        }
    }
    
    hex_view->set_search_results(current_matches, pattern.size());
    search_status->setText(QString("%1 matches").arg(current_matches.size()));
}

void MemoryPanel::on_search_next() {
    if (current_matches.empty()) return;
    current_match_idx = (current_match_idx + 1) % current_matches.size();
    hex_view->set_active_match(current_match_idx);
    search_status->setText(QString("Match %1 of %2").arg(current_match_idx + 1).arg(current_matches.size()));
}

void MemoryPanel::on_search_prev() {
    if (current_matches.empty()) return;
    current_match_idx = (current_match_idx - 1 + current_matches.size()) % current_matches.size();
    hex_view->set_active_match(current_match_idx);
    search_status->setText(QString("Match %1 of %2").arg(current_match_idx + 1).arg(current_matches.size()));
}

} // namespace memu8086::ui