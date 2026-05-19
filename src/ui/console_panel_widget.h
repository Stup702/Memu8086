// memu8086
// Copyright (C) 2026 Animesh Barua Mugdha
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

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