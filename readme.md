OSM Style
=========

Osm map style mod that can be combined with terrain shading.

OSM Configuration
=================

The instructions below are derrived from switch2osm and
modified for Ubuntu 15.10.

	https://switch2osm.org/serving-tiles/manually-building-a-tile-server-14-04/

Install shared dependencies

	sudo apt-get install libboost-all-dev subversion git-core tar unzip wget bzip2 build-essential autoconf libtool libxml2-dev libgeos-dev libgeos++-dev libpq-dev libbz2-dev libproj-dev munin-node munin libprotobuf-c0-dev protobuf-c-compiler libfreetype6-dev libpng12-dev libicu-dev libgdal-dev libcairo-dev libcairomm-1.0-dev apache2 apache2-dev libagg-dev liblua5.2-dev ttf-unifont lua5.1 liblua5.1-dev node-carto
	sudo apt-get install libtiff-dev libgeotiff-epsg

postgresql / postgis
--------------------

Install database

	croot
	sudo apt-get install postgresql postgresql-contrib postgis
	sudo -u postgres -i
	createuser gisuser
	createdb -E UTF8 -O gisuser gis
	exit

Create user

	sudo useradd -m gisuser
	sudo passwd gisuser

Setup database

	sudo -u postgres psql
	\c gis
	CREATE EXTENSION postgis;
	CREATE EXTENSION hstore;
	ALTER TABLE geometry_columns OWNER TO gisuser;
	ALTER TABLE spatial_ref_sys OWNER TO gisuser;
	\q

osm2pgsql
---------

Clone osm2pgsql

	croot
	git clone git://github.com/openstreetmap/osm2pgsql.git

Install dependencies

	sudo apt-get install make cmake g++ libboost-dev libboost-system-dev libboost-filesystem-dev libexpat1-dev zlib1g-dev libbz2-dev libpq-dev libgeos-dev libgeos++-dev libproj-dev lua5.2 liblua5.2-dev

Build osm2pgsql

	cd osm2pgsql
	mkdir build
	cd build
	cmake ..
	make
	sudo make install

mapnik
------

Clone mapnik

	croot
	git clone git://github.com/mapnik/mapnik
	cd mapnik
	git checkout -b br-2.2 origin/2.2.x

Build mapnik

	python scons/scons.py configure INPUT_PLUGINS=all OPTIMIZATION=3 SYSTEM_FONTS=/usr/share/fonts/truetype/
	make
	sudo make install
	sudo ldconfig

mod_tile
--------

Clone mod_tile

	croot
	git clone git://github.com/openstreetmap/mod_tile.git
	cd mod_tile

Build mod_tile

	./autogen.sh
	./configure
	make
	sudo make install
	sudo make install-mod_tile
	sudo ldconfig

osm bright
----------

Download resources

	sudo mkdir -p /usr/local/share/maps/style
	cd /usr/local/share/maps/style
	sudo wget https://github.com/mapbox/osm-bright/archive/master.zip
	sudo wget http://data.openstreetmapdata.com/simplified-land-polygons-complete-3857.zip
	sudo wget http://data.openstreetmapdata.com/land-polygons-split-3857.zip
	sudo wget http://www.naturalearthdata.com/http//www.naturalearthdata.com/download/10m/cultural/ne_10m_populated_places_simple.zip

Unzip the resources

	sudo unzip '*.zip'
	sudo mkdir osm-bright-master/shp
	sudo mv land-polygons-split-3857 osm-bright-master/shp/
	sudo mv simplified-land-polygons-complete-3857 osm-bright-master/shp/
	# sudo mv ne_10m_populated_places_simple osm-bright-master/shp/
	sudo mkdir osm-bright-master/shp/ne_10m_populated_places_simple
	sudo mv ne_10m_populated_places_simple.dbf osm-bright-master/shp/ne_10m_populated_places_simple/
	sudo mv ne_10m_populated_places_simple.prj osm-bright-master/shp/ne_10m_populated_places_simple/
	sudo mv ne_10m_populated_places_simple.shp osm-bright-master/shp/ne_10m_populated_places_simple/
	sudo mv ne_10m_populated_places_simple.VERSION.txt osm-bright-master/shp/ne_10m_populated_places_simple/
	sudo mv ne_10m_populated_places_simple.README.html osm-bright-master/shp/ne_10m_populated_places_simple/
	sudo mv ne_10m_populated_places_simple.shx osm-bright-master/shp/ne_10m_populated_places_simple/

Create index files

	cd osm-bright-master/shp/land-polygons-split-3857
	sudo shapeindex land_polygons.shp
	cd ../simplified-land-polygons-complete-3857
	sudo shapeindex simplified_land_polygons.shp
	cd ../..

