#include "stack_panel.h"
#include "theme.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMenu>
#include <QApplication>
#include <QClipboard>
#include <QPainter>

namespace memu8086::ui {

StackDepthBar::StackDepthBar(QWidget* parent) : QWidget(parent) {
    setFixedWidth(12);
}

void StackDepthBar::set_fill_ratio(float ratio) {
    fill_ratio = qBound(0.0f, ratio, 1.0f);
    QWidget::update();
}

void StackDepthBar::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.fillRect(rect(), QColor(Theme::Color::PANEL));
    int h = rect().height();
    int fill_h = h * fill_ratio;
    QRect fill_r(0, h - fill_h, width(), fill_h);
    QLinearGradient grad(0, h, 0, h - fill_h);
    grad.setColorAt(0.0, QColor(Theme::Color::ACCENT));
    grad.setColorAt(1.0, QColor(Theme::Color::REG_CHANGED));
    p.fillRect(fill_r, grad);
}

StackPanel::StackPanel(QWidget* parent) : QWidget(parent) {
    QHBoxLayout* main_layout = new QHBoxLayout(this);
    main_layout->setContentsMargins(8, 8, 8, 8);
    
    QVBoxLayout* left_layout = new QVBoxLayout();
    lbl_ss_sp = new QLabel("SS:SP = 0000:0000  →  Linear: 0x00000", this);
    lbl_ss_sp->setFont(Theme::mono_font(12));
    lbl_ss_sp->setStyleSheet(QStringLiteral("color: %1;").arg(Theme::Color::TEXT_MUTED));
    left_layout->addWidget(lbl_ss_sp);
    
    table = new QTableWidget(0, 5, this);
    table->setHorizontalHeaderLabels({"Offset", "Address", "Value (hex)", "Value (dec)", "Note"});
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    table->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    table->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
    table->verticalHeader()->setVisible(false);
    table->setAlternatingRowColors(true);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(table, &QTableWidget::customContextMenuRequested, this, &StackPanel::show_context_menu);
    left_layout->addWidget(table);
    
    depth_bar = new StackDepthBar(this);
    
    main_layout->addLayout(left_layout);
    main_layout->addWidget(depth_bar);
}

StackPanel::StackPanel(emu8086::core::CPU& cpu, QWidget* parent)
    : StackPanel(parent) {
    m_cpu = &cpu;
}

void StackPanel::update(const emu8086::core::CPU& cpu) {
    lbl_ss_sp->setText(QString("SS:SP = %1:%2  →  Linear: 0x%3")
        .arg(cpu.regs.SS, 4, 16, QChar('0'))
        .arg(cpu.regs.SP, 4, 16, QChar('0'))
        .arg((cpu.regs.SS << 4) + cpu.regs.SP, 5, 16, QChar('0')).toUpper());

    depth_bar->set_fill_ratio((float)(0xFFFF - cpu.regs.SP) / 0xFFFF);

    uint16_t current_sp = cpu.regs.SP;
    uint16_t top_val = cpu.mem.read16((cpu.regs.SS << 4) + current_sp);

    if (current_sp != prev_sp_val || (current_sp == prev_sp_val && top_val != prev_top_val)) {
        flash_row0 = true;
        QTimer::singleShot(150, this, [this]() { flash_row0 = false; table->viewport()->update(); });
    }
    prev_sp_val = current_sp;
    prev_top_val = top_val;

    table->setRowCount(0);
    for (int i = 0; i < 32; ++i) {
        uint32_t offset = current_sp + i * 2;
        if (offset > 0xFFFF) break;

        uint32_t addr = (cpu.regs.SS << 4) + offset;
        uint16_t val = cpu.mem.read16(addr);

        table->insertRow(i);
        auto* it_off = new QTableWidgetItem(QString("SP+%1").arg(i * 2, 2, 16, QChar('0')).toUpper());
        auto* it_addr = new QTableWidgetItem(QString("%1:%2").arg(cpu.regs.SS, 4, 16, QChar('0')).arg(offset, 4, 16, QChar('0')).toUpper());
        auto* it_valh = new QTableWidgetItem(QString("0x%1").arg(val, 4, 16, QChar('0')).toUpper());
        auto* it_vald = new QTableWidgetItem(QString::number(val));
        auto* it_note = new QTableWidgetItem("");

        it_off->setFont(Theme::mono_font(12));
        it_addr->setFont(Theme::mono_font(12));
        it_valh->setFont(Theme::mono_font(12));
        it_vald->setFont(Theme::mono_font(12));

        if (i == 0) {
            it_note->setText("← SP");
            it_note->setForeground(QColor(Theme::Color::ACCENT));
            QColor bg = flash_row0 ? QColor(Theme::Color::REG_CHANGED) : QColor(Theme::Color::ACCENT);
            bg.setAlpha(flash_row0 ? 64 : 24); // #4A9EFF18 is 24 alpha
            it_off->setBackground(bg); it_addr->setBackground(bg);
            it_valh->setBackground(bg); it_vald->setBackground(bg); it_note->setBackground(bg);
        } else if (offset == cpu.regs.BP) {
            it_note->setText("← BP");
            it_note->setForeground(QColor(Theme::Color::SYN_LABEL));
        } else if (val >= 0x0100 && val < 0xFFF0) {
            it_note->setText("[ret?]");
            it_note->setForeground(QColor(Theme::Color::WARNING));
        }

        table->setItem(i, 0, it_off); table->setItem(i, 1, it_addr);
        table->setItem(i, 2, it_valh); table->setItem(i, 3, it_vald);
        table->setItem(i, 4, it_note);
    }
}

void StackPanel::refresh() {
    if (m_cpu) {
        update(*m_cpu);
    }
}

void StackPanel::show_context_menu(const QPoint& pos) {
    QTableWidgetItem* item = table->itemAt(pos);
    if (!item) return;
    int row = item->row();
    QString addr_str = table->item(row, 1)->text();
    QString val_str = table->item(row, 2)->text();

    QMenu menu(this);
    auto* act_copy_addr = menu.addAction("Copy address");
    auto* act_copy_val = menu.addAction("Copy value");
    menu.addSeparator();
    auto* act_jump = menu.addAction("Jump to in Memory Viewer");

    QAction* res = menu.exec(table->viewport()->mapToGlobal(pos));
    if (res == act_copy_addr) QApplication::clipboard()->setText(addr_str);
    else if (res == act_copy_val) QApplication::clipboard()->setText(val_str);
    else if (res == act_jump) {
        uint32_t addr = 0;
        auto parts = addr_str.split(':');
        if (parts.size() == 2) addr = (parts[0].toUInt(nullptr, 16) << 4) + parts[1].toUInt(nullptr, 16);
        emit jump_to_memory(addr);
    }
}

} // namespace memu8086::ui