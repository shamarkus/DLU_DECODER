#include "globals.h"
#include "logDecoderClass.h"
#include "helperFunctions.h"

fileDecodingInfo::fileDecodingInfo(struct fileInfo* fileInfoStruct, int logType) : fileInfoStruct(fileInfoStruct) {
    //If log is of ATP type
	if(logType == ATP_NUM){
		this->byteNumToSkip = ATP_BYTE_NUM_TO_SKIP - 1; 
		this->byteNumForLine = MAX_ATP_PARAMS_BIT_SIZE/8;

		if(ATP_parameterInfo != NULL){
		    this->parameterInfoVec = *ATP_parameterInfo;
		}
		else{
			parseLabelsConfigFile(ATP_EnumeratedLabels,logType,fileInfoStruct->directoryPath);

			parseParameterConfigFile(this->parameterInfoVec,logType,fileInfoStruct->directoryPath,ATP_EnumeratedLabels,ATP_StringLabels);
			ATP_parameterInfo = &(this->parameterInfoVec); 
		}
		this->stringLabels = ATP_StringLabels;
		this->enumeratedLabels = ATP_EnumeratedLabels;
	}
		//if log is of ATO type
	else{
		this->byteNumToSkip = ATO_BYTE_NUM_TO_SKIP - 1; 
		this->byteNumForLine = MAX_ATO_PARAMS_BIT_SIZE/8;

		if(ATO_parameterInfo != NULL){
			this->parameterInfoVec = *ATO_parameterInfo;
		}
		else{
			parseLabelsConfigFile(ATO_EnumeratedLabels,logType,fileInfoStruct->directoryPath);

			parseParameterConfigFile(this->parameterInfoVec,logType,fileInfoStruct->directoryPath,ATO_EnumeratedLabels,ATO_StringLabels);
			ATO_parameterInfo = &(this->parameterInfoVec); 
		}
		this->stringLabels = ATO_StringLabels;
		this->enumeratedLabels = ATO_EnumeratedLabels;
	}
}

//Destructor
fileDecodingInfo::~fileDecodingInfo(){
	char filePath[MAX_STRING_SIZE];
	sprintf(filePath,"%s_%s%s",this->fileInfoStruct->directoryPath,this->fileInfoStruct->fileName,TXT_SUFFIX);
	free(this->fileInfoStruct);
}

//Temporary
unsigned long long prevDate;
//Temporary

int fileDecodingInfo::getFileType(){
	return this->fileInfoStruct->fileType;
}

int fileDecodingInfo::getCoreType(){
	return this->fileInfoStruct->core;
}

int fileDecodingInfo::getLogType(){
	return this->fileInfoStruct->logType;
}
FILE* fileDecodingInfo::getOutputFile(){
	return this->fileInfoStruct->outputFile;
}
char* fileDecodingInfo::getDirectoryPath(){
	return this->fileInfoStruct->directoryPath;
}
char* fileDecodingInfo::getFileName(){
	return this->fileInfoStruct->fileName;
}
char* fileDecodingInfo::getOutputFileName(){
	return this->fileInfoStruct->outputFileName;
}

