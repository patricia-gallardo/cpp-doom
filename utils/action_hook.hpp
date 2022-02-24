#pragma once

#include <variant>

template <std::size_t, class, class>
struct alternative_index_helper;

template <std::size_t index, class T, class U>
struct alternative_index_helper<index, T, std::variant<U>> {
  static constexpr std::size_t count = std::is_same_v<T, U>;
  static constexpr std::size_t value = index;
};

template <std::size_t index, class T, class U, class... Types>
struct alternative_index_helper<index, T, std::variant<U, Types...>> {
  static constexpr std::size_t count = std::is_same_v<T, U> + alternative_index_helper<index + 1, T, std::variant<Types...>>::count;
  static constexpr std::size_t value = std::is_same_v<T, U> ? index : alternative_index_helper<index + 1, T, std::variant<Types...>>::value;
};

template <class T, class U>
struct alternative_index {
  static_assert(alternative_index_helper<0, T, U>::count == 1, "There needs to be exactly one of the given type in the variant");
  static constexpr std::size_t value = alternative_index_helper<0, T, U>::value;
};
template <class T, class U>
inline constexpr std::size_t alternative_index_v = alternative_index<T, U>::value;

static_assert(alternative_index_v<long, std::variant<int, long, bool>> == 1);

template <class... Ts>
struct overloaded : Ts... {
  using Ts::operator()...;
};
template <class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

struct mobj_t;
struct player_t;
struct pspdef_t;
struct thinker_t;
struct floormove_t;
struct polyevent_t;
struct plat_t;
struct ceiling_t;
struct light_t;
struct ssthinker_t;
struct vldoor_t;
struct phase_t;
struct acs_t;
struct pillar_t;
struct polydoor_t;
struct floorWaggle_t;
struct lightflash_t;
struct strobe_t;
struct glow_t;
struct fireflicker_t;
struct slidedoor_t;

struct valid_hook {
  explicit valid_hook(bool is_valid)
      : valid(is_valid) { }
  constexpr bool is_valid() const { return valid; }

private:
  bool valid = false;
};

constexpr bool operator==(const valid_hook & a, const valid_hook & b) {
  return a.is_valid() == b.is_valid();
}

constexpr bool operator!=(const valid_hook & a, const valid_hook & b) {
  return !(a == b);
}

using null_hook                    = std::monostate;
using zero_param_action            = void (*)();
using mobj_param_action            = void (*)(mobj_t *);
using player_psp_param_action      = void (*)(player_t *, pspdef_t *);
using thinker_param_action         = void (*)(thinker_t *);
using floormove_param_action       = void (*)(floormove_t *);
using polyevent_param_action       = void (*)(polyevent_t *);
using plat_param_action            = void (*)(plat_t *);
using ceiling_param_action         = void (*)(ceiling_t *);
using light_param_action           = void (*)(light_t *);
using ssthinker_param_action       = void (*)(ssthinker_t *);
using vldoor_param_action          = void (*)(vldoor_t *);
using phase_param_action           = void (*)(phase_t *);
using acs_param_action             = void (*)(acs_t *);
using pillar_param_action          = void (*)(pillar_t *);
using polydoor_param_action        = void (*)(polydoor_t *);
using floorWaggle_param_action     = void (*)(floorWaggle_t *);
using lightflash_param_action      = void (*)(lightflash_t *);
using strobe_param_action          = void (*)(strobe_t *);
using glow_param_action            = void (*)(glow_t *);
using fireflicker_param_action     = void (*)(fireflicker_t *);
using mobj_player_psp_param_action = void (*)(mobj_t *, player_t *, pspdef_t *);
using slidedoor_param_action       = void (*)(slidedoor_t *);
using action_hook                  = std::variant<
    null_hook,
    valid_hook,
    zero_param_action,
    mobj_param_action,
    player_psp_param_action,
    thinker_param_action,
    floormove_param_action,
    polyevent_param_action,
    plat_param_action,
    ceiling_param_action,
    light_param_action,
    ssthinker_param_action,
    vldoor_param_action,
    phase_param_action,
    acs_param_action,
    pillar_param_action,
    polydoor_param_action,
    floorWaggle_param_action,
    lightflash_param_action,
    strobe_param_action,
    glow_param_action,
    fireflicker_param_action,
    slidedoor_param_action,
    mobj_player_psp_param_action>;

static_assert(alternative_index_v<null_hook, action_hook> == 0);

