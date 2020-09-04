#!/usr/bin/python

import os
import sys
import signal
import subprocess
import time

def Shutdown():
    for p in children:
        p.send_signal(signal.SIGTERM)

def CheckVars(vars):
    for v in vars:
        if not v in os.environ:
            print '%s environment variable not set' % v
            sys.exit(-1)

print "Starting iotnotify test"

test_name='iotnotify'
n_devices=3
server_script = "server.py"
device_script = "device.py"

signal.signal(signal.SIGTERM, Shutdown)
children = []
logfiles=[]


def run_test():
    err=0
    print "Starting server process %s ..." % server_script

    srv_fp = open("server.log", 'w')
    srv_proc = subprocess.Popen([sys.executable, server_script], stdout=srv_fp, stderr=srv_fp, stdin=subprocess.PIPE)
    logfiles.append("server.log")
    children.append(srv_proc)

    time.sleep(1)

    retcode = srv_proc.poll()
    if retcode:
        print "Server startup failed with code %s" % retcode
        return retcode

    for dev in range(n_devices):
        print "Starting device %d ..." % dev

        dev_fp = open("device_%d.log" % dev, 'w')
        p = subprocess.Popen([sys.executable, device_script, "127.0.0.1:15000", str(1000 + dev)], stdout=dev_fp, stderr=dev_fp)
        children.append(p)
        logfiles.append("device_%d.log" % dev)

    time.sleep(3)
    srv_proc.communicate("\n\n\n")

    for p in children:
        if p.wait() != 0:
            err = p.returncode;
            print "Exit code %d" % err
    return err


def show_logs():
    for log_file in logfiles:
        print ''
        print "##### Log file %s #####" % log_file
        print ''
        fp = open (log_file, 'r')
        for line in fp:
            print line
        fp.close()
        os.remove(log_file)


if __name__ == '__main__':
    ret = run_test()
    show_logs()
    sys.exit(ret)

