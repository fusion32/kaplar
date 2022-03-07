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

	XML_NodeTag item_tag;
	XML_NodeAttributes item_attr;
	while(xml_read_node(xml, &item_tag, &item_attr)){
		if(!string_eq(item_tag.text, "item")){
			printf("unexpected <items> child <%s>\n", item_tag.text);
			return -1;
		}

		// parse item_attr
		printf("    <item ");
		for(i32 i = 0; i < item_attr.num_attributes; i += 1){
			printf("%s=\"%s\" ",
				item_attr.attributes[i].key,
				item_attr.attributes[i].value);
		}
		if(item_attr.self_closed)
			printf("/>\n");
		else
			printf(">\n");

		if(!item_attr.self_closed){
			XML_NodeTag attr_tag;
			XML_NodeAttributes attr_attr;
			while(xml_read_node(xml, &attr_tag, &attr_attr)){
				if(!string_eq(attr_tag.text, "attribute")){
					printf("unexpected <item> child <%s>\n", attr_tag.text);
					return -1;
				}

				// parse attr_attr
				printf("        <attribute ");
				for(i32 i = 0; i < attr_attr.num_attributes; i += 1){
					printf("%s=\"%s\" ",
						attr_attr.attributes[i].key,
						attr_attr.attributes[i].value);
				}
				printf("/>\n");

				if(!attr_attr.self_closed){
					printf("unexpected not self_closed <attribute> node\n");
					return -1;
				}
			}

			printf("    </item>\n");
			if(!xml_close_node(xml, &item_tag))
				break;
		}
	}
	xml_close_node(xml, &items_tag);

	if(xml_error(xml)){
		printf("%s\n", xml_error_string(xml));
		return -1;
	}

	return 0;
}
