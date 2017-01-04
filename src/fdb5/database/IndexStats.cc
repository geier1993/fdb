/*
 * (C) Copyright 1996-2016 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "fdb5/database/IndexStats.h"

namespace fdb5 {

//----------------------------------------------------------------------------------------------------------------------

class NullIndexStats : public IndexStatsContent {
public:

    virtual void add(const IndexStatsContent&) {
        NOTIMP;
    }

    virtual void report(std::ostream& out, const char* indent) const {
        NOTIMP;
    }
};

//----------------------------------------------------------------------------------------------------------------------


IndexStats::IndexStats() :
    content_(new NullIndexStats()) {
    content_->attach();
}

IndexStats::IndexStats(IndexStatsContent* p) :
    content_(p) {
    content_->attach();
}

IndexStats::~IndexStats() {
   content_->detach();
}

IndexStats::IndexStats(const IndexStats& s) : content_(s.content_) {
    content_->attach();
}

IndexStats& IndexStats::operator=(const IndexStats& s) {
    content_->detach();
    content_ = s.content_;
    content_->attach();
    return *this;
}

void IndexStats::add(const IndexStats& s)
{
    content_->add(*s.content_);
}

void IndexStats::report(std::ostream& out, const char* indent) const
{
    content_->report(out, indent);
}

IndexStatsContent::~IndexStatsContent()
{
}


//----------------------------------------------------------------------------------------------------------------------

} // namespace fdb5