void fileDecodingInfo::decodeFile(){
	struct headerInfo* headerStruct = &(this->headerInfoStruct);
	int logType = this->fileInfoStruct->logType;

	int numParameters = (logType == ATO_NUM) ? MAX_ATO_PARAMS : MAX_ATP_PARAMS;
	int numLineBits = (logType == ATO_NUM) ? MAX_ATO_PARAMS_BIT_SIZE : MAX_ATP_PARAMS_BIT_SIZE;
	int paramsCharSize = this->byteNumForLine;
	int headerCharSize = headerStruct->headerByteSize;
	int skipCharSize = 1;

	char curParams[numParameters][MAX_SHORT_STRING_SIZE + 1];	
	char curLine[numLineBits + 1];
	char curHeader[headerStruct->headerBitSize + 1];
	char curSkipLine[MAX_SHORT_STRING_SIZE];
	curLine[0] = '\0';
	curHeader[0] = '\0';

	char* headerP = curHeader;
	char* lineP = curLine;
	unsigned int skipSeqNum = 0;
	int curChar;
	
	printHeader(numParameters);

	prevDate = 0;

	std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
	while(curChar != EOF){
		curChar = fgetc(this->fileInfoStruct->inputFile);

		if(skipCharSize){
			if(skipSeq(curChar,skipSeqNum))
			{
				if(skipSeqNum == OMNIPOTENT_SET)
				{
					skipCharSize = 0;
				}
				else
				{
					skipSeqNum = 0;	
				}

			}
			else
			{
				//do nothing
			}
		}
		else if(headerCharSize){
			headerP = fast_strcat(headerP,byteArray[curChar]);	
			headerCharSize--;
		}
		else if(paramsCharSize){
			lineP = fast_strcat(lineP,byteArray[curChar]);
			paramsCharSize--;
		}
		else{
			decodeLine(curHeader,curLine,curParams);
			printLine(curParams,numParameters);
			//Re-initialize -- Skip over nullish sequence
			memset(curHeader,0,headerStruct->headerBitSize);
			memset(curLine,0,numLineBits);
			headerP = curHeader;
			lineP = curLine;
			skipSeqNum = 0;
			skipCharSize = 1;

			//Account for the current character - 1
			headerCharSize = headerStruct->headerByteSize;
			paramsCharSize = this->byteNumForLine;
		}
	}
	std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
	std::cout << "Time To Decode = " << (double) std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count()/ 1000000 << "[s]" << std::endl;

	//Swap FILE* pointers
	swapFilePointers();
}

void fileDecodingInfo::decodeLine(char* curHeader, char* curLine, char (*curParams)[MAX_SHORT_STRING_SIZE + 1]){
	//Get timestamp for first 2 params
	struct headerInfo* headerStruct = &(this->headerInfoStruct);
	char binaryParam[MAX_SHORT_STRING_SIZE + 1];
	char resultStr[MAX_SHORT_STRING_SIZE];
	class parameterInfo* parameterObj = this->parameterInfoVec[0];

	memcpy(binaryParam,curHeader + headerStruct->timeBitPos, headerStruct->timeBitSize);
	binaryParam[headerStruct->timeBitSize] = '\0';

	(parameterObj->*(parameterObj->binaryToString))(binaryParam,curParams[0]);

	parameterObj = this->parameterInfoVec[1];
	(parameterObj->*(parameterObj->binaryToString))(binaryParam,curParams[1]);
	strcat(curParams[1],HEADER_TIME_SUFFIX);
	
	//Temporary
	unsigned long long curDate = parameterObj->unsignedBinaryToDecimal(binaryParam);
	if((curDate>>32) - prevDate > 1 && prevDate != 0){
		printf("Gap right before %s\n",curParams[1]);
	}
	prevDate = curDate >> 32;
	//Temporary

	for(int i = 2; i < this->parameterInfoVec.size(); i++){
		parameterObj = this->parameterInfoVec[i];

		memcpy(binaryParam,curLine + parameterObj->getFirstBitPosition(), parameterObj->getBitCount());
		
		binaryParam[parameterObj->getBitCount()] = '\0';

		(parameterObj->*(parameterObj->binaryToString))(binaryParam,curParams[parameterObj->getParameterID()]);
	}
}

void fileDecodingInfo::printHeader(int numParameters){
	for(int i = 0; i < numParameters; i++){
		fprintf(this->fileInfoStruct->outputFile,"%s\t",this->stringLabels[i]);
	}
	fprintf(this->fileInfoStruct->outputFile,"\n");
}

void fileDecodingInfo::printLine(char (*curParams)[MAX_SHORT_STRING_SIZE + 1],int numParameters){
	for(int i = 0; i < numParameters; i++){
		fprintf(this->fileInfoStruct->outputFile,"%s\t",curParams[i]);
	}
	fprintf(this->fileInfoStruct->outputFile,"\n");
}

void fileDecodingInfo::swapFilePointers(){
	struct fileInfo* fInfo = this->fileInfoStruct;
	fclose(fInfo->inputFile);
	rewind(fInfo->outputFile);

	fInfo->inputFile = fInfo->outputFile;
	sprintf(fInfo->outputFileName,"%s%s%s%s",fInfo->directoryPath,OUTPUT_FILES_DIRECTORY,fInfo->fileName,CSV_SUFFIX);

	if(nonRecursiveNameCheck(fInfo->outputFileName) != NULL){
		fInfo->outputFile = fopen(fInfo->outputFileName,"w+");
	}
	else{
		printf("Error - failed to make an output file -- exiting now\n");
		exit(0);
	}
}

