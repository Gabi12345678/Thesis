#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include "inetdb.hpp"
#include "mcotime.h"
#include "common.h"

using namespace inetdb;

const char * dbName = "inetdb";

#define MAX_HOST_LENGTH 50
#define MAX_AGENT_LENGTH 100
#define MAX_HOSTS 100

#ifdef DISK_DATABASE
//#define DATABASE_SEGMENT_SIZE  1024*1024*1024
#define DATABASE_SEGMENT_SIZE  512*1024*1024
#else
//#define DATABASE_SEGMENT_SIZE  8L*1024*1024*1024
#define DATABASE_SEGMENT_SIZE  1024*1024*1024
#endif
//#define CACHE_SEGMENT_SIZE  8L*1024*1024*1024
#define CACHE_SEGMENT_SIZE  512*1024*1024
#define INPUT_BUF_SIZE 64*1024

uint4 string_hash(char const* s) 
{
    uint4 h = 0;
    unsigned char const* p = (unsigned char const*)s;
    while (*p != 0) { 
        h = (h*31) + *p++;
    }
    return h;
}

uint4 load_database(mco_db_h db)
{
    mco_trans_h trans;
    char buf[INPUT_BUF_SIZE];
    
    uint4 day_buf[MCO_SEQ_TILE_SIZE];
    char user_buf[MCO_SEQ_TILE_SIZE*16];
    uint4 ip_buf[MCO_SEQ_TILE_SIZE];
    uint4 host_buf[MCO_SEQ_TILE_SIZE];
    uint4 url_buf[MCO_SEQ_TILE_SIZE];
    uint4 agent_buf[MCO_SEQ_TILE_SIZE];

    uint4 j = 0;
    uint4 n_events = 0;
    uint4 n_hosts = 0;
    uint4 n_browsers = 0;
    Event event;
    time_t start = time(NULL);
    int line = 0;

    CHECK(mco_trans_start(db, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &trans));
    CHECK(event.create(trans));

    while (fgets(buf, sizeof(buf), stdin)) { 
        if (strncmp(buf, "set", 3) != 0) { 
            char host_name[256];            
            char url[32*1024];            
            char user_hash[64];
            int ip[4];
            int year, month, day;
            int hash2;
            int n;
            int rc = sscanf(buf, "%d-%d-%d ! %s ! %d.%d.%d.%d ! %s ! %s ! %d ! %n", &year, &month, &day, user_hash, 
                            &ip[0], &ip[1], &ip[2], &ip[3], host_name, url, &hash2, &n);            
            line += 1;
            if (rc != 11) {
                fprintf(stderr, "Bad line %u: '%s'\n", line, buf);
            } else {
                char* agent = buf + n;
                char* dst = agent;
                char* src = agent;
                Browser browser;
                Host host;
                uint4 host_id;
                uint4 browser_id;
                unsigned long user_words[2] = {0,0};

                sscanf(user_hash, "%16lx%16lx", &user_words[0], &user_words[1]);
                do { 
                    if (*src != '\"') { 
                        *dst++ = *src;
                    }
                } while (*src++ != '\0');
                
                host_name[MAX_HOST_LENGTH-1] = '\0';
                agent[MAX_AGENT_LENGTH-1] = '\0';
                
                rc = Host::by_name::find(trans, host_name, strlen(host_name), host);
                if (rc == MCO_S_NOTFOUND) { 
                    host_id = ++n_hosts;
                    CHECK(host.create(trans));                    
                    host.id = host_id;
                    host.name = host_name;
                    CHECK(host.checkpoint());
                } else {
                    host_id = host.id;
                }
                rc = Browser::by_agent::find(trans, agent, strlen(agent), browser);
                if (rc == MCO_S_NOTFOUND) { 
                    browser_id = ++n_browsers;
                    CHECK(browser.create(trans));                    
                    browser.id = browser_id;
                    browser.agent = agent;
                    CHECK(browser.checkpoint());
                } else {
                    browser_id = browser.id;
                }
                if (j == MCO_SEQ_TILE_SIZE) { 
                    CHECK(event.day_append(day_buf, j));
                    CHECK(event.user_append(user_buf, j));
                    CHECK(event.ip_append(ip_buf, j));
                    CHECK(event.host_append(host_buf, j));
                    CHECK(event.url_append(url_buf, j));
                    CHECK(event.agent_append(agent_buf, j));
                    j = 0;
                }
                day_buf[j] = (year*12 + month)*31 + day;
                memcpy(user_buf + j*16, user_words, 16);
                ip_buf[j] = (ip[0] << 24) | (ip[1] << 16) | (ip[2] << 8) | ip[3];
                host_buf[j] = host_id;
                url_buf[j] = string_hash(url);
                agent_buf[j] = browser_id;
                j += 1;
                n_events += 1;
                if (n_events % 1000000 == 0) { 
                    printf("Load %u events, %u hosts, %u browsers in %ld seconds...\r", n_events, n_hosts, n_browsers, (long)(time(NULL) - start));
                    fflush(stdout);
                }                
            }
        }
    }
    CHECK(event.day_append(day_buf, j));
    CHECK(event.user_append(user_buf, j));
    CHECK(event.ip_append(ip_buf, j));
    CHECK(event.host_append(host_buf, j));
    CHECK(event.url_append(url_buf, j));
    CHECK(event.agent_append(agent_buf, j));
    
    CHECK(mco_trans_commit(trans));
    printf("Load %u events, %u hosts, %u browsers in %ld seconds\n", n_events, n_hosts, n_browsers, (long)(time(NULL) - start));
    return n_hosts;
}

