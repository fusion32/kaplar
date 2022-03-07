#ifndef KAPLAR_XML_HH_
#define KAPLAR_XML_HH_ 1

#include "common.hh"

struct XML_NodeTag{
	char text[32];
};

struct XML_NodeAttribute{
	char key[32];
	char value[128];
};

struct XML_NodeAttributes{
	bool self_closed;
	i32 num_attributes;
	XML_NodeAttribute attributes[10];
};

struct XML_State;
XML_State *xml_init_from_file(const char *filename);
void xml_free(XML_State *state);

bool xml_error(XML_State *xml);
char *xml_error_string(XML_State *xml);
bool xml_read_node(XML_State *xml, XML_NodeTag *outt, XML_NodeAttributes *outn);
bool xml_close_node(XML_State *xml, XML_NodeTag *tag);

#endif //KAPLAR_XML_HH_
