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
 * $Id: request.h 3531 2008-03-06 09:50:56Z martin $
 */

/**
 * @defgroup request request
 * @{ @ingroup libfast
 */

#ifndef REQUEST_H_
#define REQUEST_H_

#include <fcgiapp.h>
#include <library.h>

typedef struct request_t request_t;

/**
 * A HTTP request, encapsulates FCGX_Request.
 *
 * The response is also handled through the request object.
 */
struct request_t {
	
	/**
	 * Add a cookie to the reply (Set-Cookie header).
	 *
	 * @param name		name of the cookie to set
	 * @param value		value of the cookie
	 */
	void (*add_cookie)(request_t *this, char *name, char *value);
	
	/**
	 * Get a cookie the client sent in the request.
	 *
	 * @param name		name of the cookie
	 * @return			cookie value, NULL if no such cookie found
	 */
	char* (*get_cookie)(request_t *this, char *name);
	
	/**
	 * Get the request path relative to the application.
	 *
	 * @return			path
	 */
	char* (*get_path)(request_t *this);
	
	/**
	 * Get the base path of the application.
	 *
	 * @return			base path
	 */
	char* (*get_base)(request_t *this);
	
	/**
	 * Get the remote host address of this request.
	 *
	 * @return			host address as string
	 */
	char* (*get_host)(request_t *this);
	
	/**
	 * Get the user agent string.
	 *
	 * @return			user agent string
	 */
	char* (*get_user_agent)(request_t *this);
		
	/**
	 * Get a post/get variable included in the request.
	 *
	 * @param name		name of the POST/GET variable
	 * @return			value, NULL if not found
	 */
	char* (*get_query_data)(request_t *this, char *name);
	
	/**
	 * Close the session and it's context after handling.
	 */
	void (*close_session)(request_t *this);
	
	/**
	 * Has the session been closed by close_session()?
	 *
	 * @return			TRUE if session has been closed
	 */
	bool (*session_closed)(request_t *this);
	
	/**
	 * Redirect the client to another location.
	 *
	 * @param fmt		location format string
	 * @param ...		variable argument for fmt
	 */
	void (*redirect)(request_t *this, char *fmt, ...);
	
	/**
	 * Redirect the client to the referer.
	 */
	void (*to_referer)(request_t *this);
		
	/**
	 * Set a template value.
	 *
	 * @param key		key to set
	 * @param value		value to set key to
	 */
	void (*set)(request_t *this, char *key, char *value);
	
	/**
	 * Set a template value using format strings.
	 *
	 * Format string is in the form "key=value", where printf like format
	 * substitution occurs over the whole string.
	 *
	 * @param format	printf like format string
	 * @param ...		variable argument list
	 */
	void (*setf)(request_t *this, char *format, ...);
	
	/**
	 * Render a template.
	 *
	 * The render() function additionally sets a HDF variable "base"
	 * which points to the root of the web application and allows to point to
	 * other targets without to worry about path location.
	 *
	 * @param template	clearsilver template file location
	 */
	void (*render)(request_t *this, char *template);
	
	/**
	 * Stream a format string to the client.
	 *
	 * Stream is not closed and may be called multiple times to allow
	 * server-push functionality.
	 *
	 * @param format	printf like format string
	 * @param ...		argmuent list to format string
	 * @return			number of streamed bytes, < 0 if stream closed
	 */
	int (*streamf)(request_t *this, char *format, ...);
	
	/**
	 * Serve a request with headers and a body.
	 *
	 * @param headers	HTTP headers, \n separated
	 * @param chunk		body to write to output
	 */
	void (*serve)(request_t *this, char *headers, chunk_t chunk);
	
	/**
	 * Increase the reference count to the stream.
	 *
	 * @return			this with increased refcount
	 */
	request_t* (*get_ref)(request_t *this);
	
	/**
	 * Destroy the request_t.
	 */
	void (*destroy) (request_t *this);
};

/**
 * Create a request from the fastcgi struct.
 *
 * @param fd			file descripter opened with FCGX_OpenSocket
 * @param debug			no stripping, no compression, timing information
 */
request_t *request_create(int fd, bool debug);

#endif /* REQUEST_H_ @} */
