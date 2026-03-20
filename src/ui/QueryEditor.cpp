#include "QueryEditor.h"
#include "services/DatabaseService.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QShortcut>
#include <QKeySequence>
#include <QFont>
#include <QFontDatabase>
#include <QSyntaxHighlighter>
#include <QRegularExpression>
#include <QTextCharFormat>
#include <QHeaderView>
#include <QApplication>

namespace PGViewer {

// ── SQL Syntax Highlighter ────────────────────────────────────────────────────

class SqlHighlighter : public QSyntaxHighlighter {
public:
    explicit SqlHighlighter(QTextDocument* parent) : QSyntaxHighlighter(parent)
    {
        // ── Keywords ──────────────────────────────────────────────────────
        QTextCharFormat kwFmt;
        kwFmt.setForeground(QColor("#2980B9"));
        kwFmt.setFontWeight(QFont::Bold);

        const QStringList keywords = {
            "SELECT","FROM","WHERE","AND","OR","NOT","IN","IS","AS","ON",
            "INSERT","INTO","VALUES","UPDATE","SET","DELETE","TRUNCATE",
            "CREATE","TABLE","VIEW","INDEX","SCHEMA","DATABASE","DROP","ALTER","ADD",
            "JOIN","LEFT","RIGHT","INNER","OUTER","FULL","CROSS","NATURAL",
            "GROUP","BY","ORDER","HAVING","LIMIT","OFFSET","DISTINCT","ALL",
            "UNION","INTERSECT","EXCEPT","WITH","RECURSIVE",
            "CASE","WHEN","THEN","ELSE","END",
            "BEGIN","COMMIT","ROLLBACK","TRANSACTION","SAVEPOINT",
            "COUNT","SUM","AVG","MAX","MIN","COALESCE","NULLIF","CAST",
            "NOW","CURRENT_TIMESTAMP","CURRENT_DATE","CURRENT_USER",
            "EXISTS","BETWEEN","LIKE","ILIKE","SIMILAR","RETURNING",
            "PRIMARY","KEY","FOREIGN","REFERENCES","UNIQUE","NOT","NULL","DEFAULT",
            "CONSTRAINT","CHECK","SERIAL","BIGSERIAL","SEQUENCE"
        };
        for (const QString& k : keywords) {
            rules.append({
                QRegularExpression(
                    QString("\\b%1\\b").arg(k),
                    QRegularExpression::CaseInsensitiveOption
                ),
                kwFmt
            });
        }

        // ── Data types ────────────────────────────────────────────────────
        QTextCharFormat typeFmt;
        typeFmt.setForeground(QColor("#8E44AD"));
        const QStringList types = {
            "INTEGER","INT","BIGINT","SMALLINT","NUMERIC","DECIMAL","REAL",
            "DOUBLE","PRECISION","BOOLEAN","BOOL","TEXT","VARCHAR","CHAR",
            "BYTEA","DATE","TIME","TIMESTAMP","TIMESTAMPTZ","INTERVAL",
            "UUID","JSON","JSONB","ARRAY","HSTORE","INET","CIDR","MACADDR"
        };
        for (const QString& t : types) {
            rules.append({
                QRegularExpression(
                    QString("\\b%1\\b").arg(t),
                    QRegularExpression::CaseInsensitiveOption
                ),
                typeFmt
            });
        }

        // ── String literals ───────────────────────────────────────────────
        QTextCharFormat strFmt;
        strFmt.setForeground(QColor("#27AE60"));
        rules.append({ QRegularExpression("'[^']*'"), strFmt });
        rules.append({ QRegularExpression("\\$\\$.*\\$\\$"), strFmt });

        // ── Numbers ───────────────────────────────────────────────────────
        QTextCharFormat numFmt;
        numFmt.setForeground(QColor("#E67E22"));
        rules.append({ QRegularExpression("\\b\\d+\\.?\\d*\\b"), numFmt });

        // ── Identifiers in quotes ─────────────────────────────────────────
        QTextCharFormat quotedIdFmt;
        quotedIdFmt.setForeground(QColor("#1A252F"));
        quotedIdFmt.setFontItalic(true);
        rules.append({ QRegularExpression("\"[^\"]*\""), quotedIdFmt });

        // ── Single-line comments ──────────────────────────────────────────
        QTextCharFormat commentFmt;
        commentFmt.setForeground(QColor("#7F8C9A"));
        commentFmt.setFontItalic(true);
        rules.append({ QRegularExpression("--[^\n]*"), commentFmt });

        // ── Block comment (multi-line) ────────────────────────────────────
        commentStartExpr = QRegularExpression("/\\*");
        commentEndExpr   = QRegularExpression("\\*/");
        blockCommentFmt  = commentFmt;
    }

protected:
    void highlightBlock(const QString& text) override
    {
        // Apply single-line rules first
        for (const auto& [pattern, fmt] : rules) {
            auto it = pattern.globalMatch(text);
            while (it.hasNext()) {
                auto m = it.next();
                setFormat(m.capturedStart(), m.capturedLength(), fmt);
            }
        }

        // Multi-line block comments
        setCurrentBlockState(0);
        int startIdx = 0;

        if (previousBlockState() != 1) {
            auto m = commentStartExpr.match(text);
            startIdx = m.hasMatch() ? m.capturedStart() : -1;
        }

        while (startIdx >= 0) {
            auto endMatch = commentEndExpr.match(text, startIdx);
            int  endIdx, commentLen;
            if (endMatch.hasMatch()) {
                endIdx     = endMatch.capturedStart();
                commentLen = endIdx - startIdx + endMatch.capturedLength();
            } else {
                setCurrentBlockState(1);
                commentLen = text.length() - startIdx;
            }
            setFormat(startIdx, commentLen, blockCommentFmt);
            if (!endMatch.hasMatch()) break;
            auto next = commentStartExpr.match(text, startIdx + commentLen);
            startIdx = next.hasMatch() ? next.capturedStart() : -1;
        }
    }

private:
    struct Rule { QRegularExpression pattern; QTextCharFormat format; };
    QVector<Rule>      rules;
    QRegularExpression commentStartExpr, commentEndExpr;
    QTextCharFormat    blockCommentFmt;
};

// ── QueryEditor ───────────────────────────────────────────────────────────────

QueryEditor::QueryEditor(DatabaseService* svc, QWidget* parent)
    : QWidget(parent), m_svc(svc)
{
    buildUi();
}

void QueryEditor::buildUi()
{
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    // ── Toolbar ────────────────────────────────────────────────────────────
    auto* bar = new QWidget(this);
    bar->setFixedHeight(44);
    bar->setStyleSheet(
        "background: palette(window); border-bottom: 1px solid palette(mid);"
    );
    auto* bl = new QHBoxLayout(bar);
    bl->setContentsMargins(8, 0, 8, 0);
    bl->setSpacing(8);

    m_runBtn = new QPushButton("▶  Run  (F5)", bar);
    m_runBtn->setToolTip("Execute query (F5)\nSelect text to run partial query");
    m_runBtn->setMinimumWidth(110);

    auto* clearBtn = new QPushButton("Clear", bar);
    clearBtn->setObjectName("secondaryBtn");
    clearBtn->setToolTip("Clear editor");

    m_statusLbl = new QLabel(bar);
    m_statusLbl->setStyleSheet("font-size: 12px;");

    m_timeLbl = new QLabel(bar);
    m_timeLbl->setStyleSheet("font-size: 12px; color: palette(mid);");

    bl->addWidget(m_runBtn);
    bl->addWidget(clearBtn);
    bl->addStretch();
    bl->addWidget(m_statusLbl);
    bl->addSpacing(16);
    bl->addWidget(m_timeLbl);

    connect(m_runBtn,  &QPushButton::clicked, this, &QueryEditor::onExecute);
    connect(clearBtn,  &QPushButton::clicked, this, &QueryEditor::onClear);

    root->addWidget(bar);

    // ── Splitter: editor (top) + results (bottom) ──────────────────────────
    auto* splitter = new QSplitter(Qt::Vertical, this);
    splitter->setHandleWidth(4);

    // Editor
    m_editor = new QTextEdit(splitter);
    m_editor->setPlaceholderText(
        "-- Write your SQL query here\n"
        "-- Press F5 or click ▶ Run to execute\n"
        "-- Tip: select a portion of text to run only that part\n\n"
        "SELECT * FROM public.your_table LIMIT 100;"
    );
    m_editor->setAcceptRichText(false);

    // Choose best available monospace font
    QStringList monoFonts = {
        "Cascadia Code", "JetBrains Mono", "Fira Code",
        "Source Code Pro", "Consolas", "Courier New", "Courier"
    };
    QFont editorFont("Courier New");
    for (const QString& f : monoFonts) {
        if (QFontDatabase::families().contains(f)) {
            editorFont.setFamily(f);
            break;
        }
    }
    editorFont.setPointSize(13);
    m_editor->setFont(editorFont);

    // Tab = 4 spaces
    QFontMetrics fm(editorFont);
    m_editor->setTabStopDistance(fm.horizontalAdvance(' ') * 4);

    new SqlHighlighter(m_editor->document()); // owned by document

    // Results table
    m_model = new QStandardItemModel(this);

    m_results = new QTableView(splitter);
    m_results->setModel(m_model);
    m_results->horizontalHeader()->setStretchLastSection(false);
    m_results->horizontalHeader()->setHighlightSections(false);
    m_results->verticalHeader()->setDefaultSectionSize(24);
    m_results->setAlternatingRowColors(true);
    m_results->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_results->setSelectionBehavior(QAbstractItemView::SelectRows);

    splitter->addWidget(m_editor);
    splitter->addWidget(m_results);
    splitter->setStretchFactor(0, 2);
    splitter->setStretchFactor(1, 3);
    splitter->setSizes({250, 350});

    root->addWidget(splitter);

    // ── Keyboard shortcuts ─────────────────────────────────────────────────
    auto* f5 = new QShortcut(QKeySequence(Qt::Key_F5), this);
    connect(f5, &QShortcut::activated, this, &QueryEditor::onExecute);

    auto* ctrlEnter = new QShortcut(
        QKeySequence(Qt::CTRL | Qt::Key_Return), this
    );
    connect(ctrlEnter, &QShortcut::activated, this, &QueryEditor::onExecute);
}

// ── Slots ─────────────────────────────────────────────────────────────────────

void QueryEditor::onExecute()
{
    // Run selected text if any, otherwise run full editor content
    QString sql = m_editor->textCursor().hasSelection()
                  ? m_editor->textCursor().selectedText()
                  : m_editor->toPlainText().trimmed();

    // Replace Qt's special Unicode paragraph separator with newline
    sql.replace(QChar(0x2029), '\n');
    sql = sql.trimmed();

    if (sql.isEmpty()) return;

    m_runBtn->setEnabled(false);
    m_statusLbl->setText("Running…");
    m_statusLbl->setStyleSheet("color: #3498DB; font-size: 12px;");
    m_timeLbl->clear();
    QApplication::processEvents(); // keep UI responsive during sync call

    QueryResult res = m_svc->executeQuery(sql);

    m_runBtn->setEnabled(true);
    displayResult(res);
    emit statusMessage(res.summary());
}

void QueryEditor::displayResult(const QueryResult& res)
{
    m_model->clear();
    m_timeLbl->setText(QString("Elapsed: %1 ms").arg(res.elapsedMs));

    if (!res.success) {
        m_statusLbl->setText("❌  " + res.errorMessage);
        m_statusLbl->setStyleSheet("color: #E74C3C; font-size: 12px;");
        return;
    }

    if (res.hasData()) {
        m_model->setColumnCount(res.columns.size());
        m_model->setHorizontalHeaderLabels(res.columns);

        for (const auto& row : res.rows) {
            QList<QStandardItem*> items;
            for (const QVariant& val : row) {
                auto* cell = new QStandardItem(
                    val.isNull() ? "(null)" : val.toString()
                );
                if (val.isNull())
                    cell->setForeground(QColor("#95A5A6"));
                items << cell;
            }
            m_model->appendRow(items);
        }

        // Auto-resize columns
        m_results->resizeColumnsToContents();
        for (int c = 0; c < m_model->columnCount(); ++c) {
            if (m_results->columnWidth(c) > 350)
                m_results->setColumnWidth(c, 350);
        }

        m_statusLbl->setText(
            QString("✔  %1 row(s) returned").arg(res.rows.size())
        );
        m_statusLbl->setStyleSheet("color: #27AE60; font-size: 12px;");
    } else {
        m_statusLbl->setText(
            QString("✔  Query OK — %1 row(s) affected")
            .arg(res.rowsAffected)
        );
        m_statusLbl->setStyleSheet("color: #27AE60; font-size: 12px;");
    }
}

void QueryEditor::onClear()
{
    m_editor->clear();
    m_model->clear();
    m_statusLbl->clear();
    m_timeLbl->clear();
}

void QueryEditor::onFormatQuery()
{
    // Basic uppercase keyword formatting
    QString sql = m_editor->toPlainText();
    const QStringList keywords = {
        "select","from","where","and","or","join","left","right","inner",
        "outer","on","group","by","order","having","limit","offset",
        "insert","into","values","update","set","delete","with","union"
    };
    for (const QString& kw : keywords) {
        QRegularExpression re(QString("\\b%1\\b").arg(kw),
                              QRegularExpression::CaseInsensitiveOption);
        sql.replace(re, kw.toUpper());
    }
    m_editor->setPlainText(sql);
}

} // namespace PGViewer
