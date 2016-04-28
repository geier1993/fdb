/*
 * (C) Copyright 1996-2016 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @file   Type.h
/// @author Baudouin Raoult
/// @author Tiago Quintino
/// @date   April 2016

#ifndef fdb5_Type_H
#define fdb5_Type_H

#include <string>

#include "eckit/memory/NonCopyable.h"
#include "eckit/types/Types.h"

class MarsTask;
class MarsRequest;

namespace fdb5 {

class DB;

//----------------------------------------------------------------------------------------------------------------------

class Type : private eckit::NonCopyable {

public: // methods

    Type(const std::string& name, const std::string& type);

    virtual ~Type();

    virtual void toKey(std::ostream& out,
                       const std::string& keyword,
                       const std::string& value) const ;

    virtual void getValues(const MarsRequest& request,
                           const std::string& keyword,
                           eckit::StringList& values,
                           const MarsTask& task,
                           const DB* db) const;

    friend std::ostream& operator<<(std::ostream& s,const Type& x);

public: // class methods

    static const Type& lookup(const std::string& keyword);

    const std::string& type() const;

private: // methods

    virtual void print( std::ostream& out ) const = 0;

protected: // members

    std::string name_;
    std::string type_;

};

//----------------------------------------------------------------------------------------------------------------------

} // namespace fdb5

#endif
