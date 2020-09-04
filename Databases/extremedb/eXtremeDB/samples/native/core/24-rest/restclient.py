#!/usr/bin/env python

from __future__ import print_function

import httplib
import json
import sys


REST_SRV_HOST = '127.0.0.1'
REST_SRV_PORT = 8083


def api_get(path):
    conn = httplib.HTTPConnection(REST_SRV_HOST, REST_SRV_PORT, timeout=3);
    conn.request('GET', path)
    rsp = conn.getresponse()

    if rsp.status != 200:
        raise RuntimeError('Server returned error {} ({})'.format(
            rsp.status, rsp.reason))

    data = rsp.read()
    conn.close()

    return json.loads(data)


def get_db_stats(db):
    rsp = api_get('/api/db/{}/stat'.format(db))

    print('*', db)

    print('  memory:')
    print('   free pages: ', rsp['mem']['free_pages'])
    print('   total pages:', rsp['mem']['total_pages'])
    print('   page size:  ', rsp['mem']['page_size'])
    print()

    print('  classes:')
    for c in rsp['classes']:
        print('  *', c['table_name'])
        print('    objects:   ', c['objects_num'])
        print('    versions:  ', c['versions_num'])
        print('    core pages:', c['core_pages'])
        print('    blob pages:', c['blob_pages'])
        print('    core space:', c['core_space'])
        print()


def main():
    rsp = api_get('/api/db')

    for db in rsp['databases']:
        get_db_stats(db)

    return 0


if __name__ == '__main__':
    status = main()
    sys.exit(status)
