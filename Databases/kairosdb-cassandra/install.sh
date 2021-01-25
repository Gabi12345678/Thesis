rm kairosdb -R
rm udf/target -R

wget https://github.com/kairosdb/kairosdb/releases/download/v1.2.1/kairosdb-1.2.1-1.tar.gz
tar -xzf kairosdb-1.2.1-1.tar.gz
rm kairosdb-1.2.1-1.tar.gz

cd udf
mvn package
cd ..
cp udf/target/udf-1.0-SNAPSHOT.jar kairosdb/lib/
cp ~/.m2/repository/org/apache/commons/commons-math3/3.6.1/commons-math3-3.6.1.jar kairosdb/lib/
cp ~/.m2/repository/org/apache/commons/commons-lang3/3.0/commons-lang3-3.0.jar kairosdb/lib/
cp ~/.m2/repository/commons-io/commons-io/2.3/commons-io-2.3.jar kairosdb/lib/
cp ~/.m2/repository/org/apache/commons/commons-math/2.1/commons-math-2.1.jar kairosdb/lib/
cp ~/.m2/repository/de/ruedigermoeller/fst/1.37/fst-1.37.jar kairosdb/lib/

echo "kairosdb.service.udf=master.UdfModule" >> kairosdb/conf/udf.properties

sudo docker stop cassandra
sudo docker rm cassandra
sudo docker run --name cassandra -p 9042:9042 -d cassandra:latest

rm kairosdb/conf/kairosdb.properties
cp for-install/kairosdb.properties kairosdb/conf/ 
