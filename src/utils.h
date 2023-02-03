#ifndef UTILS_H
#define UTILS_H

void remove_ext (char* myStr, char extSep, char pathSep);
void strstripleft(const char *source_str, char *dest_str);
void error(char* msg);
void debugmsg(char *msg);
bool isempty(const char *str);
bool notempty(const char *str);

#endif // UTILS_H
