/*
 * (C) Copyright 1996-2013 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "eckit/option/SimpleOption.h"
#include "eckit/option/CmdArgs.h"

#include "fdb5/tools/FDBTool.h"
#include "fdb5/database/Index.h"
#include "fdb5/rules/Schema.h"
#include "fdb5/toc/TocHandler.h"

using namespace std;
using namespace eckit;
using namespace fdb5;

//----------------------------------------------------------------------------------------------------------------------

class ListVisitor : public EntryVisitor {
 public:
    ListVisitor(const Key& dbKey,
                const fdb5::Schema& schema,
                const eckit::option::CmdArgs& args) :
        dbKey_(dbKey),
        schema_(schema),
        args_(args)
    {
    }

private:
    virtual void visit(const Index& index,
                       const std::string &indexFingerprint,
                       const std::string &fieldFingerprint,
                       const eckit::PathName &path,
                       eckit::Offset offset,
                       eckit::Length length);

    const Key& dbKey_;
    const fdb5::Schema& schema_;
    const eckit::option::CmdArgs& args_;
};

void ListVisitor::visit(const Index& index,
                        const std::string &indexFingerprint,
                        const std::string &fieldFingerprint,
                        const eckit::PathName &path,
                        eckit::Offset offset,
                        eckit::Length length) {

    Key field(fieldFingerprint, schema_.ruleFor(dbKey_, index.key()));

    std::cout << dbKey_ << index.key() << field;

    bool location = false;
    args_.get("location", location);

    if(location) { std::cout << " (" <<  path << ", " << offset << ", " << length << ")"; }

    std::cout << std::endl;

}

//----------------------------------------------------------------------------------------------------------------------

class FDBList : public FDBTool {

public: // methods

    FDBList(int argc, char **argv) : FDBTool(argc, argv) {
        options_.push_back(new eckit::option::SimpleOption<bool>("location", "Also print the location of each field"));

    }

private: // methods

    virtual void run();

    static void usage(const std::string &tool);

    void listToc(const eckit::PathName& path, const option::CmdArgs &args);

};

void FDBList::usage(const std::string &tool) {

    eckit::Log::info() << std::endl << "Usage: " << tool << " [--location] [path1|request1] [path2|request2] ..." << std::endl;
    FDBTool::usage(tool);
}

void FDBList::listToc(const eckit::PathName& path, const option::CmdArgs& args) {

    Log::info() << "Listing " << path << std::endl;

    fdb5::TocHandler handler(path);
    Key key = handler.databaseKey();
    Log::info() << "Database key " << key << std::endl;

    fdb5::Schema schema(path / "schema");

    std::vector<Index *> indexes = handler.loadIndexes();
    ListVisitor visitor(key, schema, args);

    for (std::vector<Index *>::const_iterator i = indexes.begin(); i != indexes.end(); ++i) {
        (*i)->entries(visitor);
    }

    handler.freeIndexes(indexes);
}

void FDBList::run() {

    eckit::option::CmdArgs args(&FDBList::usage, -1, options_);

    Log::info() << args << std::endl;

    for (size_t i = 0; i < args.count(); ++i) {
        listToc( databasePath(args.args(i)) , args);
    }
}

//----------------------------------------------------------------------------------------------------------------------

int main(int argc, char **argv) {
    FDBList app(argc, argv);
    return app.start();
}

