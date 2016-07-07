#include "ObjectMgr.h"
#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "Player.h"
#include "AccountMgr.h"
#include "time.h"
#include <stdio.h>
#include "Bag.h"
#include "Common.h"
#include "Config.h"
#include "DatabaseEnv.h"
#include "DBCStructure.h"
#include "Define.h"
#include "Field.h"
#include "GameEventMgr.h"
#include "Item.h"
#include "ItemPrototype.h"
#include "Language.h"
#include "Log.h"
#include "ObjectGuid.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "QueryResult.h"
#include "ScriptMgr.h"
#include "SharedDefines.h"
#include "Transaction.h"
#include "WorldSession.h"
#include <sstream>
#include <string>
#include <stdlib.h>




class automatic : public CreatureScript
{
public:
	automatic() : CreatureScript("automatic") { }


	bool OnGossipHello(Player* player, Creature* creature)
	{
		player->ADD_GOSSIP_ITEM_EXTENDED(7, "Existiert mein Charakter noch?", GOSSIP_SENDER_MAIN, 0, "Der Name lautet: ", 0, true);
		player->ADD_GOSSIP_ITEM_EXTENDED(7, "Ubertrage meinen Charakter auf einen anderen Account! [Unwideruflich / 5000 Gold]", GOSSIP_SENDER_MAIN, 2, "Der Accountname lautet: ", 0, true);
		player->ADD_GOSSIP_ITEM(7, "Ich brauche Hilfe!", GOSSIP_SENDER_MAIN, 3);
		player->ADD_GOSSIP_ITEM(7, "Danke! Bringt mich zurueck!", GOSSIP_SENDER_MAIN, 1);
		player->PlayerTalkClass->SendGossipMenu(1, creature->GetGUID());
		return true;
	}

	bool OnGossipSelectCode(Player * player, Creature* /*creature*/, uint32 /*sender*/, uint32 action, const char* code){

		switch (action){


		case 0:
		{
			std::string codes = code;

			PreparedStatement* charselbyname = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHARACTER_BYNAME);
			charselbyname->setString(0, codes);
			PreparedQueryResult result = CharacterDatabase.Query(charselbyname);

			if (!result){
				player->GetSession()->SendNotification("Es wurde kein Charakter mit dem angegebenen Namen gefunden.");
				return true;
			}

			if (result){

				
				Field* feld = result->Fetch();
				uint32 guid = feld[0].GetInt32();
				uint32 account = feld[1].GetInt32();
				std::string name = feld[2].GetCString();
				//uint32 level = feld[3].GetInt32();
				uint32 totaltime = feld[4].GetInt32();
				
				PreparedStatement* getaccountnamebyid = LoginDatabase.GetPreparedStatement(LOGIN_SEL_ACCOUNT_BYID);
				getaccountnamebyid->setInt32(0, account);
				PreparedQueryResult ergebnis = LoginDatabase.Query(getaccountnamebyid);

				if (!result){
					player->GetSession()->SendNotification("Error");
					return true;
				}

				
				Field* ergfeld = ergebnis->Fetch();
				std::string accname = ergfeld[0].GetCString();
				uint32 spielzeith = totaltime / 60 / 60;
				uint32 spielzeit = totaltime / 60 / 60 / 24;

				std::ostringstream pp;
				pp << "Folgende Daten wurden gefunden \nGuid: " << guid << "\nAccountname: " << accname << "\nSpielzeit in Stunden: " << spielzeith << "\nSpielzeit in Tagen: " << spielzeit;
					ChatHandler(player->GetSession()).PSendSysMessage(pp.str().c_str(),
					player->GetName());
					return true;
			}
			return true;

		}break;


		case 2:
		{

			std::string codes = code;
			PreparedStatement* stmt = LoginDatabase.GetPreparedStatement(LOGIN_SEL_ACCOUNT_ID_BY_NAME);
			stmt->setString(0, codes);
			PreparedQueryResult result = LoginDatabase.Query(stmt);

			if (!result){
				player->GetSession()->SendNotification("Account wurde nicht gefunden!");
				return true;
			}

			Field* feld = result->Fetch();
			uint32 accid = feld[0].GetInt32();


			PreparedStatement* charsum = CharacterDatabase.GetPreparedStatement(CHAR_SEL_SUM_CHARS);
			charsum->setInt32(0, accid);
			PreparedQueryResult ergebnis = CharacterDatabase.Query(charsum);

			if (!ergebnis){
				player->GetSession()->SendNotification("Error beim Counten der Charaktere!");
				return true;
			}

			Field* felder = ergebnis->Fetch();
			uint32 charactersum = felder[0].GetInt32();

			if (charactersum <= 9 && player->HasEnoughMoney(5000*GOLD)){
				PreparedStatement* updateacc = CharacterDatabase.GetPreparedStatement(CHAR_UPD_ACCOUNT_ID);
				updateacc->setInt32(0, accid);
				updateacc->setInt32(1, player->GetGUID());
				CharacterDatabase.Execute(updateacc);
				player->ModifyMoney(-5000 * GOLD);
				player->GetSession()->SendNotification("Der Accounttausch wurde vollzogen");
				ChatHandler(player->GetSession()).PSendSysMessage("Der Accounttausch wurde vollzogen.",
					player->GetName());

				return true;
			}

			if (!player->HasEnoughMoney(5000 * GOLD)){
				player->GetSession()->SendNotification("Du hast nicht genug Gold");
			}

			if (charactersum == 10){
				player->GetSession()->SendNotification("Du hast zu viele Charaktere auf deinem Account.");
				return true;
			}
			return true;
		}


		return true;

		}

		return true;
	}


	bool OnGossipSelect(Player * player, Creature * creature, uint32 /*uiSender*/, uint32 uiAction){

		switch (uiAction)
		{
		case 1:
		{
			player->Recall();
			return true;
		}break;

		case 3:
		{
			
			player->PlayerTalkClass->ClearMenus();

			player->ADD_GOSSIP_ITEM(7, "Zu den Features", GOSSIP_SENDER_MAIN, 4);
			player->PlayerTalkClass->SendGossipMenu(907, creature->GetGUID());
			return true;
		}

		case 4:{

			ChatHandler(player->GetSession()).PSendSysMessage("Um den Charaktertransfer durchzufuehren, benoetigst du 5k Gold. Du wirst aufgefordert den Accountnamen einzugeben zu welchem der Charakter transferiert werden soll. Also nicht deinen eigenen, sondern der Zielaccount! Einmal bestaetigt kann der Transfer nicht mehr zurueckgenommen werden. Sobald du dich ausloggst ist der Charakter verschwunden und auf dem anderen Account verf�gbar.",
				player->GetName());
			return true;
		}break;
		
			return true;
		}
		return true;
	}

};

void AddSC_automaticsupport()
{
	new automatic();
}