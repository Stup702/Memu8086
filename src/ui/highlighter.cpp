#include "highlighter.h"
#include <QColor>

namespace memu8086::ui {

AsmHighlighter::AsmHighlighter(QTextDocument* parent)
    : QSyntaxHighlighter(parent) {
    
    Rule rule;

    // Labels
    rule.pattern = QRegularExpression(QStringLiteral("\\b[A-Za-z0-9_]+:"));
    rule.format.setForeground(QColor("#C984FF"));
    rule.format.setFontWeight(QFont::Bold);
    rules.append(rule);

    // Directives
    rule.pattern = QRegularExpression(QStringLiteral("\\b(DB|DW|DD|ORG|EQU|RESB|RESW|SEGMENT|ENDS|PROC|ENDP|TIMES|\\.MODEL|\\.CODE|\\.DATA|\\.STACK)\\b"), QRegularExpression::CaseInsensitiveOption);
    rule.format.setForeground(QColor("#FF7E7E"));
    rule.format.setFontWeight(QFont::Normal);
    rules.append(rule);

    // Mnemonics
    rule.pattern = QRegularExpression(QStringLiteral("\\b(MOV|ADD|SUB|MUL|DIV|AND|OR|XOR|NOT|CMP|JMP|JE|JNE|JZ|JNZ|JL|JLE|JG|JGE|JB|JA|JBE|JAE|JS|JNS|JO|JNO|JP|JNP|JCXZ|CALL|RET|RETF|PUSH|POP|LOOP|LOOPE|LOOPNE|NOP|HLT|INT|IRET|INTO|XLAT|LEA|LDS|LES|XCHG|INC|DEC|NEG|CBW|CWD|SHL|SHR|SAL|SAR|ROL|ROR|RCL|RCR|ADC|SBB|DAA|DAS|AAA|AAS|AAM|AAD|MOVSB|MOVSW|STOSB|STOSW|LODSB|LODSW|SCASB|SCASW|CMPSB|CMPSW|REP|REPE|REPNE|REPZ|REPNZ|PUSHF|POPF|LAHF|SAHF|CLC|STC|CMC|CLD|STD|CLI|STI|WAIT|LOCK)\\b"), QRegularExpression::CaseInsensitiveOption);
    rule.format.setForeground(QColor("#4A9EFF"));
    rules.append(rule);

    // Registers
    rule.pattern = QRegularExpression(QStringLiteral("\\b(AX|BX|CX|DX|AH|AL|BH|BL|CH|CL|DH|DL|SI|DI|SP|BP|IP|CS|DS|ES|SS)\\b"), QRegularExpression::CaseInsensitiveOption);
    rule.format.setForeground(QColor("#FF9E64"));
    rules.append(rule);

    // Numbers (Hex, Bin, Dec)
    rule.pattern = QRegularExpression(QStringLiteral("\\b[0-9][0-9A-Fa-f]*[Hh]\\b|\\b0x[0-9A-Fa-f]+\\b|\\b[01]+[Bb]\\b|\\b\\d+\\b"));
    rule.format.setForeground(QColor("#B5EAD7"));
    rules.append(rule);

    // Comments and Strings apply last to override keywords inside them
    comment_rule.pattern = QRegularExpression(QStringLiteral(";[^\n]*"));
    comment_rule.format.setForeground(QColor("#6B7D6A"));
    comment_rule.format.setFontItalic(true);

    string_rule.pattern = QRegularExpression(QStringLiteral("(\"[^\"]*\"|'[^']*')"));
    string_rule.format.setForeground(QColor("#F7C948"));
}

void AsmHighlighter::highlightBlock(const QString& text) {
    for (const Rule& rule : std::as_const(rules)) {
        QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
        while (matchIterator.hasNext()) {
            QRegularExpressionMatch match = matchIterator.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }
    
    // Overrides
    for (const Rule& rule : {string_rule, comment_rule}) {
        QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
        while (matchIterator.hasNext()) {
            QRegularExpressionMatch match = matchIterator.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }
}

} // namespace memu8086::ui