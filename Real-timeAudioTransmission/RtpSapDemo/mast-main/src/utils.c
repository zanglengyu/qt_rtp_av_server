/*

  utils.c

  MAST: Multicast Audio Streaming Toolkit
  Copyright (C) 2019  Nicholas Humfrey
  License: MIT

*/

#include "config.h"
#include "mast.h"

#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <time.h>
#include <dirent.h>
#include <errno.h>

int running = TRUE;
int exit_code = 0;
int quiet = 0;
int verbose = 0;

static void termination_handler(int signum)
{
    running = FALSE;
    switch(signum) {
    case SIGTERM:
        mast_info("Got termination signal");
        break;
    case SIGINT:
        mast_info("Got interupt signal");
        break;
    }
    signal(signum, termination_handler);
}


void setup_signal_hander()
{
    signal(SIGTERM, termination_handler);
    signal(SIGINT, termination_handler);
    signal(SIGHUP, termination_handler);
}

void mast_log(mast_log_level level, const char *fmt, ...)
{
    time_t t = time(NULL);
    char *time_str;
    va_list args;

    // Display the message level
    switch(level) {
    case mast_LOG_DEBUG:
        if (!verbose)
            return;
        fprintf(stderr, "[DEBUG]   ");
        break;
    case mast_LOG_INFO:
        if (quiet)
            return;
        fprintf(stderr, "[INFO]    ");
        break;
    case mast_LOG_WARN:
        fprintf(stderr, "[WARNING] ");
        break;
    case mast_LOG_ERROR:
        fprintf(stderr, "[ERROR]   ");
        break;
    default:
        fprintf(stderr, "[UNKNOWN] ");
        break;
    }

    // Display timestamp
    time_str = ctime(&t);
    time_str[strlen(time_str) - 1] = 0; // remove \n
    fprintf(stderr, "%s  ", time_str);

    // Display the error message
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");
    va_end(args);

    // If an erron then stop
    if (level == mast_LOG_ERROR) {
        // Exit with a non-zero exit code if there was a fatal error
        exit_code++;
        if (running) {
            // Quit gracefully
            running = 0;
        } else {
            fprintf(stderr, "Fatal error while quiting; exiting immediately.\n");
            exit(-1);
        }
    }
}

int mast_read_file_string(const char* filename, char* buffer, size_t buffer_len)
{
    int bytes;
    int retcode = -1;

    // Open the file for reading
    FILE* file = fopen(filename, "rb");
    if (!file) {
        mast_error(
            "Failed to open file '%s': %s",
            filename,
            strerror(errno)
        );
        return retcode;
    }

    // Read as much as we can into the buffer
    bytes = fread(buffer, 1, buffer_len - 1, file);
    if (bytes <= 0) {
        mast_error(
            "Error reading from file '%s': %s",
            filename,
            strerror(errno)
        );

        // FIXME: check that buffer wasn't too small

    } else {
        // Terminate the string
        buffer[bytes] = '\0';
        retcode = 0;
    }

    fclose(file);

    return retcode;
}



int mast_directory_exists(const char* path)
{
    DIR* dir = opendir(path);
    if (dir) {
        /* Directory exists. */
        closedir(dir);
        return TRUE;
    } else if (ENOENT == errno) {
        /* Directory does not exist. */
        return FALSE;
    } else {
        /* opendir() failed for some other reason. */
        mast_error(
            "checking if directory '%s' exists: %s",
            path,
            strerror(errno)
        );
        return FALSE;
    }
}


const char* mast_encoding_names[MAST_ENCODING_MAX] = {
    [MAST_ENCODING_L8] = "L8",
    [MAST_ENCODING_L16] = "L16",
    [MAST_ENCODING_L24] = "L24",
    [MAST_ENCODING_PCMU] = "PCMU",
    [MAST_ENCODING_PCMA] = "PCMA",
    [MAST_ENCODING_G722] = "G722",
    [MAST_ENCODING_GSM] = "GSM"
};


const char* mast_encoding_name(int encoding)
{
    if (encoding > 0 && encoding < MAST_ENCODING_MAX) {
        return mast_encoding_names[encoding];
    } else {
        return NULL;
    }
}

int mast_encoding_lookup(const char* name)
{
    int i;

    for(i=0; i< MAST_ENCODING_MAX; i++) {
        if (strcmp(mast_encoding_names[i], name) == 0)
            return i;
    }
    return -1;
}
