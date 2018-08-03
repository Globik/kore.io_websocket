## postgreSQL listen / notify built-in
 Made some changes in the source code. See pgsql_1.c
 
 LISTEN / NOTIFY has your own "dbname" in  kore_pgsql_register(q_name,"dbname=postgres"); 
 
 Lives "standalonish" while an application running. Realy is not. Its inserted in some List or so.
 
 So far so good but I don't know if this standalone interface will be blocking other connections.
 
