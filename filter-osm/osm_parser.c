/*
 * Copyright (c) 2016 Jeff Boody
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>
#include "osm_parser.h"

#define LOG_TAG "osm"
#include "osm_log.h"

/***********************************************************
* private                                                  *
***********************************************************/

static void osm_parser_print(osm_parser_t* self,
                             const char* fmt, ...)
{
	assert(self);
	assert(fmt);

	char buf[4096];
	va_list argptr;
	va_start(argptr, fmt);
	vsnprintf(buf, 4096, fmt, argptr);
	va_end(argptr);
	fprintf(self->out, "%s", buf);
}

static void osm_parser_printdb(osm_parser_t* self,
                               const char* fmt, ...)
{
	assert(self);
	assert(fmt);

	char buf[4096];
	va_list argptr;
	va_start(argptr, fmt);
	vsnprintf(buf, 4096, fmt, argptr);
	va_end(argptr);
	fprintf(self->db, "%s", buf);
}

static void osm_parser_parseStart(void* _self,
                                  const XML_Char* name,
                                  const XML_Char** atts)
{
	assert(_self);
	assert(name);
	assert(atts);

	osm_parser_t* self = (osm_parser_t*) _self;

	// print and skip base element to avoid reading entire
	// osm database to memory
	if(self->indent == 0)
	{
		osm_parser_printElemBegin(self, name, 0);
		osm_parser_printElemAttsSet(self, atts);
		osm_parser_printElemEnd(self, 0);
		self->indent = 1;
		return;
	}

	int line = XML_GetCurrentLineNumber(self->xml);
	osm_element_t* current = osm_element_new(self->current,
	                                         line, name, atts);
	if(current == NULL)
	{
		self->error = 1;
		return;
	}
	self->current = current;
	++self->indent;
}

static void osm_parser_parseEnd(void* _self,
                                const XML_Char* name)
{
	assert(_self);
	assert(name);

	osm_parser_t* self = (osm_parser_t*) _self;
	--self->indent;

	// print and skip base element to avoid reading entire
	// osm database to memory
	// or evaluate the root element(s)
	if(self->indent == 0)
	{
		osm_parser_printElemClose(self, name, 0);
		return;
	}

	osm_element_t* parent = NULL;
	if(self->indent == 1)
	{
		osm_element_eval(self->current, self, self->indent);
		osm_element_delete(&self->current);
	}
	else
	{
		parent = self->current->parent;
	}
	self->current = parent;
}

static void osm_parser_specialChars(const XML_Char* a,
                                    int max,
                                    XML_Char* b)
{
	assert(a);
	assert(b);

	// initialize b
	b[0] = '\0';

	int i   = 0;
	int len = 0;
	while(1)
	{
		// eat invalid characters
		if((a[i] == '\n') ||
		   (a[i] == '\t') ||
		   (a[i] == '\r'))
		{
			++i;
			continue;
		}

		// check for word boundary
		if(a[i] == '\0')
		{
			return;
		}

		if(a[i] == '&')
		{
			if((len + 5) >= max)
			{
				LOGE("invalid %s", a);
				return;
			}

			b[len]     = '&';
			b[len + 1] = 'a';
			b[len + 2] = 'm';
			b[len + 3] = 'p';
			b[len + 4] = ';';
			b[len + 5] = '\0';
			len += 5;
			++i;
		}
		else if(a[i] == '"')
		{
			if((len + 6) >= max)
			{
				LOGE("invalid %s", a);
				return;
			}

			b[len]     = '&';
			b[len + 1] = 'q';
			b[len + 2] = 'u';
			b[len + 3] = 'o';
			b[len + 4] = 't';
			b[len + 5] = ';';
			b[len + 6] = '\0';
			len += 6;
			++i;
		}
		else if(a[i] == '\'')
		{
			if((len + 6) >= max)
			{
				LOGE("invalid %s", a);
				return;
			}

			b[len]     = '&';
			b[len + 1] = 'a';
			b[len + 2] = 'p';
			b[len + 3] = 'o';
			b[len + 4] = 's';
			b[len + 5] = ';';
			b[len + 6] = '\0';
			len += 6;
			++i;
		}
		else if(a[i] == '<')
		{
			if((len + 4) >= max)
			{
				LOGE("invalid %s", a);
				return;
			}

			b[len]     = '&';
			b[len + 1] = 'l';
			b[len + 2] = 't';
			b[len + 3] = ';';
			b[len + 4] = '\0';
			len += 4;
			++i;
		}
		else if(a[i] == '>')
		{
			if((len + 4) >= max)
			{
				LOGE("invalid %s", a);
				return;
			}

			b[len]     = '&';
			b[len + 1] = 'g';
			b[len + 2] = 't';
			b[len + 3] = ';';
			b[len + 4] = '\0';
			len += 4;
			++i;
		}
		else
		{
			if((len + 1) >= max)
			{
				LOGE("invalid %s", a);
				return;
			}

			// append character to word
			b[len]     = a[i];
			b[len + 1] = '\0';
			++len;
			++i;
		}
	}
}

