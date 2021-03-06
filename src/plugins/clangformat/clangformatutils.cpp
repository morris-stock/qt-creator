/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of Qt Creator.
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
****************************************************************************/

#include "clangformatutils.h"

#include "clangformatconstants.h"

#include <coreplugin/icore.h>
#include <cpptools/cppcodestylesettings.h>
#include <texteditor/tabsettings.h>
#include <projectexplorer/project.h>
#include <projectexplorer/session.h>

using namespace clang;
using namespace format;
using namespace llvm;
using namespace CppTools;
using namespace ProjectExplorer;
using namespace TextEditor;

namespace ClangFormat {

static void applyTabSettings(clang::format::FormatStyle &style, const TabSettings &settings)
{
    style.IndentWidth = static_cast<unsigned>(settings.m_indentSize);
    style.TabWidth = static_cast<unsigned>(settings.m_tabSize);

    if (settings.m_tabPolicy == TabSettings::TabsOnlyTabPolicy)
        style.UseTab = FormatStyle::UT_Always;
    else if (settings.m_tabPolicy == TabSettings::SpacesOnlyTabPolicy)
        style.UseTab = FormatStyle::UT_Never;
    else
        style.UseTab = FormatStyle::UT_ForContinuationAndIndentation;

    if (settings.m_continuationAlignBehavior == TabSettings::NoContinuationAlign) {
        style.ContinuationIndentWidth = 0;
        style.AlignAfterOpenBracket = FormatStyle::BAS_DontAlign;
    } else {
        style.ContinuationIndentWidth = style.IndentWidth;
        style.AlignAfterOpenBracket = FormatStyle::BAS_Align;
    }
}

static void applyCppCodeStyleSettings(clang::format::FormatStyle &style,
                                      const CppCodeStyleSettings &settings)
{
    style.IndentCaseLabels = settings.indentSwitchLabels;
    style.AlignOperands = settings.alignAssignments;
    style.NamespaceIndentation = FormatStyle::NI_None;
    if (settings.indentNamespaceBody)
        style.NamespaceIndentation = FormatStyle::NI_All;

    style.BraceWrapping.IndentBraces = false;
    if (settings.indentBlockBraces) {
        if (settings.indentClassBraces && settings.indentEnumBraces
                && settings.indentNamespaceBraces && settings.indentFunctionBraces) {
            style.BraceWrapping.IndentBraces = true;
        } else {
            style.BreakBeforeBraces = FormatStyle::BS_GNU;
        }
    }

    if (settings.bindStarToIdentifier || settings.bindStarToRightSpecifier)
        style.PointerAlignment = FormatStyle::PAS_Right;
    else
        style.PointerAlignment = FormatStyle::PAS_Left;

    style.AccessModifierOffset = settings.indentAccessSpecifiers
            ? 0
            : - static_cast<int>(style.IndentWidth);
}

static Utils::FileName projectPath()
{
    const Project *project = SessionManager::startupProject();
    if (project)
        return project->projectDirectory();

    return Utils::FileName();
}

static Utils::FileName globalPath()
{
    return Utils::FileName::fromString(Core::ICore::userResourcePath());
}

static QString configForFile(Utils::FileName fileName)
{
    Utils::FileName topProjectPath = projectPath();
    if (topProjectPath.isEmpty())
        return QString();

    QDir projectDir(fileName.parentDir().toString());
    while (!projectDir.exists(Constants::SETTINGS_FILE_NAME)
           && !projectDir.exists(Constants::SETTINGS_FILE_ALT_NAME)) {
        if (projectDir.path() == topProjectPath.toString()
            || !Utils::FileName::fromString(projectDir.path()).isChildOf(topProjectPath)
            || !projectDir.cdUp()) {
            return QString();
        }
    }

    if (projectDir.exists(Constants::SETTINGS_FILE_NAME))
        return projectDir.filePath(Constants::SETTINGS_FILE_NAME);
    return projectDir.filePath(Constants::SETTINGS_FILE_ALT_NAME);
}

static clang::format::FormatStyle constructStyle(bool isGlobal,
                                                 const QByteArray &baseStyle = QByteArray())
{
    if (!baseStyle.isEmpty()) {
        // Try to get the style for this base style.
        Expected<FormatStyle> style = getStyle(baseStyle.toStdString(),
                                               "dummy.cpp",
                                               baseStyle.toStdString());
        if (style)
            return *style;

        handleAllErrors(style.takeError(), [](const ErrorInfoBase &) {
            // do nothing
        });
        // Fallthrough to the default style.
    }

    FormatStyle style = getLLVMStyle();
    style.BreakBeforeBraces = FormatStyle::BS_Custom;

    const CppCodeStyleSettings codeStyleSettings = isGlobal
            ? CppCodeStyleSettings::currentGlobalCodeStyle()
            : CppCodeStyleSettings::currentProjectCodeStyle()
              .value_or(CppCodeStyleSettings::currentGlobalCodeStyle());
    const TabSettings tabSettings = isGlobal
            ? CppCodeStyleSettings::currentGlobalTabSettings()
            : CppCodeStyleSettings::currentProjectTabSettings();

    applyTabSettings(style, tabSettings);
    applyCppCodeStyleSettings(style, codeStyleSettings);

    return style;
}

void createStyleFileIfNeeded(bool isGlobal)
{
    Utils::FileName path = isGlobal ? globalPath() : projectPath();
    const QString configFile = path.appendPath(Constants::SETTINGS_FILE_NAME).toString();

    if (QFile::exists(configFile))
        return;

    std::fstream newStyleFile(configFile.toStdString(), std::fstream::out);
    if (newStyleFile.is_open()) {
        newStyleFile << clang::format::configurationAsText(constructStyle(isGlobal));
        newStyleFile.close();
    }
}

static QByteArray configBaseStyleName(const QString &configFile)
{
    if (configFile.isEmpty())
        return QByteArray();

    QFile config(configFile);
    if (!config.open(QIODevice::ReadOnly))
        return QByteArray();

    const QByteArray content = config.readAll();
    const char basedOnStyle[] = "BasedOnStyle:";
    int basedOnStyleIndex = content.indexOf(basedOnStyle);
    if (basedOnStyleIndex < 0)
        return QByteArray();

    basedOnStyleIndex += sizeof(basedOnStyle) - 1;
    const int endOfLineIndex = content.indexOf('\n', basedOnStyleIndex);
    return content
        .mid(basedOnStyleIndex, endOfLineIndex < 0 ? -1 : endOfLineIndex - basedOnStyleIndex)
        .trimmed();
}

clang::format::FormatStyle styleForFile(Utils::FileName fileName)
{
    bool isGlobal = false;
    QString configFile = configForFile(fileName);
    if (configFile.isEmpty()) {
        Utils::FileName path = fileName = globalPath();
        fileName.appendPath(Constants::SAMPLE_FILE_NAME);
        createStyleFileIfNeeded(true);
        configFile = path.appendPath(Constants::SETTINGS_FILE_NAME).toString();
    }

    Expected<FormatStyle> style = format::getStyle("file",
                                                   fileName.toString().toStdString(),
                                                   "none");
    if (style)
        return *style;

    handleAllErrors(style.takeError(), [](const ErrorInfoBase &) {
        // do nothing
    });

    return constructStyle(isGlobal, configBaseStyleName(configFile));
}

clang::format::FormatStyle currentProjectStyle()
{
    return styleForFile(projectPath().appendPath(Constants::SAMPLE_FILE_NAME));
}

clang::format::FormatStyle currentGlobalStyle()
{
    return styleForFile(globalPath().appendPath(Constants::SAMPLE_FILE_NAME));
}

}
