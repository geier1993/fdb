/*
 * (C) Copyright 1996-2016 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Baudouin Raoult
/// @author Tiago Quintino
/// @date   April 2016

#ifndef fdb5_TocDbStats_H
#define fdb5_TocDbStats_H

#include <set>
#include <map>
#include <iosfwd>

#include "eckit/filesystem/PathName.h"

#include "fdb5/database/DbStats.h"
#include "fdb5/database/IndexStats.h"

namespace fdb5 {

//----------------------------------------------------------------------------------------------------------------------


class TocDbStats : public DbStatsContent {
public:

    TocDbStats();

    size_t tocRecordsCount_;

    unsigned long long tocFileSize_;
    unsigned long long schemaFileSize_;
    unsigned long long ownedFilesSize_;
    unsigned long long adoptedFilesSize_;
    unsigned long long indexFilesSize_;

    size_t ownedFilesCount_;
    size_t adoptedFilesCount_;
    size_t indexFilesCount_;

    TocDbStats& operator+= (const TocDbStats& rhs);

    virtual void add(const DbStatsContent&);

    virtual void report(std::ostream &out, const char* indent) const;
};


//----------------------------------------------------------------------------------------------------------------------


class TocIndexStats : public IndexStatsContent {
public:

    TocIndexStats();

    size_t fieldsCount_;
    size_t duplicatesCount_;

    unsigned long long fieldsSize_;
    unsigned long long duplicatesSize_;


    TocIndexStats& operator+= (const TocIndexStats& rhs);

    virtual void add(const IndexStatsContent&);

    virtual void report(std::ostream &out, const char* indent) const;
};


//----------------------------------------------------------------------------------------------------------------------


class TocDataStats : public IndexStatsContent {
public:

    TocDataStats();

    std::set<eckit::PathName> allDataFiles_;
    std::set<eckit::PathName> activeDataFiles_;
    std::map<eckit::PathName, size_t> dataUsage_;

    TocDataStats& operator+= (const TocDataStats& rhs);

    virtual void add(const IndexStatsContent&);

    virtual void report(std::ostream &out, const char* indent) const;
};


//----------------------------------------------------------------------------------------------------------------------

} // namespace fdb5

#endif