static_assert(alternative_index_v<valid_hook, action_hook> == 1);
constexpr int valid_hook_action_hook = alternative_index_v<valid_hook, action_hook>;

static_assert(alternative_index_v<mobj_param_action, action_hook> == 3);
constexpr int mobj_param_action_hook = alternative_index_v<mobj_param_action, action_hook>;

static_assert(alternative_index_v<player_psp_param_action, action_hook> == 4);
constexpr int player_psp_action_hook = alternative_index_v<player_psp_param_action, action_hook>;

static_assert(alternative_index_v<thinker_param_action, action_hook> == 5);
constexpr int thinker_param_action_hook = alternative_index_v<thinker_param_action, action_hook>;

static_assert(alternative_index_v<mobj_player_psp_param_action, action_hook> == 23);
constexpr int mobj_player_psp_param_action_hook = alternative_index_v<mobj_player_psp_param_action, action_hook>;

// Historically, "think_t" is yet another
//  function pointer to a routine to handle
//  an actor.
using actionf_t = action_hook;
using think_t   = actionf_t;

// Doubly linked list of actors.
struct thinker_t {
  struct thinker_t * prev {};
  struct thinker_t * next {};
  think_t            function {};
};

constexpr void call_thinker(thinker_t * thinker) {
  std::visit(
      overloaded {
          [&](const zero_param_action & callback) { callback(); },
          [&](const mobj_param_action & callback) { callback(reinterpret_cast<mobj_t *>(thinker)); },
          [&](const player_psp_param_action & callback) { callback(reinterpret_cast<player_t *>(thinker), reinterpret_cast<pspdef_t *>(thinker)); },
          [&](const thinker_param_action & callback) { callback(thinker); },
          [&](const floormove_param_action & callback) { callback(reinterpret_cast<floormove_t *>(thinker)); },
          [&](const polyevent_param_action & callback) { callback(reinterpret_cast<polyevent_t *>(thinker)); },
          [&](const plat_param_action & callback) { callback(reinterpret_cast<plat_t *>(thinker)); },
          [&](const ceiling_param_action & callback) { callback(reinterpret_cast<ceiling_t *>(thinker)); },
          [&](const light_param_action & callback) { callback(reinterpret_cast<light_t *>(thinker)); },
          [&](const ssthinker_param_action & callback) { callback(reinterpret_cast<ssthinker_t *>(thinker)); },
          [&](const vldoor_param_action & callback) { callback(reinterpret_cast<vldoor_t *>(thinker)); },
          [&](const phase_param_action & callback) { callback(reinterpret_cast<phase_t *>(thinker)); },
          [&](const acs_param_action & callback) { callback(reinterpret_cast<acs_t *>(thinker)); },
          [&](const pillar_param_action & callback) { callback(reinterpret_cast<pillar_t *>(thinker)); },
          [&](const polydoor_param_action & callback) { callback(reinterpret_cast<polydoor_t *>(thinker)); },
          [&](const floorWaggle_param_action & callback) { callback(reinterpret_cast<floorWaggle_t *>(thinker)); },
          [&](const lightflash_param_action & callback) { callback(reinterpret_cast<lightflash_t *>(thinker)); },
          [&](const strobe_param_action & callback) { callback(reinterpret_cast<strobe_t *>(thinker)); },
          [&](const glow_param_action & callback) { callback(reinterpret_cast<glow_t *>(thinker)); },
          [&](const fireflicker_param_action & callback) { callback(reinterpret_cast<fireflicker_t *>(thinker)); },
          [&](const slidedoor_param_action & callback) { callback(reinterpret_cast<slidedoor_t *>(thinker)); },
          [&](const mobj_player_psp_param_action & callback) { callback(reinterpret_cast<mobj_t *>(thinker), reinterpret_cast<player_t *>(thinker), reinterpret_cast<pspdef_t *>(thinker)); },
          [&](const auto &) { return; } },
      thinker->function);
}

constexpr bool is_valid(const action_hook & hook) {
  return std::visit(overloaded {
                        [](const valid_hook & ref) { return ref.is_valid(); },
                        [](const auto &) { return false; } },
                    hook);
}

constexpr bool action_hook_has_value(const action_hook & hook) {
  return hook.index() != 0;
}

constexpr bool action_hook_is_empty(const action_hook & hook) {
  return !action_hook_has_value(hook);
}
