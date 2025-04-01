/*
 featherpad/syntax.cpp
 */

#include "singleton.h"
#include "ui_fp.h"

#include <QMimeDatabase>
#include <QFileInfo>
#include <QRegularExpression>
#include <QTimer>

namespace FeatherPad {

/*!
 * \brief Helper that fetches a QMimeType for a given file.
 */
static QMimeType getMimeType(const QFileInfo& fInfo) {
    static QMimeDatabase mimeDatabase;
    return mimeDatabase.mimeTypeForFile(fInfo);
}

/*!
 * \brief A lookup table for commonly used file extensions mapped to FeatherPad language keys.
 * Extension checks are done case-sensitively or insensitively as appropriate.
 */
static const struct {
    QString extension;   // e.g. ".cpp", ".py"
    bool caseSensitive;  // whether matching is case-sensitive
    QString language;    // e.g. "cpp", "python"
} extensionLanguageMap[] = {{".cpp", true, "cpp"},
                            {".cxx", true, "cpp"},
                            {".h", true, "cpp"},
                            {".c", true, "c"},
                            {".sh", true, "sh"},
                            {".ebuild", true, "sh"},
                            {".eclass", true, "sh"},
                            {".zsh", true, "sh"},
                            {".rb", true, "ruby"},
                            {".lua", true, "lua"},
                            {".nelua", true, "lua"},
                            {".py", true, "python"},
                            {".pl", true, "perl"},
                            {".pro", true, "qmake"},
                            {".pri", true, "qmake"},
                            {".tr", true, "troff"},
                            {".t", true, "troff"},
                            {".roff", true, "troff"},
                            {".tex", true, "LaTeX"},
                            {".ltx", true, "LaTeX"},
                            {".latex", true, "LaTeX"},
                            {".lyx", true, "LaTeX"},
                            {".xml", false, "xml"},
                            {".svg", false, "xml"},
                            {".qrc", true, "xml"},
                            {".rdf", true, "xml"},
                            {".docbook", true, "xml"},
                            {".fnx", true, "xml"},
                            {".ts", true, "xml"},
                            {".menu", true, "xml"},
                            {".kml", false, "xml"},
                            {".xspf", false, "xml"},
                            {".asx", false, "xml"},
                            {".nfo", true, "xml"},
                            {".dae", true, "xml"},
                            {".css", true, "css"},
                            {".qss", true, "css"},
                            {".scss", true, "scss"},
                            {".p", true, "pascal"},
                            {".pas", true, "pascal"},
                            {".desktop", true, "desktop"},
                            {".desktop.in", true, "desktop"},
                            {".directory", true, "desktop"},
                            {".kvconfig", true, "config"},
                            {".service", true, "config"},
                            {".mount", true, "config"},
                            {".timer", true, "config"},
                            {".pls", false, "config"},
                            {".js", true, "javascript"},
                            {".hx", true, "javascript"},
                            {".java", true, "java"},
                            {".json", true, "json"},
                            {".qml", true, "qml"},
                            {".log", false, "log"},
                            {".php", true, "php"},
                            {".diff", true, "diff"},
                            {".patch", true, "diff"},
                            {".srt", true, "srt"},
                            {".theme", true, "theme"},
                            {".fountain", true, "fountain"},
                            {".yml", true, "yaml"},
                            {".yaml", true, "yaml"},
                            {".m3u", false, "m3u"},
                            {".htm", false, "html"},
                            {".html", false, "html"},
                            {".markdown", true, "markdown"},
                            {".md", true, "markdown"},
                            {".mkd", true, "markdown"},
                            {".rst", true, "reST"},
                            {".dart", true, "dart"},
                            {".go", true, "go"},
                            {".rs", true, "rust"},
                            {".tcl", true, "tcl"},
                            {".tk", true, "tcl"},
                            {".toml", true, "toml"}};

/*!
 * \brief A lookup table for specific filenames (all compared case-insensitively) to language keys.
 */
static const QHash<QString, QString> specialFilenamesMap = {{"makefile", "makefile"},
                                                            {"makefile.am", "makefile"},
                                                            {"makelist", "makefile"},
                                                            {"pkgbuild", "sh"},  // Arch PKGBUILD
                                                            {"fstab", "sh"},
                                                            {"changelog", "changelog"},
                                                            {"gtkrc", "gtkrc"},
                                                            {"control", "deb"},
                                                            {"mirrorlist", "config"},
                                                            {"themerc", "openbox"},
                                                            {"bashrc", "sh"},
                                                            {"bash_profile", "sh"},
                                                            {"bash_functions", "sh"},
                                                            {"bash_logout", "sh"},
                                                            {"bash_aliases", "sh"},
                                                            {"xprofile", "sh"},
                                                            {"profile", "sh"},
                                                            {"mkshrc", "sh"},
                                                            {"zprofile", "sh"},
                                                            {"zlogin", "sh"},
                                                            {"zshrc", "sh"},
                                                            {"zshenv", "sh"},
                                                            {"cmakelists.txt", "cmake"}};

/*!
 * \brief A lookup table for MIME types to language keys.
 */
static const QHash<QString, QString> mimeLanguageMap = {{"text/x-c++", "cpp"},
                                                        {"text/x-c++src", "cpp"},
                                                        {"text/x-c++hdr", "cpp"},
                                                        {"text/x-chdr", "cpp"},
                                                        {"text/x-c", "c"},
                                                        {"text/x-csrc", "c"},
                                                        {"application/x-shellscript", "sh"},
                                                        {"text/x-shellscript", "sh"},
                                                        {"application/x-ruby", "ruby"},
                                                        {"text/x-lua", "lua"},
                                                        {"application/x-perl", "perl"},
                                                        {"text/x-makefile", "makefile"},
                                                        {"text/x-cmake", "cmake"},
                                                        {"application/vnd.nokia.qt.qmakeprofile", "qmake"},
                                                        {"text/troff", "troff"},
                                                        {"application/x-troff-man", "troff"},
                                                        {"text/x-tex", "LaTeX"},
                                                        {"application/x-lyx", "LaTeX"},
                                                        {"text/html", "html"},
                                                        {"application/xhtml+xml", "html"},
                                                        {"application/xml", "xml"},
                                                        {"application/xml-dtd", "xml"},
                                                        {"text/feathernotes-fnx", "xml"},
                                                        {"audio/x-ms-asx", "xml"},
                                                        {"text/x-nfo", "xml"},
                                                        {"text/css", "css"},
                                                        {"text/x-scss", "scss"},
                                                        {"text/x-pascal", "pascal"},
                                                        {"text/x-changelog", "changelog"},
                                                        {"application/x-desktop", "desktop"},
                                                        {"audio/x-scpls", "config"},
                                                        {"application/vnd.kde.kcfgc", "config"},
                                                        {"application/javascript", "javascript"},
                                                        {"text/javascript", "javascript"},
                                                        {"text/x-java", "java"},
                                                        {"application/json", "json"},
                                                        {"application/schema+json", "json"},
                                                        {"text/x-qml", "qml"},
                                                        {"text/x-log", "log"},
                                                        {"application/x-php", "php"},
                                                        {"text/x-php", "php"},
                                                        {"application/x-theme", "theme"},
                                                        {"text/x-diff", "diff"},
                                                        {"text/x-patch", "diff"},
                                                        {"text/markdown", "markdown"},
                                                        {"audio/x-mpegurl", "m3u"},
                                                        {"application/vnd.apple.mpegurl", "m3u"},
                                                        {"text/x-go", "go"},
                                                        {"text/rust", "rust"},
                                                        {"text/x-tcl", "tcl"},
                                                        {"text/tcl", "tcl"},
                                                        {"application/toml", "toml"}};

//------------------------------------------------------------------------------
/*!
 * \brief Utility: checks if \a baseName matches any special filename (case-insensitive).
 */
static QString languageForSpecialFilename(const QString& baseName) {
    const QString key = baseName.toLower();
    if (specialFilenamesMap.contains(key)) {
        return specialFilenamesMap.value(key);
    }
    return QString();
}

//------------------------------------------------------------------------------
/*!
 * \brief Utility: checks if \a fname ends with a known extension from \c extensionLanguageMap.
 * Returns an empty string if not found.
 */
static QString languageForExtension(const QString& fname) {
    const QString lowerFname = fname.toLower();
    for (auto& entry : extensionLanguageMap) {
        // If case-insensitive, compare with lowerFname; otherwise compare directly
        if (entry.caseSensitive) {
            if (fname.endsWith(entry.extension)) {
                return entry.language;
            }
        }
        else {
            if (lowerFname.endsWith(entry.extension)) {
                return entry.language;
            }
        }
    }
    return QString();  // not found
}

//------------------------------------------------------------------------------
/*!
 * \brief Utility: checks if the QMimeType is recognized in \c mimeLanguageMap.
 * Returns an empty string if not found.
 */
static QString languageForMime(const QMimeType& mimeType) {
    const QString mime = mimeType.name();
    if (mimeLanguageMap.contains(mime)) {
        return mimeLanguageMap.value(mime);
    }
    // Check parent mime types as fallback:
    for (const auto& parentMime : mimeType.parentMimeTypes()) {
        if (mimeLanguageMap.contains(parentMime)) {
            return mimeLanguageMap.value(parentMime);
        }
    }
    return QString();  // not found
}

//------------------------------------------------------------------------------
/*!
 * \brief Resolves symlinks and returns the final canonical path if possible,
 * otherwise the original or symlink target.
 */
static QString resolvedFilePath(const QString& filename) {
    QFileInfo info(filename);
    if (!info.exists())
        return filename;

    if (info.isSymLink()) {
        const QString finalTarget = info.canonicalFilePath();
        if (!finalTarget.isEmpty()) {
            return finalTarget;
        }
        else {
            return info.symLinkTarget();
        }
    }
    return filename;
}

//------------------------------------------------------------------------------
/*!
 * \brief Determine and set the program language of a TextEdit based on filename, extension or MIME type.
 * Falls back to "url".
 */
void FPwin::setProgLang(TextEdit* textEdit) {
    if (!textEdit)
        return;

    QString fname = textEdit->getFileName();
    if (fname.isEmpty())
        return;

    // If it's a symlink, resolve it
    fname = resolvedFilePath(fname);

    // If file ends with ".sub", do not set any language => default "url"
    if (fname.endsWith(".sub", Qt::CaseInsensitive)) {
        return;
    }

    // Step 1: Check special filenames (Makefile, PKGBUILD, etc.)
    QString baseName = QFileInfo(fname).fileName();  // or fname.section('/', -1);
    QString lang = languageForSpecialFilename(baseName);
    if (!lang.isEmpty()) {
        textEdit->setProg(lang);
        return;
    }

    // Step 2: If there's an extension, try extension-based detection
    lang = languageForExtension(fname);
    if (!lang.isEmpty()) {
        textEdit->setProg(lang);
        return;
    }

    // Step 3: If all else fails, check MIME type
    {
        QFileInfo fInfo(fname);
        if (fInfo.exists()) {
            QMimeType mimeType = getMimeType(fInfo);
            // Python might come as text/x-python3, etc.
            // So, do a quick check for "text/x-python" prefix:
            const QString mimeName = mimeType.name();
            if (mimeName.startsWith("text/x-python")) {
                lang = "python";
            }
            else {
                // Fallback to direct mapping:
                lang = languageForMime(mimeType);
            }
        }
        else {
            // If file doesn't exist on disk, fallback to "url"
            lang = "url";
        }
    }

    if (lang.isEmpty()) {
        // Finally, fallback to "url"
        lang = "url";
    }

    textEdit->setProg(lang);
}

//------------------------------------------------------------------------------
void FPwin::toggleSyntaxHighlighting() {
    const int count = ui->tabWidget->count();
    if (count == 0)
        return;

    bool enableSH = ui->actionSyntax->isChecked();
    if (enableSH)
        makeBusy();  // it may take a while with huge texts

    for (int i = 0; i < count; ++i) {
        auto tabPage = qobject_cast<TabPage*>(ui->tabWidget->widget(i));
        if (!tabPage)
            continue;

        TextEdit* textEdit = tabPage->textEdit();
        syntaxHighlighting(textEdit, enableSH, textEdit->getLang());
    }

    // Update language button for the current tab
    if (auto tabPage = qobject_cast<TabPage*>(ui->tabWidget->currentWidget())) {
        updateLangBtn(tabPage->textEdit());
    }

    if (enableSH) {
        // Defer unbusy so the UI can refresh
        QTimer::singleShot(0, this, &FPwin::unbusy);
    }
}

//------------------------------------------------------------------------------
void FPwin::syntaxHighlighting(TextEdit* textEdit, bool highlight, const QString& lang) {
    if (!textEdit || textEdit->isUneditable()) {
        return;
    }

    if (highlight) {
        QString progLan = lang.isEmpty() ? textEdit->getProg() : lang;

        // Some special checks
        if (progLan.isEmpty() || progLan == "help") {
            return;
        }

        Config config = static_cast<FPsingleton*>(qApp)->getConfig();
        const qint64 textSize = textEdit->getSize();
        const qint64 maxSize = config.getMaxSHSize() * 1024LL * 1024LL;
        if (textSize > maxSize) {
            // Warn user if active tab is this textEdit
            QTimer::singleShot(100, textEdit, [=]() {
                auto tabPage = qobject_cast<TabPage*>(ui->tabWidget->currentWidget());
                if (tabPage && tabPage->textEdit() == textEdit) {
                    showWarningBar(
                        tr("<center><b><big>The size limit for syntax highlighting is exceeded.</big></b></center>"));
                }
            });
            return;
        }

        // Create the highlighter if it doesn't exist
        if (!qobject_cast<Highlighter*>(textEdit->getHighlighter())) {
            QPoint topLeft(0, 0);
            QTextCursor start = textEdit->cursorForPosition(topLeft);

            // Use geometry to get bottom-right
            QPoint bottomRight(textEdit->width(), textEdit->height());
            QTextCursor end = textEdit->cursorForPosition(bottomRight);

            textEdit->setDrawIndetLines(config.getShowWhiteSpace());
            textEdit->setVLineDistance(config.getVLineDistance());

            auto highlighter = new Highlighter(
                textEdit->document(), progLan, start, end, textEdit->hasDarkScheme(), config.getShowWhiteSpace(),
                config.getShowEndings(), config.getWhiteSpaceValue(),
                config.customSyntaxColors().isEmpty()
                    ? (textEdit->hasDarkScheme() ? config.darkSyntaxColors() : config.lightSyntaxColors())
                    : config.customSyntaxColors());
            textEdit->setHighlighter(highlighter);
        }

        // Connect signals after syntax highlighting is set
        QTimer::singleShot(0, textEdit, [this, textEdit]() {
            if (textEdit->isVisible()) {
                formatTextRect();
                matchBrackets();
            }
            connect(textEdit, &TextEdit::updateBracketMatching, this, &FPwin::matchBrackets);
            connect(textEdit, &QPlainTextEdit::blockCountChanged, this, &FPwin::formatOnBlockChange);
            connect(textEdit, &TextEdit::updateRect, this, &FPwin::formatTextRect);
            connect(textEdit, &TextEdit::resized, this, &FPwin::formatTextRect);
            connect(textEdit->document(), &QTextDocument::contentsChange, this, &FPwin::formatOnTextChange);
        });
    }
    else {
        // If turning highlighting off, remove the highlighter and disconnect signals
        auto highlighter = qobject_cast<Highlighter*>(textEdit->getHighlighter());
        if (highlighter) {
            disconnect(textEdit->document(), &QTextDocument::contentsChange, this, &FPwin::formatOnTextChange);
            disconnect(textEdit, &TextEdit::resized, this, &FPwin::formatTextRect);
            disconnect(textEdit, &TextEdit::updateRect, this, &FPwin::formatTextRect);
            disconnect(textEdit, &QPlainTextEdit::blockCountChanged, this, &FPwin::formatOnBlockChange);
            disconnect(textEdit, &TextEdit::updateBracketMatching, this, &FPwin::matchBrackets);

            // Remove bracket highlights
            QList<QTextEdit::ExtraSelection> es = textEdit->extraSelections();
            int n = textEdit->getRedSel().count();
            while (n > 0 && !es.isEmpty()) {
                es.removeLast();
                --n;
            }
            textEdit->setRedSel(QList<QTextEdit::ExtraSelection>());
            textEdit->setExtraSelections(es);

            // Turn off indentation lines
            textEdit->setDrawIndetLines(false);
            textEdit->setVLineDistance(0);

            delete highlighter;
        }
    }
}

//------------------------------------------------------------------------------
void FPwin::formatOnTextChange(int /*position*/, int charsRemoved, int charsAdded) const {
    if (charsRemoved > 0 || charsAdded > 0) {
        // Defer so the layout manager can update
        QTimer::singleShot(0, this, &FPwin::formatTextRect);
    }
}

//------------------------------------------------------------------------------
void FPwin::formatOnBlockChange(int /* newBlockCount */) const {
    formatTextRect();
}

//------------------------------------------------------------------------------
void FPwin::formatTextRect() const {
    // This function is supposed to be called for the current tab
    auto tabPage = qobject_cast<TabPage*>(ui->tabWidget->currentWidget());
    if (!tabPage)
        return;

    TextEdit* textEdit = tabPage->textEdit();
    auto highlighter = qobject_cast<Highlighter*>(textEdit->getHighlighter());
    if (!highlighter)
        return;

    QPoint topLeft(0, 0);
    QTextCursor start = textEdit->cursorForPosition(topLeft);

    QPoint bottomRight(textEdit->width(), textEdit->height());
    QTextCursor end = textEdit->cursorForPosition(bottomRight);

    highlighter->setLimit(start, end);

    QTextBlock block = start.block();
    while (block.isValid() && block.blockNumber() <= end.blockNumber()) {
        if (auto data = static_cast<TextBlockData*>(block.userData())) {
            if (!data->isHighlighted()) {
                highlighter->rehighlightBlock(block);
            }
        }
        block = block.next();
    }
}

}  // namespace FeatherPad
