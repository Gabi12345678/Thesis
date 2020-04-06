from datetime import datetime
from tqdm import tqdm

lines = 1000
columns = 1000
data_path='/home/gabi/influx/dataset/synth_1K.txt'



udf_template_path = '/home/gabi/monetdb/code2/cd_udf_template.sql'
tmp_data_path = '/home/gabi/monetdb/code2/data.csv'
udf_path = '/home/gabi/monetdb/code2/udf.sql'
start_time=1583000000

print("Generating data file")
f = open(data_path, "r")
g = open(tmp_data_path, "w")
for i in tqdm(range(lines)):
        values = f.readline()[:-1].split(" ")
        time = datetime.fromtimestamp(start_time + i * 10).strftime("%Y-%m-%d %H:%M:%S")
        values = [time] + values[:columns]
        g.write(",".join(values) + "\n")
f.close()
g.close()

print("Generating UDF")
f = open(udf_template_path, "r")
g = open(udf_path, "w")

types_string = ""
for i in range(columns):
        if i != 0:
                types_string = types_string + ", "
        types_string = types_string + "d" + str(i) + " FLOAT"
column_names = ""
for i in range(columns):
        if i != 0:
                column_names = column_names + ", "
        column_names = column_names + "d" + str(i)
for l in f.readlines():
        g.write( l.replace("<types>", types_string).replace("<column_names>", column_names).replace("<lines>", str(lines)).replace("<columns>", str(columns)) )
f.close()
g.close()
