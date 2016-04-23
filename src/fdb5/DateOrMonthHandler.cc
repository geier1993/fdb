/*
 * (C) Copyright 1996-2016 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "eckit/exception/Exceptions.h"
#include "eckit/utils/Translator.h"

#include "eckit/types/Date.h"
#include "marslib/MarsRequest.h"

#include "fdb5/KeywordType.h"
#include "fdb5/DateOrMonthHandler.h"

namespace fdb5 {

//----------------------------------------------------------------------------------------------------------------------

DateOrMonthHandler::DateOrMonthHandler(const std::string &name, const std::string &type) :
    KeywordHandler(name, type) {
}

DateOrMonthHandler::~DateOrMonthHandler() {
}

void DateOrMonthHandler::getValues(const MarsRequest &request,
                                   const std::string &keyword,
                                   eckit::StringList &values,
                                   const MarsTask &task,
                                   const DB *db) const {
    std::vector<eckit::Date> dates;
    request.getValues(keyword, dates);

    if (dates.empty()) {
        std::vector<long> months;
        request.getValues("month", months);

        for (std::vector<long>::const_iterator i = months.begin(); i != months.end(); ++i) {
            dates.push_back((*i) * 100 + 1);
        }
    }

    eckit::Translator<long, std::string> t;

    values.reserve(dates.size());

    for (std::vector<eckit::Date>::const_iterator i = dates.begin(); i != dates.end(); ++i) {
        values.push_back(t(i->yyyymmdd()));
    }
}

void DateOrMonthHandler::print(std::ostream &out) const {
    out << "DateOrMonthHandler(" << name_ << ")";
}

static KeywordHandlerBuilder<DateOrMonthHandler> handler("DateOrMonth");

//----------------------------------------------------------------------------------------------------------------------

} // namespace fdb5
