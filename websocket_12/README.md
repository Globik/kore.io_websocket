## postgreSQL listen / notify built-in
1. Issue listen asynchronous at loaded time. Listener has your own state machine and your own struct kore_pgsql.
    If state_done - you must no call kore_pgsql_continue(pgsql);	
2. Other "normal" asynchronous queries in app have your own state machine and struct kore_pgsql.
   If state_done - you must call kore_pgsql_continue(pgsql_2);	to complete a query properly.
3. Asynchronous queries from within websocket instance in this implementation are independend.
   Pgsql istance is NOT bound to connection->hdlr_extra. I wanna leave the hdlr_extra for other shared data between websocket clients.
   Trying to assign the metainfo to the pgsql->arg. For the time being. 
4. Websocket's pgsql I made global. F. knows why. Bad experience if it was right in place - initialization and to callback binding.
5. A ll thing "Works" like "expected by me". 

## TODO

1. Make one state machine for all asynchronous queries. May be to use pgsql->conn->name as a basis for a state managment??
2. Maybe I should issue a LISTEN command synchronous at loaded time??? It's simpler than standalone state machine and other pgsql instance.

## What does dislike me in kore pgsql

1. I don't want to use sunchronous queries in websocket workflows.
2. I don't want to bind the pgsql instance to the connection->hdlr_extra in case of websocket. No way.
3. Sorry to say, kore connects to database synchronically.
4. No PQsetnonblocking.
5. Kore pgsql does NOT support multiple queries at ones. Any bulk operations are impossible.
6. I don't know if it is a good parctice in every coming websocket requests do that for every query??? =>
```
pgsql2=kore_calloc(1, sizeof(*pgsql2));
kore_pgsql_init(pgsql2);
int b=2;
kore_pgsql_bind_callback(pgsql2,db_state_change2,(void*)b);

```
Maybe it is simplier to do like this at ones at loaded time(at run time)???
I don't want to write always and always for every other query its own state machine.
