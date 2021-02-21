cat splits/datasets_split.* > datasets.tar.gz
tar -zxvf datasets.tar.gz
mv datasets/* ./
rm datasets -r
rm datasets.tar.gz
