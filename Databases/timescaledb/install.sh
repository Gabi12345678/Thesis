mkdir dstree/out
sudo chmod 777 dstree/out

sudo apt update && sudo apt -y upgrade

wget --quiet -O - https://www.postgresql.org/media/keys/ACCC4CF8.asc | sudo apt-key add -
RELEASE=$(lsb_release -cs)
echo "deb http://apt.postgresql.org/pub/repos/apt/ ${RELEASE}"-pgdg main | sudo tee  /etc/apt/sources.list.d/pgdg.list

sudo apt update
sudo apt install -y postgresql-11

sudo su - postgres <<EOF
echo "Setting password 'postgres' to user 'postgres'"
psql -c "alter user postgres with password 'postgres'"
createdb master -O postgres
EOF

sudo add-apt-repository -y ppa:timescale/timescaledb-ppa
sudo apt-get update

sudo apt install -y timescaledb-postgresql-11
sudo apt install -y postgresql-plpython-11

sudo -u postgres createuser $USER
sudo -u postgres createdb $USER
sudo -u postgres psql -c "ALTER USER $USER WITH SUPERUSER;"

sudo -u postgres sh -c "echo \"shared_preload_libraries = 'timescaledb'\" >> /etc/postgresql/11/main/postgresql.conf"

echo "Please be ready to input the password. Press [ENTER] to continue. If the password request times out, please run the following command separately \"sudo -u postgres /etc/init.d/postgresql restart\""
read line

sudo -u postgres /etc/init.d/postgresql restart
