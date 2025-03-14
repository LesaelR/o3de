/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */


#include "EditorDefs.h"

#include "ErrorReport.h"

// Editor
#include "ErrorReportDialog.h"

//////////////////////////////////////////////////////////////////////////
QString CErrorRecord::GetErrorText() const
{
    QString str = error.trimmed();

    const char* sModuleName = "";
    // Module name
    switch (module)
    {
    case VALIDATOR_MODULE_UNKNOWN:
        sModuleName = "";
        break;
    case VALIDATOR_MODULE_RENDERER:
        sModuleName = "Renderer";
        break;
    case VALIDATOR_MODULE_3DENGINE:
        sModuleName = "Engine";
        break;
    case VALIDATOR_MODULE_SYSTEM:
        sModuleName = "System";
        break;
    case VALIDATOR_MODULE_AUDIO:
        sModuleName = "Audio";
        break;
    case VALIDATOR_MODULE_MOVIE:
        sModuleName = "Movie";
        break;
    case VALIDATOR_MODULE_EDITOR:
        sModuleName = "Editor";
        break;
    case VALIDATOR_MODULE_NETWORK:
        sModuleName = "Network";
        break;
    case VALIDATOR_MODULE_PHYSICS:
        sModuleName = "Physics";
        break;
    case VALIDATOR_MODULE_FEATURETESTS:
        sModuleName = "FeatureTests";
        break;
    case VALIDATOR_MODULE_SHINE:
        sModuleName = "UI";
        break;
    }
    str = QStringLiteral("[%1]\t[%2]\t%3").arg(count, 2).arg(QString(sModuleName).leftJustified(6), error).trimmed();

    if (!file.isEmpty())
    {
        str += QString("\t") + file;
    }
    else
    {
        str += QString("\t ");
    }
    if (pItem)
    {
        switch (pItem->GetType())
        {
        case EDB_TYPE_MATERIAL:
            str += QString("\t Material=\"");
            break;
        case EDB_TYPE_PARTICLE:
            str += QString("\t Particle=\"");
            break;
        case EDB_TYPE_MUSIC:
            str += QString("\t Music=\"");
            break;
        default:
            str += QString("\t Item=\"");
        }
        str += pItem->GetFullName() + "\"";
    }
    if (pObject)
    {
        str += QString("\t Object=\"") + pObject->GetName() + "\"";
    }
    return str;
}

//////////////////////////////////////////////////////////////////////////
// CError Report.
//////////////////////////////////////////////////////////////////////////
CErrorReport::CErrorReport()
{
    m_errors.reserve(100);
    m_bImmediateMode = true;
    m_bShowErrors = true;
    m_pObject = nullptr;
    m_pItem = nullptr;
    m_pParticle = nullptr;
}

//////////////////////////////////////////////////////////////////////////
void CErrorReport::ReportError(CErrorRecord& err)
{
    static bool bNoRecurse = false;
    if (bNoRecurse)
    {
        return;
    }

    bNoRecurse = true;
    if (m_bImmediateMode)
    {
        if (err.module == VALIDATOR_MODULE_EDITOR && err.severity == static_cast<int>(VALIDATOR_ERROR))
        {
            Warning(err.error.toUtf8().data());
        }
        else
        {
            // Show dialog if first character of warning is !.
            if (!err.error.isEmpty() && err.error[0] == '!')
            {
                Warning(err.error.toUtf8().data());
            }
            else
            {
                m_errors.push_back(err);
            }
        }
    }
    else
    {
        if (err.pObject == nullptr && m_pObject != nullptr)
        {
            err.pObject = m_pObject;
        }
        else if (err.pItem == nullptr && m_pItem != nullptr)
        {
            err.pItem = m_pItem;
        }
        m_errors.push_back(err);
    }
    bNoRecurse = false;
}

//////////////////////////////////////////////////////////////////////////
void CErrorReport::Clear()
{
    m_errors.clear();
}

//////////////////////////////////////////////////////////////////////////
inline bool SortErrorsByModule(const CErrorRecord& e1, const CErrorRecord& e2)
{
    if (e1.module == e2.module)
    {
        if (e1.error == e2.error)
        {
            return e1.file < e2.file;
        }
        else
        {
            return e1.error < e2.error;
        }
    }
    return e1.module < e2.module;
}

