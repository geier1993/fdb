/*
 * (C) Copyright 1996-2016 ECMWF.
 * 
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0. 
 * In applying this licence, ECMWF does not waive the privileges and immunities 
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @file   TocDBReader.h
/// @author Baudouin Raoult
/// @author Tiago Quintino
/// @date   Mar 2016

#ifndef fdb_TocDBReader_H
#define fdb_TocDBReader_H

#include "fdb5/TocDB.h"
#include "fdb5/TocActions.h"

namespace fdb5 {

//----------------------------------------------------------------------------------------------------------------------

/// DB that implements the FDB on POSIX filesystems

class TocDBReader : public TocDB {

public: // methods

    TocDBReader(const Key& key);

    virtual ~TocDBReader();

private: // methods

    Index::Field findField(const Key& key);

    virtual eckit::DataHandle* retrieve(const MarsTask& task, const Key& key) const;

    /// Opens an Index with the associated path
    virtual Index* openIndex( const eckit::PathName& path ) const;

    virtual void print( std::ostream& out ) const;

private: // members

    TocReverseIndexes toc_;

};

//----------------------------------------------------------------------------------------------------------------------

} // namespace fdb5

#endif
