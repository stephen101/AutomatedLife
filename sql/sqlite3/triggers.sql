create trigger node_ad after delete on node for each row 
begin
	-- clear all edges working with this node
	delete from edge where fk_node_from = OLD.id or fk_node_to = OLD.id;
end;

-- collection triggers
create trigger collection_ad after delete on collection for each row
begin
	-- clear all our nodes
	delete from node where fk_collection=OLD.id;
	-- and node counts
	delete from node_count where fk_collection=OLD.id;
	-- clear collection meta data
	delete from collection_meta where fk_collection=OLD.id;
end;