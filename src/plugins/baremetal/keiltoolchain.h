/****************************************************************************
**
** Copyright (C) 2019 Denis Shienkov <denis.shienkov@gmail.com>
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

#pragma once

#include <projectexplorer/abi.h>
#include <projectexplorer/toolchain.h>
#include <projectexplorer/toolchainconfigwidget.h>

QT_BEGIN_NAMESPACE
class QPlainTextEdit;
class QPushButton;
class QTextEdit;
QT_END_NAMESPACE

namespace Utils {
class FileName;
class PathChooser;
}

namespace ProjectExplorer { class AbiWidget; }

namespace BareMetal {
namespace Internal {

// KeilToolchain

class KeilToolchain final : public ProjectExplorer::ToolChain
{
    Q_DECLARE_TR_FUNCTIONS(KeilToolchain)

public:
    QString typeDisplayName() const final;

    void setTargetAbi(const ProjectExplorer::Abi &abi);
    ProjectExplorer::Abi targetAbi() const final;

    bool isValid() const final;

    MacroInspectionRunner createMacroInspectionRunner() const final;
    ProjectExplorer::Macros predefinedMacros(const QStringList &cxxflags) const final;

    Utils::LanguageExtensions languageExtensions(const QStringList &cxxflags) const final;
    ProjectExplorer::WarningFlags warningFlags(const QStringList &cxxflags) const final;

    BuiltInHeaderPathsRunner createBuiltInHeaderPathsRunner() const final;
    ProjectExplorer::HeaderPaths builtInHeaderPaths(const QStringList &cxxFlags,
                                                    const Utils::FileName &) const final;
    void addToEnvironment(Utils::Environment &env) const final;
    ProjectExplorer::IOutputParser *outputParser() const final;

    QVariantMap toMap() const final;
    bool fromMap(const QVariantMap &data) final;

    std::unique_ptr<ProjectExplorer::ToolChainConfigWidget> createConfigurationWidget() final;

    bool operator ==(const ToolChain &other) const final;

    void setCompilerCommand(const Utils::FileName &file);
    Utils::FileName compilerCommand() const final;

    QString makeCommand(const Utils::Environment &env) const final;

    ToolChain *clone() const final;

protected:
    KeilToolchain(const KeilToolchain &tc) = default;

private:
    KeilToolchain();

    ProjectExplorer::Abi m_targetAbi;
    Utils::FileName m_compilerCommand;

    friend class KeilToolchainFactory;
    friend class KeilToolchainConfigWidget;
};

// KeilToolchainFactory

class KeilToolchainFactory final : public ProjectExplorer::ToolChainFactory
{
    Q_OBJECT

public:
    explicit KeilToolchainFactory();

    QList<ProjectExplorer::ToolChain *> autoDetect(
            const QList<ProjectExplorer::ToolChain *> &alreadyKnown) final;

    bool canCreate() final;
    ProjectExplorer::ToolChain *create() final;

    ProjectExplorer::ToolChain *restore(const QVariantMap &data) final;

private:
    QList<ProjectExplorer::ToolChain *> autoDetectToolchains(const Candidates &candidates,
                                                             const QList<ProjectExplorer::ToolChain *> &alreadyKnown) const;
    QList<ProjectExplorer::ToolChain *> autoDetectToolchain(
            const Candidate &candidate, Core::Id language) const;
};

// KeilToolchainConfigWidget

class KeilToolchainConfigWidget final : public ProjectExplorer::ToolChainConfigWidget
{
    Q_OBJECT

public:
    explicit KeilToolchainConfigWidget(KeilToolchain *tc);

private:
    void applyImpl() final;
    void discardImpl() final { setFromToolchain(); }
    bool isDirtyImpl() const final;
    void makeReadOnlyImpl() final;

    void setFromToolchain();
    void handleCompilerCommandChange();

    Utils::PathChooser *m_compilerCommand = nullptr;
    ProjectExplorer::AbiWidget *m_abiWidget = nullptr;
    ProjectExplorer::Macros m_macros;
};

} // namespace Internal
} // namespace BareMetal
