import subprocess
from datetime import datetime
from tqdm import tqdm
from influxdb import InfluxDBClient
import argparse
import os

# Arguments
parser = argparse.ArgumentParser(description = 'Script to run K-Means in Influx')
parser.add_argument('--file', nargs='?', type=str, help='path to the dataset file', default='../../dataset/synth_1K.txt')
parser.add_argument('--lines', nargs='*', type=int, required=True,
        help='list of integers representing the number of lines to try out. Used together with --columns. For example "--lines 10 20 --columns 2 4" will try (10, 2), (10, 4), (20, 2), (20, 4)')
parser.add_argument('--columns', nargs='*', type=int, required=True,
        help='list of integers representing the number of columns to try out. Used together with --lines. For example "--lines 10 20 --columns 2 4" will try (10, 2), (10, 4), (20, 2), (20, 4)')
parser.add_argument('--tick_time', nargs='?', type=str, help="Time interval at which to run the UDF. This has to be strictly larger than the actual running time to not start a second Kmeans in the meantime.", default="10s")
parser.add_argument('--start_time', nargs='?', type=int, help = "Epoch time in second for the first measurement. All others will be 10 seconds apart", default=1583000000)

args = parser.parse_args()

args.file = os.path.abspath(args.file)
args.home_path = os.path.expanduser("~")
args.db_directory = os.path.abspath(args.home_path + '/.influxdb')
args.kapacitor_path = os.path.abspath('../kapacitor-1.5.4-1/usr/bin/kapacitor')
args.kapacitord_path = os.path.abspath('../kapacitor-1.5.4-1/usr/bin/kapacitord')
args.kapacitor_config_template_path = os.path.abspath("template_kapacitor.config")
args.kapacitor_config_path = os.path.abspath('kapacitor.config')
args.kmeans_handler_template_path = os.path.abspath('template_kmeans_handler.py')
args.kmeans_handler_path = os.path.abspath('kmeans_handler.py')
args.influxd_path = os.path.abspath('../influxdb-1.7.10-1/usr/bin/influxd')
args.tick_template_path = os.path.abspath('template_udf.tick')
args.tick_path = os.path.abspath('udf.tick')
args.implementation_path = os.path.abspath('../../kmeans')
args.kapacitor_library = os.path.abspath('../kapacitor-master/udf/agent/py')

def generate_from_template(template_path, file_path, changes):
        f = open(template_path, "r")
        g = open(file_path, "w")
        for line in f.readlines():
                new_line = line
                for before, after in changes:
                        new_line = new_line.replace(before, after)
                g.write(new_line)
        f.close()
        g.close()

generate_from_template(template_path = args.kapacitor_config_template_path, file_path = args.kapacitor_config_path,
                        changes = [("<handler_path>", args.kmeans_handler_path), ("<home_path>", args.home_path)])
generate_from_template(template_path = args.tick_template_path, file_path = args.tick_path,
                        changes = [("<tick_time>", args.tick_time)])
generate_from_template(template_path = args.kmeans_handler_template_path, file_path = args.kmeans_handler_path,
                        changes = [("<implementation>", args.implementation_path), ("<kapacitor_library>", args.kapacitor_library)])

influx = subprocess.Popen([args.influxd_path], stdout = subprocess.DEVNULL, stderr = subprocess.PIPE)
for line in influx.stderr:
        if 'Listening on HTTP' in str(line):
                print("Influx is ready to receive data")
                influx_devnull = subprocess.Popen(["cat"], stdin = influx.stderr, stdout = subprocess.DEVNULL)
                break

kapacitor = subprocess.Popen([args.kapacitord_path, "-config", args.kapacitor_config_path], stdout = subprocess.DEVNULL, stderr = subprocess.PIPE)
for l in kapacitor.stderr:
        if 'starting HTTP service' in str(l):
                print("Kapacitor is ready to receive data")
                kapacitor_udf = subprocess.Popen(["grep", "udf:"], stdin = kapacitor.stderr, stdout = subprocess.PIPE)
                break
subprocess.run([args.kapacitor_path, "disable", "udf"])

# Function to get database storage roon
def get_size(start_path = args.db_directory):
        total_size = 0
        for dirpath, dirnames, filenames in os.walk(start_path):
                for f in filenames:
                        fp = os.path.join(dirpath, f)
                        # skip if it is symbolic link
                        if not os.path.islink(fp):
                                total_size += os.path.getsize(fp)
        return total_size
        
for line in args.lines:
        for column in args.columns:
                print("Running UDF for (" + str(line) + ", " + str(column) + ")")

                client = InfluxDBClient(database = 'master')
                client.drop_database('master')

                initial_size = get_size()
                client.create_database('master')
                initial_time = datetime.now()

                f = open(args.file, "r")
                l = []
                for x in tqdm(range(line)):
                        s = f.readline()[:-1].split(" ")
                        v = {}
                        for y in range(column):
                                v["dim" + str(y)] = float(s[y])
                        body = {"measurement": "puncte", "time": (args.start_time + 10 * x) * 1000000000, "fields": v }
                        l.append(body)
                        if len(l) == 100:
                                client.write_points(l)
                                l = []
                client.write_points(l)

                final_time = datetime.now()
                final_size = get_size()

                seconds = (final_time - initial_time).total_seconds()

                print("Memory in bytes: ", final_size - initial_size)
                print("Memory in megabytes: ", float(final_size - initial_size) / 1024.0 / 1024.0)
                print("Througput: ", line / seconds)


                subprocess.run([args.kapacitor_path, "define", "udf", "-tick", args.tick_path])
                subprocess.run([args.kapacitor_path, "enable", "udf"])
                for l in kapacitor_udf.stdout:
                        print(l)
                        if 'Total time' in str(l):
                                break
                subprocess.run([args.kapacitor_path, "disable", "udf"])


kapacitor.terminate()
kapacitor_udf.terminate()

influx.terminate()
influx_devnull.terminate()

subprocess.run(["rm", args.kapacitor_config_path])
subprocess.run(["rm", args.tick_path])
subprocess.run(["rm", args.kmeans_handler_path])
