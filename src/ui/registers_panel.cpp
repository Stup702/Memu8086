#include "registers_panel.h"
#include "theme.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QTableWidget>
#include <QHeaderView>
#include <QPainter>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QEnterEvent>

namespace memu8086::ui {

FlagWidget::FlagWidget(const QString& name, const QString& tooltip_text, QWidget* p) 
    : QWidget(p), flag_name(name), tip_text(tooltip_text) {
    setFixedSize(26, 24);
    setToolTip(tip_text);
    flash_timer = new QTimer(this);
    flash_timer->setSingleShot(true);
    connect(flash_timer, &QTimer::timeout, this, [this]{ flashing = false; update(); });
}

void FlagWidget::set_value(bool val, bool changed) {
    if (val != value) {
        value = val;
        if (changed) { flashing = true; flash_timer->start(150); }
        update();
    }
}

void FlagWidget::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    QRect r = rect().adjusted(1, 1, -1, -1);
    
    if (flashing) {
        p.setBrush(QColor(Theme::Color::REG_CHANGED));
        p.setPen(Qt::NoPen);
    } else if (value) {
        p.setBrush(QColor(Theme::Color::ACCENT));
        p.setPen(Qt::NoPen);
    } else {
        p.setBrush(Qt::NoBrush);
        p.setPen(QColor(Theme::Color::BORDER));
    }
    p.drawRoundedRect(r, 4, 4);
    
    p.setPen(value ? Qt::white : QColor(Theme::Color::TEXT_MUTED));
    p.setFont(Theme::ui_font(10));
    p.drawText(r, Qt::AlignCenter, flag_name);
}

void FlagWidget::enterEvent(QEnterEvent* e) { QWidget::enterEvent(e); }
void FlagWidget::leaveEvent(QEvent* e) { QWidget::leaveEvent(e); }

RegistersPanel::RegistersPanel(emu8086::core::CPU& cpu_ref, QWidget* parent) 
    : QWidget(parent), cpu(cpu_ref) {
    prev_regs = cpu.regs;
    setup_ui();
}

void RegistersPanel::setup_ui() {
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(8, 8, 8, 8);
    
    // Section 1: GPRs
    QGroupBox* grp_gpr = new QGroupBox("General Registers");
    QVBoxLayout* lay_gpr = new QVBoxLayout(grp_gpr);
    QTableWidget* table = new QTableWidget(8, 4);
    table->setHorizontalHeaderLabels({"Name", "Hex", "Decimal", "Binary"});
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    table->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
    table->verticalHeader()->setVisible(false);
    table->setShowGrid(false);
    table->setSelectionMode(QAbstractItemView::NoSelection);
    table->setFocusPolicy(Qt::NoFocus);
    table->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    table->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    
    QStringList regs = {"AX", "BX", "CX", "DX", "SI", "DI", "SP", "BP"};
    for (int i = 0; i < 8; ++i) {
        table->setRowHeight(i, 28);
        QTableWidgetItem* name_item = new QTableWidgetItem(regs[i]);
        name_item->setForeground(QColor(Theme::Color::ACCENT));
        QFont f = Theme::mono_font(13); f.setBold(true); name_item->setFont(f);
        table->setItem(i, 0, name_item);
        
        reg_hex_items[regs[i]] = new QTableWidgetItem("0x0000");
        reg_hex_items[regs[i]]->setFont(Theme::mono_font(13));
        table->setItem(i, 1, reg_hex_items[regs[i]]);
        
        reg_dec_items[regs[i]] = new QTableWidgetItem("0");
        reg_dec_items[regs[i]]->setForeground(QColor(Theme::Color::TEXT_MUTED));
        table->setItem(i, 2, reg_dec_items[regs[i]]);
        
        reg_bin_items[regs[i]] = new QTableWidgetItem("0000 0000 0000 0000");
        reg_bin_items[regs[i]]->setForeground(QColor(Theme::Color::TEXT_DIM));
        reg_bin_items[regs[i]]->setFont(Theme::mono_font(12));
        table->setItem(i, 3, reg_bin_items[regs[i]]);

        for (int c = 0; c < 4; ++c) {
            if (auto* it = table->item(i, c)) it->setToolTip(it->text());
        }
    }

    // Dynamically calculate exact required height to prevent clipping on any OS/DPI scaling
    int exact_height = table->horizontalHeader()->sizeHint().height() + (8 * 28) + 2;
    table->setFixedHeight(exact_height);

    lay_gpr->addWidget(table);
    layout->addWidget(grp_gpr);
    
    // Section 2: Segments
    QGroupBox* grp_seg = new QGroupBox("Segments & IP");
    QGridLayout* lay_seg = new QGridLayout(grp_seg);
    QStringList segs = {"CS", "DS", "ES", "SS", "IP"};
    for (int i=0; i<segs.size(); ++i) {
        QLabel* lbl = new QLabel(segs[i] + ": 0000");
        lbl->setFont(Theme::mono_font(13));
        reg_labels[segs[i]] = lbl;
        lay_seg->addWidget(lbl, i / 3, i % 3);
    }
    lbl_cs_ip = new QLabel("CS:IP = 0000:0000  →  0x00000");
    lbl_cs_ip->setStyleSheet(QStringLiteral("color: %1;").arg(Theme::Color::TEXT_MUTED));
    lay_seg->addWidget(lbl_cs_ip, 2, 0, 1, 3);
    layout->addWidget(grp_seg);
    
    // Section 3: Flags
    QGroupBox* grp_flags = new QGroupBox("Flags");
    QHBoxLayout* lay_flags = new QHBoxLayout(grp_flags);
    struct FlagInfo { QString abbr; QString full; };
    QList<FlagInfo> flgs = {
        {"OF", "Overflow Flag"}, {"DF", "Direction Flag"}, {"IF", "Interrupt Flag"},
        {"TF", "Trap Flag"},     {"SF", "Sign Flag"},      {"ZF", "Zero Flag"},
        {"AF", "Auxiliary Flag"},{"PF", "Parity Flag"},    {"CF", "Carry Flag"}
    };
    for (const auto& f : flgs) {
        flag_widgets[f.abbr] = new FlagWidget(f.abbr, f.full, this);
        lay_flags->addWidget(flag_widgets[f.abbr]);
    }
    lay_flags->addStretch();
    layout->addWidget(grp_flags);
    
    layout->addStretch();
}

