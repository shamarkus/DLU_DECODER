#ifndef GLOBALS_H
#define GLOBALS_H

#include <iostream>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <vector>
#include <cstring>
#include <fstream>
#include <locale.h>
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>
#include <chrono>
#include <unistd.h>
#include <ctype.h>

class parameterInfo;

#define ATO_NUM 0 
#define ATP_NUM 1 
#define AT_UNDEFINED_NUM 2
#define ATP_STR "80_"
#define ATO_STR "c0_"

#define ATP_PARAMETERS_FILENAME "configFiles\\ATP_PARAMS.txt"
#define ATO_PARAMETERS_FILENAME "configFiles\\ATO_PARAMS.txt"

#define ATP_ENUMERATED_LABELS_FILENAME "configFiles\\ATP_LABELS.txt"
#define ATO_ENUMERATED_LABELS_FILENAME "configFiles\\ATO_LABELS.txt"

#define INPUT_FILES_DIRECTORY "inputFiles\\"
#define OUTPUT_FILES_DIRECTORY "outputFiles\\"
#define OUTPUT_FILES_NUM 12

#define HEADER_TIME_SUFFIX ".000"

#define MAX_ATP_LABELS 26
#define MAX_ATO_LABELS 149
#define MAX_ATP_VALUES 99
#define MAX_ATO_VALUES 256
#define MAX_ATP_PARAMS 1765
#define MAX_ATO_PARAMS 3095
#define MAX_ATP_PARAMS_BIT_SIZE 12496
#define MAX_ATO_PARAMS_BIT_SIZE 20176
#define MAX_COLUMN_SIZE 3096
#define MAX_LINE_SIZE 131072
#define ARG_NUM_AFTER_DATE_TIME 3
#define CHARS_AFTER_TIMESTAMP 21

#define MAX_CONFIG_LINE_SIZE 2700
#define MAX_STRING_SIZE 512
#define MAX_SHORT_STRING_SIZE 64
#define MAX_TIME_STRING_SIZE 25
#define MAX_ENUMERATED_SIZE 50
#define MAX_CHARS_SIZE 256
#define MAX_HEADER_BIT_SIZE 248
#define MAX_HEADER_BYTE_SIZE 31
#define HEADER_TIME_BIT_POS 136
#define INNER_HEADER_BIT_POS 11144
#define HEADER_TIME_BIT_SIZE 32
#define MAX_BYTE_SIZE 8
#define TXT_SUFFIX ".txt" 
#define CSV_SUFFIX ".csv"

#define UNSIGNED_INTEGER 0
#define SIGNED_INTEGER 1
#define NOT_APPLICABLE -1

#define ATP_BYTE_NUM_TO_SKIP 1261
#define ATO_BYTE_NUM_TO_SKIP 4

#define DISPLAY_TYPE_ENUMERATED 0
#define DISPLAY_TYPE_HEXADECIMAL 1
#define DISPLAY_TYPE_DECIMAL 2
#define DISPLAY_TYPE_BINARY 3
#define DISPLAY_TYPE_DATE 4
#define DISPLAY_TYPE_TIME 5

#define DECIMAL_PRECISION 3
#define DATE_TIME_BIT_COUNT 64
#define DATE_TIME_SHORT_BIT_COUNT 32
#define TIME_SINCE_1900_01_01 2208988800

#define SECS_PER_HOUR (60 * 60)
#define SECS_PER_DAY (SECS_PER_HOUR * 24)

#define ATO_VERIFIED_SET_1 0b10101010101010000
#define ATO_VERIFIED_SET_2 0b10101011101010000	
#define ATO_VERIFIED_SET_3 0b10101011101000000	

#define ATP_VERIFIED_SET_1 0b0

#define UTC0_TIME "UTC+0" 
#define UTC4_TIME "EDT"
#define UTC5_TIME "EST"
#define UTC0_TIME_NUM 0 
#define UTC4_TIME_NUM 4
#define UTC5_TIME_NUM 5

#define MAX_INIT_TIMESTAMPS 11

#define CORE_MULT 0
#define CORE_ONE 1
#define CORE_TWO 2

#define GENERAL_OUTPUT_NAME "DLUOutputFile"

#define ARGV_PARENT_DIR 1
#define ARGV_FILES 2
#define ARGV_LOG_TYPE 3
#define ARGV_TIMES 4
#define ARGV_PARAMS 5
#define ARGV_OUTPUT_NAME 6 

#define OMAP_FILE_TYPE 0
#define TXT_FILE_TYPE 1
#define CSV_FILE_TYPE 2

extern std::vector<class parameterInfo*> *ATP_parameterInfo;
extern std::vector<class parameterInfo*> *ATO_parameterInfo;

extern char ATP_EnumeratedLabels[MAX_ATP_LABELS][MAX_ATO_VALUES][MAX_SHORT_STRING_SIZE];
extern char ATO_EnumeratedLabels[MAX_ATO_LABELS][MAX_ATO_VALUES][MAX_SHORT_STRING_SIZE];
extern char ATP_StringLabels[MAX_ATP_PARAMS][MAX_STRING_SIZE];
extern char ATO_StringLabels[MAX_ATO_PARAMS][MAX_STRING_SIZE];

extern short argN[MAX_COLUMN_SIZE];

extern struct tm __tm_buf;

#endif
