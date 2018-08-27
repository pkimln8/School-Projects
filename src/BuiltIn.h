#ifndef __BUILTIN_H
#define __BUILTIN_H

#include <stdbool.h>
#include <obstack.h>
#include <stdlib.h>
#include <termios.h>
#include "esh.h"

void esh_built_in_commands(struct esh_pipeline *pipe);

#endif
