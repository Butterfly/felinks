#ifndef EL__DOCUMENT_DOM_ECMASCRIPT_SPIDERMONKEY_HTML_HTMLMODELEMENT_H
#define EL__DOCUMENT_DOM_ECMASCRIPT_SPIDERMONKEY_HTML_HTMLMODELEMENT_H

#include "document/dom/ecmascript/spidermonkey/html/HTMLElement.h"
#include "ecmascript/spidermonkey/util.h"

extern const JSClass HTMLModElement_class;
extern const JSFunctionSpec HTMLModElement_funcs[];
extern const JSPropertySpec HTMLModElement_props[];

struct MOD_struct {
	struct HTMLElement_struct html;
	unsigned char *cite;
	unsigned char *date_time;
};

void make_MOD_object(JSContext *ctx, struct dom_node *node);
void done_MOD_object(struct dom_node *node);
#endif