mkdir dstree/out
sudo chmod 777 dstree/out

wget https://dl.influxdata.com/influxdb/releases/influxdb-1.7.10_linux_amd64.tar.gz
tar xvfz influxdb-1.7.10_linux_amd64.tar.gz
rm influxdb-1.7.10_linux_amd64.tar.gz

wget https://dl.influxdata.com/kapacitor/releases/kapacitor-1.5.4_linux_amd64.tar.gz
tar xvfz kapacitor-1.5.4_linux_amd64.tar.gz
rm kapacitor-1.5.4_linux_amd64.tar.gz

wget https://github.com/influxdata/kapacitor/archive/master.zip
unzip master.zip
rm master.zip