/***********************************************************
* public                                                   *
***********************************************************/

osm_parser_t* osm_parser_new(const char* fname_in,
                             const char* fname_out,
                             const char* fname_db)
{
	assert(fname_in);
	assert(fname_out);
	assert(fname_db);

	osm_parser_t* self = (osm_parser_t*)
	                     malloc(sizeof(osm_parser_t));
	if(self == NULL)
	{
		LOGE("malloc failed");
		return NULL;
	}

	self->in = fopen(fname_in, "r");
	if(self->in == NULL)
	{
		LOGE("invalid %s", fname_in);
		goto fail_in;
	}

	self->out = fopen(fname_out, "w");
	if(self->out == NULL)
	{
		LOGE("invalid %s", fname_out);
		goto fail_out;
	}

	self->db = fopen(fname_db, "w");
	if(self->db == NULL)
	{
		LOGE("invalid %s", fname_db);
		goto fail_db;
	}

	self->xml = XML_ParserCreate("UTF-8");
	if(self->xml == NULL)
	{
		LOGE("XML_ParserCreate failed");
		goto fail_parser_create;
	}
	XML_SetUserData(self->xml, (void*) self);
	XML_SetElementHandler(self->xml,
	                      osm_parser_parseStart,
	                      osm_parser_parseEnd);

	self->indent  = 0;
	self->error   = 0;
	self->current = NULL;

	// success
	return self;

	// failure
	fail_parser_create:
		fclose(self->db);
	fail_db:
		fclose(self->out);
	fail_out:
		fclose(self->in);
	fail_in:
		osm_parser_delete(&self);
	return NULL;
}

void osm_parser_delete(osm_parser_t** _self)
{
	assert(_self);

	osm_parser_t* self = *_self;
	if(self)
	{
		XML_ParserFree(self->xml);
		fclose(self->db);
		fclose(self->out);
		fclose(self->in);
		free(self);
		*_self = NULL;
	}
}

int osm_parser_parse(osm_parser_t* self)
{
	assert(self);

	// print the xml header
	osm_parser_print(self, "%s", "<?xml version='1.0' encoding='UTF-8'?>\n");

	// print the search header
	osm_parser_printdb(self, "%s", "<?xml version='1.0' encoding='UTF-8'?>\n");
	osm_parser_printdb(self, "%s", "<db>\n");

	int done = 0;
	while(done == 0)
	{
		void* buf = XML_GetBuffer(self->xml, 4096);
		if(buf == NULL)
		{
			LOGE("XML_GetBuffer buf=NULL");
			return 0;
		}

		int bytes = fread(buf, 1, 4096, self->in);
		if(bytes < 0)
		{
			LOGE("read failed");
			return 0;
		}

		done = (bytes == 0) ? 1 : 0;
		if(XML_ParseBuffer(self->xml, bytes, done) == 0)
		{
			// make sure str is null terminated
			char* str = (char*) buf;
			str[(bytes > 0) ? (bytes - 1) : 0] = '\0';

			enum XML_Error e = XML_GetErrorCode(self->xml);
			LOGE("XML_ParseBuffer err=%s, bytes=%i, buf=%s",
			     XML_ErrorString(e), bytes, str);
			return 0;
		}

		// reset internal error flag
		if(self->error)
		{
			LOGE("internal error");
			self->error = 0;
		}
	}

	osm_parser_printdb(self, "%s", "</db>\n");

	return 1;
}

