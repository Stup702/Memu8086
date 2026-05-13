#pragma once

#include <QDialog>
#include <QString>

namespace memu8086::ui {

class ExamplesDialog : public QDialog {
    Q_OBJECT
public:
    explicit ExamplesDialog(QWidget* parent = nullptr);

signals:
    void load_example(const QString& source);
};

} // namespace memu8086::ui