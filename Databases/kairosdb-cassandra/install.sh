rm kairosdb -R
rm udf/target -R

wget https://github.com/kairosdb/kairosdb/releases/download/v1.2.1/kairosdb-1.2.1-1.tar.gz
tar -xzf kairosdb-1.2.1-1.tar.gz
rm kairosdb-1.2.1-1.tar.gz

cd udf
mvn package
cd ..
cp udf/target/udf-1.0-SNAPSHOT.jar kairosdb/lib/
cp ~/.m2/repository/black/ninia/jep/3.9.0/jep-3.9.0.jar kairosdb/lib

echo "kairosdb.service.udf=master.UdfModule" >> kairosdb/conf/udf.properties
alg_path=$(realpath ../../Algorithms)
echo "kairosdb.udf.kmeans.implementation=${alg_path}/kmeans" >> kairosdb/conf/udf.properties
echo "kairosdb.udf.cd.implementation=${alg_path}/centroid_decomposition" >> kairosdb/conf/udf.properties
echo "kairosdb.udf.recov.implementation=${alg_path}/recovdb" >> kairosdb/conf/udf.properties
