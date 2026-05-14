#include "variables_panel.h"
#include "theme.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QMenu>
#include <QTimer>

namespace memu8086::ui {

VariablesPanel::VariablesPanel(QWidget* parent) : QWidget(parent) {
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(4, 4, 4, 4);
    layout->setSpacing(6);

    // Add Watch Bar
    QHBoxLayout* watch_bar = new QHBoxLayout();
    watch_name = new QLineEdit(this);
    watch_name->setPlaceholderText("Name...");
    watch_addr = new QLineEdit(this);
    watch_addr->setPlaceholderText("Addr (0x...)");
    watch_size = new QComboBox(this);
    watch_size->addItems({"byte", "word"});
    btn_add = new QPushButton("+ Add", this);
    
    watch_bar->addWidget(watch_name);
    watch_bar->addWidget(watch_addr);
    watch_bar->addWidget(watch_size);
    watch_bar->addWidget(btn_add);
    layout->addLayout(watch_bar);

    // Table
    table = new QTableWidget(0, 7, this);
    table->setHorizontalHeaderLabels({"Name", "Address", "Hex", "High", "Low", "Dec", "Type"});
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    table->horizontalHeader()->setSectionResizeMode(6, QHeaderView::ResizeToContents);
    table->verticalHeader()->setVisible(false);
    table->setAlternatingRowColors(true);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setContextMenuPolicy(Qt::CustomContextMenu);
    
    layout->addWidget(table);

    connect(btn_add, &QPushButton::clicked, this, &VariablesPanel::on_add_watch);
    connect(table, &QTableWidget::customContextMenuRequested, this, &VariablesPanel::show_context_menu);
}

void VariablesPanel::set_symbols(const std::map<std::string, uint16_t>& syms) {
    symbols = syms;
}

void VariablesPanel::update(const emu8086::core::CPU& cpu) {
    rebuild_table(cpu);
    
    QTimer::singleShot(150, this, [this]() {
        for (int r = 0; r < table->rowCount(); ++r) {
            for (int c : {2, 3, 4, 5}) {
                auto* it = table->item(r, c);
                if (it && it->foreground().color() == QColor(Theme::Color::REG_CHANGED)) {
                    it->setForeground(QColor(Theme::Color::TEXT));
                }
            }
        }
    });
}

void VariablesPanel::refresh(const emu8086::assembler::AssemblyResult& res) {
    set_symbols(res.symbols);
    // Table is fully updated on tick during MainWindow::refresh_panels()
}

void VariablesPanel::refresh() {
    // Failsafe parameter-less refresh if invoked without refs, intentionally a no-op 
    // unless wired properly with CPU refs like StackPanel above.
}

void VariablesPanel::rebuild_table(const emu8086::core::CPU& cpu) {
    table->setRowCount(0);
    int row = 0;

    auto add_header = [&](const QString& title) {
        table->insertRow(row);
        auto* item = new QTableWidgetItem(title);
        item->setBackground(QColor(Theme::Color::HEADER));
        item->setForeground(QColor(Theme::Color::TEXT_MUTED));
        item->setFlags((item->flags() & ~Qt::ItemIsEnabled) | Qt::ItemIsSelectable);
        table->setItem(row, 0, item);
        table->setSpan(row, 0, 1, 7);
        row++;
    };

    auto add_row = [&](const QString& name, uint32_t addr, uint16_t val, const QString& type, bool is_reg) {
        table->insertRow(row);
        auto* it_name = new QTableWidgetItem(name);
        if (is_reg) it_name->setForeground(QColor(Theme::Color::TEXT_MUTED));
        
        auto* it_addr = new QTableWidgetItem(is_reg ? "-" : QString("0x%1").arg(addr, 5, 16, QChar('0')).toUpper());
        auto* it_valh = new QTableWidgetItem(QString("0x%1").arg(val, type == "byte" ? 2 : 4, 16, QChar('0')).toUpper());
        auto* it_high = new QTableWidgetItem(type == "word" ? QString("0x%1").arg(val >> 8, 2, 16, QChar('0')).toUpper() : "-");
        auto* it_low = new QTableWidgetItem(QString("0x%1").arg(val & 0xFF, 2, 16, QChar('0')).toUpper());
        auto* it_vald = new QTableWidgetItem(QString::number(val));
        auto* it_type = new QTableWidgetItem(type);
        
        it_addr->setFont(Theme::mono_font(12));
        it_valh->setFont(Theme::mono_font(12));
        it_high->setFont(Theme::mono_font(12));
        it_low->setFont(Theme::mono_font(12));
        it_vald->setFont(Theme::mono_font(12));

        QString key = name;
        if (prev_values.contains(key) && prev_values[key] != val) {
            it_valh->setForeground(QColor(Theme::Color::REG_CHANGED));
            it_high->setForeground(QColor(Theme::Color::REG_CHANGED));
            it_low->setForeground(QColor(Theme::Color::REG_CHANGED));
            it_vald->setForeground(QColor(Theme::Color::REG_CHANGED));
        }
        prev_values[key] = val;

        table->setItem(row, 0, it_name); table->setItem(row, 1, it_addr);
        table->setItem(row, 2, it_valh); table->setItem(row, 3, it_high);
        table->setItem(row, 4, it_low);  table->setItem(row, 5, it_vald);
        table->setItem(row, 6, it_type);
        row++;
    };

    add_header("Registers");
    add_row("AX", 0, cpu.regs.AX, "word", true); add_row("BX", 0, cpu.regs.BX, "word", true);
    add_row("CX", 0, cpu.regs.CX, "word", true); add_row("DX", 0, cpu.regs.DX, "word", true);
    add_row("SI", 0, cpu.regs.SI, "word", true); add_row("DI", 0, cpu.regs.DI, "word", true);
    add_row("SP", 0, cpu.regs.SP, "word", true); add_row("BP", 0, cpu.regs.BP, "word", true);
    add_row("IP", 0, cpu.regs.IP, "word", true);

    add_header("Symbols");
    for (const auto& sym : symbols) {
        uint32_t addr = (cpu.regs.DS << 4) + sym.second;
        uint16_t val = cpu.mem.read16(addr);
        add_row(QString::fromStdString(sym.first), addr, val, "word", false);
    }

    add_header("Watches");
    for (int i = 0; i < watches.size(); ++i) {
        const auto& w = watches[i];
        uint16_t val = w.size == 1 ? cpu.mem.read8(w.addr) : cpu.mem.read16(w.addr);
        add_row(w.name, w.addr, val, w.size == 1 ? "byte" : "word", false);
        
        QPushButton* btn_del = new QPushButton("✕");
        btn_del->setFlat(true);
        btn_del->setFixedSize(20, 20);
        connect(btn_del, &QPushButton::clicked, this, [this, n = w.name]() { remove_watch(n); });
        
        QWidget* w_container = new QWidget();
        QHBoxLayout* l = new QHBoxLayout(w_container);
        l->setContentsMargins(4, 0, 4, 0);
        l->addWidget(new QLabel(w.size == 1 ? "byte" : "word"));
        l->addStretch();
        l->addWidget(btn_del);
        table->setCellWidget(row - 1, 6, w_container);
        table->item(row - 1, 6)->setText(""); // Clear original text
    }
}

void VariablesPanel::on_add_watch() {
    QString name = watch_name->text().trimmed();
    QString addr_str = watch_addr->text().trimmed();
    if (name.isEmpty() || addr_str.isEmpty()) return;

    bool ok;
    uint32_t addr = addr_str.toUInt(&ok, 16);
    if (!ok) addr = addr_str.toUInt(&ok, 10);
    if (!ok && addr_str.startsWith("0x")) addr = addr_str.mid(2).toUInt(&ok, 16);

    if (ok) {
        watches.append({name, addr, watch_size->currentText() == "byte" ? 1 : 2});
        watch_name->clear();
        watch_addr->clear();
    }
}

void VariablesPanel::remove_watch(const QString& name) {
    for (int i = 0; i < watches.size(); ++i) {
        if (watches[i].name == name) {
            watches.removeAt(i);
            break;
        }
    }
}

void VariablesPanel::show_context_menu(const QPoint& pos) {
    QTableWidgetItem* item = table->itemAt(pos);
    if (!item) return;
    QString addr_str = table->item(item->row(), 1)->text();
    if (addr_str == "-") return;

    QMenu menu(this);
    auto* act_jump = menu.addAction("Show in Memory Viewer");
    if (menu.exec(table->viewport()->mapToGlobal(pos)) == act_jump) {
        emit show_in_memory(addr_str.toUInt(nullptr, 16));
    }
}

} // namespace memu8086::ui