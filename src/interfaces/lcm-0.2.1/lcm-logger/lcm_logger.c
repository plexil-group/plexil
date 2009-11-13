#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <inttypes.h>
#include <getopt.h>

#include <glib.h>

#include <lcm/lcm.h>

#include "glib_util.h"

#define DEFAULT_MAX_WRITE_QUEUE_SIZE_MB 100

GMainLoop *_mainloop;

static inline int64_t timestamp_seconds(int64_t v)
{
    return v/1000000;
}

static inline int64_t timestamp_now()
{
    struct timeval tv;
    gettimeofday (&tv, NULL);
    return (int64_t) tv.tv_sec * 1000000 + tv.tv_usec;
}

typedef struct logger logger_t;
struct logger
{
    lcm_eventlog_t *log;

    char    fname[PATH_MAX];
    lcm_t    *lcm;

    int64_t max_write_queue_size;

    GThread *write_thread;
    GAsyncQueue *write_queue;
    GMutex * mutex;

    // these members controlled by mutex
    int64_t write_queue_size;
    int write_thread_exit_flag;

    // these members controlled by write thread
    int64_t nevents;
    int64_t logsize;
    int64_t events_since_last_report;
    int64_t last_report_time;
    int64_t last_report_logsize;
    int64_t time0;

    int64_t dropped_packets_count;
    int64_t last_drop_report_utime;
    int64_t last_drop_report_count;
};

static void * 
write_thread(void *user_data)
{
    logger_t *logger = (logger_t*) user_data;

    while(1) {
        void *msg = g_async_queue_pop(logger->write_queue);

        // Should the write thread exit?
        g_mutex_lock(logger->mutex);
        if(logger->write_thread_exit_flag) {
            g_mutex_unlock(logger->mutex);
            return NULL;
        }
        g_mutex_unlock(logger->mutex);

        // nope.  write the event to disk
        lcm_eventlog_event_t *le = (lcm_eventlog_event_t*) msg;

        lcm_eventlog_write_event(logger->log, le);

        // bookkeeping, cleanup
        int64_t offset_utime = le->timestamp - logger->time0;
        logger->nevents++;
        logger->events_since_last_report ++;
        logger->logsize += 4 + 8 + 8 + 4 + le->channellen + 4 + le->datalen;

        int64_t sz = sizeof(lcm_eventlog_t) + le->channellen + 1 + le->datalen;
        free(le);
        g_mutex_lock(logger->mutex);
        logger->write_queue_size -= sz;
        g_mutex_unlock(logger->mutex);

        if (offset_utime - logger->last_report_time > 1000000) {
            double dt = (offset_utime - logger->last_report_time)/1000000.0;

            double tps =  logger->events_since_last_report / dt;
            double kbps = (logger->logsize - logger->last_report_logsize) / dt / 1024.0;

            printf("Summary: %s ti:%4"PRIi64"sec Events: %-9"PRIi64" ( %4"PRIi64" MB )      TPS: %8.2f       KB/s: %8.2f\n", 
                    logger->fname,
                    timestamp_seconds(offset_utime),
                    logger->nevents, logger->logsize/1048576, 
                    tps, kbps);
            logger->last_report_time = offset_utime;
            logger->events_since_last_report = 0;
            logger->last_report_logsize = logger->logsize;
        }
    }
}

