#ifndef LOGGER_H
#define LOGGER_H

extern char* programName;

void findProgramName(char* programPath);
void printProgamName();
void printNoInput();
void printInvalidDatabaseCommand(const char* arg, const char* error);
#endif
