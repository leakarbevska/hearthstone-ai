#include <fstream>
#include "json/reader.h"
#include "game-engine/card-database.h"

namespace GameEngine {

	static Card::Rarity GetRarity(Json::Value const& json_card)
	{
		if (!json_card.isMember("rarity")) return Card::RARITY_UNKNOWN;

		std::string rarity = json_card["rarity"].asString();

		if (rarity == "FREE") return Card::RARITY_FREE;
		if (rarity == "COMMON") return Card::RARITY_COMMON;
		if (rarity == "RARE") return Card::RARITY_RARE;
		if (rarity == "EPIC") return Card::RARITY_EPIC;
		if (rarity == "LEGENDARY") return Card::RARITY_LEGENDARY;

		throw std::runtime_error("parse error");
	}

	static Card::MinionRace GetMinionRace(Json::Value const& json_card)
	{
		if (!json_card.isMember("race")) return Card::RACE_NORMAL;

		std::string race = json_card["race"].asString();
		if (race == "BEAST") return Card::RACE_BEAST;
		if (race == "DEMON") return Card::RACE_DEMON;
		if (race == "DRAGON") return Card::RACE_DRAGON;
		if (race == "MECHANICAL") return Card::RACE_MECH;
		if (race == "MURLOC") return Card::RACE_MURLOC;
		if (race == "PIRATE") return Card::RACE_PIRATE;
		if (race == "TOTEM") return Card::RACE_TOTEM;

		throw std::runtime_error("unknown race");
	}

	static void ProcessMinionCardMechanics(Json::Value const& json_card, Card & new_card)
	{
		if (!json_card.isMember("mechanics")) return;

		Json::Value json_mechanics = json_card["mechanics"];

		if (json_mechanics.isArray() == false) return;

		new_card.data.minion.taunt = false;
		new_card.data.minion.charge = false;
		new_card.data.minion.shield = false;
		new_card.data.minion.stealth = false;

		for (auto const& json_mechanic: json_mechanics)
		{
			std::string mechanic = json_mechanic.asString();
			if (mechanic == "TAUNT") {
				new_card.data.minion.taunt = true;
			}
			else if (mechanic == "CHARGE") {
				new_card.data.minion.charge = true;
			}
			else if (mechanic == "DIVINE_SHIELD") {
				new_card.data.minion.shield = true;
			}
			else if (mechanic == "STEALTH") {
				new_card.data.minion.stealth = true;
			}
			else if (mechanic == "FORGETFUL") { // TODO
			}
			else if (mechanic == "FREEZE") { // TODO
			}
			else if (mechanic == "POISONOUS") { // TODO
			}
			else if (mechanic == "WINDFURY") { // TODO
			}
			else if (mechanic == "OVERLOAD") { // TODO
			}
			else if (mechanic == "AURA" ||
				mechanic == "DEATHRATTLE" ||
				mechanic == "INSPIRE" ||
				mechanic == "BATTLECRY" ||
				mechanic == "SPELLPOWER" ||
				mechanic == "COMBO" ||
				mechanic == "ENRAGED" ||
				mechanic == "ADJACENT_BUFF" ||
				mechanic == "InvisibleDeathrattle" ||
				mechanic == "ImmuneToSpellpower") {
				// write hard-coded
			}
			else {
				throw std::runtime_error("unknown error");
			}
		}
	}

	CardDatabase::CardDatabase()
	{
		this->Clear();
	}

	bool CardDatabase::ReadFromJsonFile(std::string const & filepath)
	{
		Json::Reader reader;
		Json::Value cards_json;

		std::ifstream cards_file(filepath);

		if (reader.parse(cards_file, cards_json, false) == false) return false;

		return this->ReadFromJson(cards_json);
	}

	bool CardDatabase::ReadFromJson(Json::Value const & cards_json)
	{
		this->Clear();

		if (cards_json.isArray() == false) return false;

		for (auto const& card_json : cards_json)
		{
			this->AddCard(card_json);
		}

		return true;
	}

	bool CardDatabase::AddCard(Json::Value const & card)
	{
		std::string type = card["type"].asString();

		Card new_card;

		if (type == "MINION") {
			return this->AddMinionCard(card, new_card);
		}
		else if (type == "SPELL") {
			return this->AddSpellCard(card, new_card);
		}
		else {
			// ignored
			return true;
		}
	}

	bool CardDatabase::AddMinionCard(Json::Value const & json_card, Card & new_card)
	{
		std::string origin_id = json_card["id"].asString();

		new_card.rarity = GetRarity(json_card);
		new_card.type = Card::TYPE_MINION;
		new_card.cost = json_card["cost"].asInt();
		new_card.data.minion.Clear();
		new_card.data.minion.attack = json_card["attack"].asInt();
		new_card.data.minion.hp = json_card["health"].asInt();
		new_card.data.minion.race = GetMinionRace(json_card);

		ProcessMinionCardMechanics(json_card, new_card);

		new_card.id = this->GetAvailableCardId();

		this->AddCard(new_card, origin_id);
		return true;
	}

	bool CardDatabase::AddSpellCard(Json::Value const & json_card, Card & new_card)
	{
		std::string origin_id = json_card["id"].asString();

		new_card.rarity = GetRarity(json_card);
		new_card.type = Card::TYPE_SPELL;
		new_card.cost = json_card["cost"].asInt();
		new_card.id = this->GetAvailableCardId();
		new_card.data.spell.Clear();

		this->AddCard(new_card, origin_id);
		return true;
	}

	void CardDatabase::AddCard(Card const & card, std::string const& origin_id)
	{
		this->cards[card.id] = card;
		this->origin_id_map[origin_id] = card.id;
	}

	void CardDatabase::Clear()
	{
		this->cards.clear();
		this->next_card_id = 1;
	}

	int CardDatabase::GetAvailableCardId()
	{
		int id = this->next_card_id;
		++this->next_card_id;
		return id;
	}

	Card CardDatabase::GetCard(int card_id) const
	{
		auto it = this->cards.find(card_id);
		if (it == this->cards.end())
		{
			Card ret;
			ret.MarkInvalid();
			return ret;
		}

		return it->second;
	}

	std::unordered_map<std::string, int> const & CardDatabase::GetOriginalIdMap() const
	{
		return this->origin_id_map;
	}

	int CardDatabase::GetCardIdFromOriginalId(std::string const & origin_id) const
	{
		auto it = this->origin_id_map.find(origin_id);
		if (it == this->origin_id_map.end()) return -1;
		return it->second;
	}

} // namespace