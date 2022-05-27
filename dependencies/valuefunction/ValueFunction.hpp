/*
Copyright (c) 2022 Felipe Manga

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

/*

// Example use:

#include <iostream>
#include "ValueFunction.hpp"

int main() {
    vf::Value::addBasicConverters(); // initialize converters

    vf::Function func = [](int a, float b){return a + b;}; // assign like you'd do with std::function
    std::string result = func({1, "4.5"}); // vf::Functions take a std::vector of vf::Values
    std::cout << "str:[" << result << "]" << std::endl; // prints "str:[5.5000000]"

    vf::Function vfun = [](int a){std::cout << a << std::endl;};
    vfun.arguments.push_back("99.8");
    vfun();

    return 0;
}

*/

#pragma once

#include <any>
#include <cstddef>
#include <functional>
#include <string>
#include <tuple>
#include <type_traits>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>
#include <vector>

namespace vf {

    class Value {
        struct converterHashFunc {
            std::size_t operator () (const std::pair<std::type_index, std::type_index>& pair) const {
                return pair.first.hash_code() * 31 + pair.second.hash_code();
            }
        };

        using Converter = std::function<void(const std::any&, void*)>;

        using ConverterMap = std::unordered_map<std::pair<std::type_index, std::type_index>, Converter, converterHashFunc>;

        static ConverterMap& getConverters() {
            static ConverterMap converters;
            return converters;
        }

        template <typename T>
        struct arg_type : public arg_type<decltype(&T::operator())> {};

        template <typename ReturnType, typename ... Args>
        struct arg_type<ReturnType(&)(Args...)> {
            using type = std::remove_reference_t<decltype(std::get<0>(std::tuple<std::remove_reference_t<std::remove_cv_t<Args>>...>()))>;
        };

        template <typename ClassType, typename ReturnType, typename... Args>
        struct arg_type<ReturnType(ClassType::*)(Args...) const> {
            using type = std::remove_reference_t<decltype(std::get<0>(std::tuple<std::remove_reference_t<std::remove_cv_t<Args>>...>()))>;
        };

        std::any value;

    public:
        Value() = default;

        template<typename Type>
        Value(const Type& value) : value{value} {}

        template<typename Type>
        Value(Type&& value) : value{std::move(value)} {}

        Value(const Value& other) : value{other.value} {}

        Value(Value&& other) : value{std::move(other.value)} {}

        template<typename Type>
        Value& operator = (const Type& value) {this->value = value; return *this;}

        template<typename Type>
        Value& operator = (Type&& value) {this->value = std::move(value); return *this;}

        Value& operator = (const Value& other) {this->value = other.value; return *this;}

        Value& operator = (Value&& other) {this->value = std::move(other.value); return *this;}

        template<typename Type>
        operator Type () const {
            auto key = std::make_pair(std::type_index(value.type()), std::type_index(typeid(Type)));

            if (key.first == key.second)
                return std::any_cast<Type>(value);

            auto& converters = getConverters();
            auto it = converters.find(key);

            Type ret{};
            if (it != converters.end())
                it->second(value, &ret);

            return ret;
        }

        bool empty() const {
            return !value.has_value();
        }

        void reset() {
            value.reset();
        }

        template<typename Type, bool exact = false>
        bool has() const {
            auto key = std::make_pair(std::type_index(value.type()), std::type_index(typeid(Type)));
            if (key.first == key.second)
                return std::any_cast<Type>(value);

            if (!exact) {
                auto& converters = getConverters();
                auto it = converters.find(key);
                return it != converters.end();
            }

            return false;
        }

        template<typename Func, typename This = typename arg_type<Func>::type, typename That = std::result_of_t<Func(This)>>
        static void addConverter(const Func& func) {
            auto& converters = getConverters();
            auto key = std::make_pair(std::type_index(typeid(This)), std::type_index(typeid(That)));
            converters[key] = [func](const std::any& value, void* target) {
                *reinterpret_cast<That*>(target) = func(std::any_cast<This>(value));
            };
        }

        template<typename Type>
        static void addIntConverters() {
            addConverter([](Type v){return static_cast<float>(v);});
            addConverter([](Type v){return static_cast<signed char>(v);});
            addConverter([](Type v){return static_cast<bool>(v);});
            addConverter([](Type v){return static_cast<char>(v);});
            addConverter([](Type v){return static_cast<short>(v);});
            addConverter([](Type v){return static_cast<int>(v);});
            addConverter([](Type v){return static_cast<long>(v);});
            addConverter([](Type v){return static_cast<long long>(v);});
            addConverter([](Type v){return static_cast<double>(v);});
            addConverter([](Type v){return static_cast<unsigned char>(v);});
            addConverter([](Type v){return static_cast<unsigned short>(v);});
            addConverter([](Type v){return static_cast<unsigned int>(v);});
            addConverter([](Type v){return static_cast<unsigned long>(v);});
            addConverter([](Type v){return static_cast<unsigned long long>(v);});
            addConverter([](Type v){return std::to_string(v);});
            addConverter([](const std::string& str){return static_cast<Type>(atoi(str.c_str()));});
            addConverter([](const char* str){return static_cast<Type>(atoi(str));});
            addConverter([](char* str){return static_cast<Type>(atoi(str));});
        }

