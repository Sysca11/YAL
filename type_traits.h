#pragma once
#include <type_traits>
#include<string>
#include<string_view>
struct bitwise_movable_t
{
};
namespace YAL
{
    template <typename TP>
    struct is_bitwise_movable
    {
        constexpr operator bool()
        {
            return std::is_arithmetic_v<TP> || std::is_same_v<TP, std::string> || std::is_same_v<TP, std::string_view> || std::is_base_of_v<bitwise_movable_t, std::decay_t<TP>>;
        }
    };
    template<typename TP>
    struct type_as_param{
        using TYPE=typename std::conditional_t<(sizeof(TP)<=8 && std::is_pod_v<TP>),TP,TP&>;
    };
    template<typename TP>
    using type_as_param_t=typename type_as_param<TP>::TYPE;
    template<typename T>
    static void* cast_to_pointer(T x){
        static_assert(sizeof(T)==8);
        return *(void**)&x;
    }
    template<typename TP>
    struct object_public_wrapper:public TP{};
    template<typename TP>
    struct lambda_operator_getter{
        lambda_operator_getter(TP const&){}
        operator void*(){
            return cast_to_pointer(&object_public_wrapper<TP>::operator());
        }
    };
    template<typename T>
    constexpr string_view to_view(T& x) {
        return string_view((char*)&x, sizeof(x));
    }
    template<typename T>
    struct dummy_type {
        using type = bool;
    };
}; // namespace YAL