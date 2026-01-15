// SPDX-FileCopyrightText: 2025 Manuel Schneider
// SPDX-License-Identifier: MIT

#pragma once
#include <QObject>
#include <albert/export.h>
#include <albert/extension.h>
#include <map>

namespace albert
{

///
/// The common extension pool.
///
/// Clients can add their extensions, while services can track extensions by
/// listening to the signals added/removed or any particular extension
/// interface using ExtensionWatcher.
///
/// \ingroup core_extension
///
class ALBERT_EXPORT ExtensionRegistry : public QObject
{
    Q_OBJECT

public:

    /// Add extension to the registry
    bool registerExtension(Extension*);

    /// Remove extension from the registry
    void deregisterExtension(Extension*);

    /// Get map of all registered extensions
    const std::map<QString,Extension*> &extensions() const;

signals:

    /// Emitted when an extension has been registered.
    void added(albert::Extension*);

    /// Emitted when an extension has been deregistered.
    void removed(albert::Extension*);

private:

    std::map<QString,Extension*> extensions_;
};

}
