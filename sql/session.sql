-- secure signed cookie https://www.meetspaceapp.com/2016/04/19/cookie-signing-postgresql.html
create extension pgcrypto;
create table session(
id uuid not null default gen_random_uuid(),
user_id uuid not null,
key text not null default gen_salt('md5'));

insert into session(user_id) values(gen_random_uuid());

select(id || '-' || encode(hmac(id::text,key,'sha256'),'hex')) as cookie from session;

select user_id from session where (id || '-' || encode(hmac(id::text,key,'sha256'),'hex'))='38897a58-65...'

create index session_cookie_idx on session(id || '-' || encode(hmac(id::text,key,'sha256'),'hex'));
