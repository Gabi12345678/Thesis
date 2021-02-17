mkdir dstree/out
sudo chmod 777 dstree/out

wget https://downloads.apache.org/druid/0.19.0/apache-druid-0.19.0-bin.tar.gz
tar -xf apache-druid-0.19.0-bin.tar.gz
rm apache-druid-0.19.0-bin.tar.gz

mkdir apache-druid-0.19.0/extensions/udf

cd udf
mvn package
cp target/udf-1.0-SNAPSHOT.jar ../apache-druid-0.19.0/extensions/udf
cd ..

rm -r apache-druid-0.19.0/conf
cp -r for-install/conf apache-druid-0.19.0/

cp ~/.m2/repository/org/apache/commons/commons-math/2.1/commons-math-2.1.jar apache-druid-0.19.0/lib/
cp ~/.m2/repository/de/ruedigermoeller/fst/1.37/fst-1.37.jar apache-druid-0.19.0/lib/

pip3 install pydruid