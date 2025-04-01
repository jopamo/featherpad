#include "highlighter.h"

namespace FeatherPad {

/*!
 * \brief SH_MultiLineQuote highlights multi/single-line quotes for bash.
 *
 * This function attempts to detect and apply proper quote formatting (both single and double quotes)
 * in a multi-line context (e.g., continued from a previous block). It also considers here-doc
 * delimiters and additional complexities introduced by bash-like syntax.
 */
void Highlighter::SH_MultiLineQuote(const QString& text) {
    int index = 0;
    QRegularExpressionMatch quoteMatch;
    QRegularExpression quoteExpression = mixedQuoteMark;

    const int prevState = previousBlockState();
    const int initialState = currentBlockState();

    // Distinguish single/double-quoted states from the previous block.
    bool wasDoubleQuoted = (prevState == doubleQuoteState || prevState == SH_MixedDoubleQuoteState ||
                            prevState == SH_MixedSingleQuoteState);
    bool wasQuoted = (wasDoubleQuoted || prevState == singleQuoteState);

    // Check if the previous block ended with a here-doc delimiter (which can affect quoting).
    QTextBlock prevBlock = currentBlock().previous();
    if (prevBlock.isValid()) {
        auto* prevData = static_cast<TextBlockData*>(prevBlock.userData());
        // If we ended a here-doc in a line like: VAR="$(cat<<EOF"
        if (prevData && prevData->getProperty()) {
            wasQuoted = wasDoubleQuoted = true;
        }
    }

    // Check if the current block has a pending here-doc delimiter
    auto* curData = static_cast<TextBlockData*>(currentBlock().userData());
    int hereDocDelimPos = -1;
    if (curData && !curData->labelInfo().isEmpty()) {
        // If the block has a label/delimiter, find the position of the hereDocDelimiter.
        hereDocDelimPos = text.indexOf(hereDocDelimiter);
        // Skip all quoted hereDocDelimiter positions.
        while (hereDocDelimPos > -1 && isQuoted(text, hereDocDelimPos, /*isStart=*/true)) {
            hereDocDelimPos = text.indexOf(hereDocDelimiter, hereDocDelimPos + 2);
        }
    }

    // If we weren't already in a quote, try to locate the first quote in the current line.
    // Otherwise, we continue quoting from the previous line.
    if (!wasQuoted) {
        index = text.indexOf(quoteExpression);
        // Skip escaped quotes and also skip comment sections.
        while (SH_SkipQuote(text, index, /*isStartQuote=*/true)) {
            index = text.indexOf(quoteExpression, index + 1);
        }
        // If the quote is beyond the here-doc delimiter, ignore it.
        if (index >= 0 && hereDocDelimPos > -1 && index > hereDocDelimPos) {
            index = -1;
        }
        // Check if it is single or double quote by actual character match
        if (index >= 0) {
            if (text.at(index) == quoteMark.pattern().at(0)) {
                quoteExpression = quoteMark;
            }
            else {
                quoteExpression = singleQuoteMark;
            }
        }
    }
    else {
        // We are already in a quote from the previous line
        if (wasDoubleQuoted) {
            quoteExpression = quoteMark;
        }
        else {
            quoteExpression = singleQuoteMark;
        }
    }

    // Main loop: search for matching quotes (single or double) until the line ends.
    while (index >= 0) {
        // If we get the "mixedQuoteMark", we need to decide again if it’s single or double
        if (quoteExpression == mixedQuoteMark) {
            if (text.at(index) == quoteMark.pattern().at(0)) {
                quoteExpression = quoteMark;
            }
            else {
                quoteExpression = singleQuoteMark;
            }
        }

        // Find the corresponding end-quote
        int endIndex = -1;
        if (index == 0 && wasQuoted) {
            // We are continuing a quote from the very start of the line
            endIndex = text.indexOf(quoteExpression, 0, &quoteMatch);
        }
        else {
            // Normal search: from just after the quote
            endIndex = text.indexOf(quoteExpression, index + 1, &quoteMatch);
        }

        // Skip any escaped quotes or comments at the end index
        while (SH_SkipQuote(text, endIndex, /*isStartQuote=*/false)) {
            endIndex = text.indexOf(quoteExpression, endIndex + 1, &quoteMatch);
        }

        // Calculate the length of the quote, applying formatting
        int quoteLength = 0;
        if (endIndex == -1) {
            // If we cannot find the closing quote, mark this block as still inside the quote
            if (quoteExpression != quoteMark || hereDocDelimPos == -1) {
                setCurrentBlockState(
                    (quoteExpression == quoteMark)
                        ? (initialState == SH_DoubleQuoteState
                               ? SH_MixedDoubleQuoteState
                               : (initialState == SH_SingleQuoteState ? SH_MixedSingleQuoteState : doubleQuoteState))
                        : singleQuoteState);
            }
            else if (curData && curData->openNests() > 0) {
                // Something like: VAR="$(cat<<EOF
                curData->setProperty(true);
            }
            quoteLength = text.length() - index;
        }
        else {
            // Found closing quote
            quoteLength = endIndex - index + quoteMatch.capturedLength();
        }

        // Apply formatting to the detected quote range
        if (quoteExpression == quoteMark) {
            setFormatWithoutOverwrite(index, quoteLength, quoteFormat, neutralFormat);
        }
        else {
            setFormat(index, quoteLength, altQuoteFormat);
        }

        // Within this quote range, see if there's any URL that needs special formatting
        highlightUrlsWithinQuote(text, index, quoteLength);

        // Next iteration might flip back to "mixedQuoteMark" if we find a different type of quote
        quoteExpression = mixedQuoteMark;
        index = text.indexOf(quoteExpression, index + quoteLength);

        // Again, skip any escaped quotes or comment sections
        while (SH_SkipQuote(text, index, /*isStartQuote=*/true)) {
            index = text.indexOf(quoteExpression, index + 1);
        }
        if (hereDocDelimPos > -1 && index > hereDocDelimPos) {
            index = -1;  // Stop if we passed the here-doc delimiter
        }
    }
}

/*!
 * \brief Helper function to highlight URLs within a quoted text range.
 */
void Highlighter::highlightUrlsWithinQuote(const QString& text, int start, int length) {
    QString segment = text.mid(start, length);
    int urlIndex = 0;
    QRegularExpressionMatch urlMatch;

    while ((urlIndex = segment.indexOf(urlPattern, urlIndex, &urlMatch)) > -1) {
        setFormat(start + urlIndex, urlMatch.capturedLength(), urlInsideQuoteFormat);
        urlIndex += urlMatch.capturedLength();
    }
}

/*!
 * \brief SH_SkipQuote checks if we should skip the quote at a given position,
 *        e.g., if it is escaped or within a comment, URL, or other special format.
 */
bool Highlighter::SH_SkipQuote(const QString& text, const int pos, bool isStartQuote) {
    if (isEscapedQuote(text, pos, isStartQuote)) {
        return true;
    }
    // We skip if the format is already set to comment, URL, or quoted text.
    QTextCharFormat fmt = format(pos);
    return (fmt == neutralFormat || fmt == commentFormat || fmt == urlFormat || fmt == quoteFormat ||
            fmt == altQuoteFormat || fmt == urlInsideQuoteFormat);
}

/*!
 * \brief formatInsideCommand handles highlighting of bash command substitution variables $(...).
 *
 * The function processes characters one by one, handling nesting, quotes, parentheses, comments, etc.
 * It returns the updated index up to which text has been processed.
 */
int Highlighter::formatInsideCommand(const QString& text,
                                     int minOpenNests,
                                     int& nestCount,
                                     QSet<int>& quotes,
                                     bool isHereDocStart,
                                     int index) {
    int parenDepth = 0;
    int currentIndex = index;
    bool doubleQuoted = quotes.contains(nestCount);
    bool inComment = false;

    const int initialOpenNests = nestCount;
    const int textLen = text.length();

    while (nestCount > minOpenNests && currentIndex < textLen) {
        // Skip any chars already formatted as comments
        while (currentIndex < textLen && format(currentIndex) == commentFormat) {
            ++currentIndex;
        }

        if (currentIndex >= textLen)
            break;

        QChar c = text.at(currentIndex);

        // Single quote
        if (c == QLatin1Char('\'')) {
            handleSingleQuote(text, currentIndex, inComment, doubleQuoted, isHereDocStart, nestCount);
            continue;
        }
        // Double quote
        else if (c == QLatin1Char('\"')) {
            handleDoubleQuote(text, currentIndex, inComment, doubleQuoted);
            continue;
        }
        // Possible start of new command substitution
        else if (c == QLatin1Char('$')) {
            handleDollarSign(text, currentIndex, inComment, doubleQuoted, isHereDocStart, parenDepth, nestCount,
                             quotes);
            continue;
        }
        // Opening parenthesis
        else if (c == QLatin1Char('(')) {
            handleOpenParenthesis(currentIndex, doubleQuoted, inComment, parenDepth);
            continue;
        }
        // Closing parenthesis
        else if (c == QLatin1Char(')')) {
            if (handleCloseParenthesis(currentIndex, doubleQuoted, inComment, parenDepth, nestCount, initialOpenNests,
                                       quotes)) {
                // If handleCloseParenthesis returns true, we've closed a code block
                continue;
            }
            // else no code block closed; just a normal character
        }
        // Possible comment sign
        else if (c == QLatin1Char('#')) {
            handleCommentSign(text, currentIndex, inComment, doubleQuoted);
            continue;
        }
        // Default / non-special character
        else {
            handleDefaultChar(currentIndex, inComment, doubleQuoted);
            continue;
        }
    }

    if (nestCount < minOpenNests) {
        nestCount = minOpenNests;  // Should never happen, but just to be safe
    }

    // Preserve the quoting state if still double-quoted
    if (doubleQuoted) {
        // If no subcommand changed the block state, set ourselves to double-quote
        if (!isHereDocStart && currentBlockState() != SH_SingleQuoteState) {
            setCurrentBlockState(SH_DoubleQuoteState);
        }
        quotes.insert(initialOpenNests);
    }
    else {
        quotes.remove(initialOpenNests);
    }

    return currentIndex;
}

//------------------------------------------------------------------------------
// Below are the new helper methods to keep formatInsideCommand() more readable.
//------------------------------------------------------------------------------

void Highlighter::handleSingleQuote(const QString& text,
                                    int& currentIndex,
                                    bool inComment,
                                    bool doubleQuoted,
                                    bool isHereDocStart,
                                    int& nestCount) {
    if (inComment) {
        setFormat(currentIndex, 1, commentFormat);
        ++currentIndex;
        return;
    }

    if (doubleQuoted) {
        // If inside double quotes, just format the single quote as double-quoted text
        setFormat(currentIndex, 1, quoteFormat);
        ++currentIndex;
        return;
    }

    // Check if this single quote is escaped
    if (isEscapedQuote(text, currentIndex, /*isStartQuote=*/true)) {
        ++currentIndex;
        return;
    }

    // Otherwise, search for the matching closing single quote
    int end = text.indexOf(singleQuoteMark, currentIndex + 1);
    while (isEscapedQuote(text, end, /*isStartQuote=*/false)) {
        end = text.indexOf(singleQuoteMark, end + 1);
    }

    if (end == -1) {
        // No matching end, so highlight until the end of line
        setFormat(currentIndex, text.length() - currentIndex, altQuoteFormat);
        if (!isHereDocStart) {
            setCurrentBlockState(SH_SingleQuoteState);
        }
        currentIndex = text.length();
    }
    else {
        setFormat(currentIndex, end - currentIndex + 1, altQuoteFormat);
        currentIndex = end + 1;
    }
}

void Highlighter::handleDoubleQuote(const QString& text, int& currentIndex, bool inComment, bool& doubleQuoted) {
    if (inComment) {
        setFormat(currentIndex, 1, commentFormat);
    }
    else if (!isEscapedQuote(text, currentIndex, /*isStartQuote=*/true)) {
        // Toggle double-quoted state
        doubleQuoted = !doubleQuoted;
        setFormat(currentIndex, 1, quoteFormat);
    }
    else {
        // If escaped, just move on
    }
    ++currentIndex;
}

void Highlighter::handleDollarSign(const QString& text,
                                   int& currentIndex,
                                   bool inComment,
                                   bool doubleQuoted,
                                   bool isHereDocStart,
                                   int& parenDepth,
                                   int& nestCount,
                                   QSet<int>& quotes) {
    if (inComment) {
        setFormat(currentIndex, 1, commentFormat);
        ++currentIndex;
        return;
    }

    // If this is the start of a code block e.g. "$("
    if (text.mid(currentIndex, 2) == QLatin1String("$(")) {
        setFormat(currentIndex, 2, neutralFormat);
        ++nestCount;

        // Recurse into the code block from currentIndex + 2
        currentIndex = formatInsideCommand(text, nestCount - 1, nestCount, quotes, isHereDocStart, currentIndex + 2);
    }
    else {
        // Not a code block, just a variable reference
        if (doubleQuoted) {
            setFormat(currentIndex, 1, quoteFormat);
        }
        else {
            setFormat(currentIndex, 1, neutralFormat);
        }
        ++currentIndex;
    }
}

void Highlighter::handleOpenParenthesis(int& currentIndex, bool doubleQuoted, bool inComment, int& parenDepth) {
    if (doubleQuoted) {
        setFormat(currentIndex, 1, quoteFormat);
    }
    else if (inComment) {
        setFormat(currentIndex, 1, commentFormat);
    }
    else {
        setFormat(currentIndex, 1, neutralFormat);
        if (!isEscapedChar(currentBlock().text(), currentIndex)) {
            ++parenDepth;
        }
    }
    ++currentIndex;
}

/*!
 * \return true if a code block was closed by this parenthesis, false otherwise.
 */
bool Highlighter::handleCloseParenthesis(int& currentIndex,
                                         bool doubleQuoted,
                                         bool inComment,
                                         int& parenDepth,
                                         int& nestCount,
                                         int initialOpenNests,
                                         QSet<int>& quotes) {
    if (doubleQuoted) {
        setFormat(currentIndex, 1, quoteFormat);
        ++currentIndex;
        return false;
    }

    if (!isEscapedChar(currentBlock().text(), currentIndex)) {
        --parenDepth;
        if (parenDepth < 0) {
            // We just closed a code block
            setFormat(currentIndex, 1, neutralFormat);
            quotes.remove(initialOpenNests);
            --nestCount;
            // Refresh states
            initialOpenNests = nestCount;
            parenDepth = 0;  // reset
            ++currentIndex;
            return true;
        }
        else if (inComment) {
            setFormat(currentIndex, 1, commentFormat);
        }
        else {
            setFormat(currentIndex, 1, neutralFormat);
        }
    }
    else if (inComment) {
        setFormat(currentIndex, 1, commentFormat);
    }
    else {
        setFormat(currentIndex, 1, neutralFormat);
    }
    ++currentIndex;
    return false;
}

void Highlighter::handleCommentSign(const QString& text, int& currentIndex, bool& inComment, bool doubleQuoted) {
    if (inComment) {
        // Already in a comment, keep formatting
        setFormat(currentIndex, 1, commentFormat);
        ++currentIndex;
        return;
    }

    if (doubleQuoted) {
        // Treat it as text if inside double quotes
        setFormat(currentIndex, 1, quoteFormat);
    }
    else {
        // If '#' is at start or preceded by space, treat as comment
        if (currentIndex == 0 || (currentIndex > 0 && text.at(currentIndex - 1).isSpace())) {
            inComment = true;
            setFormat(currentIndex, 1, commentFormat);
        }
        else {
            setFormat(currentIndex, 1, neutralFormat);
        }
    }
    ++currentIndex;
}

void Highlighter::handleDefaultChar(int& currentIndex, bool inComment, bool doubleQuoted) {
    if (inComment) {
        setFormat(currentIndex, 1, commentFormat);
    }
    else if (doubleQuoted) {
        setFormat(currentIndex, 1, quoteFormat);
    }
    else {
        setFormat(currentIndex, 1, neutralFormat);
    }
    ++currentIndex;
}

//------------------------------------------------------------------------------

/*!
 * \brief SH_CmndSubstVar highlights command substitution variables "$( ... )" for bash.
 *
 * It leverages formatInsideCommand() for deeper logic and keeps track of open subcommands,
 * quotes, and so on. Returns true if forced highlighting of the next block is necessary.
 */
bool Highlighter::SH_CmndSubstVar(const QString& text,
                                  TextBlockData* currentBlockData,
                                  int oldOpenNests,
                                  const QSet<int>& oldOpenQuotes) {
    if (progLan != QLatin1String("sh") || !currentBlockData) {
        return false;
    }

    const int prevState = previousBlockState();
    int curState = currentBlockState();
    bool isHereDocStart = (curState < -1 || curState >= endState);

    // Gather open nests and quotes from the previous block
    int nestCount = 0;
    QSet<int> openQuotes;
    QTextBlock prevBlock = currentBlock().previous();
    if (prevBlock.isValid()) {
        if (auto* prevData = static_cast<TextBlockData*>(prevBlock.userData())) {
            nestCount = prevData->openNests();
            openQuotes = prevData->openQuotes();
        }
    }

    int startIndex = 0;
    // If we had an unclosed single/double quote from the previous line, close it first
    if (nestCount > 0 && (prevState == SH_SingleQuoteState || prevState == SH_DoubleQuoteState ||
                          prevState == SH_MixedDoubleQuoteState || prevState == SH_MixedSingleQuoteState)) {
        startIndex = closeOpenQuoteFromPreviousBlock(text, prevState, isHereDocStart);
    }

    // If there's no unclosed code block or quote, search for a new "$(" from startIndex on
    while (startIndex < text.length()) {
        if (nestCount == 0) {
            static QRegularExpression codeBlockStart(R"(\$\()");  // Matches "$("
            int foundPos = text.indexOf(codeBlockStart, startIndex);
            if (foundPos == -1 || format(foundPos) == commentFormat) {
                // No new code block found or it's commented out
                break;
            }
            // Found a new code block
            ++nestCount;
            setFormat(foundPos, 2, neutralFormat);
            startIndex = foundPos + 2;
        }
        // Highlight inside the command substitution
        startIndex = formatInsideCommand(text, 0, nestCount, openQuotes, isHereDocStart, startIndex);
    }

    if (!openQuotes.isEmpty()) {
        currentBlockData->insertOpenQuotes(openQuotes);
    }
    if (nestCount > 0) {
        currentBlockData->insertNestInfo(nestCount);
        // If this is a here-doc start, modify state to reflect open code blocks
        if (isHereDocStart) {
            // A domain-specific hack: adjusting state to track # of open blocks
            // in negative or positive direction
            (curState > 0) ? curState += 2 * (nestCount + 3) : curState -= 2 * (nestCount + 3);
            setCurrentBlockState(curState);
        }
    }

    // If we changed the nestCount or openQuotes, we may need to re-highlight next block
    if (nestCount != oldOpenNests || openQuotes != oldOpenQuotes) {
        return true;
    }
    return false;
}

/*!
 * \brief Helper to close an open quote from a previous block before searching for new code blocks.
 * \return the index at which we can resume searching for "$(" in the current block.
 */
int Highlighter::closeOpenQuoteFromPreviousBlock(const QString& text, int prevState, bool isHereDocStart) {
    int startIndex = 0;
    if (prevState == SH_SingleQuoteState || prevState == SH_MixedSingleQuoteState) {
        int end = text.indexOf(singleQuoteMark);
        while (isEscapedQuote(text, end, /*isStartQuote=*/false)) {
            end = text.indexOf(singleQuoteMark, end + 1);
        }
        if (end == -1) {
            setFormat(0, text.length(), altQuoteFormat);
            if (!isHereDocStart) {
                setCurrentBlockState(SH_SingleQuoteState);
            }
            return text.length();  // entire line is single-quoted
        }
        else {
            setFormat(0, end + 1, altQuoteFormat);
            startIndex = end + 1;
        }
    }
    // If it’s an open double quote, we do not forcibly close it here.
    // We handle double-quoted text as we parse along in formatInsideCommand().
    return startIndex;
}

}  // namespace FeatherPad
