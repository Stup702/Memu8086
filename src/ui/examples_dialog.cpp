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
    {"Hello World", ".model small\n.stack 100h\n\n.data\n    msg db 'Hello, Architecture!', 13, 10, '$' ; 13=CR, 10=LF, $=terminator\n\n.code\nmain proc\n    ; Initialize Data Segment\n    mov ax, @data\n    mov ds, ax\n\n    ; Print string interrupt\n    mov ah, 09h\n    lea dx, msg\n    int 21h\n\n    ; Exit to DOS safely\n    mov ah, 4Ch\n    int 21h\nmain endp\nend main\n"},
    {"Echoing a Character (Read and Print)", ".model small\n.stack 100h\n.code\nmain proc\n    ; Read a single character from keyboard into AL\n    mov ah, 01h\n    int 21h\n\n    ; Move the read character from AL to DL for printing\n    mov dl, al\n\n    ; Print the character in DL\n    mov ah, 02h\n    int 21h\n\n    mov ah, 4Ch\n    int 21h\nmain endp\nend main\n"},
    {"Adding Two 16-bit Variables", ".model small\n.data\n    num1 dw 1234h\n    num2 dw 5678h\n    sum  dw ?\n.code\nmain proc\n    mov ax, @data\n    mov ds, ax\n\n    mov ax, num1    ; Load first number into AX\n    add ax, num2    ; Add second number to AX\n    mov sum, ax     ; Store result in sum\n\n    mov ah, 4Ch\n    int 21h\nmain endp\nend main\n"},
    {"Multiplication and Division", ".model small\n.code\nmain proc\n    ; --- Multiplication ---\n    mov al, 5       ; AL = 5\n    mov bl, 10      ; BL = 10\n    mul bl          ; AX = AL * BL (AX becomes 50)\n\n    ; --- Division ---\n    mov ax, 100     ; AX = 100 (Dividend)\n    mov bl, 7       ; BL = 7 (Divisor)\n    div bl          ; AL = Quotient (14), AH = Remainder (2)\n\n    mov ah, 4Ch\n    int 21h\nmain endp\nend main\n"},
    {"Bitwise Operations (Masking)", ".model small\n.code\nmain proc\n    mov al, 11001100b ; Load binary value\n\n    and al, 11110000b ; Mask lower 4 bits (AL = 11000000b)\n    or  al, 00001111b ; Set lower 4 bits (AL = 11001111b)\n    xor al, 11111111b ; Invert all bits (AL = 00110000b)\n\n    mov ah, 4Ch\n    int 21h\nmain endp\nend main\n"},
    {"Using the LOOP Instruction (Print A to Z)", ".model small\n.stack 100h\n.code\nmain proc\n    mov cx, 26      ; Loop counter (26 letters)\n    mov dl, 'A'     ; ASCII value of 'A'\n    \n    mov ah, 02h     ; Prepare for character output\n\nprint_loop:\n    int 21h         ; Print character in DL\n    inc dl          ; Increment to next letter\n    loop print_loop ; Decrements CX; if CX != 0, jump to print_loop\n\n    mov ah, 4Ch\n    int 21h\nmain endp\nend main\n"}
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