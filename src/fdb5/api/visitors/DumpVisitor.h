/*
 * (C) Copyright 2018- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Simon Smart
/// @date   November 2018

#ifndef fdb5_api_visitor_DumpVisitor_H
#define fdb5_api_visitor_DumpVisitor_H

#include "fdb5/api/visitors/QueryVisitor.h"
#include "fdb5/api/visitors/QueueStringLogTarget.h"
#include "fdb5/api/helpers/DumpIterator.h"
#include "fdb5/database/DB.h"

namespace fdb5 {
namespace api {
namespace visitor {

/// @note Helper classes for LocalFDB

//----------------------------------------------------------------------------------------------------------------------

class DumpVisitor : public QueryVisitor<DumpElement> {

public:

    DumpVisitor(eckit::Queue<std::string>& queue, bool simple) :
        QueryVisitor(queue),
        out_(new QueueStringLogTarget(queue)),
        simple_(simple) {}

    bool visitIndexes() override { return false; }
    bool visitEntries() override { return false; }

    void visitDatabase(const DB& db) override {
        db.dump(out_, simple_);
    }
    void visitIndex(const Index&) override { NOTIMP; }
    void visitDatum(const Field&, const Key&) override { NOTIMP; }

private:
    eckit::Channel out_;
    bool simple_;
};

//----------------------------------------------------------------------------------------------------------------------

} // namespace visitor
} // namespace api
} // namespace fdb5

#endif
