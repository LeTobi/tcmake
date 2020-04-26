#ifndef TC_EXCEPTION_H
#define TC_EXCEPTION_H

#include <exception>
#include <string>
#include <list>
#include <ctime>

namespace tobilib
{
    class Exception: public std::exception
    {
    private:
        mutable std::string what_str;

    public:
        time_t creation;
        std::string msg;
        std::list<std::string> trace;

        template <class strT>
        Exception(strT txt): msg(txt)
        {
            time(&creation);
        };

        Exception()
        {
            time(&creation);
        };

        template <class strT>
        Exception& operator+= (strT txt)
        {
            msg+=txt;
            return *this;
        }

        const char* what() const noexcept
        {
            what_str="";
            what_str += "[";
            what_str += ctime(&creation);
            what_str.resize(what_str.size()-1); // '\n' entfernen
            what_str += "] \"";
            what_str += msg;
            what_str += "\" ";
            for (auto& it: trace)
            {
                what_str+=it;
                if (it!=trace.back())
                    what_str+=", ";
            }
            return what_str.c_str();
        };
    };
}

#endif