//////////////////////////////////////////////////////////////////////////
void CErrorReport::Display()
{
    if (m_errors.empty() || !m_bShowErrors)
    {
        SetImmediateMode(true);
        return;
    }

    // Sort by module.
    std::sort(m_errors.begin(), m_errors.end(), SortErrorsByModule);

    std::vector<CErrorRecord> errorList;
    errorList.swap(m_errors);

    m_errors.reserve(errorList.size());

    QString prevName = "";
    QString prevFile = "";
    for (int i = 0; i < errorList.size(); i++)
    {
        CErrorRecord& err = errorList[i];
        if (err.error != prevName || err.file != prevFile)
        {
            err.count = 1;
            m_errors.push_back(err);
        }
        else if (!m_errors.empty())
        {
            m_errors.back().count++;
        }
        prevName = err.error;
        prevFile = err.file;
    }

    // Log all errors.
    CryLogAlways("========================= Errors =========================");
    for (int i = 0; i < m_errors.size(); i++)
    {
        CErrorRecord& err = m_errors[i];
        QString str = err.GetErrorText();
        CryLogAlways("%3d) %s", i, str.toUtf8().data());
    }
    CryLogAlways("========================= End Errors =========================");

    ICVar* const noErrorReportWindowCVar = gEnv && gEnv->pConsole ? gEnv->pConsole->GetCVar("sys_no_error_report_window") : nullptr;
    if (noErrorReportWindowCVar && noErrorReportWindowCVar->GetIVal() == 0)
    {
        CErrorReportDialog::Open(this);
    }

    SetImmediateMode(true);
}

//////////////////////////////////////////////////////////////////////////
bool CErrorReport::IsEmpty() const
{
    return m_errors.empty();
}

//////////////////////////////////////////////////////////////////////////
CErrorRecord& CErrorReport::GetError(int i)
{
    assert(i >= 0 && i < m_errors.size());
    return m_errors[i];
};

//////////////////////////////////////////////////////////////////////////
void CErrorReport::SetImmediateMode(bool bEnable)
{
    if (bEnable != m_bImmediateMode)
    {
        Clear();
        m_bImmediateMode = bEnable;
    }
}

//////////////////////////////////////////////////////////////////////////
void CErrorReport::Report(SValidatorRecord& record)
{
    if ((record.flags & VALIDATOR_FLAG_IGNORE_IN_EDITOR))
    {
        return;
    }

    CErrorRecord err;
    if (record.text)
    {
        err.error = record.text;
    }
    if (record.description)
    {
        err.description = record.description;
    }
    if (record.file)
    {
        err.file = record.file;
    }
    else
    {
        err.file = m_currentFilename;
    }
    err.severity = (CErrorRecord::ESeverity)record.severity;

    err.assetScope = record.assetScope;

    err.flags = 0;
    if (record.flags & VALIDATOR_FLAG_FILE)
    {
        err.flags |= CErrorRecord::FLAG_NOFILE;
    }
    if (record.flags & VALIDATOR_FLAG_TEXTURE)
    {
        err.flags |= CErrorRecord::FLAG_TEXTURE;
    }
    if (record.flags & VALIDATOR_FLAG_SCRIPT)
    {
        err.flags |= CErrorRecord::FLAG_SCRIPT;
    }
    if (record.flags & VALIDATOR_FLAG_AI)
    {
        err.flags |= CErrorRecord::FLAG_AI;
    }

    err.module = record.module;
    err.pObject = m_pObject;
    err.pItem = m_pItem;

    ReportError(err);
}

//////////////////////////////////////////////////////////////////////////
void CErrorReport::SetCurrentValidatorObject(CBaseObject* pObject)
{
    m_pObject = pObject;
}

//////////////////////////////////////////////////////////////////////////
void CErrorReport::SetCurrentValidatorItem(CBaseLibraryItem* pItem)
{
    m_pItem = pItem;
}

//////////////////////////////////////////////////////////////////////////
void CErrorReport::SetCurrentFile(const QString& file)
{
    m_currentFilename = file;
}
