// Copyright (c) 2023-2024 Manuel Schneider

#include "action.h"

albert::Action::Action(QString i, QString t, std::function<void()> f) noexcept :
    id(std::move(i)), text(std::move(t)), function(std::move(f))
{}

