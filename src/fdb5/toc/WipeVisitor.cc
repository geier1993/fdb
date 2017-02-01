/*
 * (C) Copyright 1996-2016 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include <sys/stat.h>
#include <dirent.h>

#include "eckit/log/Bytes.h"
#include "eckit/log/Plural.h"

#include "fdb5/toc/TocHandler.h"
#include "fdb5/toc/TocIndex.h"
#include "fdb5/toc/WipeVisitor.h"

namespace fdb5 {

//----------------------------------------------------------------------------------------------------------------------

class StdDir {

    DIR *d_;

  public:

    StdDir(const eckit::PathName &p) {
        d_ = opendir(p.localPath());
    }

    ~StdDir() {
        if (d_) closedir(d_);
    }

    operator DIR *() {
        return d_;
    }

};


//----------------------------------------------------------------------------------------------------------------------

WipeVisitor::WipeVisitor(TocDB& db) : TocReportVisitor(db) {

    const eckit::PathName& directory = db_.directory();

    eckit::Log::info() << "Scanning "
                       << directory
                       << std::endl;

    scan(directory.realName());

    eckit::Log::info() << "Found "
                       << eckit::Plural(files_.size(), "file")
                       << ", size: "
                       << eckit::Bytes(total_)
                       << std::endl;

}

// eckit PathName::match() does not return hidden files starting with '.'
void WipeVisitor::scan(const eckit::PathName &directory) {


    StdDir d(directory);

    if (d == 0) {
        throw eckit::FailedSystemCall(std::string("opendir(") + std::string(directory) + ")");
    }

    struct dirent *e;

    while ( (e = readdir(d)) != 0) {

        if (e->d_name[0] == '.') {
            if (e->d_name[1] == 0 || (e->d_name[1] == '.' && e->d_name[2] == 0)) {
                continue;
            }
        }

        eckit::PathName path(directory / e->d_name);
        files_.insert( path );

        eckit::Stat::Struct info;
        SYSCALL(eckit::Stat::lstat(path.localPath(), &info));

        if (S_ISDIR(info.st_mode)) {
            scan(path);
        } else {
            total_ += path.size();
        }
    }
}

const eckit::PathName &WipeVisitor::mark(const eckit::PathName &path) const {
    std::set<eckit::PathName>::iterator j = files_.find(path.realName());
    if (j == files_.end()) {
        std::ostringstream oss;
        oss << "Unexpected file " << path << ". The file may have been added after the start of the command";
        throw eckit::UserError(oss.str());
    }
    files_.erase(j);
    return path;
}


void WipeVisitor::report(std::ostream &out) const {

    const eckit::PathName& directory = db_.directory();

    TocHandler handler(directory);
    out << "FDB owner: " << handler.dbOwner() << std::endl;
    out << std::endl;

    out << "Metadata files to delete:" << std::endl;
    out << "    " << mark(handler.tocPath());
    out << "    " << mark(handler.schemaPath());

    out << std::endl;

    size_t cnt = 0;
    out << "Data files to delete:" << std::endl;
    for (std::map<eckit::PathName, size_t>::const_iterator i = dataUsage_.begin(); i != dataUsage_.end(); ++i) {
        if (i->first.dirName().sameAs(directory)) {
            out << "    " << mark(i->first) << std::endl;
            cnt++;
        }
    }
    if (cnt == 0) {
        out << "    - NONE -" << std::endl;
    }
    out << std::endl;

    cnt = 0;
    out << "Index files to be deleted:" << std::endl;
    for (std::map<eckit::PathName, size_t>::const_iterator i = indexUsage_.begin(); i != indexUsage_.end(); ++i) {
        out << "    " << mark(i->first) << std::endl;
        cnt++;
    }
    if (cnt == 0) {
        out << "    - NONE -" << std::endl;
    }
    out << std::endl;

    cnt = 0;
    out << "Other files:" << std::endl;
    for (std::set<eckit::PathName>::const_reverse_iterator i = files_.rbegin(); i != files_.rend(); ++i) {
        out << "    " << (*i) << std::endl;
        cnt++;
    }
    if (cnt == 0) {
        out << "    - NONE -" << std::endl;
    }
    out << std::endl;

}


void WipeVisitor::wipe(std::ostream& out) const {

    const eckit::PathName& directory = db_.directory();

    db_.checkUID();

    db_.tocPath().unlink();
    db_.schemaPath().unlink();

    for (std::map<eckit::PathName, size_t>::const_iterator i = dataUsage_.begin(); i != dataUsage_.end(); ++i) {
        if (i->first.dirName().sameAs(directory)) {
            i->first.unlink();
        }
    }

    for (std::map<eckit::PathName, size_t>::const_iterator i = indexUsage_.begin(); i != indexUsage_.end(); ++i) {
        i->first.unlink();
    }

    for (std::set<eckit::PathName>::const_reverse_iterator i = files_.rbegin(); i != files_.rend(); ++i) {
        if (i->isDir() && !i->isLink()) {
            i->rmdir();
        } else {
            i->unlink();
        }
    }

    directory.rmdir();
}


//----------------------------------------------------------------------------------------------------------------------

} // namespace fdb5
