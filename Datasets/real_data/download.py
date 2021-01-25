import itertools
import subprocess
import sys

stations = ["bs", "cedarpoint", "disl", "katrinacut", "meaher", "middlebay", "perdido"]
years = range(int(sys.argv[1]), int(sys.argv[2]))

subprocess.run(["rm", "-R", "data"])
subprocess.run(["mkdir", "data"])
for s, y in itertools.product( stations, years ):
    subprocess.run(["wget", "https://arcos.disl.org/main/download_met_annual?file_name=%s_met_%d" % (s, y)])
    subprocess.run(["unzip", "download_met_annual?file_name=%s_met_%d" % (s, y)])
    subprocess.run(["rm", "download_met_annual?file_name=%s_met_%d" % (s, y)])
    subprocess.run(["mv", "%s_met_%d.txt" % (s, y), "./data/"])