parameterInfo::parameterInfo(char* line,char (*enumeratedLabels)[MAX_ATO_VALUES][MAX_SHORT_STRING_SIZE],char (*stringLabels)[MAX_STRING_SIZE]){
	this->parameterID = fast_atoi(strtok(line,"\t"));
	strcpy(stringLabels[this->parameterID],strtok(NULL,"\t"));
	this->unsignedInt = fast_atoi(strtok(NULL,"\t"));
	this->firstBitPosition = fast_atoi(strtok(NULL,"\t"));
	this->bitCount = fast_atoi(strtok(NULL,"\t"));
	this->firstByte = fast_atoi(strtok(NULL,"\t"));
	this->lastByte = fast_atoi(strtok(NULL,"\t"));
	this->quantum = atof(strtok(NULL,"\t"));
	this->offset = fast_atoi(strtok(NULL,"\t"));
	this->displayType = fast_atoi(strtok(NULL,"\t"));
	this->enumeratedLabel = fast_atoi(strtok(NULL,"\t"));
	this->decimalCount = fast_atoi(strtok(NULL,"\t"));

	char* unit = strtok(NULL,"\t");
	if(unit[0] != '-' && unit[1] != '1'){
		this->unit = (char*) malloc(MAX_SHORT_STRING_SIZE);
		strcpy(this->unit,unit);
	}
	else{
		//do nothing
	}

	if(this->firstBitPosition > INNER_HEADER_BIT_POS){
		this->firstBitPosition += MAX_HEADER_BIT_SIZE;
	}
	else{
		//do nothing
	}

	//Function declaration based on the above variables
	if(this->displayType == DISPLAY_TYPE_ENUMERATED){
		this->enumeratedLabels = enumeratedLabels;
		this->binaryToString = &parameterInfo::unsignedBinaryToEnumeratedLabelStr;
	}
	else if(this->displayType == DISPLAY_TYPE_HEXADECIMAL){
		this->binaryToString = &parameterInfo::unsignedBinaryToHexadecimalStr;
	}
	else if(this->displayType == DISPLAY_TYPE_HEXADECIMAL){
		this->binaryToString = &parameterInfo::binaryToBinaryStr;
	}
	else if(this->displayType == DISPLAY_TYPE_DATE){
		this->binaryToString = &parameterInfo::unsignedBinaryToDateStr;
	}
	else if(this->displayType == DISPLAY_TYPE_TIME){
		if(this->decimalCount != -1){
			this->binaryToString = &parameterInfo::unsignedBinaryToDecimalTimeStr;
		}
		else{
			this->binaryToString = &parameterInfo::unsignedBinaryToTimeStr;
		}
	}
	else {
		//this->displayType == DISPLAY_TYPE_DECIMAL
		if(this->decimalCount != -1){
			if(this->unsignedInt == SIGNED_INTEGER){
				this->binaryToString = &parameterInfo::signedBinaryToDecimalPrecisionStr;
			}
			else{
				this->binaryToString = &parameterInfo::unsignedBinaryToDecimalPrecisionStr;
			}
		}
		else if(this->quantum == 1){
			if(this->unsignedInt == SIGNED_INTEGER){
				this->binaryToString = &parameterInfo::signedBinaryToIntegerStr;
			}
			else{
				this->binaryToString = &parameterInfo::unsignedBinaryToIntegerStr;
			}
		}
		else{
			if(this->unsignedInt == SIGNED_INTEGER){
				this->binaryToString = &parameterInfo::signedBinaryToDecimalStr;
			}
			else{
				this->binaryToString = &parameterInfo::unsignedBinaryToDecimalStr;
			}
		}
	}
}

//Destructor
parameterInfo::~parameterInfo(){
	if(NULL != this->unit){
		free(this->unit);
	}
}