Configure osm bright

	sudo vim osm-bright/osm-bright.osm2pgsql.mml

	# replace zip lines and add type shape
	"file": "/usr/local/share/maps/style/osm-bright-master/shp/land-polygons-split-3857/land_polygons.shp", 
	"type": "shape"
	"file": "/usr/local/share/maps/style/osm-bright-master/shp/simplified-land-polygons-complete-3857/simplified_land_polygons.shp", 
	"type": "shape",
	"file": "/usr/local/share/maps/style/osm-bright-master/shp/ne_10m_populated_places_simple/ne_10m_populated_places_simple.shp", 
	"type": "shape"

	# replace neplaces "srs" and "srs-name" lines with:
	"srs": "+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs"

Configure style sheet

	sudo cp configure.py.sample configure.py
	sudo vim configure.py

	# replace lines in configure.py
	config["path"] = path.expanduser("/usr/local/share/maps/style")
	config["postgis"]["dbname"]   = "gis"

	sudo ./make.py
	cd ../OSMBright/
	sudo su
	carto project.mml > OSMBright.xml
	exit

renderd
-------

Configure renderd

	sudo vim /usr/local/etc/renderd.conf
	socketname=/var/run/renderd/renderd.sock
	plugins_dir=/usr/local/lib/mapnik/input
	font_dir=/usr/share/fonts/truetype
	XML=/usr/local/share/maps/style/OSMBright/OSMBright.xml
	HOST=localhost

	# move mod_tile to home directory due to space
	sudo mkdir /var/run/renderd
	sudo chown gisuser:gisuser /var/run/renderd
	sudo mkdir /home/osm
	sudo chmod 777 /home/osm
	sudo mkdir /home/osm/mod_tile
	sudo chown gisuser:gisuser /home/osm/mod_tile
	sudo ln -s /home/osm/mod_tile /var/lib/mod_tile

mod_tile
--------

Configure mod_tile

	sudo vim /etc/apache2/conf-available/mod_tile.conf

	# add this line
	LoadModule tile_module /usr/lib/apache2/modules/mod_tile.so

	sudo vim /etc/apache2/sites-available/000-default.conf

	# add these lines after admin email line
	LoadTileConfigFile /usr/local/etc/renderd.conf
	ModTileRenderdSocketName /var/run/renderd/renderd.sock
	ModTileRequestTimeout 0
	ModTileMissingRequestTimeout 30

Restart apache

	sudo a2enconf mod_tile
	sudo service apache2 reload

move database
-------------

	sudo su
	pg_ctlcluster 9.4 main stop
	cp -aRv /var/lib/postgresql /home/osm/postgresql
	vim /etc/postgresql/9.4/main/postgresql.conf

	# change data directory
	data_directory = '/home/osm/postgresql/9.4/main'

	pg_ctlcluster 9.4 main restart
	exit

osm data
--------

download planet

	wget http://ftp5.gwdg.de/pub/misc/openstreetmap/planet.openstreetmap.org/pbf/planet-151116.osm.pbf

download osmosis

	# http://wiki.openstreetmap.org/wiki/Osmosis#Latest_stable_version
	croot
	mkdir osmosis
	cd osmosis
	wget http://bretth.dev.openstreetmap.org/osmosis-build/osmosis-latest.tgz
	tar -xzf osmosis-latest.tgz

crop planet (e.g.)

	osmosis --read-pbf file="planet-151116.osm.pbf" --bounding-box top=72.0 left=-170.0 bottom=18.0 right=-66.0 --write-pbf file="US-151116.osm.pbf"
	osmosis --read-pbf file="planet-151116.osm.pbf" --bounding-box top=50.0 left=-126.0 bottom=24.0 right=-65.0 --write-pbf file="US48-151116.osm.pbf"
	osmosis --read-pbf file="US48-151116.osm.pbf" --bounding-box top=41.10419 left=-109.11621 bottom=36.88841 right=-101.88721 --write-pbf file="CO-151116.osm.pbf"

import osm data

	croot
	cd osm2pgsql
	su gisuser
	osm2pgsql --slim -d gis ../CO-151116.osm.pbf

start renderd
------------

	sudo -u gisuser renderd -f -c /usr/local/etc/renderd.conf
	<in a separate window>
	sudo service apache2 reload

slippymap
---------

	croot
	cd mod_tile
	sudo mkdir /var/www/html/osm_tiles
	sudo cp slippymap.html /var/www/html/osm_tiles/slippymap.html
	sudo chown -R gisuser:gisuser /var/www/html/osm_tile

	sudo vim /var/www/html/osm_tiles/slippymap.html

	# change default lat/lon for Boulder, CO
	var lat=40.01476;
	var lon=-105.25331;

	# open in browser
	http://localhost/osm_tiles/slippymap.html

OSM Configuration (LEGACY)
==========================

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
