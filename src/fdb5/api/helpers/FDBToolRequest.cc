/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "fdb5/api/helpers/FDBToolRequest.h"

#include <algorithm>

#include "metkit/MarsExpension.h"
#include "metkit/MarsParser.h"
#include "metkit/MarsRequest.h"

#include "eckit/exception/Exceptions.h"

#include "fdb5/LibFdb5.h"


namespace fdb5 {

//----------------------------------------------------------------------------------------------------------------------


std::vector<FDBToolRequest> FDBToolRequest::requestsFromString(const std::string& request_str,
                                                               const std::vector<std::string> minimumKeys,
                                                               bool raw,
                                                               const std::string& verb) {


    std::string full_string = verb + "," + request_str; // Use a dummy verb
    std::istringstream in(full_string);
    metkit::MarsParser parser(in);
    auto parsedRequests = parser.parse();
    ASSERT(parsedRequests.size() == 1);

    for (const auto& r : parsedRequests) {
        eckit::Log::debug<LibFdb5>() << "Parsed request: " << static_cast<const metkit::MarsRequest&>(r) << std::endl;
        checkMinimumKeys(r, minimumKeys);
    }

    std::vector<FDBToolRequest> requests;

    if (raw) {
        std::copy(parsedRequests.begin(), parsedRequests.end(), std::back_inserter(requests));
    } else {

        // We want to use (default) inherited requests, as this allows use to use
        // TypeParam with a certain amount of meaning. But we also want to be able
        // to use sparse requests for the FDB. So we unset anything that has not
        // been originally specified by the user.

        auto&& ps(parsedRequests.front().params());
        std::set<std::string> setParams(std::make_move_iterator(ps.begin()),
                                        std::make_move_iterator(ps.end()));

        bool inherit = true;
        metkit::MarsExpension expand(inherit);
        auto expandedRequests = expand.expand(parsedRequests);

        for (metkit::MarsRequest& request : expandedRequests) {

            for (const auto& param : request.params()) {
                if (std::find(setParams.begin(), setParams.end(), param) == setParams.end()) {
                    request.unsetValues(param);
                }
            }
            eckit::Log::debug<LibFdb5>() << "Expanded request: " << request << std::endl;
            requests.emplace_back(FDBToolRequest(request, false, minimumKeys));
        }
    }

    return requests;
}


FDBToolRequest::FDBToolRequest(const metkit::MarsRequest& r,
                               bool all,
                               const std::vector<std::string>& minimumKeySet) :
    request_(r),
    all_(all) {

    checkMinimumKeys(request_, minimumKeySet);
}

FDBToolRequest::FDBToolRequest(eckit::Stream& s) :
    request_(s) {
    s >> all_;
}

const metkit::MarsRequest& FDBToolRequest::request() const {
    return request_;
}

bool FDBToolRequest::all() const {
    return all_;

}

void FDBToolRequest::print(std::ostream &s, const char* cr, const char* tab) const {

    if (all_) {
        s << " -- ALL --";
    } else {
        request_.dump(s, cr, tab);
    }
}

void FDBToolRequest::encode(eckit::Stream& s) const {
    s << request_;
    s << all_;
}

void FDBToolRequest::checkMinimumKeys(const metkit::MarsRequest& request, const std::vector<std::string>& minimumKeys) {
    for (std::vector<std::string>::const_iterator j = minimumKeys.begin(); j != minimumKeys.end(); ++j) {
        if (!request.has(*j)) {
            throw eckit::UserError("Please provide a value for '" + (*j) + "'");
        }
    }
}

//----------------------------------------------------------------------------------------------------------------------

} // namespace fdb5

