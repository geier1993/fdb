/*
 * (C) Copyright 1996-2016 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @file   Retriever.h
/// @author Baudouin Raoult
/// @author Tiago Quintino
/// @date   Mar 2016

#ifndef fdb5_Retriever_H
#define fdb5_Retriever_H

#include <iosfwd>
#include <cstdlib>
#include <map>

#include "eckit/memory/NonCopyable.h"

namespace eckit {
class DataHandle;
}

class MarsTask;

namespace fdb5 {

class Key;
class Op;
class DB;
class Schema;

//----------------------------------------------------------------------------------------------------------------------

class Retriever : public eckit::NonCopyable {

public: // methods

    Retriever();

    ~Retriever();

    /// Retrieves the data selected by the MarsRequest to the provided DataHandle
    /// @returns  data handle to read from

    eckit::DataHandle *retrieve(const MarsTask &task) const;

    friend std::ostream &operator<<(std::ostream &s, const Retriever &x) {
        x.print(s);
        return s;
    }

private: // methods

    void print(std::ostream &out) const;

    eckit::DataHandle *retrieve(const MarsTask &task, const Schema &schema, bool sorted) const;

private: // data

    const Schema& schema_;

    mutable std::map<Key,DB*> databases_;

};

//----------------------------------------------------------------------------------------------------------------------

} // namespace fdb5

#endif
