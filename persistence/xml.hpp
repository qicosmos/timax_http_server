//
// Created by QY on 2016-12-30.
//

#ifndef SERIALIZE_XML_HPP
#define SERIALIZE_XML_HPP

#include "reflection.hpp"
namespace xml
{
    template <typename InputIt, typename T, typename F>
    T join(InputIt first, InputIt last, const T& delim, const F& f)
    {
        T t = f(*first++);
        while (first != last)
        {
            t += delim;
            t += f(*first++);
        }
        return t;
    }

    std::string render_xml_value(nullptr_t) { return "null"; }
    std::string render_xml_value(bool b) { return b ? "true" : "false"; };
    std::string render_xml_value(double d) { return std::to_string(d); };
    std::string render_xml_value(int d) { return std::to_string(d); }
    std::string render_xml_value(const std::string& s){ return s; }
    std::string render_string(const std::string& s)
    {
        std::stringstream ss;
        ss<<"<";
        ss << s;
        ss<<">";
        return ss.str();
    }

    std::string render_tail_string(const std::string& s)
    {
        std::stringstream ss;
        ss<<"</";
        ss << s;
        ss<<">";
        return ss.str();
    }

    template<typename T, typename = std::enable_if_t<is_reflection<T>::value>>
    std::string to_xml(T&& t);

    template<typename T, typename = std::enable_if_t<is_reflection<T>::value>>
    std::string render_xml_value(T&& t)
    {
        return to_xml(std::forward<T>(t));
    }

    template<typename K, typename V>
    std::string render_xml_value(const std::map<K, V>& o)
    {
        return std::string{ "{" }
               +join(o.cbegin(), o.cend(), std::string{ "," },
                     [](const auto& jsv) {
                         return render_key(jsv.first) + ":" + render_xml_value(jsv.second);
                     })
               + "}";
    }

    template<typename T>
    std::string render_xml_value(const std::vector<T>& v)
    {
        return std::string{ "[" }
               +join(v.cbegin(), v.cend(), std::string{ "," },
                     [](const auto& jsv) {
                         return render_xml_value(jsv);
                     })
               + "]";
    }

    template<typename T, typename =std::enable_if_t<std::is_arithmetic<T>::value>>
    std::string render_key(T&& t)
    {
        return render_string(render_xml_value(std::forward<T>(t)));
    }

    template<typename T>
    std::string render_key(const std::string& s)
    {
        return render_string(s);
    }

    template<typename T, typename = std::enable_if_t<is_reflection<T>::value>>
    std::string to_xml(T&& t)
    {
        std::string s="<"+get_name<T>()+">";
        for_each(std::forward<T>(t), [&s](const auto& v, size_t I, bool is_last){ //magic for_each struct std::forward<T>(t)
            s += render_string(get_name<T>(I))  +
                    render_xml_value(v) + render_tail_string(get_name<T>(I));
//            if(!is_last)
//                s+="\n";
        }, [&s](const auto& o, size_t i, bool is_last)
                 {
                     std::cout<<i<<std::endl;
                     s += render_string(get_name<T>(i)) + std::string{":"} + to_xml(o);
                     if(!is_last)
                         s+=",";
                 });
        s+="</"+get_name<T>()+">";
        std::cout<<s<<std::endl;
        return s;
    }
}
#endif //SERIALIZE_XML_HPP
