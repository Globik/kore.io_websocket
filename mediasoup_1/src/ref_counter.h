#include <strings.h>
#include <stdlib.h>
#include <stdio.h>
#include <glib.h>
int refdebug;
#define j_refcount(refptr,type,member) \
((type*)((char*)(refptr) - offsetof(type,member)))

typedef struct j_refcnt j_refcnt;
 struct j_refcnt {
	gint count;
	void(*free)(const j_refcnt*);
};

#define j_refcount_init(refp,free_fn){ \
if(!refdebug){ \
j_refcount_init_nodebug(refp,free_fn); \
}else{ \
j_refcount_init_debug(refp,free_fn); \
} \
}

#define j_refcount_init_debug(refp,free_fn){ \
(refp)->count=1; \
g_print("[%s:%s:%d:init] %p (%d)\n",__FILE__,__FUNCTION__,__LINE__, refp,(refp)->count); \
(refp)->free=free_fn; \
}

#define j_refcount_init_nodebug(refp, free_fn){ \
(refp)->count=1; \
(refp)->free=free_fn; \
}

#define j_refcount_inc(refp){ \
if(!refdebug){ \
j_refcount_increase_nodebug(refp); \
}else{ \
j_refcount_increase_debug(refp); \
} \
}
#define j_refcount_increase_nodebug(refp){ \
g_atomic_int_inc((gint*)&(refp)->count); \
}
#define j_refcount_increase_debug(refp){ \
g_print("[%s:%s:%d:increase] %p (%d)\n",__FILE__,__FUNCTION__,__LINE__, refp,(refp)->count+1); \
g_atomic_int_inc((gint*)&(refp)->count); \
}
#define j_refcount_dec(refp){ \
if(!refdebug){ \
j_refcount_decrease_nodebug(refp); \
}else{ \
j_refcount_decrease_debug(refp); \
} \
}
#define j_refcount_decrease_debug(refp){ \
g_print("[%s:%s:%d: decrease] %p (%d)\n",__FILE__,__FUNCTION__,__LINE__, refp, (refp)->count-1); \
if(g_atomic_int_dec_and_test((gint*)&(refp)->count)) { \
(refp)->free(refp); \
} \
}
#define j_refcount_decrease_nodebug(refp){ \
if(g_atomic_int_dec_and_test((gint*)&(refp)->count)){ \
(refp)->free(refp); \
} \
}
