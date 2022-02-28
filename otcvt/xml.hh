#ifndef KAPLAR_XML_HH_
#define KAPLAR_XML_HH_ 1

#include "common.hh"

struct XML_State;
XML_State *xml_init_from_file(const char *filename);
void xml_free(XML_State *state);

struct XML_NodeTag{
	bool parsing_error;
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

#endif //KAPLAR_XML_HH_
