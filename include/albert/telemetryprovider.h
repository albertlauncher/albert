// SPDX-FileCopyrightText: 2024 Manuel Schneider

#pragma once
#include <QJsonObject>
#include <albert/export.h>
#include <albert/extension.h>


// PRIVATE API - DO NOT USE!


namespace albert
{
class ALBERT_EXPORT TelemetryProvider : virtual public Extension
{
public:
    virtual QJsonObject telemetryData() const = 0;

protected:
    ~TelemetryProvider() override;
};
}
