//
// Created by 张德文 on 2022/8/16.
//

#include "mapped_file.h"

#include <iostream>
#include <fcntl.h>
#include <sys/mman.h>  // 定义mmap和munmap函数
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "system_failure.h"


void mapped_file_params_base::normalize() {
    // TODO
}

mapped_file_source::mapped_file_source()
: m_pimpl(new impl_type) {

}

template<typename Path>
mapped_file_source::mapped_file_source(const basic_mapped_file_params<Path>& p) {
    init();
    open(p);
}

template<typename Path>
mapped_file_source::mapped_file_source(const Path& path,
                                       size_type  length,
                                       intmax_t offset) {
    init();
    open(path, length, offset);
}

template<typename Path>
void mapped_file_source::open(const basic_mapped_file_params<Path>& p) {
    param_type params(p);

    open_impl(params);
}

template<typename Path>
void mapped_file_source::open(const Path& path,
                              size_type  length,
                              intmax_t offset) {
    param_type params(path);
    params.length = length;
    params.offset = offset;
    open_impl(params);
}

bool mapped_file_source::is_open() const {
    return m_pimpl->is_open();
}

void mapped_file_source::close() {
    m_pimpl->close();
}

// safe_bool is explicitly qualified below to please msvc 7.1
mapped_file_source::operator mapped_file_source::safe_bool() const
{ return m_pimpl->error() ? &safe_bool_helper::x : 0; }

bool mapped_file_source::operator!() const
{ return m_pimpl->error(); }

mapped_file_source::mapmode mapped_file_source::flags() const
{ return m_pimpl->flags(); }

mapped_file_source::size_type mapped_file_source::size() const
{ return m_pimpl->size(); }

void mapped_file_source::open_impl(const param_type& p) {
    m_pimpl->open(p);
}

const char* mapped_file_source::data() const {
    return m_pimpl->data();
}

const char* mapped_file_source::begin() const {
    return data();
}

const char* mapped_file_source::end() const {
    return data() + size();
}

int mapped_file_source::alignment() {
    return mapped_file_impl::alignment();
}

void mapped_file_source::init() {
    m_pimpl.reset(new impl_type);
}


mapped_file::mapped_file(const mapped_file& other) : m_delegate(other.m_delegate) {

}

template<typename Path>
mapped_file::mapped_file(const basic_mapped_file_params<Path>& p) {
    open(p);
}

template<typename Path>
mapped_file::mapped_file(const Path& path,
            mapmode flags,
            size_type length,
            stream_offset offset) {
    open(path, flags, length, offset);
}

template<typename Path>
void mapped_file::open(const Path& path,
                       mapmode flags,
                       size_type length,
                       stream_offset offset) {
    param_type p(path);
    p.flags = flags;
    p.length = length;
    p.offset = offset;
    open(p);
}

template<typename Path>
void mapped_file::open(const Path& path,
          size_type length,
          stream_offset offset) {
    param_type p(path);
    p.length = length;
    p.offset = offset;
    open(p);
}

void mapped_file::resize(stream_offset new_size) {
    m_delegate.m_pimpl->resize(new_size);
}

mapped_file_impl::mapped_file_impl() {
    clear(false);
}

mapped_file_impl::~mapped_file_impl() {
    try {
        close();
    } catch (...) {

    }
}

void mapped_file_impl::open(param_type p) {
    if (is_open()) {
        return;
    }
    p.normalize();
    open_file(p);
    map_file(p);
    m_params = p;
}

void mapped_file_impl::close() {
    if (m_data == nullptr) {
        return;
    }
    bool error = false;
    error = !unmap_file() || error;
    if (m_handle >= 0) {
        error = ::close(m_handle) != 0 || error;
    }
    clear(error);
    if (error) {
        throw_system_failure("failed closing mapped file");
    }
}

void mapped_file_impl::resize(stream_offset new_size) {
    if (!is_open()) {
        system_failure("file is closed");
        return;
    }
    if (flags() & mapped_file::priv) {
        system_failure("can't resize private mapped file");
        return;
    }

    if (!(flags() & mapped_file::readwrite)) {
        system_failure("can't resize readonly mapped file");
        return;
    }
    if (m_params.offset >= new_size) {
        system_failure("can't resize below mapped offset");
        return;
    }
    if (!unmap_file()) {
        cleanup_and_throw("failed unmapping file");
        return;
    }

    if (ftruncate(m_handle, new_size) == -1) {
        cleanup_and_throw("failed resizing mapped file");
        return;
    }

    m_size = new_size;
    param_type p(m_params);
    map_file(p);
    m_params = p;
}

int mapped_file_impl::alignment() {
    return static_cast<int>(sysconf(_SC_PAGESIZE));
}

void mapped_file_impl::open_file(param_type p) {
    bool readonly = p.flags != mapped_file::readwrite;

    int flags = (readonly ? O_RDONLY : O_RDWR);
    if (p.new_file_size != 0 && !readonly) {
        flags |= (O_CREAT | O_TRUNC);
    }
#ifdef _LARGFILE64_SOURCE
    flags |= O_LARGEFILE;
#endif
    errno = 0;
    m_handle = ::open(p.path.c_str(), flags, S_IRWXU);
    if (errno != 0) {
        cleanup_and_throw("failed opening file");
        return;
    }

    if (p.new_file_size != 0 && !readonly) {
        if (ftruncate(m_handle, p.new_file_size) == -1) {
            cleanup_and_throw("failed setting file size");
            return;
        }
    }

    bool success = true;
    if (p.length != max_length) {
        m_size = p.length;
    } else {
        struct stat info;
        success = ::fstat(m_handle, &info) != -1;
        m_size = info.st_size;
    }

    if (!success) {
        cleanup_and_throw("failed querying file size");
        return;
    }
}

void mapped_file_impl::try_map_file(param_type p) {
    bool priv = p.flags == mapped_file::priv;
    bool readonly = p.flags == mapped_file::readonly;

    // mmap 函数是 unix/linux下的系统调用。
    void* data = ::mmap(const_cast<char*>(p.hint),
                        m_size,
                        readonly? PROT_READ : (PROT_READ | PROT_WRITE),
                        priv ? MAP_PRIVATE : MAP_SHARED,
                        m_handle,
                        p.offset);

    if (data == MAP_FAILED) {
        cleanup_and_throw("failed mapping file");
        return;
    }

    m_data = static_cast<char*>(data);
}

void mapped_file_impl::map_file(param_type &p) {
    try {
        try_map_file(p);
    } catch (const std::exception&) {
        if (p.hint) {
            p.hint = nullptr;
            try_map_file(p);
        } else {
            throw;
        }
    }
}

bool mapped_file_impl::unmap_file() {
    return ::munmap(m_data, m_size) == 0;
}

void mapped_file_impl::clear(bool error) {
    m_params = param_type();
    m_data = nullptr;
    m_size = 0;
    m_handle = 0;
    m_error = error;
}

void mapped_file_impl::cleanup_and_throw(const char *msg) {

    int error = errno;
    if (m_handle >= 0) {
        ::close(m_handle);
    }

    clear(true);
    try {
        throw_system_failure(msg);
    } catch (...) {

    }
}