//
// Created by QY on 2016-12-30.
//

#ifndef SERIALIZE_JSON_HPP
#define SERIALIZE_JSON_HPP

#include "reflection.hpp"

namespace json
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

    std::string render_json_value(nullptr_t) { return "null"; }
    std::string render_json_value(bool b) { return b ? "true" : "false"; };
    std::string render_json_value(double d) { return std::to_string(d); };
    std::string render_json_value(int d) { return std::to_string(d); }
    std::string render_json_value(const std::string& s){ return s; }
    std::string render_string(const std::string& s)
    {
        std::stringstream ss;
        ss << std::quoted(s);
        return ss.str();
    }

    template<typename T, typename = std::enable_if_t<is_reflection<T>::value>>
    std::string to_json(T&& t);

    template<typename T, typename = std::enable_if_t<is_reflection<T>::value>>
    std::string render_json_value(T&& t)
    {
        return to_json(std::forward<T>(t));
    }

    template<typename K, typename V>
    std::string render_json_value(const std::map<K, V>& o)
    {
        return std::string{ "{" }
               +join(o.cbegin(), o.cend(), std::string{ "," },
                     [](const auto& jsv) {
                         return render_key(jsv.first) + ":" + render_json_value(jsv.second);
                     })
               + "}";
    }

    template<typename T>
    std::string render_json_value(const std::vector<T>& v)
    {
        return std::string{ "[" }
               +join(v.cbegin(), v.cend(), std::string{ "," },
                     [](const auto& jsv) {
                         return render_json_value(jsv);
                     })
               + "]";
    }

    template<typename T, typename =std::enable_if_t<std::is_arithmetic<T>::value>>
    std::string render_key(T&& t)
    {
        return render_string(render_json_value(std::forward<T>(t)));
    }

    template<typename T>
    std::string render_key(const std::string& s)
    {
        return render_string(s);
    }

    template<typename T, typename = std::enable_if_t<is_reflection<T>::value>>
    std::string to_json(T&& t)
    {
        std::string s="{";
        for_each(std::forward<T>(t), [&s](const auto& v, size_t I, bool is_last){ //magic for_each struct std::forward<T>(t)
            s += render_string(get_name<T>(I)) + std::string{":"} +
                 render_json_value(v);
            if(!is_last)
                s+=",";
        }, [&s](const auto& o, size_t i, bool is_last)
                 {
                     std::cout<<i<<std::endl;
                     s += render_string(get_name<T>(i)) + std::string{":"} + to_json(o);
                     if(!is_last)
                         s+=",";
                 });
        s+="}";
        std::cout<<s<<std::endl;
        return s;
    }

    template<typename T>
    std::string to_json(const std::vector<T>& v)
    {
        return render_json_value(v);
    };

    template<typename... Args>
    std::string to_json(std::tuple<Args...> tp)
    {
        std::string s="[";
        apply_tuple([&s](const auto& v, size_t I, bool is_last){
            s+=render_json_value(v);
            if(!is_last)
                s+=",";
        }, tp, std::make_index_sequence<sizeof...(Args)>{});
        s+="]";
        return s;
    }
}
#endif //SERIALIZE_JSON_HPP
