/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
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

#include "pchpaths.h"
#include "precompiledheaderstorageinterface.h"

#include <sqlitetransaction.h>
#include <sqliteexception.h>

#include <utils/smallstringview.h>

namespace ClangBackEnd {

template<typename Database=Sqlite::Database>
class PrecompiledHeaderStorage final : public PrecompiledHeaderStorageInterface
{
    using ReadStatement = typename Database::ReadStatement;
    using WriteStatement = typename Database::WriteStatement;
public:
    PrecompiledHeaderStorage(Database &database)
        : transaction(database)
        , database(database)
    {
        transaction.commit();
    }

    void insertProjectPrecompiledHeader(ProjectPartId projectPartId,
                                        Utils::SmallStringView pchPath,
                                        long long pchBuildTime) override
    {
        try {
            Sqlite::ImmediateTransaction transaction{database};

            insertProjectPrecompiledHeaderStatement.write(projectPartId.projectPathId,
                                                          pchPath,
                                                          pchBuildTime);

            transaction.commit();
        } catch (const Sqlite::StatementIsBusy &) {
            insertProjectPrecompiledHeader(projectPartId, pchPath, pchBuildTime);
        }
    }

    void deleteProjectPrecompiledHeader(ProjectPartId projectPartId) override
    {
        try {
            Sqlite::ImmediateTransaction transaction{database};

            deleteProjectPrecompiledHeaderStatement.write(projectPartId.projectPathId);

            transaction.commit();
        } catch (const Sqlite::StatementIsBusy) {
            deleteProjectPrecompiledHeader(projectPartId);
        }
    }

    void deleteProjectPrecompiledHeaders(const ProjectPartIds &projectPartIds) override
    {
        try {
            Sqlite::ImmediateTransaction transaction{database};

            for (ProjectPartId projectPartId : projectPartIds)
                deleteProjectPrecompiledHeaderStatement.write(projectPartId.projectPathId);

            transaction.commit();
        } catch (const Sqlite::StatementIsBusy) {
            deleteProjectPrecompiledHeaders(projectPartIds);
        }
    }

    void insertSystemPrecompiledHeaders(const ProjectPartIds &projectPartIds,
                                        Utils::SmallStringView pchPath,
                                        long long pchBuildTime) override
    {
        try {
            Sqlite::ImmediateTransaction transaction{database};

            for (ProjectPartId projectPartId : projectPartIds) {
                insertSystemPrecompiledHeaderStatement.write(projectPartId.projectPathId,
                                                             pchPath,
                                                             pchBuildTime);
            }
            transaction.commit();
        } catch (const Sqlite::StatementIsBusy) {
            insertSystemPrecompiledHeaders(projectPartIds, pchPath, pchBuildTime);
        }
    }

    void deleteSystemPrecompiledHeaders(const ProjectPartIds &projectPartIds) override
    {
        try {
            Sqlite::ImmediateTransaction transaction{database};

            for (ProjectPartId projectPartId : projectPartIds)
                deleteSystemPrecompiledHeaderStatement.write(projectPartId.projectPathId);

            transaction.commit();
        } catch (const Sqlite::StatementIsBusy) {
            deleteSystemPrecompiledHeaders(projectPartIds);
        }
    }

    FilePath fetchSystemPrecompiledHeaderPath(ProjectPartId projectPartId) override
    {
        try {
            Sqlite::DeferredTransaction transaction{database};

            auto value = fetchSystemPrecompiledHeaderPathStatement.template value<FilePath>(
                projectPartId.projectPathId);

            if (value)
                return *value;

            transaction.commit();
        } catch (const Sqlite::StatementIsBusy) {
            return fetchSystemPrecompiledHeaderPath(projectPartId);
        }

        return FilePath("");
    }

    FilePath fetchPrecompiledHeader(ProjectPartId projectPartId) const
    {
        try {
            Sqlite::DeferredTransaction transaction{database};

            auto value = fetchPrecompiledHeaderStatement.template value<FilePath>(
                projectPartId.projectPathId);

            if (value)
                return *value;

            transaction.commit();
        } catch (const Sqlite::StatementIsBusy) {
            return fetchPrecompiledHeader(projectPartId);
        }

        return FilePath("");
    }

    PchPaths fetchPrecompiledHeaders(ProjectPartId projectPartId) const
    {
        try {
            Sqlite::DeferredTransaction transaction{database};

            auto value = fetchPrecompiledHeadersStatement.template value<PchPaths, 2>(
                projectPartId.projectPathId);

            if (value)
                return *value;

            transaction.commit();
        } catch (const Sqlite::StatementIsBusy) {
            return fetchPrecompiledHeaders(projectPartId);
        }

        return {};
    }

public:
    Sqlite::ImmediateNonThrowingDestructorTransaction transaction;
    Database &database;
    WriteStatement insertProjectPrecompiledHeaderStatement{
        "INSERT INTO precompiledHeaders(projectPartId, projectPchPath, projectPchBuildTime) "
        "VALUES(?001,?002,?003) "
        "ON CONFLICT (projectPartId) DO UPDATE SET projectPchPath=?002,projectPchBuildTime=?003",
        database};
    WriteStatement insertSystemPrecompiledHeaderStatement{
        "INSERT INTO precompiledHeaders(projectPartId, systemPchPath, systemPchBuildTime) "
        "VALUES(?001,?002,?003) "
        "ON CONFLICT (projectPartId) DO UPDATE SET systemPchPath=?002,systemPchBuildTime=?003",
        database};
    WriteStatement deleteProjectPrecompiledHeaderStatement{
        "UPDATE OR IGNORE precompiledHeaders SET projectPchPath=NULL,projectPchBuildTime=NULL "
        "WHERE projectPartId = ?",
        database};
    WriteStatement deleteSystemPrecompiledHeaderStatement{
        "UPDATE OR IGNORE precompiledHeaders SET systemPchPath=NULL,systemPchBuildTime=NULL "
        "WHERE projectPartId = ?",
        database};
    ReadStatement fetchSystemPrecompiledHeaderPathStatement{
        "SELECT systemPchPath FROM precompiledHeaders WHERE projectPartId = ?", database};
    mutable ReadStatement fetchPrecompiledHeaderStatement{
        "SELECT ifnull(nullif(projectPchPath, ''), systemPchPath) "
        "FROM precompiledHeaders WHERE projectPartId = ?",
        database};
    mutable ReadStatement fetchPrecompiledHeadersStatement{
        "SELECT projectPchPath, systemPchPath FROM precompiledHeaders WHERE projectPartId = ?",
        database};
};

}
