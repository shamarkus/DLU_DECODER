#ifndef LOGPARSER_H_
#define LOGPARSER_H_

#include "globals.h"
#include "logDecoderClass.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <stdbool.h>
#include <ctype.h>

struct recordInfo {
	time_t epochTime;
	int secondFraction;
	char logArgs[MAX_COLUMN_SIZE][MAX_SHORT_STRING_SIZE];
};

class fileParsingInfo : public fileDecodingInfo{ 
	private:
		struct recordInfo* curRecord;
		struct recordInfo* prevRecord;
		
		short* argN;
		int numArguments;
		
		int UTC;
		time_t startTime;
		time_t endTime; 
	public:
		fileParsingInfo(struct fileInfo* fileInfoStruct, int logType,time_t startTime,time_t endTime,short *argN,int numArguments);
		~fileParsingInfo();

		int getNumArguments();

		bool changeInRecords();
		char* convertEpochToString(char UTCTimestamp[]);
		void writeRecordInfo();
		void writeFirstRecord(struct recordInfo* recordArray[],int numInitFiles);
		int updateSecondFraction(int numInitFiles);
		void tokenizeLine();
		static void writeHeader(FILE* outputFile,int argC,char (*stringLabels)[MAX_STRING_SIZE],short* argN);
		void writeHeaderLine();			
		void writeChangingRecords();
		void parseLogFile();
};

int checkMultCores(std::vector<fileParsingInfo*> &fileVec);

void repeatCommas(FILE* outputFile,int times);

void writeCoreHeader(FILE* outputFile,int multCores,int argC);

void concatFiles(FILE* outputFile,std::vector<fileParsingInfo*> &fileVec);

void makeCombinedOutput(char* parentDir,char* outputFileName,std::vector<fileParsingInfo*> &fileVec);


#endif