        template<typename Type>
        static void addFloatConverters() {
            addConverter([](Type v){return static_cast<float>(v);});
            addConverter([](Type v){return static_cast<double>(v);});
            addConverter([](Type v){return v != 0;});
            addConverter([](Type v){return static_cast<signed char>(v + Type(0.5));});
            addConverter([](Type v){return static_cast<char>(v + Type(0.5));});
            addConverter([](Type v){return static_cast<short>(v + Type(0.5));});
            addConverter([](Type v){return static_cast<int>(v + Type(0.5));});
            addConverter([](Type v){return static_cast<long>(v + Type(0.5));});
            addConverter([](Type v){return static_cast<long long>(v + Type(0.5));});
            addConverter([](Type v){return static_cast<unsigned char>(v + Type(0.5));});
            addConverter([](Type v){return static_cast<unsigned short>(v + Type(0.5));});
            addConverter([](Type v){return static_cast<unsigned int>(v + Type(0.5));});
            addConverter([](Type v){return static_cast<unsigned long>(v + Type(0.5));});
            addConverter([](Type v){return static_cast<unsigned long long>(v + Type(0.5));});
            addConverter([](Type v){return std::to_string(v);});
            addConverter([](const std::string& str){return static_cast<Type>(atof(str.c_str()));});
            addConverter([](const char* str){return static_cast<Type>(atof(str));});
            addConverter([](char* str){return static_cast<Type>(atof(str));});
        }

        static void addBasicConverters() {
            addIntConverters<signed char>();
            addIntConverters<char>();
            addIntConverters<short>();
            addIntConverters<int>();
            addIntConverters<long>();
            addIntConverters<long long>();
            addIntConverters<double>();
            addIntConverters<unsigned char>();
            addIntConverters<unsigned short>();
            addIntConverters<unsigned int>();
            addIntConverters<unsigned long>();
            addIntConverters<unsigned long long>();
            addFloatConverters<float>();
            addFloatConverters<double>();

            addConverter([](bool v){return v ? "true" : "false";});
            addConverter([](const char* v){return v && v[0] == 't';});
            addConverter([](char* v){return v && v[0] == 't';});
            addConverter([](const std::string& v){return v == "true";});
        }
    };

    class Function {
    private:
        // For generic types, directly use the result of the signature of its 'operator()'
        template <typename T>
        struct function_traits : public function_traits<decltype(&T::operator())> {};

        template <typename ReturnType, typename ... Args>
        struct function_traits<ReturnType(&)(Args...)> {
            enum { arity = sizeof...(Args) };
            typedef ReturnType result_type;

            template <typename Func, bool, typename Tuple, unsigned int ...I>
            struct helper {
                static Value call(Func&& func, Value* args){
                    return helper<Func, sizeof...(I) + 1 == std::tuple_size<Tuple>::value, Tuple, I..., sizeof...(I)>::call(func, args);
                }
            };

            template <typename Func, unsigned int ...I>
            struct helper<Func, true, std::tuple<Args...>, I...> {
                static Value call(Func&& func, Value* args) {
                    return func(static_cast<Args>(args[I])...);
                }
            };

            template <typename Func>
            static Value call(Func&& func, Value * args) {
                return helper<Func, sizeof...(Args) == 0, std::tuple<Args...>>::call(func, args);
            }
        };

        // we specialize for pointers to member function
        template <typename ClassType, typename ReturnType, typename... Args>
        struct function_traits<ReturnType(ClassType::*)(Args...) const> {
            enum { arity = sizeof...(Args) }; // arity is the number of arguments.

            typedef ReturnType result_type;

            template <typename Func, bool, typename Tuple, unsigned int ...I>
            struct helper {
                static Value call(Func&& func, Value* args){
                    return helper<Func, sizeof...(I) + 1 == std::tuple_size<Tuple>::value, Tuple, I..., sizeof...(I)>::call(func, args);
                }
            };

            template <typename Func, unsigned int ...I>
            struct helper<Func, true, std::tuple<Args...>, I...> {
                static Value call(Func&& func, Value* args) {
                    if constexpr (std::is_void_v<ReturnType>) {
                        func(static_cast<Args>(args[I])...);
                        return {};
                    } else {
                        return func(static_cast<Args>(args[I])...);
                    }
                }
            };

            template <typename Func>
            static Value call(Func&& func, Value * args) {
                return helper<Func, sizeof...(Args) == 0, std::tuple<Args...>>::call(func, args);
            }
        };

        std::function<void(Value&, std::vector<Value>&)> call;
        std::size_t argCount;

        static inline std::vector<Value>** getVarArgsPtr() {
            static std::vector<Value>* ptr = nullptr;
            return &ptr;
        }

    public:
        std::vector<Value> defaults;
        std::vector<Value> arguments;
        Value result;

        static std::vector<Value>& varArgs() {
            return **getVarArgsPtr();
        }

        Function() : call([](Value& ret, std::vector<Value>&){}), argCount(0) {}

        Function(Function&& other) {
            argCount = other.argCount;
            call = other.call;
        }

        Function(const Function& other) = default;

        Function(const Function&& other) : call(other.call), argCount(other.argCount) {}

        Function& operator = (Function&& other) = default;

        Function& operator = (const Function& other) = default;

        template<typename NativeFunction>
        Function(NativeFunction&& func) : argCount(function_traits<NativeFunction>::arity) {
            call = [func](Value& result, std::vector<Value>& arguments) {
                *getVarArgsPtr() = &arguments;
                result = function_traits<NativeFunction>::call(func, arguments.data());
            };
        }

        template<typename ... Arg>
        void setDefault(Arg ... arg) {
            defaults = {std::forward<Arg>(arg)...};
        }

        const Value& operator () () {
            for (int i = 0, max = std::min(defaults.size(), argCount); i < max; ++i) {
                arguments.push_back(defaults[i]);
            }
            while (arguments.size() < argCount) {
                arguments.push_back(Value{});
            }
            call(result, arguments);
            arguments.clear();

            return result;
        }

        const Value& operator () (const std::vector<Value>& args) {
            arguments = args;
            (*this)();
            return result;
        }
    };

}
