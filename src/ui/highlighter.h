#pragma once

#include <QSyntaxHighlighter>
#include <QRegularExpression>
#include <QTextCharFormat>

namespace memu8086::ui {

class AsmHighlighter : public QSyntaxHighlighter {
    Q_OBJECT
public:
    explicit AsmHighlighter(QTextDocument* parent);

protected:
    void highlightBlock(const QString& text) override;

private:
    struct Rule { QRegularExpression pattern; QTextCharFormat format; };
    QList<Rule> rules;
    Rule comment_rule;
    Rule string_rule;
};

} // namespace memu8086::ui