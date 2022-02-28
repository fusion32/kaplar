// NOTE: This is the simplest XML parser for the purpose of
// parsing old OpenTibia XML files.

#include "xml.hh"
#include <stdio.h>
#include <string.h>

enum : int {
	// single-character tokens use their ASCII code values
	// '/', '<', '=', '>'

	TOKEN_EOF = 128,
	TOKEN_IDENT,
	TOKEN_STRING,
	TOKEN_INVALID,	// special token to signal an error on the lexer
};

struct XML_Token{
	int token;
	char text[256];
};

struct XML_State{
	// TODO: If we want to use XML for anything we would need
	// proper diagnostics like error messages with filename
	// and line.

	u8 *buf;
	i32 bufpos;
	i32 bufend;

	u32 tokbuf_pos;
	XML_Token tokbuf[8];
};

static void xml_next_token(XML_State *xml, XML_Token *tok);

XML_State *xml_init_from_file(const char *filename){
	i32 buflen;
	u8 *buf = (u8*)read_entire_file(filename, &buflen);
	if(!buf)
		return NULL;

	// NOTE: Some XML files have this UTF-8 BOM. Skip it.
	i32 start_pos = 0;
	if(buflen >= 3 && buf[0] == 0xEF
	&& buf[1] == 0xBB && buf[2] == 0xBF){
		start_pos = 3;
	}

	XML_State *xml = (XML_State*)malloc_no_fail(sizeof(XML_State));
	xml->buf = buf;
	xml->bufpos = start_pos;
	xml->bufend = buflen;
	xml->tokbuf_pos = 0;
	for(i32 i = 0; i < NARRAY(xml->tokbuf); i += 1)
		xml_next_token(xml, &xml->tokbuf[i]);
	return xml;
}

void xml_free(XML_State *xml){
	free(xml->buf);
	free(xml);
}

static
bool xml_can_read(XML_State *xml, i32 bytes){
	return (xml->bufend - xml->bufpos) >= bytes;
}

static
bool xml_eof(XML_State *xml){
	return !xml_can_read(xml, 1);
}

static
u8 xml_read_one(XML_State *xml){
	u8 result = 0;
	if(xml_can_read(xml, 1))
		result = xml->buf[xml->bufpos];
	xml->bufpos += 1;
	return result;
}

static
u8 xml_peek_one(XML_State *xml){
	u8 result = 0;
	if(xml_can_read(xml, 1))
		result = xml->buf[xml->bufpos];
	return result;
}

static
void xml_skip_one(XML_State *xml){
	xml->bufpos += 1;
}

static
bool xml_match_skip(XML_State *xml, i32 n, ...){
	if(!xml_can_read(xml, n))
		return false;

	va_list ap;
	va_start(ap, n);
	for(i32 i = 0; i < n; i += 1){
		u8 ch = va_arg(ap, u8);
		if(ch != xml->buf[xml->bufpos + i]){
			va_end(ap);
			return false;
		}
	}
	va_end(ap);
	xml->bufpos += n;
	return true;
}

static
u8 xml_peek(XML_State *xml, i32 offset){
	u8 result = 0;
	if(xml_can_read(xml, offset + 1))
		result = xml->buf[xml->bufpos + offset];
	return result;
}

static
bool ch_is_alpha(int ch){
	return (ch >= 65 && ch <= 90) // A-Z characters
		|| (ch >= 97 && ch <= 122); // a-z characters
}

static
bool xml_lex_ident(XML_State *xml, XML_Token *tok, u8 ch){
	if(!ch_is_alpha(ch))
		return false; // error: unexpected character

	i32 insert_pos = 0;
	tok->text[insert_pos++] = ch;
	while(1){
		ch = xml_peek_one(xml);
		if(xml_eof(xml) || !ch_is_alpha(ch))
			break;
		if(insert_pos >= (sizeof(tok->text) - 1))
			return false; // impl error: identifier length is too large
		tok->text[insert_pos++] = ch;
		xml_skip_one(xml);
	}

	tok->text[insert_pos] = 0;
	tok->token = TOKEN_IDENT;
	return true;
}

