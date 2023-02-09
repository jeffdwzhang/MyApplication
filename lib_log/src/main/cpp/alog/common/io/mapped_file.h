//
// Created by 张德文 on 2022/8/16.
//

#ifndef MYAPPLICATION_MAPPED_FILE_H
#define MYAPPLICATION_MAPPED_FILE_H

#include <cstddef>
#include <string>
#include <filesystem>
#include "positioning.h"

#include "file_handle.h"


// Forward declarations
class mapped_file_source;
class mapped_file_sink;
class mapped_file;
class mapped_file_impl;

class mapped_file_base {
public:
    enum mapmode {
        readonly = 1,
        readwrite = 2,
        priv = 4
    };
};

struct mapped_file_params_base {

    mapped_file_params_base()
    : flags(static_cast<mapped_file_base::mapmode>(0)),
    offset(0), length(static_cast<std::size_t>(-1)), new_file_size(0), hint(nullptr) {}

private:
    friend class mapped_file_impl;
    void normalize();

public:
    mapped_file_base::mapmode flags;
    stream_offset             offset;
    std::size_t               length;
    stream_offset             new_file_size;
    const char*               hint;
};


template<typename Path>
struct basic_mapped_file_params : mapped_file_params_base
{
    typedef mapped_file_params_base base_type;

    // Default constructor
    basic_mapped_file_params() { }

    // Construction from a Path
    explicit basic_mapped_file_params(const Path& p) : path(p) { }

//    // Construction from a path of a different type
    template<typename PathT>
    explicit basic_mapped_file_params(const PathT& p) {
//        path = p;
    }

    basic_mapped_file_params(const basic_mapped_file_params& other)
    : base_type(other), path(other.path)
    { }

    template<typename PathT>
    basic_mapped_file_params(const basic_mapped_file_params<PathT>& other)
            : base_type(other), path(other.path)
    { }

    typedef Path path_type;
    Path path;
};

typedef basic_mapped_file_params<std::string> mapped_file_params;

class mapped_file_source : public mapped_file_base {
private:
    struct safe_bool_helper {
        int x;
    };

    typedef int safe_bool_helper::*       safe_bool;
    typedef mapped_file_impl              impl_type;
    typedef basic_mapped_file_params<std::filesystem::path> param_type;
    friend class mapped_file;
    friend class mapped_file_impl;

public:

    typedef char     char_type;

    typedef std::size_t      size_type;
    typedef const char*      iterator;
    static const size_type max_length =  static_cast<size_type>(-1);

    // 默认构造器
    mapped_file_source();

    template<typename Path>
    explicit mapped_file_source(const basic_mapped_file_params<Path>& p);

    template<typename Path>
    explicit mapped_file_source(const Path& path,
                                size_type  length = max_length,
                                intmax_t offset = 0);

//    mapped_file_source(mapped_file_source& other);

    template<typename Path>
    void open(const basic_mapped_file_params<Path>& p);

    template<typename Path>
    void open(const Path& path,
              size_type  length = max_length,
              intmax_t offset = 0);

    bool is_open() const;
    void close();
    operator safe_bool() const;
    bool operator!() const;
    mapmode flags() const;

    size_type size() const;
    const char* data() const;
    iterator begin() const;
    iterator end() const;

    static int alignment();

private:
    void init();
    void open_impl(const param_type& p);

    std::shared_ptr<impl_type> m_pimpl;
};

class mapped_file : public mapped_file_base {
private:
    typedef mapped_file_source         delegate_type;
    typedef delegate_type::safe_bool   safe_bool;
    typedef basic_mapped_file_params<std::filesystem::path> param_type;

    friend class mapped_file_sink;

public:

    typedef char           char_type;
    typedef mapped_file_source::size_type  size_type;
    typedef char*          iterator;
    typedef const char*    const_iterator;

    static const size_type max_length = delegate_type::max_length;

    mapped_file() {}

    template<typename Path>
    explicit mapped_file(const basic_mapped_file_params<Path>& p);

    template<typename Path>
    mapped_file(const Path& path,
                mapmode flags,
                size_type length = max_length,
                stream_offset offset = 0);

    mapped_file(const mapped_file& other);

    template<typename Path>
    void open(const basic_mapped_file_params<Path>& p) {
        m_delegate.open_impl(p);
    }

    template<typename Path>
    void open(const Path& path,
              mapmode flags,
              size_type length = max_length,
              stream_offset offset = 0);

    template<typename Path>
    void open(const Path& path,
              size_type length = max_length,
              stream_offset offset = 0);

    operator mapped_file_source&() {
        return m_delegate;
    }

    operator const mapped_file_source&() const {
        return m_delegate;
    }

    bool is_open() const {
        return m_delegate.is_open();
    }
    void close() {
        m_delegate.close();
    }
    operator safe_bool() const {
        return m_delegate;
    }
    bool operator!() {
        return !m_delegate;
    }
    mapmode flags() const {
        return m_delegate.flags();
    }

    size_type size() const {
        return m_delegate.size();
    }

    char* data() const {
        return (flags() != readonly) ? const_cast<char *>(m_delegate.data()) : nullptr;
    }

    const char* const_data() const {
        return m_delegate.data();
    }

    iterator begin() const {
        return data();
    }

    const_iterator const_begin() const {
        return const_data();
    }

    iterator end() const {
        return data() + size();
    }
    const_iterator const_end() const {
        return const_data() + size();
    }

    static int alignment() {
        return mapped_file_source::alignment();
    }

    void resize(stream_offset new_size);

private:
    delegate_type m_delegate;
};

class mapped_file_sink : private mapped_file {

public:

    using mapped_file::mapmode;
    using mapped_file::readonly;
    using mapped_file::readwrite;
    using mapped_file::priv;

    using mapped_file::max_length;
    using mapped_file::is_open;
    using mapped_file::close;
    using mapped_file::operator safe_bool;

    mapped_file_sink();

    template<typename Path>
    void open(const Path& path,
              size_type  length = max_length,
              intmax_t offset = 0,
              mapmode flags = readwrite);

};

class mapped_file_impl {

public:

    typedef mapped_file_source::size_type size_type;
    typedef mapped_file_source::param_type param_type;
    typedef mapped_file_source::mapmode    mapmode;
    static const size_type max_length =  mapped_file_source::max_length;

    mapped_file_impl();
    ~mapped_file_impl();

    void open(param_type p);
    bool is_open() const { return m_data != nullptr && m_handle >= 0; }
    void close();
    bool error() const {
        return m_error;
    }
    mapmode flags() const {
        return m_params.flags;
    }
    std::size_t size() const {
        return m_size;
    }
    char* data() const {
        return m_data;
    }
    void resize(stream_offset new_size);
    static int alignment();
private:

    void open_file(param_type p);
    void try_map_file(param_type p);
    void map_file(param_type& p);
    bool unmap_file();
    void clear(bool error);
    void cleanup_and_throw(const char* msg);
    param_type m_params;
    char*  m_data;
    stream_offset m_size;
    file_handle m_handle;
    bool m_error;
};

#endif //MYAPPLICATION_MAPPED_FILE_H
