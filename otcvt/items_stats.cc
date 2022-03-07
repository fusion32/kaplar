// NOTE: All item attributes taken from "opentibia/src/items.cpp":
//
//type
//name
//article
//plural
//description
//runespellname
//weight
//showcount
//armor
//defense
//extradef
//attack
//rotateto
//moveable
//blockprojectile
//pickupable
//allowpickupable
//floorchange
//corpsetype
//containersize
//fluidsource
//readable
//writeable
//maxtextlen
//writeonceitemid
//weapontype
//slottype
//ammotype
//shoottype
//effect
//range
//stopduration
//decayto
//transformequipto
//transformdeequipto
//duration
//showduration
//charges
//showcharges
//breakchance
//ammoaction
//hitchance
//maxhitchance
//invisible
//speed
//healthgain
//healthticks
//managain
//manaticks
//manashield
//skillsword
//skillaxe
//skillclub
//skilldist
//skillfish
//skillshield
//skillfist
//maxhitpoints
//maxhitpointspercent
//maxmanapoints
//maxmanapointspercent
//soulpoints
//soulpointspercent
//magicpoints
//magicpointspercent
//absorbpercentall
//absorbpercentallelements
//absorbpercentenergy
//absorbpercentfire
//absorbpercentpoison
//absorbpercentearth
//absorbpercentice
//absorbpercentholy
//absorbpercentdeath
//absorbpercentlifedrain
//absorbpercentmanadrain
//absorbpercentdrown
//absorbpercentphysical
//suppressdrunk
//suppressenergy
//suppressfire
//suppresspoison
//suppressdrown
//suppressfreeze
//suppressdazzle
//suppresscurse
//preventitemloss
//preventskillloss
//combattype
//replaceable
//partnerdirection
//malesleeper
//femalesleeper
//nosleeper
//elementice
//elementearth
//elementfire
//elementenergy
//currency

#include "xml.hh"

