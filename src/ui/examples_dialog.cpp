#include "examples_dialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidget>
#include <QPushButton>

namespace memu8086::ui {

struct ExampleProgram {
    QString name;
    QString code;
};

static const QList<ExampleProgram> EXAMPLES = {
    {"Hello World (DOS INT 21h)", "org 100h\n\nmov dx, msg\nmov ah, 09h\nint 21h\nint 20h\n\nmsg db 'Hello, World!$'\n"},
    {"Print Alphabet", "org 100h\n\nmov cx, 26\nmov dl, 'A'\n\nprint_loop:\nmov ah, 02h\nint 21h\ninc dl\nloop print_loop\n\nint 20h\n"}
};

ExamplesDialog::ExamplesDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle("Load Example");
    setMinimumSize(300, 400);

    QVBoxLayout* layout = new QVBoxLayout(this);
    QListWidget* list = new QListWidget(this);
    for (const auto& ex : EXAMPLES) {
        list->addItem(ex.name);
    }
    layout->addWidget(list);

    QHBoxLayout* btn_layout = new QHBoxLayout();
    QPushButton* btn_load = new QPushButton("Load");
    QPushButton* btn_cancel = new QPushButton("Cancel");
    btn_layout->addStretch();
    btn_layout->addWidget(btn_cancel);
    btn_layout->addWidget(btn_load);
    layout->addLayout(btn_layout);

    connect(btn_cancel, &QPushButton::clicked, this, &QDialog::reject);
    
    auto do_load = [this, list]() {
        int idx = list->currentRow();
        if (idx >= 0 && idx < EXAMPLES.size()) {
            emit load_example(EXAMPLES[idx].code);
            accept();
        }
    };
    connect(list, &QListWidget::itemDoubleClicked, this, do_load);
    connect(btn_load, &QPushButton::clicked, this, do_load);
}

} // namespace memu8086::ui