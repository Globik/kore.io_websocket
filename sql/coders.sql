-- psql postgres
-- \i /home/globik/kore.io_websocket/pgsql/sql/coders.sql
-- listen on_coders
CREATE OR REPLACE FUNCTION notify_coders() RETURNS TRIGGER AS $$

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
PERFORM pg_notify('on_coders', notification::text);
RETURN NULL;
END;
$$ LANGUAGE plpgsql;

DROP TRIGGER IF EXISTS notif_coders ON coders;

CREATE TRIGGER  notif_coders
AFTER INSERT OR UPDATE OR DELETE ON coders
FOR EACH ROW EXECUTE PROCEDURE notify_coders();
