/*
 * "$Id: mxml.h 451 2014-01-04 21:50:06Z msweet $"
 *
 * Header file for Mini-XML, a small XML-like file parsing library.
 *
 * Copyright 2003-2014 by Michael R Sweet.
 *
 * These coded instructions, statements, and computer programs are the
 * property of Michael R Sweet and are protected by Federal copyright
 * law.  Distribution and use rights are outlined in the file "COPYING"
 * which should have been included with this file.  If this file is
 * missing or damaged, see the license at:
 *
 *     http://www.msweet.org/projects.php/Mini-XML
 */

/*
 * Prevent multiple inclusion...
 */

#ifndef _mxml_h_
#  define _mxml_h_

/*
 * Include necessary headers...
 */

#  include <stdio.h>
#  include <stdlib.h>
#  include <string.h>
#  include <ctype.h>
#  include <errno.h>


/*
 * Constants...
 */

#  define MXML_MAJOR_VERSION	2	/* Major version number */
#  define MXML_MINOR_VERSION	8	/* Minor version number */

#  define MXML_TAB		8	/* Tabs every N columns */

#  define MXML_NO_CALLBACK	0	/* Don't use a type callback */
#  define MXML_INTEGER_CALLBACK	mxml_integer_cb
					/* Treat all data as integers */
#  define MXML_OPAQUE_CALLBACK	mxml_opaque_cb
					/* Treat all data as opaque */
#  define MXML_REAL_CALLBACK	mxml_real_cb
					/* Treat all data as real numbers */
#  define MXML_TEXT_CALLBACK	0	/* Treat all data as text */
#  define MXML_IGNORE_CALLBACK	mxml_ignore_cb
					/* Ignore all non-element content */

#  define MXML_NO_PARENT	0	/* No parent for the node */

#  define MXML_DESCEND		1	/* Descend when finding/walking */
#  define MXML_NO_DESCEND	0	/* Don't descend when finding/walking */
#  define MXML_DESCEND_FIRST	-1	/* Descend for first find */

#  define MXML_WS_BEFORE_OPEN	0	/* Callback for before open tag */
#  define MXML_WS_AFTER_OPEN	1	/* Callback for after open tag */
#  define MXML_WS_BEFORE_CLOSE	2	/* Callback for before close tag */
#  define MXML_WS_AFTER_CLOSE	3	/* Callback for after close tag */

#  define MXML_ADD_BEFORE	0	/* Add node before specified node */
#  define MXML_ADD_AFTER	1	/* Add node after specified node */
#  define MXML_ADD_TO_PARENT	NULL	/* Add node relative to parent */


/*
 * Data types...
 */

typedef enum mxml_sax_event_e		/**** SAX event type. ****/
{
  MXML_SAX_CDATA,			/* CDATA node */
  MXML_SAX_COMMENT,			/* Comment node */
  MXML_SAX_DATA,			/* Data node */
  MXML_SAX_DIRECTIVE,			/* Processing directive node */
  MXML_SAX_ELEMENT_CLOSE,		/* Element closed */
  MXML_SAX_ELEMENT_OPEN			/* Element opened */
} mxml_sax_event_t;

typedef enum mxml_type_e		/**** The XML node type. ****/
{
  MXML_IGNORE = -1,			/* Ignore/throw away node @since Mini-XML 2.3@ */
  MXML_ELEMENT,				/* XML element with attributes */
  MXML_INTEGER,				/* Integer value */
  MXML_OPAQUE,				/* Opaque string */
  MXML_REAL,				/* Real value */
  MXML_TEXT,				/* Text fragment */
  MXML_CUSTOM				/* Custom data @since Mini-XML 2.1@ */
} mxml_type_t;

typedef void (*mxml_custom_destroy_cb_t)(void *);
					/**** Custom data destructor ****/

typedef void (*mxml_error_cb_t)(const char *);
					/**** Error callback function ****/

typedef struct mxml_attr_s		/**** An XML element attribute value. @private@ ****/
{
  char			*name;		/* Attribute name */
  char			*value;		/* Attribute value */
} mxml_attr_t;

typedef struct mxml_element_s		/**** An XML element value. @private@ ****/
{
  char			*name;		/* Name of element */
  int			num_attrs;	/* Number of attributes */
  mxml_attr_t		*attrs;		/* Attributes */
} mxml_element_t;

