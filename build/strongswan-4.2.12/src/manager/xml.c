/*
 * Copyright (C) 2007 Martin Willi
 * Hochschule fuer Technik Rapperswil
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 *
 * $Id: xml.c 3589 2008-03-13 14:14:44Z martin $
 */

#include "xml.h"

#include <libxml/parser.h>
#include <libxml/tree.h>


typedef struct private_xml_t private_xml_t;

/**
 * private data of xml
 */
struct private_xml_t {

	/**
	 * public functions
	 */
	xml_t public;
	
	/**
	 * root node of this xml (part)
	 */
	xmlNode *node;
	
	/**
	 * document, only for root xml_t
	 */
	xmlDoc *doc;
	
	/**
	 * Root xml_t*
	 */
	private_xml_t *root;
	
	/**
	 * number of enumerator instances
	 */
	int enums;
};

/**
 * child element enumerator
 */
typedef struct {
	/** enumerator interface */
	enumerator_t e;
	/** current child context (returned to enumerate() caller) */
	private_xml_t child;
	/** currently processing node */
	xmlNode *node;
} child_enum_t;

/**
 * Implementation of xml_t.children().enumerate().
 */
static bool child_enumerate(child_enum_t *e, private_xml_t **child,
							char **name, char **value)
{
	while (e->node && e->node->type != XML_ELEMENT_NODE)
	{
		e->node = e->node->next;
	}
	if (e->node)
	{
		xmlNode *text;
		
		text = e->node->children;
		*value = NULL;
		
		while (text && text->type != XML_TEXT_NODE)
		{
			text = text->next;
		}
		if (text)
		{
			*value = text->content;
		}
		*name = (char*)e->node->name;
		*child = &e->child;
		e->child.node = e->node->children;
		e->node = e->node->next;
		return TRUE;
	}
	return FALSE;
}

/**
 * Implementation of xml_t.get_attribute.
 */
static char* get_attribute(private_xml_t *this, char *name)
{
	return NULL;
}

/**
 * destroy enumerator, and complete tree if this was the last enumerator 
 */
static void child_destroy(child_enum_t *this)
{
	if (--this->child.root->enums == 0)
	{
		xmlFreeDoc(this->child.root->doc);
		free(this->child.root);
	}
	free(this);
}

/**
 * Implementation of xml_t.children.
 */
static enumerator_t* children(private_xml_t *this)
{
	child_enum_t *ce = malloc_thing(child_enum_t);
	ce->e.enumerate = (void*)child_enumerate;
	ce->e.destroy = (void*)child_destroy;
	ce->node = this->node;
	ce->child.public.children = (void*)children;
	ce->child.public.get_attribute = (void*)get_attribute;
	ce->child.node = NULL;
	ce->child.doc = this->doc;
	ce->child.root = this->root;
	this->root->enums++;
	return &ce->e;
}

/*
 * see header file
 */
xml_t *xml_create(char *xml)
{
	private_xml_t *this = malloc_thing(private_xml_t);
	
	this->public.get_attribute = (char*(*)(xml_t*,char*))get_attribute;
	this->public.children = (enumerator_t*(*)(xml_t*))children;
	
	this->doc = xmlReadMemory(xml, strlen(xml), NULL, NULL, 0);
	if (this->doc == NULL)
	{
		free(this);
		return NULL;
	}
	this->node = xmlDocGetRootElement(this->doc);
	this->root = this;
	this->enums = 0;
	
	return &this->public;
}