int main(int argc, char **argv){
	const char *filename = "items.xml";
	if(argc >= 2)
		filename = argv[1];

	XML_State *xml = xml_init_from_file(filename);
	if(!xml){
		printf("failed to open file `%s`\n", filename);
		return -1;
	}

	XML_NodeTag items_tag;
	XML_NodeAttributes items_attr;
	if(!xml_read_node(xml, &items_tag, &items_attr)){
		if(xml_error(xml)){
			printf("%s\n", xml_error_string(xml));
		}else{
			printf("root node not found\n");
		}
		return -1;
	}

	if(!string_eq(items_tag.text, "items")){
		printf("unexpected root node (expected = %s, got = %s)\n",
			"items", items_tag.text);
		return -1;
	}

	if(items_attr.num_attributes > 0){
		printf("unexpected <items> attributes:\n");
		for(i32 i = 0; i < items_attr.num_attributes; i += 1){
			printf("    %s=\"%s\"\n",
				items_attr.attributes[i].key,
				items_attr.attributes[i].value);
		}
		return -1;
	}

	// item node attr stats
	i32 num_node_attr_id = 0;
	i32 num_node_attr_name = 0;
	i32 num_node_attr_article = 0;
	i32 num_node_attr_plural = 0;
	i32 num_node_attr_editorsuffix = 0;
	i32 num_node_attr_other = 0;

	// item attr stats
	i32 num_item_attr_type = 0;
	i32 num_item_attr_name = 0;
	i32 num_item_attr_article = 0;
	i32 num_item_attr_plural = 0;
	i32 num_item_attr_description = 0;
	i32 num_item_attr_runespellname = 0;
	i32 num_item_attr_weight = 0;
	i32 num_item_attr_showcount = 0;
	i32 num_item_attr_armor = 0;
	i32 num_item_attr_defense = 0;
	i32 num_item_attr_extradef = 0;
	i32 num_item_attr_attack = 0;
	i32 num_item_attr_rotateto = 0;
	i32 num_item_attr_moveable = 0;
	i32 num_item_attr_blockprojectile = 0;
	i32 num_item_attr_pickupable = 0;
	i32 num_item_attr_allowpickupable = 0;
	i32 num_item_attr_floorchange = 0;
	i32 num_item_attr_corpsetype = 0;
	i32 num_item_attr_containersize = 0;
	i32 num_item_attr_fluidsource = 0;
	i32 num_item_attr_readable = 0;
	i32 num_item_attr_writeable = 0;
	i32 num_item_attr_maxtextlen = 0;
	i32 num_item_attr_writeonceitemid = 0;
	i32 num_item_attr_weapontype = 0;
	i32 num_item_attr_slottype = 0;
	i32 num_item_attr_ammotype = 0;
	i32 num_item_attr_shoottype = 0;
	i32 num_item_attr_effect = 0;
	i32 num_item_attr_range = 0;
	i32 num_item_attr_stopduration = 0;
	i32 num_item_attr_decayto = 0;
	i32 num_item_attr_transformequipto = 0;
	i32 num_item_attr_transformdeequipto = 0;
	i32 num_item_attr_duration = 0;
	i32 num_item_attr_showduration = 0;
	i32 num_item_attr_charges = 0;
	i32 num_item_attr_showcharges = 0;
	i32 num_item_attr_breakchance = 0;
	i32 num_item_attr_ammoaction = 0;
	i32 num_item_attr_hitchance = 0;
	i32 num_item_attr_maxhitchance = 0;
	i32 num_item_attr_invisible = 0;
	i32 num_item_attr_speed = 0;
	i32 num_item_attr_healthgain = 0;
	i32 num_item_attr_healthticks = 0;
	i32 num_item_attr_managain = 0;
	i32 num_item_attr_manaticks = 0;
	i32 num_item_attr_manashield = 0;
	i32 num_item_attr_skillsword = 0;
	i32 num_item_attr_skillaxe = 0;
	i32 num_item_attr_skillclub = 0;
	i32 num_item_attr_skilldist = 0;
	i32 num_item_attr_skillfish = 0;
	i32 num_item_attr_skillshield = 0;
	i32 num_item_attr_skillfist = 0;
	i32 num_item_attr_maxhitpoints = 0;
	i32 num_item_attr_maxhitpointspercent = 0;
	i32 num_item_attr_maxmanapoints = 0;
	i32 num_item_attr_maxmanapointspercent = 0;
	i32 num_item_attr_soulpoints = 0;
	i32 num_item_attr_soulpointspercent = 0;
	i32 num_item_attr_magicpoints = 0;
	i32 num_item_attr_magicpointspercent = 0;
	i32 num_item_attr_absorbpercentall = 0;
	i32 num_item_attr_absorbpercentallelements = 0;
	i32 num_item_attr_absorbpercentenergy = 0;
	i32 num_item_attr_absorbpercentfire = 0;
	i32 num_item_attr_absorbpercentpoison = 0;
	i32 num_item_attr_absorbpercentearth = 0;
	i32 num_item_attr_absorbpercentice = 0;
	i32 num_item_attr_absorbpercentholy = 0;
	i32 num_item_attr_absorbpercentdeath = 0;
	i32 num_item_attr_absorbpercentlifedrain = 0;
	i32 num_item_attr_absorbpercentmanadrain = 0;
	i32 num_item_attr_absorbpercentdrown = 0;
	i32 num_item_attr_absorbpercentphysical = 0;
	i32 num_item_attr_suppressdrunk = 0;
	i32 num_item_attr_suppressenergy = 0;
	i32 num_item_attr_suppressfire = 0;
	i32 num_item_attr_suppresspoison = 0;
	i32 num_item_attr_suppressdrown = 0;
	i32 num_item_attr_suppressfreeze = 0;
	i32 num_item_attr_suppressdazzle = 0;
	i32 num_item_attr_suppresscurse = 0;
	i32 num_item_attr_preventitemloss = 0;
	i32 num_item_attr_preventskillloss = 0;
	i32 num_item_attr_combattype = 0;
	i32 num_item_attr_replaceable = 0;
	i32 num_item_attr_partnerdirection = 0;
	i32 num_item_attr_malesleeper = 0;
	i32 num_item_attr_femalesleeper = 0;
	i32 num_item_attr_nosleeper = 0;
	i32 num_item_attr_elementice = 0;
	i32 num_item_attr_elementearth = 0;
	i32 num_item_attr_elementfire = 0;
	i32 num_item_attr_elementenergy = 0;
	i32 num_item_attr_currency = 0;
	i32 num_item_attr_other = 0;


	char aslower[256];
	XML_NodeTag item_tag;
	XML_NodeAttributes item_attr;
	while(xml_read_node(xml, &item_tag, &item_attr)){
		if(!string_eq(item_tag.text, "item")){
			printf("unexpected <items> child <%s>\n", item_tag.text);
			return -1;
		}

		for(i32 i = 0; i < item_attr.num_attributes; i += 1){
			string_ascii_tolower(aslower, sizeof(aslower),
				item_attr.attributes[i].key);
			if(string_eq(aslower, "id"))
				num_node_attr_id += 1;
			else if(string_eq(aslower, "name"))
				num_node_attr_name += 1;
			else if(string_eq(aslower, "article"))
				num_node_attr_article += 1;
			else if(string_eq(aslower, "plural"))
				num_node_attr_plural += 1;
			else if(string_eq(aslower, "editorsuffix"))
				num_node_attr_editorsuffix += 1;
			else
				num_node_attr_other += 1;
		}

		if(!item_attr.self_closed){
			XML_NodeTag attr_tag;
			XML_NodeAttributes attr_attr;
			while(xml_read_node(xml, &attr_tag, &attr_attr)){
				if(!string_eq(attr_tag.text, "attribute")){
					printf("unexpected <item> child <%s>\n", attr_tag.text);
					return -1;
				}

				for(i32 i = 0; i < attr_attr.num_attributes; i += 1){
					string_ascii_tolower(aslower, sizeof(aslower),
						attr_attr.attributes[i].key);
					if(!string_eq(aslower, "key"))
						continue;

					string_ascii_tolower(aslower, sizeof(aslower),
						attr_attr.attributes[i].value);

					if(string_eq(aslower, "type"))
						num_item_attr_type += 1;
					else if(string_eq(aslower, "name"))
						num_item_attr_name += 1;
					else if(string_eq(aslower, "article"))
						num_item_attr_article += 1;
					else if(string_eq(aslower, "plural"))
						num_item_attr_plural += 1;
					else if(string_eq(aslower, "description"))
						num_item_attr_description += 1;
					else if(string_eq(aslower, "runespellname"))
						num_item_attr_runespellname += 1;
					else if(string_eq(aslower, "weight"))
						num_item_attr_weight += 1;
					else if(string_eq(aslower, "showcount"))
						num_item_attr_showcount += 1;
					else if(string_eq(aslower, "armor"))
						num_item_attr_armor += 1;
					else if(string_eq(aslower, "defense"))
						num_item_attr_defense += 1;
					else if(string_eq(aslower, "extradef"))
						num_item_attr_extradef += 1;
					else if(string_eq(aslower, "attack"))
						num_item_attr_attack += 1;
					else if(string_eq(aslower, "rotateto"))
						num_item_attr_rotateto += 1;
					else if(string_eq(aslower, "moveable"))
						num_item_attr_moveable += 1;
					else if(string_eq(aslower, "blockprojectile"))
						num_item_attr_blockprojectile += 1;
					else if(string_eq(aslower, "pickupable"))
						num_item_attr_pickupable += 1;
					else if(string_eq(aslower, "allowpickupable"))
						num_item_attr_allowpickupable += 1;
					else if(string_eq(aslower, "floorchange"))
						num_item_attr_floorchange += 1;
					else if(string_eq(aslower, "corpsetype"))
						num_item_attr_corpsetype += 1;
					else if(string_eq(aslower, "containersize"))
						num_item_attr_containersize += 1;
					else if(string_eq(aslower, "fluidsource"))
						num_item_attr_fluidsource += 1;
					else if(string_eq(aslower, "readable"))
						num_item_attr_readable += 1;
					else if(string_eq(aslower, "writeable"))
						num_item_attr_writeable += 1;
					else if(string_eq(aslower, "maxtextlen"))
						num_item_attr_maxtextlen += 1;
					else if(string_eq(aslower, "writeonceitemid"))
						num_item_attr_writeonceitemid += 1;
					else if(string_eq(aslower, "weapontype"))
						num_item_attr_weapontype += 1;
					else if(string_eq(aslower, "slottype"))
						num_item_attr_slottype += 1;
					else if(string_eq(aslower, "ammotype"))
						num_item_attr_ammotype += 1;
					else if(string_eq(aslower, "shoottype"))
						num_item_attr_shoottype += 1;
					else if(string_eq(aslower, "effect"))
						num_item_attr_effect += 1;
					else if(string_eq(aslower, "range"))
						num_item_attr_range += 1;
					else if(string_eq(aslower, "stopduration"))
						num_item_attr_stopduration += 1;
					else if(string_eq(aslower, "decayto"))
						num_item_attr_decayto += 1;
					else if(string_eq(aslower, "transformequipto"))
						num_item_attr_transformequipto += 1;
					else if(string_eq(aslower, "transformdeequipto"))
						num_item_attr_transformdeequipto += 1;
					else if(string_eq(aslower, "duration"))
						num_item_attr_duration += 1;
					else if(string_eq(aslower, "showduration"))
						num_item_attr_showduration += 1;
					else if(string_eq(aslower, "charges"))
						num_item_attr_charges += 1;
					else if(string_eq(aslower, "showcharges"))
						num_item_attr_showcharges += 1;
					else if(string_eq(aslower, "breakchance"))
						num_item_attr_breakchance += 1;
					else if(string_eq(aslower, "ammoaction"))
						num_item_attr_ammoaction += 1;
					else if(string_eq(aslower, "hitchance"))
						num_item_attr_hitchance += 1;
					else if(string_eq(aslower, "maxhitchance"))
						num_item_attr_maxhitchance += 1;
					else if(string_eq(aslower, "invisible"))
						num_item_attr_invisible += 1;
					else if(string_eq(aslower, "speed"))
						num_item_attr_speed += 1;
					else if(string_eq(aslower, "healthgain"))
						num_item_attr_healthgain += 1;
					else if(string_eq(aslower, "healthticks"))
						num_item_attr_healthticks += 1;
					else if(string_eq(aslower, "managain"))
						num_item_attr_managain += 1;
					else if(string_eq(aslower, "manaticks"))
						num_item_attr_manaticks += 1;
					else if(string_eq(aslower, "manashield"))
						num_item_attr_manashield += 1;
					else if(string_eq(aslower, "skillsword"))
						num_item_attr_skillsword += 1;
					else if(string_eq(aslower, "skillaxe"))
						num_item_attr_skillaxe += 1;
					else if(string_eq(aslower, "skillclub"))
						num_item_attr_skillclub += 1;
					else if(string_eq(aslower, "skilldist"))
						num_item_attr_skilldist += 1;
					else if(string_eq(aslower, "skillfish"))
						num_item_attr_skillfish += 1;
					else if(string_eq(aslower, "skillshield"))
						num_item_attr_skillshield += 1;
					else if(string_eq(aslower, "skillfist"))
						num_item_attr_skillfist += 1;
					else if(string_eq(aslower, "maxhitpoints"))
						num_item_attr_maxhitpoints += 1;
					else if(string_eq(aslower, "maxhitpointspercent"))
						num_item_attr_maxhitpointspercent += 1;
					else if(string_eq(aslower, "maxmanapoints"))
						num_item_attr_maxmanapoints += 1;
					else if(string_eq(aslower, "maxmanapointspercent"))
						num_item_attr_maxmanapointspercent += 1;
					else if(string_eq(aslower, "soulpoints"))
						num_item_attr_soulpoints += 1;
					else if(string_eq(aslower, "soulpointspercent"))
						num_item_attr_soulpointspercent += 1;
					else if(string_eq(aslower, "magicpoints"))
						num_item_attr_magicpoints += 1;
					else if(string_eq(aslower, "magicpointspercent"))
						num_item_attr_magicpointspercent += 1;
					else if(string_eq(aslower, "absorbpercentall"))
						num_item_attr_absorbpercentall += 1;
					else if(string_eq(aslower, "absorbpercentallelements"))
						num_item_attr_absorbpercentallelements += 1;
					else if(string_eq(aslower, "absorbpercentenergy"))
						num_item_attr_absorbpercentenergy += 1;
					else if(string_eq(aslower, "absorbpercentfire"))
						num_item_attr_absorbpercentfire += 1;
					else if(string_eq(aslower, "absorbpercentpoison"))
						num_item_attr_absorbpercentpoison += 1;
					else if(string_eq(aslower, "absorbpercentearth"))
						num_item_attr_absorbpercentearth += 1;
					else if(string_eq(aslower, "absorbpercentice"))
						num_item_attr_absorbpercentice += 1;
					else if(string_eq(aslower, "absorbpercentholy"))
						num_item_attr_absorbpercentholy += 1;
					else if(string_eq(aslower, "absorbpercentdeath"))
						num_item_attr_absorbpercentdeath += 1;
					else if(string_eq(aslower, "absorbpercentlifedrain"))
						num_item_attr_absorbpercentlifedrain += 1;
					else if(string_eq(aslower, "absorbpercentmanadrain"))
						num_item_attr_absorbpercentmanadrain += 1;
					else if(string_eq(aslower, "absorbpercentdrown"))
						num_item_attr_absorbpercentdrown += 1;
					else if(string_eq(aslower, "absorbpercentphysical"))
						num_item_attr_absorbpercentphysical += 1;
					else if(string_eq(aslower, "suppressdrunk"))
						num_item_attr_suppressdrunk += 1;
					else if(string_eq(aslower, "suppressenergy"))
						num_item_attr_suppressenergy += 1;
					else if(string_eq(aslower, "suppressfire"))
						num_item_attr_suppressfire += 1;
					else if(string_eq(aslower, "suppresspoison"))
						num_item_attr_suppresspoison += 1;
					else if(string_eq(aslower, "suppressdrown"))
						num_item_attr_suppressdrown += 1;
					else if(string_eq(aslower, "suppressfreeze"))
						num_item_attr_suppressfreeze += 1;
					else if(string_eq(aslower, "suppressdazzle"))
						num_item_attr_suppressdazzle += 1;
					else if(string_eq(aslower, "suppresscurse"))
						num_item_attr_suppresscurse += 1;
					else if(string_eq(aslower, "preventitemloss"))
						num_item_attr_preventitemloss += 1;
					else if(string_eq(aslower, "preventskillloss"))
						num_item_attr_preventskillloss += 1;
					else if(string_eq(aslower, "combattype"))
						num_item_attr_combattype += 1;
					else if(string_eq(aslower, "replaceable"))
						num_item_attr_replaceable += 1;
					else if(string_eq(aslower, "partnerdirection"))
						num_item_attr_partnerdirection += 1;
					else if(string_eq(aslower, "malesleeper"))
						num_item_attr_malesleeper += 1;
					else if(string_eq(aslower, "femalesleeper"))
						num_item_attr_femalesleeper += 1;
					else if(string_eq(aslower, "nosleeper"))
						num_item_attr_nosleeper += 1;
					else if(string_eq(aslower, "elementice"))
						num_item_attr_elementice += 1;
					else if(string_eq(aslower, "elementearth"))
						num_item_attr_elementearth += 1;
					else if(string_eq(aslower, "elementfire"))
						num_item_attr_elementfire += 1;
					else if(string_eq(aslower, "elementenergy"))
						num_item_attr_elementenergy += 1;
					else if(string_eq(aslower, "currency"))
						num_item_attr_currency += 1;
					else
						num_item_attr_other += 1;
				}

				if(!attr_attr.self_closed){
					printf("unexpected not self_closed <attribute> node\n");
					return -1;
				}
			}

			if(!xml_close_node(xml, &item_tag))
				break;
		}
	}
	xml_close_node(xml, &items_tag);

	if(xml_error(xml)){
		printf("%s\n", xml_error_string(xml));
		return -1;
	}

	// item node attr stats
	printf("item node attr stats:\n");
	printf("\tnum_node_attr_id = %d\n", num_node_attr_id);
	printf("\tnum_node_attr_name = %d\n", num_node_attr_name);
	printf("\tnum_node_attr_article = %d\n", num_node_attr_article);
	printf("\tnum_node_attr_plural = %d\n", num_node_attr_plural);
	printf("\tnum_node_attr_editorsuffix = %d\n", num_node_attr_editorsuffix);
	printf("\tnum_node_attr_other = %d\n", num_node_attr_other);

	// item attr stats
	printf("item attr stats:\n");
	printf("\tnum_item_attr_type = %d\n", num_item_attr_type);
	printf("\tnum_item_attr_name = %d\n", num_item_attr_name);
	printf("\tnum_item_attr_article = %d\n", num_item_attr_article);
	printf("\tnum_item_attr_plural = %d\n", num_item_attr_plural);
	printf("\tnum_item_attr_description = %d\n", num_item_attr_description);
	printf("\tnum_item_attr_runespellname = %d\n", num_item_attr_runespellname);
	printf("\tnum_item_attr_weight = %d\n", num_item_attr_weight);
	printf("\tnum_item_attr_showcount = %d\n", num_item_attr_showcount);
	printf("\tnum_item_attr_armor = %d\n", num_item_attr_armor);
	printf("\tnum_item_attr_defense = %d\n", num_item_attr_defense);
	printf("\tnum_item_attr_extradef = %d\n", num_item_attr_extradef);
	printf("\tnum_item_attr_attack = %d\n", num_item_attr_attack);
	printf("\tnum_item_attr_rotateto = %d\n", num_item_attr_rotateto);
	printf("\tnum_item_attr_moveable = %d\n", num_item_attr_moveable);
	printf("\tnum_item_attr_blockprojectile = %d\n", num_item_attr_blockprojectile);
	printf("\tnum_item_attr_pickupable = %d\n", num_item_attr_pickupable);
	printf("\tnum_item_attr_allowpickupable = %d\n", num_item_attr_allowpickupable);
	printf("\tnum_item_attr_floorchange = %d\n", num_item_attr_floorchange);
	printf("\tnum_item_attr_corpsetype = %d\n", num_item_attr_corpsetype);
	printf("\tnum_item_attr_containersize = %d\n", num_item_attr_containersize);
	printf("\tnum_item_attr_fluidsource = %d\n", num_item_attr_fluidsource);
	printf("\tnum_item_attr_readable = %d\n", num_item_attr_readable);
	printf("\tnum_item_attr_writeable = %d\n", num_item_attr_writeable);
	printf("\tnum_item_attr_maxtextlen = %d\n", num_item_attr_maxtextlen);
	printf("\tnum_item_attr_writeonceitemid = %d\n", num_item_attr_writeonceitemid);
	printf("\tnum_item_attr_weapontype = %d\n", num_item_attr_weapontype);
	printf("\tnum_item_attr_slottype = %d\n", num_item_attr_slottype);
	printf("\tnum_item_attr_ammotype = %d\n", num_item_attr_ammotype);
	printf("\tnum_item_attr_shoottype = %d\n", num_item_attr_shoottype);
	printf("\tnum_item_attr_effect = %d\n", num_item_attr_effect);
	printf("\tnum_item_attr_range = %d\n", num_item_attr_range);
	printf("\tnum_item_attr_stopduration = %d\n", num_item_attr_stopduration);
	printf("\tnum_item_attr_decayto = %d\n", num_item_attr_decayto);
	printf("\tnum_item_attr_transformequipto = %d\n", num_item_attr_transformequipto);
	printf("\tnum_item_attr_transformdeequipto = %d\n", num_item_attr_transformdeequipto);
	printf("\tnum_item_attr_duration = %d\n", num_item_attr_duration);
	printf("\tnum_item_attr_showduration = %d\n", num_item_attr_showduration);
	printf("\tnum_item_attr_charges = %d\n", num_item_attr_charges);
	printf("\tnum_item_attr_showcharges = %d\n", num_item_attr_showcharges);
	printf("\tnum_item_attr_breakchance = %d\n", num_item_attr_breakchance);
	printf("\tnum_item_attr_ammoaction = %d\n", num_item_attr_ammoaction);
	printf("\tnum_item_attr_hitchance = %d\n", num_item_attr_hitchance);
	printf("\tnum_item_attr_maxhitchance = %d\n", num_item_attr_maxhitchance);
	printf("\tnum_item_attr_invisible = %d\n", num_item_attr_invisible);
	printf("\tnum_item_attr_speed = %d\n", num_item_attr_speed);
	printf("\tnum_item_attr_healthgain = %d\n", num_item_attr_healthgain);
	printf("\tnum_item_attr_healthticks = %d\n", num_item_attr_healthticks);
	printf("\tnum_item_attr_managain = %d\n", num_item_attr_managain);
	printf("\tnum_item_attr_manaticks = %d\n", num_item_attr_manaticks);
	printf("\tnum_item_attr_manashield = %d\n", num_item_attr_manashield);
	printf("\tnum_item_attr_skillsword = %d\n", num_item_attr_skillsword);
	printf("\tnum_item_attr_skillaxe = %d\n", num_item_attr_skillaxe);
	printf("\tnum_item_attr_skillclub = %d\n", num_item_attr_skillclub);
	printf("\tnum_item_attr_skilldist = %d\n", num_item_attr_skilldist);
	printf("\tnum_item_attr_skillfish = %d\n", num_item_attr_skillfish);
	printf("\tnum_item_attr_skillshield = %d\n", num_item_attr_skillshield);
	printf("\tnum_item_attr_skillfist = %d\n", num_item_attr_skillfist);
	printf("\tnum_item_attr_maxhitpoints = %d\n", num_item_attr_maxhitpoints);
	printf("\tnum_item_attr_maxhitpointspercent = %d\n", num_item_attr_maxhitpointspercent);
	printf("\tnum_item_attr_maxmanapoints = %d\n", num_item_attr_maxmanapoints);
	printf("\tnum_item_attr_maxmanapointspercent = %d\n", num_item_attr_maxmanapointspercent);
	printf("\tnum_item_attr_soulpoints = %d\n", num_item_attr_soulpoints);
	printf("\tnum_item_attr_soulpointspercent = %d\n", num_item_attr_soulpointspercent);
	printf("\tnum_item_attr_magicpoints = %d\n", num_item_attr_magicpoints);
	printf("\tnum_item_attr_magicpointspercent = %d\n", num_item_attr_magicpointspercent);
	printf("\tnum_item_attr_absorbpercentall = %d\n", num_item_attr_absorbpercentall);
	printf("\tnum_item_attr_absorbpercentallelements = %d\n", num_item_attr_absorbpercentallelements);
	printf("\tnum_item_attr_absorbpercentenergy = %d\n", num_item_attr_absorbpercentenergy);
	printf("\tnum_item_attr_absorbpercentfire = %d\n", num_item_attr_absorbpercentfire);
	printf("\tnum_item_attr_absorbpercentpoison = %d\n", num_item_attr_absorbpercentpoison);
	printf("\tnum_item_attr_absorbpercentearth = %d\n", num_item_attr_absorbpercentearth);
	printf("\tnum_item_attr_absorbpercentice = %d\n", num_item_attr_absorbpercentice);
	printf("\tnum_item_attr_absorbpercentholy = %d\n", num_item_attr_absorbpercentholy);
	printf("\tnum_item_attr_absorbpercentdeath = %d\n", num_item_attr_absorbpercentdeath);
	printf("\tnum_item_attr_absorbpercentlifedrain = %d\n", num_item_attr_absorbpercentlifedrain);
	printf("\tnum_item_attr_absorbpercentmanadrain = %d\n", num_item_attr_absorbpercentmanadrain);
	printf("\tnum_item_attr_absorbpercentdrown = %d\n", num_item_attr_absorbpercentdrown);
	printf("\tnum_item_attr_absorbpercentphysical = %d\n", num_item_attr_absorbpercentphysical);
	printf("\tnum_item_attr_suppressdrunk = %d\n", num_item_attr_suppressdrunk);
	printf("\tnum_item_attr_suppressenergy = %d\n", num_item_attr_suppressenergy);
	printf("\tnum_item_attr_suppressfire = %d\n", num_item_attr_suppressfire);
	printf("\tnum_item_attr_suppresspoison = %d\n", num_item_attr_suppresspoison);
	printf("\tnum_item_attr_suppressdrown = %d\n", num_item_attr_suppressdrown);
	printf("\tnum_item_attr_suppressfreeze = %d\n", num_item_attr_suppressfreeze);
	printf("\tnum_item_attr_suppressdazzle = %d\n", num_item_attr_suppressdazzle);
	printf("\tnum_item_attr_suppresscurse = %d\n", num_item_attr_suppresscurse);
	printf("\tnum_item_attr_preventitemloss = %d\n", num_item_attr_preventitemloss);
	printf("\tnum_item_attr_preventskillloss = %d\n", num_item_attr_preventskillloss);
	printf("\tnum_item_attr_combattype = %d\n", num_item_attr_combattype);
	printf("\tnum_item_attr_replaceable = %d\n", num_item_attr_replaceable);
	printf("\tnum_item_attr_partnerdirection = %d\n", num_item_attr_partnerdirection);
	printf("\tnum_item_attr_malesleeper = %d\n", num_item_attr_malesleeper);
	printf("\tnum_item_attr_femalesleeper = %d\n", num_item_attr_femalesleeper);
	printf("\tnum_item_attr_nosleeper = %d\n", num_item_attr_nosleeper);
	printf("\tnum_item_attr_elementice = %d\n", num_item_attr_elementice);
	printf("\tnum_item_attr_elementearth = %d\n", num_item_attr_elementearth);
	printf("\tnum_item_attr_elementfire = %d\n", num_item_attr_elementfire);
	printf("\tnum_item_attr_elementenergy = %d\n", num_item_attr_elementenergy);
	printf("\tnum_item_attr_currency = %d\n", num_item_attr_currency);
	printf("\tnum_item_attr_other = %d\n", num_item_attr_other);

	return 0;
}
