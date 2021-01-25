#!/bin/sh

sudo su <<START
pip install saxpy
pip3 install saxpy
pip install pydruid
pip3 install pydruid
pip install pandas
pip3 install pandas
pip install protobuf
pip3 install protobuf
pip install tqdm
pip3 install tqdm
pip install influxdb
pip3 install influxdb

apt update

apt install docker
apt install maven

update-java-alternatives --jre-headless --jre --set java-1.8.0-openjdk-amd64

START

cd influx
./install.sh
cd ..

cd timescaledb
./timescaledb
cd ..
