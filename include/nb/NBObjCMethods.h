
#ifndef NB_OBJC_METHODS_H
#define NB_OBJC_METHODS_H

#ifdef __APPLE__
#	include <objc/message.h>	//for "objc_msgSend"
#	include <objc/objc.h>		//for "sel_registerName"
BOOL (*BOOL_objc_msgSend_id)    (id, SEL, id)   = (BOOL (*)(id, SEL, id)) objc_msgSend;
id (*id_objc_msgSend_id)        (id, SEL, id)   = (id (*)(id, SEL, id)) objc_msgSend;
id (*id_objc_msgSend)           (id, SEL)       = (id (*)(id, SEL)) objc_msgSend;
void (*void_objc_msgSend)       (id, SEL)       = (void (*)(id, SEL)) objc_msgSend;
#endif

#endif
