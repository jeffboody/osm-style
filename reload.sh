service renderd stop
rm -rf /home/osm/mod_tile/default
cp osm.xml /etc/mapnik-osm-carto-data
service renderd start