void RegistersPanel::update_reg_row(int /*row*/, const QString& name, uint16_t val, uint16_t prev_val) {
    if (val != prev_val) {
        reg_hex_items[name]->setForeground(QColor(Theme::Color::REG_CHANGED));
        QTimer::singleShot(150, this, [this, name]() { reg_hex_items[name]->setForeground(QColor(Theme::Color::TEXT)); });
    }
    reg_hex_items[name]->setText(QString("0x%1").arg(val, 4, 16, QChar('0')).toUpper());
    QString dec_str = QString::number(val);
    if (val >= 0x8000) {
        dec_str += QString(" (%1)").arg(static_cast<int16_t>(val));
    }
    reg_dec_items[name]->setText(dec_str);
    QString bin = QString("%1").arg(val, 16, 2, QChar('0'));
    bin.insert(12, ' '); bin.insert(8, ' '); bin.insert(4, ' ');
    reg_bin_items[name]->setText(bin);
    
    QString tip = QString("%1\nHigh: 0x%2\nLow: 0x%3\nDec: %4\nASCII: '%5' '%6'")
        .arg(name).arg(val >> 8, 2, 16, QChar('0')).arg(val & 0xFF, 2, 16, QChar('0')).arg(val)
        .arg(QChar(val >> 8).isPrint() ? QChar(val >> 8) : '.').arg(QChar(val & 0xFF).isPrint() ? QChar(val & 0xFF) : '.');
    reg_hex_items[name]->setToolTip(tip);
}

void RegistersPanel::refresh() {
    update_reg_row(0, "AX", cpu.regs.AX, prev_regs.AX);
    update_reg_row(1, "BX", cpu.regs.BX, prev_regs.BX);
    update_reg_row(2, "CX", cpu.regs.CX, prev_regs.CX);
    update_reg_row(3, "DX", cpu.regs.DX, prev_regs.DX);
    update_reg_row(4, "SI", cpu.regs.SI, prev_regs.SI);
    update_reg_row(5, "DI", cpu.regs.DI, prev_regs.DI);
    update_reg_row(6, "SP", cpu.regs.SP, prev_regs.SP);
    update_reg_row(7, "BP", cpu.regs.BP, prev_regs.BP);
    
    reg_labels["CS"]->setText(QString("CS: %1").arg(cpu.regs.CS, 4, 16, QChar('0')).toUpper());
    reg_labels["DS"]->setText(QString("DS: %1").arg(cpu.regs.DS, 4, 16, QChar('0')).toUpper());
    reg_labels["ES"]->setText(QString("ES: %1").arg(cpu.regs.ES, 4, 16, QChar('0')).toUpper());
    reg_labels["SS"]->setText(QString("SS: %1").arg(cpu.regs.SS, 4, 16, QChar('0')).toUpper());
    reg_labels["IP"]->setText(QString("IP: %1").arg(cpu.regs.IP, 4, 16, QChar('0')).toUpper());
    
    uint32_t linear = (cpu.regs.CS << 4) + cpu.regs.IP;
    lbl_cs_ip->setText(QString("CS:IP = %1:%2  →  0x%3")
        .arg(cpu.regs.CS, 4, 16, QChar('0')).arg(cpu.regs.IP, 4, 16, QChar('0')).arg(linear, 5, 16, QChar('0')).toUpper());
        
    flag_widgets["OF"]->set_value(cpu.regs.flags.OF, cpu.regs.flags.OF != prev_regs.flags.OF);
    flag_widgets["DF"]->set_value(cpu.regs.flags.DF, cpu.regs.flags.DF != prev_regs.flags.DF);
    flag_widgets["IF"]->set_value(cpu.regs.flags.IF, cpu.regs.flags.IF != prev_regs.flags.IF);
    flag_widgets["TF"]->set_value(cpu.regs.flags.TF, cpu.regs.flags.TF != prev_regs.flags.TF);
    flag_widgets["SF"]->set_value(cpu.regs.flags.SF, cpu.regs.flags.SF != prev_regs.flags.SF);
    flag_widgets["ZF"]->set_value(cpu.regs.flags.ZF, cpu.regs.flags.ZF != prev_regs.flags.ZF);
    flag_widgets["AF"]->set_value(cpu.regs.flags.AF, cpu.regs.flags.AF != prev_regs.flags.AF);
    flag_widgets["PF"]->set_value(cpu.regs.flags.PF, cpu.regs.flags.PF != prev_regs.flags.PF);
    flag_widgets["CF"]->set_value(cpu.regs.flags.CF, cpu.regs.flags.CF != prev_regs.flags.CF);
    
    prev_regs = cpu.regs;
}

} // namespace memu8086::ui