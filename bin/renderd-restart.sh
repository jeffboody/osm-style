sudo rm -rf /home/gisuser/mod_tile/default
sudo service apache2 restart
sudo -u gisuser renderd -f -c /usr/local/etc/renderd.conf