void generate_reports(mco_db_h db, uint4 n_hosts)
{
    mco_trans_h trans;
    mco_cursor_t cursor;
    Event event;

    uint8 count;
    uint4 host_id;
    Host host;
    std::string host_name;
    FILE* f;
    time_t start;
    uint4 i;
    uint4 n_ip_host_pairs;
    uint4 n_url_host_pairs;
    uint4 n_user_host_pairs;
    uint4 n_agent_host_pairs;

    CHECK(mco_trans_start(db, MCO_READ_ONLY, MCO_TRANS_FOREGROUND, &trans));
    CHECK(Event::list_cursor(trans, &cursor));
    CHECK(mco_cursor_first(trans, &cursor));
    CHECK(event.from_cursor(trans, &cursor));


    /*
     * --- Unique IPs per host -----------------------------------------------------
     */
 
    {
        Sequence<uint8> count_iterator;
        Sequence<uint4> group_by_iterator;
        start = time(NULL);
        event.ip_iterator().hashAggDistinctCount(count_iterator, group_by_iterator, event.host_iterator());
        f = fopen("uniq_ips_per_host.res", "w");
        n_ip_host_pairs = 0;
        while (group_by_iterator.next(host_id)) {
            count = ++count_iterator;
            CHECK(Host::by_id::find(trans, host_id, host));
            host_name = host.name;
            fprintf(f, "%s,%lu\n", host_name.c_str(), (long)count);
            n_ip_host_pairs += count;
        }
        fclose(f);
        printf("Real number of <ip,host> pairs: %u\n", n_ip_host_pairs);
        printf("Elapsed time for unique IPs per host report is %lu seconds\n", (long)(time(NULL) - start));
    }

    /*
     * --- All events per host -----------------------------------------------------
     */

    {
        Sequence<uint8> count_iterator;
        Sequence<uint4> group_by_iterator;
        std::vector<uint4> host_ids;
        std::vector<uint8> event_counts;
        std::vector<mco_seq_no_t>permutation;
        start = time(NULL);
        event.host_iterator().hashAggCount(count_iterator, group_by_iterator);
        f = fopen("events_per_host.res", "w");
        group_by_iterator.get(host_ids);
        assert(host_ids.size() == n_hosts);
        count_iterator.get(event_counts);
        assert(event_counts.size() == n_hosts);
        Sequence<uint8>::sort(event_counts, permutation, MCO_SEQ_DESC_ORDER);
        for (i = 0; i < permutation.size(); i++) { 
            CHECK(Host::by_id::find(trans, host_ids[permutation[i]], host));
            host_name = host.name;
            fprintf(f, "%s,%lu\n", host_name.c_str(), (long)event_counts[permutation[i]]);
        }
        fclose(f);
        printf("Elapsed time for event per host report is %lu seconds\n", (long)(time(NULL) - start));
    }
    /*
     * --- Unique requests per host -----------------------------------------------------
     */    
    {        
        Sequence<uint8> count_iterator;
        Sequence<uint4> group_by_iterator;
        start = time(NULL);
        event.url_iterator().hashAggDistinctCount(count_iterator, group_by_iterator, event.host_iterator());
        f = fopen("uniq_reqs_per_host.res", "w");
        n_url_host_pairs = 0;
        while (group_by_iterator.next(host_id)) {
            count = ++count_iterator;
            CHECK(Host::by_id::find(trans, host_id, host));
            host_name = host.name;
            fprintf(f, "%s,%lu\n", host_name.c_str(), (long)count);
            n_url_host_pairs += count;
        }
        fclose(f);
        printf("Real number of <url,host> pairs: %u\n", n_url_host_pairs);
        printf("Elapsed time for unique requests per host report is %lu seconds\n", (long)(time(NULL) - start));
    }
    /*
     * --- Unique agents per host -----------------------------------------------------
     */
    {        
        Sequence<uint8> count_iterator;
        Sequence<uint4> group_by_iterator;
        start = time(NULL);
        event.agent_iterator().hashAggDistinctCount(count_iterator, group_by_iterator, event.host_iterator());
        f = fopen("uniq_agents_per_host.res", "w");
        n_agent_host_pairs = 0;
        while (group_by_iterator.next(host_id)) {
            count = ++count_iterator;
            CHECK(Host::by_id::find(trans, host_id, host));
            host_name = host.name;
            fprintf(f, "%s,%lu\n", host_name.c_str(), (long)count);
            n_agent_host_pairs += count;
        }
        fclose(f);
        printf("Real number of <agent,host> pairs: %u\n", n_agent_host_pairs);
        printf("Elapsed time for unique agents per host report is %lu seconds\n", (long)(time(NULL) - start));
    }

    /*
     * --- Unique users per host -----------------------------------------------------
     */
    {
        Sequence<uint8> count_iterator;
        Sequence<uint4> group_by_iterator;
        start = time(NULL);
        event.user_iterator().hashAggDistinctCount(count_iterator, group_by_iterator, event.host_iterator());
        f = fopen("users_per_host.res", "w");
        n_user_host_pairs = 0;
        while (group_by_iterator.next(host_id)) { 
            count = ++count_iterator;
            CHECK(Host::by_id::find(trans, host_id, host));
            host_name = host.name;
            fprintf(f, "%s,%lu\n", host_name.c_str(), (long)count);
            n_user_host_pairs += count;
        }
        fclose(f);
        printf("Real number of <user,host> pairs: %u\n", n_user_host_pairs);
        printf("Elapsed time for unique users per host report is %lu seconds\n", (long)(time(NULL) - start));
    }

    /*
     * --- Frequenters (users visiting host more than once) per host -----------------------------------------------------
     */
    {
        Sequence<uint8> count_iterator;
        Sequence<uint4> group_by_iterator;
        start = time(NULL);
        event.user_iterator().hashAggDupCount(count_iterator, group_by_iterator, event.host_iterator(), 2);
        f = fopen("frequenters_per_host.res", "w");
        while (group_by_iterator.next(host_id)) { 
            count = ++count_iterator;
            CHECK(Host::by_id::find(trans, host_id, host));
            host_name = host.name;
            fprintf(f, "%s,%lu\n", host_name.c_str(), (long)count);
            n_user_host_pairs += count;
        }
        fclose(f);
        printf("Elapsed time for frequenters per host report is %lu seconds\n", (long)(time(NULL) - start));
    }
    CHECK(mco_trans_commit(trans));
}