static void 
message_handler (const lcm_recv_buf_t *rbuf, const char *channel, void *u)
{
    logger_t *logger = (logger_t*) u;

    int channellen = strlen(channel);

    // check if the backlog of unwritten messages is too big.  If so, then
    // ignore this event
    int64_t mem_sz = sizeof(lcm_eventlog_event_t) + channellen + 1 + rbuf->data_size;
    g_mutex_lock(logger->mutex);
    int64_t mem_required = mem_sz + logger->write_queue_size;

    if(mem_required > logger->max_write_queue_size) {
        // can't write to logfile fast enough.  drop packet.
        g_mutex_unlock(logger->mutex);

        // maybe print an informational message to stdout
        int64_t now = timestamp_now();
        logger->dropped_packets_count ++;
        int rc = logger->dropped_packets_count - logger->last_drop_report_count;

        if(now - logger->last_drop_report_utime > 1000000 && rc > 0) {
            printf("Can't write to log fast enough.  Dropped %d packet%s\n", 
                    rc, rc==1?"":"s");
            logger->last_drop_report_utime = now;
            logger->last_drop_report_count = logger->dropped_packets_count;
        }
        return;
    } else {
        logger->write_queue_size = mem_required;
        g_mutex_unlock(logger->mutex);
    }

    // queue up the message for writing to disk by the write thread
    lcm_eventlog_event_t *le = (lcm_eventlog_event_t*) malloc(mem_sz);
    memset(le, 0, mem_sz);

    le->timestamp = rbuf->recv_utime;
    le->channellen = channellen;
    le->datalen = rbuf->data_size;
    // log_write_event will handle le.eventnum.

    le->channel = ((char*)le) + sizeof(lcm_eventlog_event_t);
    strcpy(le->channel, channel);
    le->data = le->channel + channellen + 1;
    assert((char*)le->data + rbuf->data_size == (char*)le + mem_sz);
    memcpy(le->data, rbuf->data, rbuf->data_size);

    g_async_queue_push(logger->write_queue, le);
}

static void usage ()
{
    fprintf (stderr, "usage: lcm-logger [options] [FILE]\n"
            "\n"
            "    LCM message logging utility.  Subscribes to all channels on an LCM\n"
            "    network, and records all messages received on that network to\n"
            "    FILE.  If FILE is not specified, then a filename is automatically\n"
            "    chosen.\n"
            "\n"
            "Options:\n"
            "\n"
            "  -f, --force                Overwrite existing files\n"
            "  -i, --increment            Automatically append a suffix to FILE\n"
            "                             such that the resulting filename does not\n"
            "                             already exist.  This option precludes -f\n"
            "  -c, --channel              Channel string to pass to lcm_subscribe.\n"
            "                             (default: \".*\")\n"
            "  -s, --strftime             Format FILE with strftime.\n" 
            "  -m, --max-unwritten-mb K   Maximum size of received but unwritten\n"
            "                             messages to store in memory before dropping\n"
            "                             messages.  (default: 100 MB)\n"
            "  -h, --help                 Shows this help text and exits\n"
            "\n");
}

