-- \i /home/globik/kore.io_websocket/pgsql/sql/banners.sql
-- create table banners(id serial primary key,alt text);
	-- listen revents
	-- insert into banners(alt) values('desc');
	-- update banners set alt='fucker';
CREATE OR REPLACE FUNCTION notify_event() RETURNS TRIGGER AS $$

DECLARE
	data json;
	notification json;
BEGIN
	IF(TG_OP = 'DELETE') THEN
		data = row_to_json(OLD);
	ELSE
		data=row_to_json(NEW);
	END IF;
	notification = json_build_object('table', TG_TABLE_NAME, 'action', TG_OP, 'data', data);
PERFORM pg_notify('revents', notification::text);
RETURN NULL;
END;
$$ LANGUAGE plpgsql;

DROP TRIGGER IF EXISTS notif_banner ON banners;

CREATE TRIGGER  notif_banner
AFTER INSERT OR UPDATE OR DELETE ON banners
FOR EACH ROW EXECUTE PROCEDURE notify_event();
