#include <kore/kore.h>
#include <kore/http.h>
#include <kore/pgsql.h>
int init(int);

int		page(struct http_request *);
int init(int state){
kore_pgsql_register("db","dbname=postgres");
return (KORE_RESULT_OK);	
}
int
page(struct http_request *req)
{
	struct kore_pgsql sql;
	char*name;
	int rows,i;
	req->status=HTTP_STATUS_INTERNAL_ERROR;
	kore_pgsql_init(&sql);
	if(!kore_pgsql_setup(&sql,"db",KORE_PGSQL_SYNC)){
	kore_pgsql_logerror(&sql);
	goto out;	
	}
	if(!kore_pgsql_query(&sql,"select*from coders")){
	kore_pgsql_logerror(&sql);
	goto out;	
	}
	rows=kore_pgsql_ntuples(&sql);
	for(i=0;i<rows;i++){
	name=kore_pgsql_getvalue(&sql,i,0);
	kore_log(LOG_NOTICE,"name: %s",name);	
	}
	req->status=HTTP_STATUS_OK;
	out:
	http_response(req, 200, NULL, 0);
	kore_pgsql_cleanup(&sql);
	return (KORE_RESULT_OK);
}
