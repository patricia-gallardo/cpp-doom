#pragma once

#include <variant>

struct mobj_s;
typedef mobj_s mobj_t;

struct player_s;
typedef player_s player_t;

struct pspdef_s;
typedef pspdef_s pspdef_t;

using zero_param_action              = void (*)();
using one_param_action               = void (*)(mobj_t *);
using two_param_action               = void (*)(player_t *, pspdef_t *);
using action_hook                    = std::variant<std::monostate, zero_param_action, one_param_action, two_param_action>;
constexpr int empty_action_hook      = 0;
constexpr int zero_param_action_hook = 1;
constexpr int one_param_action_hook  = 2;
constexpr int two_param_action_hook  = 3;
