Configure CLOUD
===============

	useradd USER -m -s /bin/bash
	passwd USER
	useradd USER sudo

	vim /etc/default/locale
	LANG="en_US.UTF-8"

	sudo apt-get update
	sudo apt-get install mosh
	locale-gen en_US.UTF-8

OSM Style
=========

Osm map style mod that can be combined with terrain shading.
The style has forked from openstreetmap-carto since the
original project began. The new style is located here.

https://github.com/jeffboody/openstreetmap-carto/tree/hd_v1

OSM Configuration
=================

The instructions below are derrived from switch2osm and
modified for Ubuntu 15.10.

	https://switch2osm.org/serving-tiles/manually-building-a-tile-server-14-04/

Install shared dependencies

	sudo apt-get install libboost-all-dev subversion git-core tar unzip wget bzip2 build-essential autoconf libtool libxml2-dev libgeos-dev libgeos++-dev libpq-dev libbz2-dev libproj-dev munin-node munin libprotobuf-c0-dev protobuf-c-compiler libfreetype6-dev libpng12-dev libicu-dev libgdal-dev libcairo-dev libcairomm-1.0-dev apache2 apache2-dev libagg-dev liblua5.2-dev ttf-unifont lua5.1 liblua5.1-dev node-carto libtiff-dev

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

Recreate database

	sudo -u postgres -i
	dropdb gis
	createdb -E UTF8 -O gisuser gis
	psql
	\c gis
	CREATE EXTENSION postgis;
	CREATE EXTENSION hstore;
	ALTER TABLE geometry_columns OWNER TO gisuser;
	ALTER TABLE spatial_ref_sys OWNER TO gisuser;
	\q
	exit
	<re-import osm data>

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

golong font
-----------

	croot
	sudo mkdir /usr/share/fonts/truetype/golong
	cd /usr/share/fonts/truetype/golong
	sudo wget http://www.squaregear.net/fonts/golong.zip
	sudo unzip golong.zip

mapnik
------

Download harfbuzz

	croot
	git clone https://github.com/behdad/harfbuzz.git
	cd harfbuzz
	sudo apt-get install autoconf automake libtool pkg-config ragel gtk-doc-tools
	./autogen.sh
	./configure
	make
	sudo make install

Clone mapnik

	croot
	git clone git://github.com/mapnik/mapnik
	cd mapnik
	git checkout -b br-v3.0.9 v3.0.9

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

Fix transparency

	# Not needed any longer for transparency but does cause
	# RGBA rather than INDEXED png images if desired
	# https://lists.openstreetmap.org/pipermail/dev/2008-June/010760.html
	vim gen_tile.cpp
	<replace png256 with png>

Build mod_tile

	./autogen.sh
	./configure
	make
	sudo make install
	sudo make install-mod_tile
	sudo ldconfig

renderd
-------

Configure renderd

	sudo vim /usr/local/etc/renderd.conf
	socketname=/var/run/renderd/renderd.sock
	plugins_dir=/usr/local/lib/mapnik/input
	font_dir=/usr/share/fonts/truetype
	XML=/usr/local/share/maps/style/openstreetmap-carto/osm.xml
	HOST=localhost

	# move mod_tile to home directory due to space
	sudo mkdir /var/run/renderd
	sudo chown gisuser:gisuser /var/run/renderd
	sudo adduser --home /home/gisuser gisuser
	# sudo mkdir /home/gisuser
	# sudo chmod 777 /home/gisuser
	sudo mkdir /home/gisuser/mod_tile
	sudo chown gisuser:gisuser /home/gisuser/mod_tile
	sudo ln -s /home/gisuser/mod_tile /var/lib/mod_tile

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
	cp -aRv /var/lib/postgresql /home/gisuser/postgresql
	vim /etc/postgresql/9.4/main/postgresql.conf

	# change data directory
	data_directory = '/home/gisuser/postgresql/9.4/main'

	pg_ctlcluster 9.4 main restart
	exit

osm data
--------

download planet

	wget http://ftp5.gwdg.de/pub/misc/openstreetmap/planet.openstreetmap.org/pbf/planet-161010.osm.pbf

download osmosis

	# http://wiki.openstreetmap.org/wiki/Osmosis#Latest_stable_version
	croot
	mkdir osmosis
	cd osmosis
	wget http://bretth.dev.openstreetmap.org/osmosis-build/osmosis-latest.tgz
	tar -xzf osmosis-latest.tgz

	sudo apt-get install openjdk-7-jdk

refer to this site to determine lat/lon bounding box

	http://pos-map.appspot.com/en/coordinates10.html

crop planet (e.g.)

	./osmosis/bin/osmosis --read-pbf planet.osm.pbf --bounding-box top=72.0 left=-170.0 bottom=18.0 right=-66.0 --write-xml US.osm
	./osmosis/bin/osmosis --read-pbf planet.osm.pbf --bounding-box top=51.0 left=-126.0 bottom=23.0 right=-64.0 --write-xml US48.osm
	./osmosis/bin/osmosis --read-pbf planet.osm.pbf --bounding-box top=43.0 left=-110.0 bottom=34.0 right=-100.0 --write-xml CO.osm

reformat osm data

	croot
	clean-symbols.sh CO.osm CO-clean-symbols.osm
	clean-osm CO-clean-symbols.osm CO-clean.osm | tee clean.txt

import osm data

	<Recreate database>
	croot
	cd osm2pgsql
	sudo -u gisuser osm2pgsql --slim -d gis ../CO-clean.osm

start renderd
------------

	sudo -u gisuser renderd -f -c /usr/local/etc/renderd.conf
	<in a separate window>
	sudo service apache2 reload

openstreetmap-carto
-------------------

Install project

	git clone git@github.com:jeffboody/openstreetmap-carto.git -b hd_v1
	sudo apt-get install python-yaml curl
	cd openstreetmap-carto
	./get-shapefiles.sh
	sudo mkdir /usr/local/share/maps
	sudo mkdir /usr/local/share/maps/style
	sudo ln -s <path to openstreetmap-carto> /usr/local/share/maps/style/openstreetmap-carto

Generate xml

	./scripts/yaml2mml.py
	carto project.mml > osm.xml

restart renderd

	croot
	cd osm-style/bin

	# may need to recreate /var/run/renderd
	./renderd-setup.conf

	./renderd-restart.sh

slippymap
---------

	Note: must be connected to internet to load slippymap.html

	croot
	cd mod_tile
	sudo mkdir /var/www/html/osm_tiles
	sudo cp slippymap.html /var/www/html/osm_tiles/slippymap.html
	sudo chown -R gisuser:gisuser /var/www/html/osm_tiles

	sudo vim /var/www/html/osm_tiles/slippymap.html

	# change default lat/lon for Boulder, CO
	var lat=40.01476;
	var lon=-105.25331;

	# open in browser
	http://localhost/osm_tiles/slippymap.html
