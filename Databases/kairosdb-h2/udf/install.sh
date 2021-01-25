mvn package
rm ../kairosdb/lib/udf-1.0-SNAPSHOT.jar
cp target/udf-1.0-SNAPSHOT.jar ../kairosdb/lib/
