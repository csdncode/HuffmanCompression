#ifndef HF_H_INCLUDED
#define HF_H_INCLUDED

#include <fstream>
#include <cstring>
#include <cstdio>
#include <iostream>

#define uint unsigned int
#define uchar unsigned char

typedef struct HfNode
{
    uint weight;
    uchar data;
    short parent;
    short left;
    short right;
}HfNode;

typedef struct HfCode
{
  short *code;
  uchar data;
  short index;//尾位置或者长度
}HfCode;
typedef struct StdHfCode
{
  uchar *code;
  uchar data;
  short len;
}StdHfCode;
uint* DataCount(const char *path);
short NotZero(const uint *pData);
HfNode* CreateHfTree(const uint *pData);
HfCode* CreateHfCode(const HfNode *pTree, const uint *pData);
StdHfCode* StdCode(const HfCode* pCode, const short n);
bool Compress(const char *path);
bool Decompress(const char *path);
bool IsEndData(FILE *fp);
#endif // HF_H_INCLUDED
