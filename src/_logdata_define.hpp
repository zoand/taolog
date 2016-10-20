#pragma once

#include <tchar.h>

#include <string>
#include <sstream>
#include <memory>
#include <functional>

#include <Windows.h>
#include <guiddef.h>

namespace taoetw {

#define ETW_LOGGER_MAX_LOG_SIZE (60*1024)

// һ��Ҫ�� etwlogger.h �еĶ���һ��
#pragma pack(push,1)
struct LogData
{
    GUID guid;              // ������ GUID
    SYSTEMTIME time;        // ʱ���
    unsigned int line;      // �к�
    unsigned int cch;       // �ַ���������null��
    wchar_t file[1024];     // �ļ�
    wchar_t func[1024];     // ����
    wchar_t text[ETW_LOGGER_MAX_LOG_SIZE];      // ��־
};
#pragma pack(pop)

// ������Ҫʹ�ø��������
struct LogDataUI : LogData
{
    typedef std::basic_string<TCHAR> string;
    typedef std::basic_stringstream<TCHAR> stringstream;
    typedef std::function<const wchar_t*(int i)> fnGetColumnName;

    static constexpr int data_cols = 10;

    string to_string(fnGetColumnName get_column_name) const
    {
        TCHAR tmp[128];
        stringstream ss;

        auto gc = get_column_name;

        ss << text;
        ss << L"\r\n\r\n" << string(50, L'-') << L"\r\n\r\n";
        ss << gc(0) << L"��" << id << L"\r\n";
        ss << gc(2) << L"��" << pid << L"\r\n";
        ss << gc(3) << L"��" << tid << L"\r\n";

        auto& t = time;
        _snwprintf(tmp, _countof(tmp), L"%s��%d-%02d-%02d %02d:%02d:%02d:%03d\r\n",
            gc(1),
            t.wYear, t.wMonth, t.wDay,
            t.wHour, t.wMinute, t.wSecond, t.wMilliseconds
        );

        ss << tmp;

        ss << gc(4) << L"��" << strProject   << L"\r\n";
        ss << gc(5) << L"��" << file         << L"\r\n";
        ss << gc(6) << L"��" << func         << L"\r\n";
        ss << gc(7) << L"��" << strLine      << L"\r\n";
        ss << gc(8) << L"��" << level        << L"\r\n";

        return std::move(ss.str());
    }

    // ��������
    int size() const
    {
        return data_cols;
    }

    inline const wchar_t* operator[](int i)
    {
        // ���� "" ������ null���Է�����
        if (i<0 || i > data_cols - 1)
            return L"";

        const wchar_t* value = L"";

        switch(i)
        {
        case 0: value = id;                            break;
        case 1: value = strTime.c_str();               break;
        case 2: value = strPid.c_str();                break;
        case 3: value = strTid.c_str();                break;
        case 4: value = strProject.c_str();            break;
        case 5: value = file + offset_of_file;         break;
        case 6: value = func;                          break;
        case 7: value = strLine.c_str();               break;
        case 8: value = strLevel->c_str();             break;
        case 9: value = text;                          break;
        }

        return value;
    }
    
    unsigned int pid;       // ���̱�ʶ
    unsigned int tid;       // �̱߳�ʶ
    unsigned char level;    // ��־�ȼ�

    TCHAR id[22];
    string strTime;
    string strLine;
    string strPid;
    string strTid;

    string strProject;

    int offset_of_file;

    string* strLevel;
};

typedef std::shared_ptr<LogDataUI> LogDataUIPtr;

}