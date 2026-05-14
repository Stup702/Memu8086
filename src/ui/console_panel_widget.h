#pragma once

#include <QWidget>
#include <QPlainTextEdit>

namespace emu8086::core {
    struct ConsoleState;
}

namespace memu8086::ui {

class ConsolePanelWidget : public QWidget {
    Q_OBJECT
public:
    explicit ConsolePanelWidget(emu8086::core::ConsoleState& state, QWidget* parent = nullptr);
    ~ConsolePanelWidget() override = default;

    void refresh(); // call every frame — repaints the screen
    QSize sizeHint() const override;

signals:
    void key_pressed(char c);

protected:
    bool eventFilter(QObject* obj, QEvent* event) override;

private:
    emu8086::core::ConsoleState& state;
    QPlainTextEdit* terminal;
    QString last_render;
};

} // namespace memu8086::ui