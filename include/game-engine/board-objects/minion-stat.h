#pragma once

#include <bitset>
#include "game-engine/common.h"

namespace GameEngine {
namespace BoardObjects {

class MinionStat
{
	friend std::hash<MinionStat>;

public:
	enum Flag {
		FLAG_TAUNT = 0,
		FLAG_CHARGE,
		FLAG_SHIELD,
		FLAG_STEALTH,
		FLAG_FORGETFUL,
		FLAG_FREEZE_ATTACKER,
		FLAG_FREEZED,
		FLAG_MAX
	};

	typedef std::bitset<FLAG_MAX> Flags;

public:
	MinionStat() : attack(0), hp(0), max_hp(0) {}

	bool operator==(MinionStat const& rhs) const
	{
		if (this->attack != rhs.attack) return false;
		if (this->hp != rhs.hp) return false;
		if (this->max_hp != rhs.max_hp) return false;
		if (this->flags != rhs.flags) return false;
		return true;
	}

	bool operator!=(MinionStat const& rhs) const { return !(*this == rhs); }

	void SetAttack(int attack) { this->attack = attack; }
	void SetHP(int hp) { this->hp = hp; }
	void SetMaxHP(int max_hp) { this->max_hp = max_hp; }
	void SetFlag(Flag flag, bool val) { this->flags.set(flag, val); }

	int GetAttack() const { return this->attack; }
	int GetHP() const { return this->hp; }
	int GetMaxHP() const { return this->max_hp; }
	bool GetFlag(Flag flag) const { return this->flags[flag]; }

	void ClearFlags() { this->flags.reset(); }
	void MergeFlags(MinionStat const& rhs) { this->flags |= rhs.flags; }

private:
	int attack;
	int hp;
	int max_hp;
	Flags flags;
};

}
}

namespace std {

	template <> struct hash<GameEngine::BoardObjects::MinionStat> {
		typedef GameEngine::BoardObjects::MinionStat argument_type;
		typedef std::size_t result_type;
		result_type operator()(const argument_type &s) const {
			result_type result = 0;

			GameEngine::hash_combine(result, s.attack);
			GameEngine::hash_combine(result, s.hp);
			GameEngine::hash_combine(result, s.max_hp);
			GameEngine::hash_combine(result, s.flags);

			return result;
		}
	};
}