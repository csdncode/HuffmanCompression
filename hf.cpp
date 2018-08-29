#include "hf.h"

long long FileSize = 0;
uint* DataCount(const char *path)//数据出现频率统计 path为文件路径
{
    FILE *fpRead = fopen(path, "rb");
    uint *pData = new uint[256];
    memset(pData, 0, sizeof(uint)*256);
    uchar data;
    while( feof(fpRead) == 0 )
    {
        fread((char*)&data, sizeof(uchar), 1, fpRead);
        pData[data]++;
        FileSize++;
    }
    fclose(fpRead);
    FileSize--;
    return pData;
}
short NotZero(const uint *pData)//统计出现次数不为0的数据的个数,固定长度为256
{
    short count = 0;
    for(short i = 0; i < 256; i++)
        if(pData[i] != 0)
            count++;
    return count;
}
HfNode* CreateHfTree(const uint *pData)//创建哈夫曼树 参数是所统计的数据出现次数组
{
    short n = NotZero(pData);//n为出现次数不为零的数据个数
    const short MAXLEN = n*2 - 1;
    HfNode *pTree = new HfNode[MAXLEN];


    for(short i = 0, j = 0; i < 256; i++)
    {
        if(pData[i] != 0)
        {
            pTree[j].weight = pData[i];
            pTree[j].data = (uchar)i;
            pTree[j].parent = -1;
            pTree[j].left = -1;
            pTree[j].right = -1;
            j++;
        }
    }

    if(n == 1)
        return pTree;

    short min1;//权值最小节点下标
    short min2;//权值次小节点下标
    for(; n < MAXLEN; n++)
    {
        //找最小节点
        for(short i = 0; i < n; i++)
            if(pTree[i].parent == -1)
            {
                min1 = i;
                break;
            }
        for(short i = 0; i < n; i++)
            if(pTree[i].weight < pTree[min1].weight && pTree[i].parent == -1)
                min1 = i;


        //找次小节点
        for(short i = 0; i < n; i++)
            if(pTree[i].parent == -1 && i != min1)
                min2 = i;
        for(short i = 0; i < n; i++)
        {
            if(i == min1)
                continue;
            else if(pTree[i].weight < pTree[min2].weight && pTree[i].parent == -1)
                min2 = i;
        }



        //将最小节点与次小节点合并为一个新的节点并指明着三个节点的父子关系
        pTree[n].data = (uchar)n;
        pTree[n].weight = pTree[min1].weight + pTree[min2].weight;
        pTree[n].left = min1;
        pTree[n].right = min2;
        pTree[n].parent = -1;
        pTree[min1].parent = n;
        pTree[min2].parent = n;
    }
    return pTree;
}
HfCode* CreateHfCode(const HfNode *pTree, const uint *pData)//编码函数 顺序表pTree是哈夫曼树的首地址  pData是统计数据出现次数的数组
{
    short n = NotZero(pData);//n表示要编码数据个数
    HfCode *pCode = new HfCode[n];
    for(short i = 0; i < n; i++)
    {
        short parent = pTree[i].parent;
        pCode[i].data = pTree[i].data;
        pCode[i].code = new short[n];
        for(short j = 0; j < n; j++)//编码全部初始化为-1
            pCode[i].code[j] = -1;


        short j = 0;
        short child = i;
        while(parent != -1)
        {
            if(pTree[parent].left == child)
                pCode[i].code[j] = 0;
            else if(pTree[parent].right == child)
                pCode[i].code[j] = 1;
            j++;
            child = parent;
            parent = pTree[parent].parent;
        }
        pCode[i].index = j - 1;//标记编码的开始位置（倒序）
    }
    return pCode;
}
StdHfCode* StdCode(const HfCode* pCode, const short n)
{
    StdHfCode *pStdCode = new StdHfCode[256];
    for(short i = 0; i < 256; i++)
        pStdCode[i].code = NULL;
    for(short i = 0; i < n; i++)
    {
        pStdCode[pCode[i].data].data = pCode[i].data;//复制data
        pStdCode[pCode[i].data].code = new uchar[pCode[i].index + 1];//申请code的空间
        for(short j = 0; j < pCode[i].index+1; j++)//复制code
            pStdCode[pCode[i].data].code[j] = (uchar)pCode[i].code[pCode[i].index - j];
        pStdCode[pCode[i].data].len = pCode[i].index + 1;
    }
    return pStdCode;
}
bool Compress(const char *path)//压缩函数 有隐藏bug暂未被发现 path是要压缩文件的路径 如果文件与程序在同一路径下不必输入完整路径 若不在同一路径必须输入全部路径
{
    FILE *fpRead = fopen(path, "rb");
    if(fpRead == NULL)//判断文件是否存在
        return false;
    uint *pData = DataCount(path);
    HfNode *pTree = CreateHfTree(pData);
    HfCode *pCode = CreateHfCode(pTree, pData);
    const short n = NotZero(pData);//文件数据的个数
    StdHfCode *pStdCode = StdCode(pCode, n);//编码标准化

    //构建压缩文件名称
    char *NewPath;
    bool flag = false;
    short PathSize = strlen(path);
    for(short i = PathSize-1; i >=0 ; i--)
    {
        if(path[i] == '\\')
        {
            NewPath = new char[PathSize-i+3];
            for(short j = 0; i < PathSize; i++, j++)
                NewPath[j] = path[i+1];
            flag = true;
            break;
        }
    }

    if(flag == false)
    {
        NewPath = new char[PathSize+4];
        strcpy(NewPath, path);
    }
    strcat(NewPath, ".hf");



    //创建压缩文件
    FILE *fpWrite = fopen(NewPath, "wb");
    fwrite("hf", sizeof(char), 2, fpWrite);
    short TreeSize = n*2 - 1;
    //数据全部相同处理方式
    if( TreeSize == 1)
    {
        fwrite((short*)&TreeSize, sizeof(short), 1, fpWrite);
        fwrite((long long*)&FileSize, sizeof(long long), 1, fpWrite);
        fwrite((uchar*)&pTree[0].data, sizeof(uchar), 1, fpWrite);
        fwrite("hf", sizeof(char), 2, fpWrite);
        fclose(fpRead);
        fclose(fpWrite);
        delete[] pStdCode;
        delete[] pCode;
        delete[] pData;
        delete[] pTree;
        delete[] NewPath;
        return true;
    }
    fwrite((short*)&TreeSize, sizeof(short), 1, fpWrite);//写入树的节点总数
    fwrite((long long*)&FileSize, sizeof(long long), 1, fpWrite);//写入文件数据总字节数
    for(short i = 0; i < TreeSize; i++)//将每个节点写入文件
    {
        fwrite((uchar*)&pTree[i].data, sizeof(uchar), 1, fpWrite);
        fwrite((short*)&pTree[i].left, sizeof(short), 1, fpWrite);
        fwrite((short*)&pTree[i].parent, sizeof(short), 1, fpWrite);
        fwrite((short*)&pTree[i].right, sizeof(short), 1, fpWrite);
        fwrite((uint*)&pTree[i].weight, sizeof(uint), 1, fpWrite);
    }
    fwrite("hf", sizeof(char), 2, fpWrite);//再次写入方便debug


    //读需要压缩的文件
    //FILE *fpRead = fopen(path,"rb");更换位置了 不写在这里
    uchar data;//8bit
    uchar NewData = 0;
    short i;
    short j;
    while( feof(fpRead) == 0 )
    {
        fread((char*)&data, sizeof(uchar), 1, fpRead);
        j = 0;
there:
        for(i = 0; i < 8; i++)
        {
            uchar temp = pStdCode[data].code[j];
            NewData |= (temp<<(7-i));
            j++;
            if(j == pStdCode[data].len && i != 7 )
            {
                j = 0;
                if(feof(fpRead) == 0)
                    fread((char*)&data, sizeof(uchar), 1, fpRead);
                else
                    break;
            }
            if(feof(fpRead) != 0)
                break;
        }
        fwrite((uchar*)&NewData, sizeof(uchar), 1, fpWrite);
        NewData = 0;
        if(j != pStdCode[data].len && j < pStdCode[data].len && feof(fpRead) == 0)
            goto there;
    }
    fclose(fpRead);
    fclose(fpWrite);


    //////////////////内存清理/////////////////////////////
    for(short i = 0, j = 0; i < 256; i++)
    {
        if(pStdCode[i].code != NULL)
        {
            delete[] pStdCode[i].code;
            delete[] pCode[j].code;
            j++;
        }
    }
    delete[] pStdCode;
    delete[] pCode;
    delete[] pData;
    delete[] pTree;
    delete[] NewPath;
    //remove(path);
    return true;
}

