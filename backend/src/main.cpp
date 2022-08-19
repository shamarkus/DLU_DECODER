#include "globals.h"
#include "logDecoderClass.h"
#include "logParser.h"
#include "helperFunctions.h"

std::vector<class parameterInfo*> *ATP_parameterInfo = NULL;
std::vector<class parameterInfo*> *ATO_parameterInfo = NULL;

char ATP_EnumeratedLabels[MAX_ATP_LABELS][MAX_ATO_VALUES][MAX_SHORT_STRING_SIZE];
char ATO_EnumeratedLabels[MAX_ATO_LABELS][MAX_ATO_VALUES][MAX_SHORT_STRING_SIZE];
char ATP_StringLabels[MAX_ATP_PARAMS][MAX_STRING_SIZE];
char ATO_StringLabels[MAX_ATO_PARAMS][MAX_STRING_SIZE];

short argN[MAX_COLUMN_SIZE];

void initFileVec(std::vector<fileInfo*> &fileVec,const int argc,char** argv);

void initFileObjVec(std::vector<class fileParsingInfo*> &fileObjVec,std::vector<fileInfo*> &fileVec,char** argv);

void decodeAndParse(std::vector<class fileParsingInfo*> &fileObjVec);

//Command formatting should be like this:
//DLULogDecoder.exe	ParentDir	Files	Logtype	Times	Params	Outputname
void initFileVec(std::vector<fileInfo*> &fileVec,const int argc,char** argv){
	char* parentDir = argv[ARGV_PARENT_DIR];
	char* filesArg = argv[ARGV_FILES];
	int logType = atoi(argv[ARGV_LOG_TYPE]);
	char* fileToken = strtok(filesArg,"\t");

	while(fileToken != NULL){

		char tempStr[MAX_STRING_SIZE];
		struct fileInfo* tempFileInfo = new fileInfo();

		strcpy(tempFileInfo->directoryPath,parentDir);
		strcpy(tempFileInfo->fileName,fileToken);

		sprintf(tempStr,"%s%s%s",parentDir,INPUT_FILES_DIRECTORY,fileToken);
		tempFileInfo->inputFile = fopen(tempStr,"rb");

		//Input File is a .txt file
		if(strstr(fileToken,TXT_SUFFIX) != NULL){
			tempFileInfo->fileType = TXT_FILE_TYPE;
			fileToken[strlen(fileToken) - 4] = '\0';
			sprintf(tempFileInfo->outputFileName,"%s%s%s%s",parentDir,OUTPUT_FILES_DIRECTORY,fileToken,CSV_SUFFIX);
			if(nonRecursiveNameCheck(tempFileInfo->outputFileName) != NULL){
				tempFileInfo->outputFile = fopen(tempFileInfo->outputFileName,"w+");
			}
			else{
				exit(0);
			}
		}
		//Input File is an OMAP input file
		else{
			tempFileInfo->fileType = OMAP_FILE_TYPE;
			sprintf(tempFileInfo->outputFileName,"%s_%s%s",parentDir,fileToken,TXT_SUFFIX);
			tempFileInfo->outputFile = fopen(tempFileInfo->outputFileName,"w+");
		}

		if(tempFileInfo->outputFile == NULL || tempFileInfo->inputFile == NULL){
			printf("Unable to open input file from input directory OR make output file in output directory\n");
			exit(0);
		}
		else{
			tempFileInfo->core = atoi(strtok(NULL,"\t"));
			tempFileInfo->logType = logType;
	
			fileVec.push_back(tempFileInfo);
			fileToken = strtok(NULL,"\t");
		}
	}
}

void initFileObjVec(std::vector<class fileParsingInfo*> &fileObjVec,std::vector<fileInfo*> &fileVec,char** argv){
	char* times = argv[ARGV_TIMES];
	char* params = argv[ARGV_PARAMS];
	int numParams = 0;

	time_t startTime = (time_t) atoi(strtok(times,"\t"));
	time_t endTime = (time_t) atoi(strtok(NULL,"\t"));
	
	char* paramsToken = strtok(params,"\t");
	while(paramsToken != NULL){
		argN[numParams] = atoi(paramsToken);
		
		numParams++;
		paramsToken = strtok(NULL,"\t");
	}

	for(auto fileInfo : fileVec){
		class fileParsingInfo* fileObj = new fileParsingInfo(fileInfo,fileInfo->logType,startTime,endTime,argN,numParams);
		fileObjVec.push_back(fileObj);
	}
}

//If need be, the following file will be decoded
//Afterwards it will be parsed to a .csv file
void decodeAndParse(std::vector<class fileParsingInfo*> &fileObjVec){
	int i = 1;
	for(auto fileObj : fileObjVec){
		printf("File Number : %d\n",i++);
		if(fileObj->getFileType() == OMAP_FILE_TYPE){
			fileObj->decodeFile();
		}
		else{
			//Do nothing
		}
		fileObj->parseLogFile();
	}
}

int main(int argc,char** argv){
	
	//Time Start for setting all file objects and data structures
	std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

	std::setlocale(LC_ALL,"en_US.UTF8");

	//Create struct for input/output information
	std::vector<fileInfo*> fileVec; 
	initFileVec(fileVec,argc,argv);

	//Create an object for every file to parse
	std::vector<class fileParsingInfo*> fileObjVec;
	initFileObjVec(fileObjVec,fileVec,argv);

	//Time End for setting all file objects and data structures
	std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
	std::cout << "Time To Set Data Structs = " << (double) std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count()/ 1000000 << "[s]" << std::endl;

	decodeAndParse(fileObjVec);

	if(fileObjVec.size() > 1){
		makeCombinedOutput(argv[ARGV_PARENT_DIR],argv[ARGV_OUTPUT_NAME],fileObjVec);
	}

	//Deletion of all dynamically created objects
	if(ATP_parameterInfo != NULL){
		for(auto parameterObj : *ATP_parameterInfo){
			delete parameterObj;
		}
	}
	if(ATO_parameterInfo != NULL){
		for(auto parameterObj : *ATO_parameterInfo){
			delete parameterObj;
		}
	}
	for(auto fileObj : fileObjVec){
		delete fileObj;
	}

	return 0;
}

