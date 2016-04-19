/*
 * (C) Copyright 1996-2016 ECMWF.
 * 
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0. 
 * In applying this licence, ECMWF does not waive the privileges and immunities 
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @file   Archiver.h
/// @author Baudouin Raoult
/// @author Tiago Quintino
/// @date   Mar 2016

#ifndef fdb5_Archiver_H
#define fdb5_Archiver_H

#include "eckit/memory/NonCopyable.h"
#include "eckit/filesystem/PathName.h"
#include "eckit/io/Length.h"
#include "eckit/io/DataBlob.h"

#include "fdb5/DB.h"

namespace eckit   { class DataHandle; }

class MarsTask;

namespace fdb5 {

class Key;

//----------------------------------------------------------------------------------------------------------------------

class Archiver : public eckit::NonCopyable {

public: // methods

    Archiver();
    
    ~Archiver();

    void write(const eckit::DataBlobPtr blob);

    /// Archives the data in the buffer and described by the fdb5::Key
    /// @param key metadata identifying the data
    /// @param data buffer
    /// @param length buffer length
    ///
    void write(const Key& key, const void* data, size_t length);

    void adopt(const Key& key, const eckit::PathName& path, eckit::Offset offset, eckit::Length length);

    /// Flushes all buffers and closes all data handles into a consistent DB state
    /// @note always safe to call
    void flush();

    friend std::ostream& operator<<(std::ostream& s, const Archiver& x) { x.print(s); return s; }

private: // methods

    void print(std::ostream& out) const;

    DB& session(const Key& key);

private: // members

    friend class BaseArchiveVisitor;

    typedef std::map< Key, eckit::SharedPtr<DB> > store_t;

    store_t databases_;

    std::string fdbWriterDB_;

    std::vector<Key> prev_;

    DB* current_;

};

//----------------------------------------------------------------------------------------------------------------------

} // namespace fdb5

#endif