typedef struct mxml_text_s		/**** An XML text value. @private@ ****/
{
  int			whitespace;	/* Leading whitespace? */
  char			*string;	/* Fragment string */
} mxml_text_t;

typedef struct mxml_custom_s		/**** An XML custom value. @private@ ****/
{
  void			*data;		/* Pointer to (allocated) custom data */
  mxml_custom_destroy_cb_t destroy;	/* Pointer to destructor function */
} mxml_custom_t;

typedef union mxml_value_u		/**** An XML node value. @private@ ****/
{
  mxml_element_t	element;	/* Element */
  int			integer;	/* Integer number */
  char			*opaque;	/* Opaque string */
  double		real;		/* Real number */
  mxml_text_t		text;		/* Text fragment */
  mxml_custom_t		custom;		/* Custom data @since Mini-XML 2.1@ */
} mxml_value_t;

struct mxml_node_s			/**** An XML node. @private@ ****/
{
  mxml_type_t		type;		/* Node type */
  struct mxml_node_s	*child;		/* First child node */
  struct mxml_node_s	*last_child;	/* Last child node */
  mxml_value_t		value;		/* Node value */
  int			ref_count;	/* Use count */
  void			*user_data;	/* User data */
};

typedef struct mxml_node_s mxml_node_t;	/**** An XML node. ****/

struct mnode_t	*mxmlSAXLoadFd(mxml_node_t *top, int fd,
			               mxml_type_t (*cb)(mxml_node_t *),
			               mxml_sax_cb_t sax, void *sax_data);
extern mxml_node_t	*mxmlSAXLoadFile(mxml_node_t *top, FILE *fp,
			                 mxml_type_t (*cb)(mxml_node_t *),
			                 mxml_sax_cb_t sax, void *sax_data);
extern mxml_node_t	*mxmlSAXLoadString(mxml_node_t *top, const char *s,
			                   mxml_type_t (*cb)(mxml_node_t *),
			                   mxml_sax_cb_t sax, void *sax_data);
extern int		mxmlSetCDATA(mxml_node_t *node, const char *data);
extern int		mxmlSetCustom(mxml_node_t *node, void *data,
			              mxml_custom_destroy_cb_t destroy);
extern void		mxmlSetCustomHandlers(mxml_custom_load_cb_t load,
			                      mxml_custom_save_cb_t save);
extern int		mxmlSetElement(mxml_node_t *node, const char *name);
extern void		mxmlSetErrorCallback(mxml_error_cb_t cb);
extern int		mxmlSetInteger(mxml_node_t *node, int integer);
extern int		mxmlSetOpaque(mxml_node_t *node, const char *opaque);
extern int		mxmlSetReal(mxml_node_t *node, double real);
extern int		mxmlSetText(mxml_node_t *node, int whitespace,
			            const char *string);
extern int		mxmlSetTextf(mxml_node_t *node, int whitespace,
			             const char *format, ...)
#    ifdef __GNUC__
__attribute__ ((__format__ (__printf__, 3, 4)))
#    endif /* __GNUC__ */
;
extern int		mxmlSetUserData(mxml_node_t *node, void *data);
extern void		mxmlSetWrapMargin(int column);
extern mxml_node_t	*mxmlWalkNext(mxml_node_t *node, mxml_node_t *top,
			              int descend);
extern mxml_node_t	*mxmlWalkPrev(mxml_node_t *node, mxml_node_t *top,
			              int descend);


/*
 * Semi-private functions...
 */

extern void		mxml_error(const char *format, ...);
extern mxml_type_t	mxml_ignore_cb(mxml_node_t *node);
extern mxml_type_t	mxml_integer_cb(mxml_node_t *node);
extern mxml_type_t	mxml_opaque_cb(mxml_node_t *node);
extern mxml_type_t	mxml_real_cb(mxml_node_t *node);


/*
 * C++ support...
 */

#  ifdef __cplusplus
}
#  endif /* __cplusplus */
#endif /* !_mxml_h_ */


/*
 * End of "$Id: mxml.h 451 2014-01-04 21:50:06Z msweet $".
 */
