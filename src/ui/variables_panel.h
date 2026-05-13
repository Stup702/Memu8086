#pragma once

#include <QWidget>
#include <QTableWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QList>
#include <QMap>
#include <map>
#include <string>
#include "core/cpu.h"
#include "assembler/assembler.h"

namespace memu8086::ui {

class VariablesPanel : public QWidget {
    Q_OBJECT
public:
    explicit VariablesPanel(QWidget* parent = nullptr);

    void set_symbols(const std::map<std::string, uint16_t>& syms);
    void update(const emu8086::core::CPU& cpu);

    // Aliases to seamlessly attach to MainWindow
    void refresh(const emu8086::assembler::AssemblyResult& res);
    void refresh();

signals:
    void show_in_memory(uint32_t addr);

private slots:
    void on_add_watch();
    void remove_watch(const QString& name);
    void show_context_menu(const QPoint& pos);

private:
    QTableWidget* table;
    QLineEdit* watch_name;
    QLineEdit* watch_addr;
    QComboBox* watch_size;
    QPushButton* btn_add;

    struct WatchEntry { QString name; uint32_t addr; int size; };
    QList<WatchEntry> watches;
    std::map<std::string, uint16_t> symbols;
    QMap<QString, uint16_t> prev_values;

    void rebuild_table(const emu8086::core::CPU& cpu);
};

} // namespace memu8086::ui