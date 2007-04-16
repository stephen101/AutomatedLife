delimiter // ;

-- node triggers
--create trigger node_ai after insert on node for each row 
--	call update_node_count (NEW.fk_collection, NEW.type_major, 1); //
	
create trigger node_ad after delete on node for each row 
begin
	-- decrement node count for this collection/node type
	call update_node_count (OLD.fk_collection, OLD.type_major, -1);
	-- get rid of degree table entries
	delete from degree where fk_node = OLD.id;
	-- clear all edges working with this node
	delete from edge where fk_node_from = OLD.id or fk_node_to = OLD.id;
end; //

-- collection triggers
create trigger collection_ad after delete on collection for each row
begin
	-- clear all our nodes
	delete from node where fk_collection=OLD.id;
	-- and node counts
	delete from node_count where fk_collection=OLD.id;
	-- clear collection meta data
	delete from collection_meta where fk_collection=OLD.id;
end; //

-- edge triggers
create trigger edge_ad after delete on edge for each row
begin
	-- get rid of our entries in edge_query table
	delete from edge_query where id = OLD.id;
	-- now update all nodes' edges that were affected by this deletion
	call flag_edge_affected_nodes(OLD.fk_node_from, OLD.fk_node_to);
end; //

create trigger edge_ai after insert on edge for each row
begin
	-- after creating an edge between two existing nodes
	-- we must update any nodes that either node is connected to
	call flag_edge_affected_nodes(NEW.fk_node_from, NEW.fk_node_to);
end; //

create trigger edge_au after update on edge for each row
begin
	-- we must just update the edge_query table for this node, as the strength has changed
	if @batch_mode != 1 then
		insert ignore into dirty_node (fk_node) values (NEW.fk_node_from);
	end if;
end; //

delimiter ; //