int main(int argc, char *argv[])
{
    setlinebuf (stdout);

    char logpath[PATH_MAX];
    memset (logpath, 0, sizeof (logpath));

    // set some defaults
    int force = 0;
    int auto_increment = 0;
    int use_strftime = 0;
    char *chan_regex = strdup(".*");
    double max_write_queue_size_mb = DEFAULT_MAX_WRITE_QUEUE_SIZE_MB;

    char *optstring = "fic:shm:";
    int c;
    struct option long_opts[] = { 
        { "force", no_argument, 0, 'f' },
        { "increment", required_argument, 0, 'i' },
        { "channel", required_argument, 0, 'c' },
        { "strftime", required_argument, 0, 's' },
        { "max-unwritten-mb", required_argument, 0, 'm' },
        { 0, 0, 0, 0 }
    };

    while ((c = getopt_long (argc, argv, optstring, long_opts, 0)) >= 0)
    {
        switch (c) {
            case 'f':
                force = 1;
                break;
            case 'c':
                free(chan_regex);
                chan_regex = strdup(optarg);
                break;
            case 'i':
                auto_increment = 1;
                break;
            case 's':
                use_strftime = 1;
                break;
            case 'm':
                max_write_queue_size_mb = strtod(optarg, NULL);
                if(max_write_queue_size_mb <= 0) {
                    usage();
                    return 1;
                }
                break;
            case 'h':
            default:
                usage();
                return 1;
        };
    }

    if (optind == argc) {
        strcpy (logpath, "lcmlog-%Y-%m-%d");
        auto_increment = 1;
        use_strftime = 1;
    } else if (optind == argc - 1) {
        strncpy (logpath, argv[optind], sizeof (logpath));
    } else if (optind < argc-1) {
        usage ();
        return 1;
    }

    // initialize GLib threading
    g_thread_init(NULL);

    logger_t logger;
    memset (&logger, 0, sizeof (logger));
    logger.time0 = timestamp_now();
    logger.max_write_queue_size = (int64_t)(max_write_queue_size_mb * (1 << 20));

    // maybe run the filename through strftime
    if (use_strftime) {
        time_t now = time (NULL);
        strftime (logger.fname, sizeof (logger.fname), logpath, 
                localtime (&now));
    } else {
        strcpy (logger.fname, logpath);
    }

    if (auto_increment) {
        /* Loop through possible file names until we find one that doesn't
         * already exist.  This way, we never overwrite an existing file. */
        char tmpname[PATH_MAX];
        int filenum = -1;
        struct stat sbuf;
        do {
            filenum++;
            snprintf (tmpname, sizeof (tmpname), "%s.%02d", logger.fname, 
                    filenum);
        } while (0 == stat (tmpname, &sbuf));

        if (errno != ENOENT) {
            perror ("Error: checking for previous logs");
            return -1;
        }

        strcpy (logger.fname, tmpname);
    } else if (! force) {
        struct stat sbuf;
        if (0 == stat (logger.fname, &sbuf)) {
            fprintf (stderr, "Refusing to overwrite existing file \"%s\"\n",
                    logger.fname);
            return 1;
        }
    }

    // create directories if needed
    char *dirpart = g_path_get_dirname (logger.fname);
    if (! g_file_test (dirpart, G_FILE_TEST_IS_DIR)) {
        mkdir_with_parents (dirpart, 0755);
    }
    free (dirpart);

    fprintf (stderr, "Opening log file \"%s\"\n", logger.fname);

    // open output file
    logger.log = lcm_eventlog_create(logger.fname, "w");
    if (logger.log == NULL) {
        perror ("Error: fopen failed");
        return -1;
    }

    // create write thread
    logger.write_thread_exit_flag = 0;
    logger.mutex = g_mutex_new();
    logger.write_queue_size = 0;
    logger.write_queue = g_async_queue_new();
    logger.write_thread = g_thread_create(write_thread, &logger, TRUE, NULL);

    // begin logging
    logger.lcm = lcm_create (NULL);
    if (!logger.lcm) {
        fprintf (stderr, "Couldn't initialize LCM!");
        return -1;
    }
    
    lcm_subscribe(logger.lcm, chan_regex, message_handler, &logger);
    free(chan_regex);

    _mainloop = g_main_loop_new (NULL, FALSE);
    signal_pipe_glib_quit_on_kill ();
    glib_mainloop_attach_lcm (logger.lcm);

    // main loop
    g_main_loop_run (_mainloop);
    
    fprintf(stderr, "Logger exiting\n");

    // stop the write thread
    g_mutex_lock(logger.mutex);
    logger.write_thread_exit_flag = 1;
    g_mutex_unlock(logger.mutex);
    g_async_queue_push(logger.write_queue, &logger.write_thread_exit_flag);
    g_thread_join(logger.write_thread);
    g_mutex_free(logger.mutex);

    // cleanup.  This isn't strictly necessary, do it to be pedantic and so that
    // leak checkers don't complain
    glib_mainloop_detach_lcm (logger.lcm);
    lcm_destroy (logger.lcm);
    lcm_eventlog_destroy (logger.log);

    for(void *msg = g_async_queue_try_pop(logger.write_queue); msg; 
            msg=g_async_queue_try_pop(logger.write_queue)) {
        if(msg == &logger.write_thread_exit_flag)
            continue;
        free(msg);
    }
    g_async_queue_unref(logger.write_queue);
}
