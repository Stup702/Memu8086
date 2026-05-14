#include "custom_dock.h"
#include "theme.h"
#include <QMouseEvent>
#include <QApplication>
#include <QStyle>

namespace memu8086::ui {

// ─── DockTitleBar ────────────────────────────────────────────────────────────

DockTitleBar::DockTitleBar(const QString& title, QDockWidget* parent)
    : QWidget(parent), dock(parent)
{
    setAutoFillBackground(true);
    setFixedHeight(32);

    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(10, 0, 4, 0);
    layout->setSpacing(2);

    title_label = new QLabel(title.toUpper(), this);
    title_label->setFont(Theme::ui_font(11));
    // color applied via QSS

    layout->addWidget(title_label);
    layout->addStretch();

    // Float/dock-back button
    btn_float = new QToolButton(this);
    btn_float->setFixedSize(24, 24);
    btn_float->setToolTip("Undock panel");
    btn_float->setObjectName("dock_btn_float");
    btn_float->setText("⊟");
    btn_float->setFont(Theme::ui_font(14));
    connect(btn_float, &QToolButton::clicked, this, [this]() {
        dock->setFloating(!dock->isFloating());
    });
    layout->addWidget(btn_float);

    // Close button
    btn_close = new QToolButton(this);
    btn_close->setFixedSize(24, 24);
    btn_close->setToolTip("Close panel");
    btn_close->setObjectName("dock_btn_close");
    btn_close->setText("✕");
    btn_close->setFont(Theme::ui_font(14));
    connect(btn_close, &QToolButton::clicked, dock, &QDockWidget::close);
    layout->addWidget(btn_close);

    update_float_state(false);
}

void DockTitleBar::set_title(const QString& title) {
    title_label->setText(title.toUpper());
}

void DockTitleBar::update_float_state(bool is_floating) {
    if (is_floating) {
        btn_float->setText("⊞");
        btn_float->setToolTip("Dock panel back");
        btn_float->setObjectName("dock_btn_dock_back");
        btn_close->hide(); // Hide custom close button (OS provides its own when floating)
    } else {
        btn_float->setText("⊟");
        btn_float->setToolTip("Undock panel");
        btn_float->setObjectName("dock_btn_float");
        btn_close->show();
    }
    btn_float->style()->unpolish(btn_float);
    btn_float->style()->polish(btn_float);
}

void DockTitleBar::mouseDoubleClickEvent(QMouseEvent*) { dock->setFloating(!dock->isFloating()); }
void DockTitleBar::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton && dock->isFloating()) { drag_start_pos = event->globalPosition().toPoint() - dock->frameGeometry().topLeft(); dragging = true; }
    QWidget::mousePressEvent(event);
}
void DockTitleBar::mouseMoveEvent(QMouseEvent* event) {
    if (dragging && dock->isFloating()) { dock->move(event->globalPosition().toPoint() - drag_start_pos); }
    QWidget::mouseMoveEvent(event);
}
void DockTitleBar::mouseReleaseEvent(QMouseEvent* event) { dragging = false; QWidget::mouseReleaseEvent(event); }

// ─── CustomDockWidget ─────────────────────────────────────────────────────────
CustomDockWidget::CustomDockWidget(const QString& title, QWidget* parent) : QDockWidget(title, parent) {
    title_bar = new DockTitleBar(title, this); setTitleBarWidget(title_bar);
    connect(this, &QDockWidget::topLevelChanged, this, &CustomDockWidget::on_float_changed);
}
void CustomDockWidget::on_float_changed(bool floating) { title_bar->update_float_state(floating); }
} // namespace memu8086::ui