//Accepts integer value, and gets corresponding string label
void parameterInfo::IntToEnumeratedLabel(unsigned long long value,char* str){
	if(value > MAX_ATO_VALUES || *this->enumeratedLabels[this->enumeratedLabel][value] == '\0'){
		sprintf(str,"? key : %llu", value);
	}
	else{
		strcpy(str,this->enumeratedLabels[this->enumeratedLabel][value]);
	}
}
void parameterInfo::IntToHexadecimal(unsigned long long value, char* str){
	sprintf(str,"%0*x",this->bitCount >> 2,value);
}
void parameterInfo::IntToInteger(long long value, char* str){
	sprintf(str,"%d",value+this->offset);
}
void parameterInfo::IntToDecimal(long long value, char* str){
	sprintf(str,"%.0f",value*this->quantum+this->offset);
}
void parameterInfo::IntToDecimalPrecision(long long value, char* str){
	sprintf(str,"%0.*f",this->decimalCount,value*this->quantum+this->offset);
}
void parameterInfo::IntToDate(unsigned long long value, char* str){
	epochTimeToDate(value >> 32,str,"%Y/%m/%d");
}
void parameterInfo::IntToTime(unsigned long long value, char* str){
	epochTimeToDate(value >> 32,str,"%H:%M:%S");
}
void parameterInfo::IntToDecimalTime(unsigned long long value, char* str){
	convertToMillisecond(((value & 0xFFC00000) >> 22) - 8,epochTimeToDate(value >> 32,str,"%H:%M:%S"));
}

unsigned long long parameterInfo::unsignedBinaryToDecimal(const char* binaryStr){
	int len = this->bitCount - 1;
	unsigned long long val = 0;
	while(*binaryStr){
		if(*binaryStr++ - '0'){
			val = val | ( (unsigned long long) 1 << len);
		}
		len--;
	}
	return val;
}

long long parameterInfo::signedBinaryToDecimal(const char* binaryStr){
	//Check if MSB is 1
	if(*binaryStr - '0'){
		int len = this->bitCount - 1;       
		long long val = 0;
		while(*binaryStr){
			if(*binaryStr++ - '1'){
				val = val | ( (long long) 1 << len);
			}
			len--;
		}
		++val *= -1;
		return val;
	}
	else{
		return (long long) unsignedBinaryToDecimal(binaryStr);
	}
}

int parameterInfo::getParameterID(){
	return this->parameterID;
}
int parameterInfo::getUnsignedInt(){
	return this->unsignedInt;
}
int parameterInfo::getFirstBitPosition(){
	return this->firstBitPosition;
}
int parameterInfo::getBitCount(){
	return this->bitCount;
}
int parameterInfo::getDisplayType(){
	return this->displayType;
}

void parameterInfo::unsignedBinaryToEnumeratedLabelStr(char* binaryStr,char* str){
	IntToEnumeratedLabel(unsignedBinaryToDecimal(binaryStr),str);
}
void parameterInfo::unsignedBinaryToHexadecimalStr(char* binaryStr,char* str){
	IntToHexadecimal(unsignedBinaryToDecimal(binaryStr),str);
}
void parameterInfo::unsignedBinaryToIntegerStr(char* binaryStr,char* str){
	IntToInteger(unsignedBinaryToDecimal(binaryStr),str);
}
void parameterInfo::signedBinaryToIntegerStr(char* binaryStr,char* str){
	IntToInteger(signedBinaryToDecimal(binaryStr),str);
}
void parameterInfo::unsignedBinaryToDecimalStr(char* binaryStr,char* str){
	IntToDecimal(unsignedBinaryToDecimal(binaryStr),str);
}
void parameterInfo::signedBinaryToDecimalStr(char* binaryStr,char* str){
	IntToDecimal(signedBinaryToDecimal(binaryStr),str);
}
void parameterInfo::unsignedBinaryToDecimalPrecisionStr(char* binaryStr,char* str){
	IntToDecimalPrecision(unsignedBinaryToDecimal(binaryStr),str);
}
void parameterInfo::signedBinaryToDecimalPrecisionStr(char* binaryStr,char* str){
	IntToDecimalPrecision(signedBinaryToDecimal(binaryStr),str);
}
void parameterInfo::binaryToBinaryStr(char* binaryStr,char* str){
	memcpy(str,binaryStr,this->bitCount);
}
void parameterInfo::unsignedBinaryToDateStr(char* binaryStr,char* str){
	IntToDate(unsignedBinaryToDecimal(binaryStr),str);
}
void parameterInfo::unsignedBinaryToTimeStr(char* binaryStr,char* str){
	IntToTime(unsignedBinaryToDecimal(binaryStr),str);
}
void parameterInfo::unsignedBinaryToDecimalTimeStr(char* binaryStr,char* str){
	IntToDecimalTime(unsignedBinaryToDecimal(binaryStr),str);
}
