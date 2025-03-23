// Copyright (c) 2023-2025 Manuel Schneider

#include "action.h"
using namespace std;

albert::Action::Action(QString i, QString t, ::function<void()> f, bool hideOnActivation) noexcept:
    id(::move(i)),
    text(::move(t)),
    function(::move(f)),
    hide_on_activation(hideOnActivation)
{}

