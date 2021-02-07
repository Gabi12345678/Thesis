#sudo rm docker-graphite-git -R
#sudo rm storage -R
#sudo rm for-install/udf/* -R

mkdir docker-graphite-git
cd docker-graphite-git
git clone https://github.com/graphite-project/docker-graphite-statsd.git
cd docker-graphite-statsd

sed -i '/psycopg2/a\ saxpy numpy setuptools\ \\' Dockerfile
sed -i 's/go==1.13.11-r0/go==1.13.14-r0/' Dockerfile

cp -f ../../for-install/storage-schemas.conf ./conf/opt/graphite/conf/
cp -f ../../for-install/carbon.conf ./conf/opt/graphite/conf/
cp -f ../../for-install/udf.py ../../for-install/udf
cp -f ../../for-install/local_settings.py ./conf/opt/graphite/webapp/graphite/
cp -f ../../for-install/graphite-statsd.conf ./conf/etc/nginx/sites-enabled/
cp -f ../../for-install/graphite-run ./conf/etc/service/graphite/run
cp -f ../../../../Algorithms/centroid_decomposition/cd_ssv.py ../../for-install/udf/
cp -f ../../../../Algorithms/kmeans/kmeans.py ../../for-install/udf/
cp -f ../../../../Algorithms/recovdb/recovery.py ../../for-install/udf/
cp -f ../../../../Algorithms/hot_sax/hotsax.py ../../for-install/udf/
cp -f ../../../../Algorithms/znormalization/znormalization.py ../../for-install/udf/
cp -f ../../../../Algorithms/screen_python/screen.py ../../for-install/udf/
cp -f ../../../../Algorithms/sax/saxtransformation.py ../../for-install/udf/
cp -f -R ../../../../Algorithms/dstree_python/dstree ../../for-install/udf/
cp -f ../../../../Algorithms/knn/knn.py ../../for-install/udf/

sudo su <<START
echo "Building with Docker"
docker build --tag custom-graphite --build-arg python_binary=python3 .
echo "Killing graphite"
docker kill graphite
echo "Removing Graphite"
docker rm graphite
echo "Restarting graphite"

docker run -d \
 --name graphite \
 --restart=always \
 -p 80:80 \
 -p 2003-2004:2003-2004 \
 -p 2023-2024:2023-2024 \
 -p 8125:8125/udp \
 -p 8126:8126 \
 -v $PWD/../../storage:/opt/graphite/storage \
 -v $PWD/../../for-install/udf:/opt/graphite/webapp/graphite/functions/custom \
 -v $PWD/../../for-install/log:/var/log \
 custom-graphite

echo "y" | docker image prune
START
