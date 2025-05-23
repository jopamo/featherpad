/*
 * Copyright (C) Pedram Pourang (aka Tsu Jan) 2014-2021 <tsujan2000@gmail.com>
 *
 * FeatherPad is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * FeatherPad is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * @license GPL-3.0+ <https://spdx.org/licenses/GPL-3.0+.html>
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <QSettings>
#include <QSize>
#include <QPoint>
#include <QFont>
#include <QColor>

#include <algorithm>

namespace FeatherPad {

// Prevent redundant writings! (Why does QSettings write to the config file when no setting is changed?)
class Settings : public QSettings {
    Q_OBJECT
   public:
    Settings(const QString& organization, const QString& application = QString(), QObject* parent = nullptr)
        : QSettings(organization, application, parent) {}
    Settings(const QString& fileName, QSettings::Format format, QObject* parent = nullptr)
        : QSettings(fileName, format, parent) {}

    void setValue(const QString& key, const QVariant& v) {
        if (value(key) == v)
            return;
        QSettings::setValue(key, v);
    }
};

class Config {
   public:
    Config();
    ~Config();

    void readConfig();
    void readShortcuts();
    void writeConfig();

    bool getRemSize() const { return remSize_; }
    void setRemSize(bool rem) { remSize_ = rem; }

    bool getRemPos() const { return remPos_; }
    void setRemPos(bool rem) { remPos_ = rem; }

    bool getRemSplitterPos() const { return remSplitterPos_; }
    void setRemSplitterPos(bool rem) { remSplitterPos_ = rem; }

    bool getIsMaxed() const { return isMaxed_; }
    void setIsMaxed(bool isMaxed) { isMaxed_ = isMaxed; }

    bool getIsFull() const { return isFull_; }
    void setIsFull(bool isFull) { isFull_ = isFull; }

    bool getDarkColScheme() const { return darkColScheme_; }
    void setDarkColScheme(bool dark) { darkColScheme_ = dark; }

    bool getThickCursor() const { return thickCursor_; }
    void setThickCursor(bool thick) { thickCursor_ = thick; }

    int getLightBgColorValue() const { return lightBgColorValue_; }
    void setLightBgColorValue(int lightness) { lightBgColorValue_ = lightness; }

    int getDarkBgColorValue() const { return darkBgColorValue_; }
    void setDarkBgColorValue(int darkness) { darkBgColorValue_ = darkness; }

    QString getDateFormat() const { return dateFormat_; }
    void setDateFormat(const QString& format) { dateFormat_ = format; }

    int getTextTabSize() const { return textTabSize_; }
    void setTextTabSize(int textTab) { textTabSize_ = textTab; }

    int getDefaultRecentFilesNumber() const { return 10; }
    int getRecentFilesNumber() const { return recentFilesNumber_; }
    void setRecentFilesNumber(int number) { recentFilesNumber_ = std::clamp(number, 0, 50); }
    int getCurRecentFilesNumber() const { return curRecentFilesNumber_; }

    bool getTabWrapAround() const { return tabWrapAround_; }
    void setTabWrapAround(bool wrap) { tabWrapAround_ = wrap; }

    bool getHideSingleTab() const { return hideSingleTab_; }
    void setHideSingleTab(bool hide) { hideSingleTab_ = hide; }

    QSize getWinSize() const { return winSize_; }
    void setWinSize(const QSize& s) { winSize_ = s; }

    QSize getPrefSize() const { return prefSize_; }
    void setPrefSize(const QSize& s) { prefSize_ = s; }

    QSize getDefaultStartSize() const { return QSize(700, 500); }
    QSize getStartSize() const { return startSize_; }
    void setStartSize(const QSize& s) { startSize_ = s; }

    QPoint getWinPos() const { return winPos_; }
    void setWinPos(const QPoint& p) { winPos_ = p; }

    int getSplitterPos() const { return splitterPos_; }
    void setSplitterPos(int pos) { splitterPos_ = pos; }

    bool getNoToolbar() const { return noToolbar_; }
    void setNoToolbar(bool noTB) { noToolbar_ = noTB; }

    bool getNoMenubar() const { return noMenubar_; }
    void setNoMenubar(bool noMB) { noMenubar_ = noMB; }

    bool getMenubarTitle() const { return menubarTitle_; }
    void setMenubarTitle(bool mt) { menubarTitle_ = mt; }

    bool getHideSearchbar() const { return hideSearchbar_; }
    void setHideSearchbar(bool hide) { hideSearchbar_ = hide; }

    bool getShowStatusbar() const { return showStatusbar_; }
    void setShowStatusbar(bool show) { showStatusbar_ = show; }

    bool getShowCursorPos() const { return showCursorPos_; }
    void setShowCursorPos(bool show) { showCursorPos_ = show; }

    bool getShowLangSelector() const { return showLangSelector_; }
    void setShowLangSelector(bool show) { showLangSelector_ = show; }

    bool getSidePaneMode() const { return sidePaneMode_; }
    void setSidePaneMode(bool sidePane) { sidePaneMode_ = sidePane; }

    int getTabPosition() const { return tabPosition_; }
    void setTabPosition(int pos) { tabPosition_ = pos; }

    QFont getFont() const { return font_; }
    void setFont(const QFont& font) { font_ = font; }
    void resetFont();

    bool getRemFont() const { return remFont_; }
    void setRemFont(bool rem) { remFont_ = rem; }

    bool getWrapByDefault() const { return wrapByDefault_; }
    void setWrapByDefault(bool wrap) { wrapByDefault_ = wrap; }

    bool getIndentByDefault() const { return indentByDefault_; }
    void setIndentByDefault(bool indent) { indentByDefault_ = indent; }

    bool getAutoReplace() const { return autoReplace_; }
    void setAutoReplace(bool autoR) { autoReplace_ = autoR; }

    bool getAutoBracket() const { return autoBracket_; }
    void setAutoBracket(bool autoB) { autoBracket_ = autoB; }

    bool getLineByDefault() const { return lineByDefault_; }
    void setLineByDefault(bool line) { lineByDefault_ = line; }

    bool getSyntaxByDefault() const { return syntaxByDefault_; }
    void setSyntaxByDefault(bool syntax) { syntaxByDefault_ = syntax; }

    bool getShowWhiteSpace() const { return showWhiteSpace_; }
    void setShowWhiteSpace(bool show) { showWhiteSpace_ = show; }

    bool getShowEndings() const { return showEndings_; }
    void setShowEndings(bool show) { showEndings_ = show; }

    bool getTextMargin() const { return textMargin_; }
    void setTextMargin(bool margin) { textMargin_ = margin; }

    int getDefaultVLineDistance() const { return 80; }
    int getVLineDistance() const { return vLineDistance_; }
    void setVLineDistance(int distance) { vLineDistance_ = distance; }

    int getDefaultMaxSHSize() const { return 2; }
    int getMaxSHSize() const { return maxSHSize_; }
    void setMaxSHSize(int max) { maxSHSize_ = max; }

    bool getSkipNonText() const { return skipNonText_; }
    void setSkipNonText(bool skip) { skipNonText_ = skip; }
    /*************************/
    bool getExecuteScripts() const { return executeScripts_; }
    void setExecuteScripts(bool execute) { executeScripts_ = execute; }
    QString getExecuteCommand() const { return executeCommand_; }
    void setExecuteCommand(const QString& command) { executeCommand_ = command; }
    /*************************/
    bool getAppendEmptyLine() const { return appendEmptyLine_; }
    void setAppendEmptyLine(bool append) { appendEmptyLine_ = append; }

    bool getRemoveTrailingSpaces() const { return removeTrailingSpaces_; }
    void setRemoveTrailingSpaces(bool remove) { removeTrailingSpaces_ = remove; }

    bool getOpenInWindows() const { return openInWindows_; }
    void setOpenInWindows(bool windows) { openInWindows_ = windows; }

    bool getNativeDialog() const { return nativeDialog_; }
    void setNativeDialog(bool native) { nativeDialog_ = native; }
    /*************************/
    bool getRecentOpened() const { return recentOpened_; }
    void setRecentOpened(bool opened) { recentOpened_ = opened; }

    QStringList getRecentFiles() const { return recentFiles_; }
    void clearRecentFiles() { recentFiles_ = QStringList(); }
    void addRecentFile(const QString& file);
    /*************************/
    QHash<QString, QString> customShortcutActions() const { return actions_; }
    void setActionShortcut(const QString& action, const QString& shortcut) { actions_.insert(action, shortcut); }
    void removeShortcut(const QString& action) {
        actions_.remove(action);
        removedActions_ << action;
    }

    bool hasReservedShortcuts() const { return (!reservedShortcuts_.isEmpty()); }
    QStringList reservedShortcuts() const { return reservedShortcuts_; }
    void setReservedShortcuts(const QStringList& s) { reservedShortcuts_ = s; }
    /*************************/
    bool getInertialScrolling() const { return inertialScrolling_; }
    void setInertialScrolling(bool inertial) { inertialScrolling_ = inertial; }
    /*************************/
    QHash<QString, QVariant> savedCursorPos() {
        readCursorPos();
        return cursorPos_;
    }
    void saveCursorPos(const QString& name, int pos) {
        readCursorPos();
        if (removedCursorPos_.contains(name))
            removedCursorPos_.removeOne(name);
        else
            cursorPos_.insert(name, pos);
    }
    void removeCursorPos(const QString& name) {
        readCursorPos();
        cursorPos_.remove(name);
        removedCursorPos_ << name;
    }
    void removeAllCursorPos() {
        readCursorPos();
        removedCursorPos_.append(cursorPos_.keys());
        cursorPos_.clear();
    }
    /*************************/
    bool getSaveLastFilesList() const { return saveLastFilesList_; }
    void setSaveLastFilesList(bool saveList) { saveLastFilesList_ = saveList; }

    QStringList getLastFiles();  // may be called only at session start and sets lasFilesCursorPos_
    QHash<QString, QVariant> getLastFilesCursorPos() const {  // is called only after getLastFiles()
        return lasFilesCursorPos_;
    }
    void setLastFileCursorPos(const QHash<QString, QVariant>& curPos) { lasFilesCursorPos_ = curPos; }
    /*************************/
    bool getAutoSave() const { return autoSave_; }
    void setAutoSave(bool as) { autoSave_ = as; }
    int getAutoSaveInterval() const { return autoSaveInterval_; }
    void setAutoSaveInterval(int i) { autoSaveInterval_ = i; }
    /*************************/
    bool getSaveUnmodified() const { return saveUnmodified_; }
    void setSaveUnmodified(bool save) { saveUnmodified_ = save; }
    /*************************/
    bool getSelectionHighlighting() const { return selectionHighlighting_; }
    void setSelectionHighlighting(bool enable) { selectionHighlighting_ = enable; }
    /*************************/
    bool getPastePaths() const { return pastePaths_; }
    void setPastePaths(bool pastPaths) { pastePaths_ = pastPaths; }
    /*************************/
    bool getCloseWithLastTab() const { return closeWithLastTab_; }
    void setCloseWithLastTab(bool close) { closeWithLastTab_ = close; }
    /*************************/
    bool getSharedSearchHistory() const { return sharedSearchHistory_; }
    void setSharedSearchHistory(bool share) { sharedSearchHistory_ = share; }
    /*************************/
    bool getDisableMenubarAccel() const { return disableMenubarAccel_; }
    void setDisableMenubarAccel(bool disable) { disableMenubarAccel_ = disable; }
    /*************************/
    bool getSysIcons() const { return sysIcons_; }
    void setSysIcons(bool sysIcons) { sysIcons_ = sysIcons; }
    /*************************/
    QString getDictPath() const { return dictPath_; }
    void setDictPath(const QString& dictPath) { dictPath_ = dictPath; }

    bool getSpellCheckFromStart() const { return spellCheckFromStart_; }
    void setSpellCheckFromStart(bool fromStart) { spellCheckFromStart_ = fromStart; }
    /*************************/
    QHash<QString, QColor> lightSyntaxColors() const { return defaultLightSyntaxColors_; }
    QHash<QString, QColor> darkSyntaxColors() const { return defaultDarkSyntaxColors_; }

    QHash<QString, QColor> customSyntaxColors() const { return customSyntaxColors_; }
    void setCustomSyntaxColors(const QHash<QString, QColor>& colors) { customSyntaxColors_ = colors; }

    int getDefaultWhiteSpaceValue() const { return darkColScheme_ ? 95 : 180; }
    int getMinWhiteSpaceValue() const { return darkColScheme_ ? 50 : 130; }
    int getMaxWhiteSpaceValue() const { return darkColScheme_ ? 140 : 230; }
    int getWhiteSpaceValue() const { return whiteSpaceValue_; }
    void setWhiteSpaceValue(int value);

    int getCurLineHighlight() const { return curLineHighlight_; }
    int getMinCurLineHighlight() const { return darkColScheme_ ? 0 : 210; }
    int getMaxCurLineHighlight() const { return darkColScheme_ ? 70 : 255; }
    void setCurLineHighlight(int value);

    void readSyntaxColors();

   private:
    QString validatedShortcut(const QVariant v, bool* isValid);
    void readCursorPos();
    void writeCursorPos();
    void setDfaultSyntaxColors();
    void writeSyntaxColors();

    bool remSize_, remPos_, remSplitterPos_, noToolbar_, noMenubar_, menubarTitle_, hideSearchbar_, showStatusbar_,
        showCursorPos_, showLangSelector_, sidePaneMode_, remFont_, wrapByDefault_, indentByDefault_, autoReplace_,
        autoBracket_, lineByDefault_, syntaxByDefault_, showWhiteSpace_, showEndings_, textMargin_, isMaxed_, isFull_,
        darkColScheme_, thickCursor_, tabWrapAround_, hideSingleTab_, executeScripts_, appendEmptyLine_,
        removeTrailingSpaces_, openInWindows_, nativeDialog_, inertialScrolling_, autoSave_, skipNonText_,
        saveUnmodified_, selectionHighlighting_, pastePaths_, closeWithLastTab_, sharedSearchHistory_,
        disableMenubarAccel_, sysIcons_;
    int vLineDistance_, tabPosition_, maxSHSize_, lightBgColorValue_, darkBgColorValue_, recentFilesNumber_,
        curRecentFilesNumber_,  // the start value of recentFilesNumber_ -- fixed during a session
        autoSaveInterval_, textTabSize_;
    QString dateFormat_;
    QSize winSize_, startSize_, prefSize_;
    QPoint winPos_;
    int splitterPos_;
    QFont font_;
    QString executeCommand_;
    bool recentOpened_;
    QStringList recentFiles_;
    bool saveLastFilesList_;
    QHash<QString, QString> actions_;
    QStringList removedActions_, reservedShortcuts_;

    QHash<QString, QVariant> cursorPos_;
    QStringList removedCursorPos_;  // used only internally for the clean-up
    bool cursorPosRetrieved_;       // used only internally for reading once

    QHash<QString, QVariant> lasFilesCursorPos_;

    QString dictPath_;
    bool spellCheckFromStart_;

    QHash<QString, QColor> defaultLightSyntaxColors_, defaultDarkSyntaxColors_, customSyntaxColors_;
    int whiteSpaceValue_, curLineHighlight_;
};

}  // namespace FeatherPad

#endif  // CONFIG_H
