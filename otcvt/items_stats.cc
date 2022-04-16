// NOTE: All item attributes taken from "opentibia/src/items.cpp":
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

bool recursive_close_node(XML_State *xml, XML_NodeTag *tag, XML_NodeAttributes *attr){
	if(!attr->self_closed){
		XML_NodeTag child_tag;
		XML_NodeAttributes child_attr;
		while(xml_read_node(xml, &child_tag, &child_attr)){
			if(!recursive_close_node(xml, &child_tag, &child_attr))
				return false;
		}
		if(!xml_close_node(xml, tag))
			return false;
	}
	return true;
}

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

	// overall stats
	i32 num_items = 0;
	i32 num_items_w_attr = 0;

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

		bool has_attr = false;
		if(!item_attr.self_closed){
			XML_NodeTag attr_tag;
			XML_NodeAttributes attr_attr;
			while(xml_read_node(xml, &attr_tag, &attr_attr)){
				if(!string_eq(attr_tag.text, "attribute")){
					printf("unexpected <item> child <%s>\n", attr_tag.text);
					return -1;
				}

				has_attr = true;
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

				// NOTE: I tried to run item.xml from TFS and they have some
				// attribute nodes that have children nodes. So this will skip
				// all children nodes and find the closing tag.
				if(!recursive_close_node(xml, &attr_tag, &attr_attr)){
					printf("unable to close attribute node\n");
					return -1;
				}
			}

			if(!xml_close_node(xml, &item_tag))
				break;
		}

		num_items += 1;
		if(has_attr)
			num_items_w_attr += 1;
	}
	xml_close_node(xml, &items_tag);

	if(xml_error(xml)){
		printf("%s\n", xml_error_string(xml));
		return -1;
	}

	#define PRINT_STAT(x) \
		printf("\t%-40s\t%d\t%g%%\n", #x, (x), (x * 100.0) / num_items)

	// overall stats
	printf("overall stats:\n");
	PRINT_STAT(num_items);
	PRINT_STAT(num_items_w_attr);

	// item node attr stats
	printf("item node attr stats:\n");
	PRINT_STAT(num_node_attr_id);
	PRINT_STAT(num_node_attr_name);
	PRINT_STAT(num_node_attr_article);
	PRINT_STAT(num_node_attr_plural);
	PRINT_STAT(num_node_attr_editorsuffix);
	PRINT_STAT(num_node_attr_other);

	// item attr stats
	printf("item attr stats:\n");
	PRINT_STAT(num_item_attr_type);
	PRINT_STAT(num_item_attr_name);
	PRINT_STAT(num_item_attr_article);
	PRINT_STAT(num_item_attr_plural);
	PRINT_STAT(num_item_attr_description);
	PRINT_STAT(num_item_attr_runespellname);
	PRINT_STAT(num_item_attr_weight);
	PRINT_STAT(num_item_attr_showcount);
	PRINT_STAT(num_item_attr_armor);
	PRINT_STAT(num_item_attr_defense);
	PRINT_STAT(num_item_attr_extradef);
	PRINT_STAT(num_item_attr_attack);
	PRINT_STAT(num_item_attr_rotateto);
	PRINT_STAT(num_item_attr_moveable);
	PRINT_STAT(num_item_attr_blockprojectile);
	PRINT_STAT(num_item_attr_pickupable);
	PRINT_STAT(num_item_attr_allowpickupable);
	PRINT_STAT(num_item_attr_floorchange);
	PRINT_STAT(num_item_attr_corpsetype);
	PRINT_STAT(num_item_attr_containersize);
	PRINT_STAT(num_item_attr_fluidsource);
	PRINT_STAT(num_item_attr_readable);
	PRINT_STAT(num_item_attr_writeable);
	PRINT_STAT(num_item_attr_maxtextlen);
	PRINT_STAT(num_item_attr_writeonceitemid);
	PRINT_STAT(num_item_attr_weapontype);
	PRINT_STAT(num_item_attr_slottype);
	PRINT_STAT(num_item_attr_ammotype);
	PRINT_STAT(num_item_attr_shoottype);
	PRINT_STAT(num_item_attr_effect);
	PRINT_STAT(num_item_attr_range);
	PRINT_STAT(num_item_attr_stopduration);
	PRINT_STAT(num_item_attr_decayto);
	PRINT_STAT(num_item_attr_transformequipto);
	PRINT_STAT(num_item_attr_transformdeequipto);
	PRINT_STAT(num_item_attr_duration);
	PRINT_STAT(num_item_attr_showduration);
	PRINT_STAT(num_item_attr_charges);
	PRINT_STAT(num_item_attr_showcharges);
	PRINT_STAT(num_item_attr_breakchance);
	PRINT_STAT(num_item_attr_ammoaction);
	PRINT_STAT(num_item_attr_hitchance);
	PRINT_STAT(num_item_attr_maxhitchance);
	PRINT_STAT(num_item_attr_invisible);
	PRINT_STAT(num_item_attr_speed);
	PRINT_STAT(num_item_attr_healthgain);
	PRINT_STAT(num_item_attr_healthticks);
	PRINT_STAT(num_item_attr_managain);
	PRINT_STAT(num_item_attr_manaticks);
	PRINT_STAT(num_item_attr_manashield);
	PRINT_STAT(num_item_attr_skillsword);
	PRINT_STAT(num_item_attr_skillaxe);
	PRINT_STAT(num_item_attr_skillclub);
	PRINT_STAT(num_item_attr_skilldist);
	PRINT_STAT(num_item_attr_skillfish);
	PRINT_STAT(num_item_attr_skillshield);
	PRINT_STAT(num_item_attr_skillfist);
	PRINT_STAT(num_item_attr_maxhitpoints);
	PRINT_STAT(num_item_attr_maxhitpointspercent);
	PRINT_STAT(num_item_attr_maxmanapoints);
	PRINT_STAT(num_item_attr_maxmanapointspercent);
	PRINT_STAT(num_item_attr_soulpoints);
	PRINT_STAT(num_item_attr_soulpointspercent);
	PRINT_STAT(num_item_attr_magicpoints);
	PRINT_STAT(num_item_attr_magicpointspercent);
	PRINT_STAT(num_item_attr_absorbpercentall);
	PRINT_STAT(num_item_attr_absorbpercentallelements);
	PRINT_STAT(num_item_attr_absorbpercentenergy);
	PRINT_STAT(num_item_attr_absorbpercentfire);
	PRINT_STAT(num_item_attr_absorbpercentpoison);
	PRINT_STAT(num_item_attr_absorbpercentearth);
	PRINT_STAT(num_item_attr_absorbpercentice);
	PRINT_STAT(num_item_attr_absorbpercentholy);
	PRINT_STAT(num_item_attr_absorbpercentdeath);
	PRINT_STAT(num_item_attr_absorbpercentlifedrain);
	PRINT_STAT(num_item_attr_absorbpercentmanadrain);
	PRINT_STAT(num_item_attr_absorbpercentdrown);
	PRINT_STAT(num_item_attr_absorbpercentphysical);
	PRINT_STAT(num_item_attr_suppressdrunk);
	PRINT_STAT(num_item_attr_suppressenergy);
	PRINT_STAT(num_item_attr_suppressfire);
	PRINT_STAT(num_item_attr_suppresspoison);
	PRINT_STAT(num_item_attr_suppressdrown);
	PRINT_STAT(num_item_attr_suppressfreeze);
	PRINT_STAT(num_item_attr_suppressdazzle);
	PRINT_STAT(num_item_attr_suppresscurse);
	PRINT_STAT(num_item_attr_preventitemloss);
	PRINT_STAT(num_item_attr_preventskillloss);
	PRINT_STAT(num_item_attr_combattype);
	PRINT_STAT(num_item_attr_replaceable);
	PRINT_STAT(num_item_attr_partnerdirection);
	PRINT_STAT(num_item_attr_malesleeper);
	PRINT_STAT(num_item_attr_femalesleeper);
	PRINT_STAT(num_item_attr_nosleeper);
	PRINT_STAT(num_item_attr_elementice);
	PRINT_STAT(num_item_attr_elementearth);
	PRINT_STAT(num_item_attr_elementfire);
	PRINT_STAT(num_item_attr_elementenergy);
	PRINT_STAT(num_item_attr_currency);
	PRINT_STAT(num_item_attr_other);
	return 0;
}
