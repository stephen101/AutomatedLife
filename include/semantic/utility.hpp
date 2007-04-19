
#ifndef _SEMANTIC_UTILITY_HPP_
#define _SEMANTIC_UTILITY_HPP_


#include <string>
#include <sstream>

#include <boost/iterator/transform_iterator.hpp>

#include <map>
#include <boost/functional/hash.hpp>
#ifdef HAVE_TR1_UNORDERED_MAP
#include <tr1/unordered_map>
#endif


namespace semantic {
    
    template <class T>
    inline T t_abs(const T& t) {
        return t * (t<0?-1:1);
    }
    
    template <typename T>
    inline T max(const T& one, const T& two) {
        return one>two?one:two;
    }
    
    template <typename T>
    inline std::string to_string(T v) {
        std::stringstream strm;
        strm << v;
        return strm.str();
    }
    
    template <typename Iterator>
    inline std::string join(Iterator i, Iterator i_end, std::string glue) {
        std::stringstream strm;
        unsigned int cnt = 0;
        while(i != i_end) {
            if (cnt++ > 0) strm << glue;
            strm << *i;
            ++i;
        }
    
        return strm.str();
    }
    
    template <typename Iterator>
    inline std::string unique_join(Iterator i, Iterator i_end, std::string glue) {
        std::stringstream strm;
        unsigned int cnt = 0;
        std::set<typename Iterator::value_type> used;
        while(i != i_end) {
            if (used.count(*i)) {++i; continue;}
            used.insert(*i);
            if (cnt++ > 0) strm << glue;
            strm << *i;
            ++i;
        }
    
        return strm.str();
    }
    
    template <typename Num>
    inline void keep_within_range(Num &val, Num min, Num max) {
        val = val<min?min:val>max?max:val;
    }
    
    template <typename T>
    inline void set_if_greater(const T &c, T &m) {
        if (c > m) m = c;
    }
    

    class empty_class {};
    namespace detail {
        // utility class to extract the first part of a pair
        template <class P>
        struct extract_first {
            typedef typename P::first_type result_type;
            const typename P::first_type & operator()(P &p) const { return p.first; }
        };
        // and for the second
        template <class P>
        struct extract_second {
            typedef typename P::second_type result_type;
            const typename P::second_type & operator()(P &p) const { return p.second; }
        };
    } // namespace detail
    
    template <class I>
    boost::transform_iterator<detail::extract_first<typename I::value_type>, I> extract_first_iterator(I it) {
        return boost::transform_iterator<detail::extract_first<typename I::value_type>, I>(it, detail::extract_first<typename I::value_type>());
    }
    #define extract_keys(x) ( extract_first_iterator(x) )
    
    template <class I>
    boost::transform_iterator<detail::extract_second<typename I::value_type>, I> extract_second_iterator(I it) {
        return boost::transform_iterator<detail::extract_second<typename I::value_type>, I>(it, detail::extract_second<typename I::value_type>());
    }
    #define extract_values(x) ( extract_second_iterator(x) )
    
} // namespace semantic


namespace maps {
    // ordered map -- will always use STLs std::map or std::multimap
    template <    typename Key,
                typename Data,
                class Compare = std::less<Key>,
                class Alloc = std::allocator<std::pair<const Key, Data> > >
    class ordered : public std::map<Key, Data, Compare, Alloc> {};

    // ordered multi_map
    template <    typename Key,
                typename Data,
                class Compare = std::less<Key>,
                class Alloc = std::allocator<std::pair<const Key, Data> > >
    class ordered_multi : public std::multimap<Key, Data, Compare, Alloc> {};

    // unordered map -- will use TR1 when available
    template <    typename Key,
                typename Data,
                class Hash = boost::hash<Key>,
                class Pred = std::equal_to<Key>,
                class Alloc = std::allocator<std::pair<const Key, Data> >,
                bool cache_hash_code = false >
    class unordered :
        #ifdef HAVE_TR1_UNORDERED_MAP
            public std::tr1::unordered_map<Key, Data, Hash, Pred, Alloc, cache_hash_code> {};
        #else
            public std::map<Key, Data, std::less<Key>, Alloc> {};
        #endif

} // namespace maps


#endif