static
bool xml_lex_string(XML_State *xml, XML_Token *tok){
	u8 ch = xml_read_one(xml);
	i32 insert_pos = 0;
	while(!xml_eof(xml) && ch != '"'){
		if(ch <= 0x1F)
			return false; // error: unexpected control character inside string

		// NOTE: We don't need to scan for escape sequences because it seems
		// that XML's escape sequences are in the form of &code; where code
		// is the name of the symbol which means it doesn't contain the symbol
		// itself. For example a double quote escape sequence would be &quot;
		// while a single quote sequence would be &apos; and so on.
		if(insert_pos >= (sizeof(tok->text) - 1))
			return false; // impl error: string length is too large
		tok->text[insert_pos++] = ch;
		ch = xml_read_one(xml);
	}
	if(xml_eof(xml))
		return false; // error: unexpected end of file
	ASSERT(ch == '"');
	tok->text[insert_pos] = 0;
	tok->token = TOKEN_STRING;
	return true;
}

static
bool xml_lex_comment(XML_State *xml){
	while(!xml_eof(xml) && !xml_match_skip(xml, 3, '-', '-', '>'))
		xml_read_one(xml);
	return !xml_eof(xml); // error: unexpected end of file
}

static
bool xml_lex_prolog(XML_State *xml){
	while(!xml_eof(xml) && !xml_match_skip(xml, 2, '?', '>'))
		xml_read_one(xml);
	return !xml_eof(xml); // error: unexpected end of file
}

static
void xml_next_token(XML_State *xml, XML_Token *tok){
	while(1){
		if(xml_eof(xml)){
			tok->token = TOKEN_EOF;
			return;
		}

		u8 ch = xml_read_one(xml);
		switch(ch){
			// skip whitespace
			case '\n':
			case '\r':
			case '\t':
			case ' ':
				continue;

			// string
			case '"':
				if(!xml_lex_string(xml, tok))
					tok->token = TOKEN_INVALID;
				return;

			// ascii tokens
			case '<':
				if(xml_match_skip(xml, 3, '!', '-', '-')){
					if(!xml_lex_comment(xml)){
						tok->token = TOKEN_INVALID;
						return;
					}
					continue;
				}else if(xml_match_skip(xml, 1, '?')){
					if(!xml_lex_prolog(xml)){
						tok->token = TOKEN_INVALID;
						return;
					}
					continue;
				}
				FALLTHROUGH;

			case '/':
			case '=':
			case '>':
				tok->token = ch;
				return;

			default:
				// NOTE: If no other token was matched, the only token
				// left is an identifier token. If it can't be matched,
				// it's a lexing error.
				if(!xml_lex_ident(xml, tok, ch))
					tok->token = TOKEN_INVALID;
				return;
		}
	}
}

static
bool xml_tokbuf_match(XML_State *xml, u32 n, ...){
	ASSERT(IS_POWER_OF_TWO(NARRAY(xml->tokbuf)));
	ASSERT(NARRAY(xml->tokbuf) >= n);

	u32 mask = NARRAY(xml->tokbuf) - 1;
	va_list ap;
	va_start(ap, n);
	for(u32 i = 0; i < n; i += 1){
		int token = va_arg(ap, int);
		u32 pos = (xml->tokbuf_pos + i) & mask;
		if(xml->tokbuf[pos].token != token){
			va_end(ap);
			return false;
		}
	}
	va_end(ap);
	return true;
}

static
void xml_tokbuf_consume(XML_State *xml, u32 n, ...){
	ASSERT(IS_POWER_OF_TWO(NARRAY(xml->tokbuf)));
	ASSERT(NARRAY(xml->tokbuf) >= n);

	u32 mask = NARRAY(xml->tokbuf) - 1;
	va_list ap;
	va_start(ap, n);
	for(u32 i = 0; i < n; i += 1){
		XML_Token *tok = va_arg(ap, XML_Token*);
		u32 pos = (xml->tokbuf_pos + i) & mask;
		if(tok) *tok = xml->tokbuf[pos];
		xml_next_token(xml, &xml->tokbuf[pos]);
	}
	xml->tokbuf_pos += n;
	va_end(ap);
}

