// Copyright (C) 2014-2018 Manuel Schneider

#pragma once
#include "core_globals.h"

extern int main(int argc, char** argv);

namespace Core {

class AlbertApp
{
    static int EXPORT_CORE run(int argc, char **argv);
    friend int ::main(int argc, char** argv);
};

}


