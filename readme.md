OSM Style
=========

Osm map style mod that can be combined with terrain shading.

OSM Configuration
=================

http://switch2osm.org/serving-tiles/building-a-tile-server-from-packages/

Change predefined data directories from /var to /home due to disk space issues.

stop
----

	sudo su

	service apache2 stop
	service renderd stop
	pg_ctlcluster 9.1 main stop

	mkdir /home/osm
	cd /home/osm

postgres
--------

	cp -aRv /var/lib/postgresql postgresql
	pg_ctlcluster 9.1 main restart

	# /etc/postgresql/9.1/main/postgresql.conf
	- data_directory = '/var/lib/postgresql/9.1/main'
	+ data_directory = '/home/osm/postgresql/9.1/main'

renderd
-------

	mkdir mod_tile
	chown www-data:www-data mod_tile

	# /etc/renderd.conf
	- tile_dir=/var/lib/mod_tile
	+ tile_dir=/home/osm/mod_tile

apache2
-------

	# /etc/apache2/sites-available/tileserver_site.conf
	- ModTileTileDir /var/lib/mod_tile
	+ ModTileTileDir /home/osm/mod_tile

restart
-------

	pg_ctlcluster 9.1 main restart
	service renderd restart
	service apache2 restart

reload
------

	service renderd stop
	rm -rf /home/osm/mod_tile/default
	cp osm.xml /etc/mapnik-osm-carto-data
	service renderd start

notes
-----

	# map links
	http://localhost/osm/slippymap.html
	http://localhost/osm/1/0/0.png
	http://localhost/osm/15/6807/12397.png

	# stylesheet
	# https://github.com/mapnik/mapnik/wiki/XMLConfigReference
	/etc/mapnik-osm-carto-data
