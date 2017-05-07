#ifndef COMMAND_H
#define COMMAND_H

/*
INTRODUCE = 0x01,
LOGIN = 0x02,
LIST = 0x05,
DOWNLOAD = 0x06,
UPLOAD = 0x07,
ACCEPT = 0x08,
CHUNK = 0x09,
DELETE = 0x0a,
RENAME = 0x0b,
COMMIT = 0x0c,
ROLLBACK = 0x0d,
COMMITRDY = 0x0e,
COMMITACK = 0x0f,
ERROR = 0x33,
EXIT = 0xff
*/

class Command
{
public:
    Command()
    {
        size = new char;
        data = new char;
    }

    Command(const char *code, int sizeInt, const char *data)
    {
        this->code = code;
        this->sizeInt = sizeInt;
        std::string sizeString = std::to_string(sizeInt);
        size = sizeString.c_str();
        this->data = data;
    }

    Command(const char *code,const char *size,const char *data)
    {
        this->code = code;
        this->size = size;
        sizeInt = atoi(size);
        this->data = data;
    }

    const char* getCode()
    {
        return code;
    }

    void setCode(const char *code)
    {
        this->code = code;
    }

    const char* getSize()
    {
        return size;
    }

    void setSize(const char* size)
    {
        this->size = size;
        sizeInt = atoi(size);
    }

    void setSize(int sizeInt)
    {
        this->sizeInt = sizeInt;
        std::string sizeString = std::to_string(sizeInt);
        size = sizeString.c_str();
    }

    const char* getData()
    {
        return data;
    }

    void setData(const char* data)
    {
        this->data = data;
    }

private:
    const char *code;
    int sizeInt;
    const char *size;
    const char *data;
};

#endif // COMMAND_H
