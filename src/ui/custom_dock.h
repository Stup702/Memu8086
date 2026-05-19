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
#include <QDockWidget>
#include <QWidget>
#include <QLabel>
#include <QToolButton>
#include <QHBoxLayout>

namespace memu8086::ui {

class DockTitleBar : public QWidget {
    Q_OBJECT
public:
    explicit DockTitleBar(const QString& title, QDockWidget* parent);
    void set_title(const QString& title);
    void update_float_state(bool is_floating);
protected:
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
private:
    QDockWidget* dock;
    QLabel* title_label;
    QToolButton* btn_float;   // undock when docked / dock-back when floating
    QToolButton* btn_close;
    QPoint drag_start_pos;
    bool dragging = false;
};

class CustomDockWidget : public QDockWidget {
    Q_OBJECT
public:
    explicit CustomDockWidget(const QString& title, QWidget* parent = nullptr);
private slots:
    void on_float_changed(bool floating);
private:
    DockTitleBar* title_bar;
};

} // namespace memu8086::ui