void osm_parser_printIndent(osm_parser_t* self, int indent)
{
	assert(self);

	int i;
	for(i = 0; i < indent; ++i)
	{
		osm_parser_print(self, "%s", "  ");
	}
}

void osm_parser_printElemBegin(osm_parser_t* self,
                               const XML_Char* name,
                               int indent)
{
	assert(self);
	assert(name);

	osm_parser_printIndent(self, indent);
	osm_parser_print(self, "<%s", name);
}

void osm_parser_printElemAttsSet(osm_parser_t* self,
                                 const XML_Char** atts)
{
	assert(self);
	assert(atts);

	while(atts[0])
	{
		osm_parser_printElemAttsPair(self, atts);
		atts += 2;
	}
}

void osm_parser_printElemAttsPair(osm_parser_t* self,
                                  const XML_Char** atts)
{
	assert(self);
	assert(atts);

	XML_Char line[4096];
	osm_parser_specialChars(atts[1], 4096, line);
	osm_parser_print(self, " %s=\"%s\"", atts[0], line);
}

void osm_parser_printElemEnd(osm_parser_t* self, int close)
{
	assert(self);

	if(close)
	{
		osm_parser_print(self, "%s", " />\n");
	}
	else
	{
		osm_parser_print(self, "%s", ">\n");
	}
}

void osm_parser_printElemClose(osm_parser_t* self,
                               const XML_Char* name,
                               int indent)
{
	assert(self);

	osm_parser_printIndent(self, indent);
	osm_parser_print(self, "</%s>\n", name);
}

void osm_parser_printDb(osm_parser_t* self,
                        int class,
                        const char* name,
                        const char* state,
                        double lat, double lon,
                        int ele)
{
	assert(self);
	assert(name);
	assert(state);

	XML_Char name2[4096];
	osm_parser_specialChars(name, 4096, name2);

	const char* class2 = osm_element_fromClass(class);

	int has_state = state[0] != '\0';
	int has_ele   = ele != 0;
	if(has_state && has_ele)
	{
		osm_parser_printdb(self,
		                   "\t<node name=\"%s\" state=\"%s\" class=\"%s\" rank=\"%i\" lat=\"%lf\" lon=\"%lf\" ele=\"%i\" />\n",
		                   name2, state, class2, class, lat, lon, ele);
	}
	else if(has_state)
	{
		osm_parser_printdb(self,
		                   "\t<node name=\"%s\" state=\"%s\" class=\"%s\" rank=\"%i\" lat=\"%lf\" lon=\"%lf\" />\n",
		                   name2, state, class2, class, lat, lon);
	}
	else if(has_ele)
	{
		osm_parser_printdb(self,
		                   "\t<node name=\"%s\" class=\"%s\" rank=\"%i\" lat=\"%lf\" lon=\"%lf\" ele=\"%i\" />\n",
		                   name2, class2, class, lat, lon, ele);
	}
	else
	{
		osm_parser_printdb(self,
		                   "\t<node name=\"%s\" class=\"%s\" rank=\"%i\" lat=\"%lf\" lon=\"%lf\" />\n",
		                   name2, class2, class, lat, lon);
	}
}