static
i32 xml_unescape_string(char *s){
	i32 invalid = 0;
	char *insert_ptr = s;
	char *scan_ptr = s;
	while(scan_ptr[0]){
		if(scan_ptr[0] == '&'){
			if(strncmp(scan_ptr, "&quot;", 6) == 0){
				insert_ptr[0] = '"';
				scan_ptr += 6;
			}else if(strncmp(scan_ptr, "&amp;", 5) == 0){
				insert_ptr[0] = '&';
				scan_ptr += 5;
			}else if(strncmp(scan_ptr, "&apos;", 6) == 0){
				insert_ptr[0] = '\'';
				scan_ptr += 6;
			}else if(strncmp(scan_ptr, "&lt;", 4) == 0){
				insert_ptr[0] = '<';
				scan_ptr += 4;
			}else if(strncmp(scan_ptr, "&gt;", 4) == 0){
				insert_ptr[0] = '>';
				scan_ptr += 4;
			}else{
				insert_ptr[0] = scan_ptr[0];
				invalid += 1;
			}
		}else{
			insert_ptr[0] = scan_ptr[0];
			scan_ptr += 1;
		}
		insert_ptr += 1;
	}
	insert_ptr[0] = 0;
	return invalid;
}

// ----------------------------------------------------------------
// Parsing
// ----------------------------------------------------------------
static
void string_copy(char *dst, i32 dstlen, const char *src){
	i32 i = 0;
	while(src[i] && i < (dstlen - 1)){
		dst[i] = src[i];
		i += 1;
	}
	dst[i] = 0;
}

static
bool string_eq(const char *s1, const char *s2){
	i32 i = 0;
	while(s1[i] != 0 && s1[i] == s2[i])
		i += 1;
	return s1[i] == s2[i];
}

bool xml_read_node(XML_State *xml, XML_NodeTag *outt, XML_NodeAttributes *outn){
	XML_Token tag, key, value;

	outt->parsing_error = false;
	if(!xml_tokbuf_match(xml, 2, '<', TOKEN_IDENT))
		return false;
	xml_tokbuf_consume(xml, 2, NULL, &tag);
	string_copy(outt->text, sizeof(outt->text), tag.text);

	i32 num_attributes = 0;
	while(xml_tokbuf_match(xml, 3, TOKEN_IDENT, '=', TOKEN_STRING)){
		xml_tokbuf_consume(xml, 3, &key, NULL, &value);
		if(num_attributes < NARRAY(outn->attributes)){
			XML_NodeAttribute *attr = &outn->attributes[num_attributes];
			string_copy(attr->key, sizeof(attr->key), key.text);
			string_copy(attr->value, sizeof(attr->value), value.text);
		}
		num_attributes += 1;
	}
	outn->num_attributes = num_attributes;

	if(xml_tokbuf_match(xml, 1, '>')){
		xml_tokbuf_consume(xml, 1, NULL);
		outn->self_closed = false;
	}else if(xml_tokbuf_match(xml, 2, '/', '>')){
		xml_tokbuf_consume(xml, 2, NULL, NULL);
		outn->self_closed = true;
	}else{
		outt->parsing_error = true;
		return false;
	}
	return true;
}

bool xml_close_node(XML_State *xml, XML_NodeTag *tag){
	if(!xml_tokbuf_match(xml, 4, '<', '/', TOKEN_IDENT, '>'))
		return false;

	XML_Token tok;
	xml_tokbuf_consume(xml, 4, NULL, NULL, &tok, NULL);
	if(!string_eq(tag->text, tok.text)){
		tag->parsing_error = true;
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

#if 0
	bool eof = false;
	while(!eof){
		XML_Token tok;
		xml_tokbuf_consume(xml, 1, &tok);
		switch(tok.token){
			case TOKEN_EOF:
				printf("TOKEN_EOF\n");
				eof = true;
				break;
			case TOKEN_IDENT:
				printf("TOKEN_IDENT: \"%s\"\n", tok.text);
				break;
			case TOKEN_STRING:
				printf("TOKEN_STRING: \"%s\"\n", tok.text);
				break;
			default:
				printf("TOKEN '%c'\n", tok.token);
				break;
		}
	}

	xml_free(xml);
	return 0;

#else

	XML_NodeTag items_tag;
	XML_NodeAttributes items_attr;
	if(!xml_read_node(xml, &items_tag, &items_attr)){
		if(items_tag.parsing_error)
			printf("failed to parse root node\n");
		else
			printf("root node not found\n");
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
			if(!xml_close_node(xml, &item_tag) || item_tag.parsing_error){
				printf("failed to close <item> node\n");
				return -1;
			}
		}
	}
	if(item_tag.parsing_error){
		printf("<item> parsing error\n");
		return -1;
	}

	if(!xml_close_node(xml, &items_tag) || items_tag.parsing_error){
		printf("failed to close <items> node\n");
		return -1;
	}

	return 0;
#endif
}