void dump_mem(mco_db_h db)
{
    mco_size32_t total_pages;
    mco_size32_t free_pages;
    CHECK(mco_db_free_pages(db, &free_pages));
    CHECK(mco_db_total_pages(db, &total_pages));
    printf("Use %d pages\n", (int)(total_pages - free_pages));
}

int main(int argc, char *argv[])
{
    mco_runtime_info_t info;
    mco_device_t dev[4];
    mco_db_params_t db_params;
    mco_db_h db;
    int n_devs = 1;
    uint4 n_hosts;

    mco_get_runtime_info(&info);
    CHECK(mco_runtime_start());
    mco_db_params_init(&db_params);

    dev[0].assignment = MCO_MEMORY_ASSIGN_DATABASE;
    dev[0].size       = DATABASE_SEGMENT_SIZE;
    if (info.mco_shm_supported)  {
        dev[0].type       = MCO_MEMORY_NAMED;
        sprintf( dev[0].dev.named.name, "%s-db", dbName );
        dev[0].dev.named.flags = 0;
        dev[0].dev.named.hint  = DATABASE_MAP_ADDRESS;
    } else { 
        dev[0].type       = MCO_MEMORY_CONV;
        dev[0].dev.conv.ptr = malloc(DATABASE_SEGMENT_SIZE);
    }

#ifdef DISK_DATABASE
    dev[1].assignment = MCO_MEMORY_ASSIGN_CACHE;
    dev[1].size       = CACHE_SEGMENT_SIZE;
    if (info.mco_shm_supported)  {
        dev[1].type       = MCO_MEMORY_NAMED;
        sprintf( dev[1].dev.named.name, "%s-cache", dbName );
        dev[1].dev.named.flags = 0;
        dev[1].dev.named.hint  = (char*)DATABASE_MAP_ADDRESS + DATABASE_SEGMENT_SIZE;
    } else { 
        dev[1].type       = MCO_MEMORY_CONV;
        dev[1].dev.conv.ptr = malloc(CACHE_SEGMENT_SIZE);
    }        
    dev[2].type       = MCO_MEMORY_FILE;
    dev[2].assignment = MCO_MEMORY_ASSIGN_PERSISTENT;
    sprintf( dev[2].dev.file.name, "%s.dbs", dbName );
    dev[2].dev.file.flags = MCO_FILE_OPEN_DEFAULT;

    dev[3].type       = MCO_MEMORY_FILE;
    dev[3].assignment = MCO_MEMORY_ASSIGN_LOG;
    sprintf( dev[3].dev.file.name, "%s.log", dbName );
    dev[3].dev.file.flags = MCO_FILE_OPEN_DEFAULT;
    
    remove(dev[2].dev.file.name);
    remove(dev[3].dev.file.name);

    n_devs += 3;

    db_params.mem_page_size = 256;
    db_params.disk_page_size = 4096;
    db_params.db_log_type = UNDO_LOG;
#else
    db_params.mem_page_size = 4096;
    db_params.disk_page_size = 0;
#endif 

    CHECK(mco_db_open_dev(dbName, inetdb_get_dictionary(), dev, n_devs, &db_params));
    CHECK(mco_db_connect(dbName, &db));
    
    n_hosts = load_database(db);
    generate_reports(db, n_hosts);

    dump_mem(db);

    if (!info.mco_shm_supported)  {
        CHECK(mco_db_disconnect(db));
        CHECK(mco_db_close(dbName));
    }
    return 0;
}
