//
// Copyright(C) 2005-2014 Simon Howard
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// DESCRIPTION:
//     OPL internal interface.
//


#ifndef OPL_INTERNAL_H
#define OPL_INTERNAL_H

#include "opl.hpp"


using opl_init_func             = int (*)(unsigned int);
using opl_shutdown_func         = void (*)();
using opl_read_port_func        = unsigned int (*)(opl_port_t);
using opl_write_port_func       = void (*)(opl_port_t, unsigned int);
using opl_set_callback_func     = void (*)(uint64_t, opl_callback_t, void *);
using opl_clear_callbacks_func  = void (*)();
using opl_lock_func             = void (*)();
using opl_unlock_func           = void (*)();
using opl_set_paused_func       = void (*)(int);
using opl_adjust_callbacks_func = void (*)(float);

using opl_driver_t = struct
{
    const char *name;

    opl_init_func             init_func;
    opl_shutdown_func         shutdown_func;
    opl_read_port_func        read_port_func;
    opl_write_port_func       write_port_func;
    opl_set_callback_func     set_callback_func;
    opl_clear_callbacks_func  clear_callbacks_func;
    opl_lock_func             lock_func;
    opl_unlock_func           unlock_func;
    opl_set_paused_func       set_paused_func;
    opl_adjust_callbacks_func adjust_callbacks_func;
};

// Sample rate to use when doing software emulation.

extern unsigned int opl_sample_rate;

#endif /* #ifndef OPL_INTERNAL_H */