bool Decompress(const char *path)//解压函数 有隐藏bug暂未被发现 path同压缩函数
{
    FILE *fpRead = fopen(path, "rb");
    if(fpRead == NULL)//判断文件是否存在
        return false;


    //生成解压后的文件名
    int len = strlen(path);
    char *filename = new char[len-2];
    for(int i = 0; i < len-3; i++)
        filename[i] = path[i];
    filename[len-3] = '\0';
    FILE *fpWrite = fopen(filename, "wb");//创建解压文件




    if((char)fgetc(fpRead) == 'h' && (char)fgetc(fpRead) == 'f')//校验
        ;
    else
        return false;
    //读取树节点总数
    short TreeSize;
    long long FileSizeTemp;
    fread((short*)&TreeSize, sizeof(short), 1, fpRead);
    fread((long long*)&FileSizeTemp, sizeof(long long), 1, fpRead);//读取原文件字节总数
    if(TreeSize == 1)
    {
        uchar data;
        fread((uchar*)&data, sizeof(uchar), 1, fpRead);
        for(long long i = 0; i < FileSizeTemp; i++)
            fwrite((uchar*)&data, sizeof(uchar), 1, fpWrite);
        fclose(fpRead);
        fclose(fpWrite);
        delete[] filename;
        return true;
    }
    HfNode *pTree = new HfNode[TreeSize];
    for(short i = 0; i < TreeSize; i++)//读取每个节点
    {
        fread((char*)&pTree[i].data, sizeof(uchar), 1, fpRead);
        fread((short*)&pTree[i].left, sizeof(short), 1, fpRead);
        fread((short*)&pTree[i].parent, sizeof(short), 1, fpRead);
        fread((short*)&pTree[i].right, sizeof(short), 1, fpRead);
        fread((uint*)&pTree[i].weight, sizeof(uint), 1, fpRead);
    }
    if((char)fgetc(fpRead) == 'h' && (char)fgetc(fpRead) == 'f')//校验
        ;
    else
        return false;

    uchar data;
    uchar bit = 0;
    short root = TreeSize - 1;
    short i;
    while(feof(fpRead) == 0)
    {
        if(FileSizeTemp == 0)
            break;
        fread((char*)&data, sizeof(uchar), 1, fpRead);
        //以下有bug//////////////////////////////////////////////////////////////////
        for(i = 0; i < 8; i++)
        {
            if(pTree[root].left == -1 && pTree[root].right == -1)
            {
                fwrite((uchar*)&pTree[root].data, sizeof(uchar), 1, fpWrite);
                FileSizeTemp--;
                if(FileSizeTemp == 0)
                    break;
                root = TreeSize - 1;
            }
            bit |= data >> (7-i);
            data = data << (i+1);
            data = data >> (i+1);
            if( bit == 0)
                root = pTree[root].left;
            else if ( bit == 1 )
                root = pTree[root].right;
            bit = 0;

        }
    }
    fclose(fpRead);
    fclose(fpWrite);
    ///////////////内存清理///////////////////////
    delete[] filename;
    delete[] pTree;
    return true;
}
