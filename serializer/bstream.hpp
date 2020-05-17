#pragma once
#include"../core.hpp"
#include"../type_alias.h"
#include"../type_traits.h"
#include"../container/smallstr.hpp"
namespace YAL::Serializer
{
	using bsize_t = u32;
	template<typename T>
	static inline constexpr bool is_string_like_v = std::_Is_any_of_v<T, std::string, std::string_view> || std::is_base_of_v<smallStr_base, T>;
	class RBStream {
		char const* data;
		//char const* enddata;
	public:
		RBStream(string_view v) {
			data = v.data();
			//enddata = data+v.size();
		}
		void readto(char* dst, int len) {
			memcpy(dst, data, len);
			data += len;
		}
		template<typename T,std::enable_if_t<std::is_arithmetic_v<T>,bool> ok=true>
		void _apply(T& v) { //for arithmetic
			memcpy(&v, data, sizeof(v));
			data += sizeof(v);
		}
		template<typename T, std::enable_if_t<is_string_like_v<T>,bool> ok=true>
		void _apply(T& v) { //for string
			bsize_t siz;
			_apply(siz);
			v.resize(siz);
			readto(v.data(), siz);
		}
		template<typename T,typename YAL::dummy_type<std::decay_t<decltype(std::declval<T>().front())>>::type ok=true,std::enable_if_t<!is_string_like_v<T>,bool> ok2=true>
		void _apply(T& v) { //for vectors
			bsize_t siz;
			_apply(siz);
			v.reserve(siz);
			for (u32 i = 0; i < siz; ++i) {
				std::decay_t<decltype(std::declval<T>().front())> d;
				_apply(d);
				v.emplace_back(std::move(d));
			}
		}
		template<typename T, typename YAL::dummy_type<std::decay_t<decltype(std::declval<T>().begin()->first)>>::type ok = true>
		void _apply(T& mp) {
			//for maps
			bsize_t siz;
			_apply(siz);
			mp.reserve(siz);
			using T1 = std::decay_t<decltype(std::declval<T>().begin()->first)>;
			using T2 = std::decay_t<decltype(std::declval<T>().begin()->second)>;
			for (u32 i = 0; i < siz; ++i) {
				T1 k; T2 v;
				_apply(k);
				_apply(v);
				mp.emplace(std::move(k), std::move(v));
			}
		}
		template<typename T
			, std::enable_if_t<std::is_invocable_v<decltype(&T::unpack), T, class YAL::Serializer::RBStream&>, bool> ok = true>
		void _apply(T& x) {
			//for custom objects
			x.unpack(*this);
		}
		template<typename... T>
		void apply(T&&... x) {
			(_apply(std::forward<T>(x)),...);
		}
	};
	template<typename T>
	class WBStreamImpl {
	public:
		T data;
		void write(const char* src, int len) {
			data.append(src, len);
		}
		template<typename T, std::enable_if_t<std::is_arithmetic_v<T>, bool> ok = true>
		void _apply(T v) { //for arithmetic
			write((char*)&v, sizeof(v));
		}
		template<typename T, std::enable_if_t<is_string_like_v<T>, bool> ok = true>
		void _apply(T const& v) { //for string
			_apply(bsize_t(v.size()));
			write(v.data(), bsize_t(v.size()));
		}
		template<typename T, typename YAL::dummy_type<std::decay_t<decltype(std::declval<T>().front())>>::type ok = true, std::enable_if_t<!is_string_like_v<T>, bool> ok2 = true>
		void _apply(T const& v) { //for vectors
			bsize_t siz = bsize_t(v.size());
			_apply(siz);
			for (u32 i = 0; i < siz; ++i) {
				_apply(v[i]);
			}
		}
		template<typename T, typename YAL::dummy_type<std::decay_t<decltype(std::declval<T>().begin()->first)>>::type ok = true>
		void _apply(T const& mp) {
			//for maps
			bsize_t siz=bsize_t(mp.size());
			_apply(siz);
			for (auto& [k, v] : mp) {
				_apply(k);
				_apply(v);
			}
		}
		template<typename TX
			, std::enable_if_t<std::is_invocable_v<decltype(&TX::pack), TX, class YAL::Serializer::WBStreamImpl<T>&>, bool> ok = true>
			void _apply(TX& x) {
			//for custom objects
				x.pack(*this);
			}
		template<typename... T>
		void apply(T&&... x) {
			(_apply(std::forward<T>(x)), ...);
		}
	};
	using WBStream = WBStreamImpl<smallStr<160>>;
};