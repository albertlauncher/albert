// SPDX-FileCopyrightText: 2025 Manuel Schneider
// SPDX-License-Identifier: MIT

#pragma once
#include <QJsonObject>
#include <albert/export.h>
#include <albert/extension.h>


// PRIVATE API - DO NOT USE!


namespace albert::detail
{
class ALBERT_EXPORT TelemetryProvider : virtual public Extension
{
public:
    virtual QJsonObject telemetryData() const = 0;

protected:
    ~TelemetryProvider() override;
};
}
