/*
 * (C) Copyright 1996-2017 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "mars_server_config.h"

#include "eckit/config/Resource.h"
#include "eckit/log/Log.h"
#include "eckit/log/Bytes.h"
#include "eckit/io/AIOHandle.h"

#include "fdb5/LibFdb.h"

#include "fdb5/io/FDBFileHandle.h"

#include "fdb5/toc/TocDBWriter.h"
#include "fdb5/toc/TocIndex.h"
#include "fdb5/toc/TocFieldLocation.h"

namespace fdb5 {

//----------------------------------------------------------------------------------------------------------------------


TocDBWriter::TocDBWriter(const Key &key, const eckit::Configuration& config) :
    TocDB(key, config),
    dirty_(false) {
    writeInitRecord(key);
    loadSchema();
    checkUID();
}

TocDBWriter::TocDBWriter(const eckit::PathName &directory, const eckit::Configuration& config) :
    TocDB(directory, config),
    dirty_(false) {

    NOTIMP; // TODO: Not clear what should occur here for writeInitRecord.

    loadSchema();
    checkUID();
}

TocDBWriter::~TocDBWriter() {
    close();
}

bool TocDBWriter::selectIndex(const Key& key) {
    currentIndexKey_ = key;

    if (indexes_.find(key) == indexes_.end()) {
        PathName indexPath(generateIndexPath(key));

        // Enforce lustre striping if requested
        if (stripeLustre()) {
            LustreStripe stripe = stripeIndexLustreSettings();
            fdb5LustreapiFileCreate(indexPath.localPath(), stripe.size_, stripe.count_);
        }

        indexes_[key] = Index(new TocIndex(key, generateIndexPath(key), 0, TocIndex::WRITE));
    }

    current_ = indexes_[key];
    current_.open();

    return true;
}

void TocDBWriter::deselectIndex() {
    current_ = Index();
    currentIndexKey_ = Key();
}

bool TocDBWriter::open() {
    return true;
}

void TocDBWriter::close() {

    eckit::Log::debug<LibFdb>() << "Closing path " << directory_ << std::endl;

    flush(); // closes the TOC entries & indexes but not data files

    deselectIndex();

    closeDataHandles();

    closeIndexes();
}


void TocDBWriter::index(const Key &key, const eckit::PathName &path, eckit::Offset offset, eckit::Length length) {
    dirty_ = true;

    if (current_.null()) {
        ASSERT(!currentIndexKey_.empty());
        selectIndex(currentIndexKey_);
    }

    Field field(TocFieldLocation(path, offset, length));

    current_.put(key, field);
}

void TocDBWriter::archive(const Key &key, const void *data, eckit::Length length) {
    dirty_ = true;

    if (current_.null()) {
        ASSERT(!currentIndexKey_.empty());
        selectIndex(currentIndexKey_);
    }

    eckit::PathName dataPath = getDataPath(current_.key());

    eckit::DataHandle &dh = getDataHandle(dataPath);

    eckit::Offset position = dh.position();

    dh.write( data, length );

    Field field (TocFieldLocation(dataPath, position, length));

    current_.put(key, field);
}

void TocDBWriter::flush() {
    if (!dirty_) {
        return;
    }

    // ensure consistent state before writing Toc entry

    flushDataHandles();
    flushIndexes();

    dirty_ = false;
    current_ = Index();
}


eckit::DataHandle *TocDBWriter::getCachedHandle( const eckit::PathName &path ) const {
    HandleStore::const_iterator j = handles_.find( path );
    if ( j != handles_.end() )
        return j->second;
    else
        return 0;
}

void TocDBWriter::closeDataHandles() {
    for ( HandleStore::iterator j = handles_.begin(); j != handles_.end(); ++j ) {
        eckit::DataHandle *dh = j->second;
        dh->close();
        delete dh;
    }
    handles_.clear();
}


eckit::DataHandle *TocDBWriter::createFileHandle(const eckit::PathName &path) {

    static size_t sizeBuffer = eckit::Resource<unsigned long>("fdbBufferSize", 64 * 1024 * 1024);

    if(stripeLustre()) {

        eckit::Log::debug<LibFdb>() << "Creating LustreFileHandle<FDBFileHandle> to " << path
                                    << " buffer size " << sizeBuffer
                                    << std::endl;

        return new LustreFileHandle<FDBFileHandle>(path, sizeBuffer, stripeDataLustreSettings());
    }

    eckit::Log::debug<LibFdb>() << "Creating FDBFileHandle to " << path
                                << " with buffer of " << eckit::Bytes(sizeBuffer)
                                << std::endl;

    return new FDBFileHandle(path, sizeBuffer);
}

eckit::DataHandle *TocDBWriter::createAsyncHandle(const eckit::PathName &path) {

    static size_t nbBuffers  = eckit::Resource<unsigned long>("fdbNbAsyncBuffers", 4);
    static size_t sizeBuffer = eckit::Resource<unsigned long>("fdbSizeAsyncBuffer", 64 * 1024 * 1024);

    if(stripeLustre()) {

        eckit::Log::debug<LibFdb>() << "Creating LustreFileHandle<AIOHandle> to " << path
                                    << " with " << nbBuffers
                                    << " buffer each with " << eckit::Bytes(sizeBuffer)
                                    << std::endl;

        return new LustreFileHandle<eckit::AIOHandle>(path, nbBuffers, sizeBuffer, stripeDataLustreSettings());
    }

    return new eckit::AIOHandle(path, nbBuffers, sizeBuffer);
}


eckit::DataHandle &TocDBWriter::getDataHandle( const eckit::PathName &path ) {
    eckit::DataHandle *dh = getCachedHandle( path );
    if ( !dh ) {
        static bool fdbAsyncWrite = eckit::Resource<bool>("fdbAsyncWrite;$FDB_ASYNC_WRITE", false);

        dh = fdbAsyncWrite ? createAsyncHandle( path ) : createFileHandle( path );
        handles_[path] = dh;
        ASSERT( dh );
        dh->openForAppend(0);
    }
    return *dh;
}

eckit::PathName TocDBWriter::generateIndexPath(const Key &key) const {
    eckit::PathName tocPath ( directory_ );
    tocPath /= key.valuesToString();
    tocPath = eckit::PathName::unique(tocPath) + ".index";
    return tocPath;
}

eckit::PathName TocDBWriter::generateDataPath(const Key &key) const {
    eckit::PathName dpath ( directory_ );
    dpath /=  key.valuesToString();
    dpath = eckit::PathName::unique(dpath) + ".data";
    return dpath;
}

eckit::PathName TocDBWriter::getDataPath(const Key &key) {
    PathStore::const_iterator j = dataPaths_.find(key);
    if ( j != dataPaths_.end() )
        return j->second;

    eckit::PathName dataPath = generateDataPath(key);

    dataPaths_[ key ] = dataPath;

    return dataPath;
}

void TocDBWriter::flushIndexes() {
    for (IndexStore::iterator j = indexes_.begin(); j != indexes_.end(); ++j ) {
        Index& idx = j->second;
        idx.flush();
        writeIndexRecord(idx);
        idx.reopen(); // Create a new btree
    }
}


void TocDBWriter::closeIndexes() {
    for (IndexStore::iterator j = indexes_.begin(); j != indexes_.end(); ++j ) {
        Index& idx = j->second;
        idx.close();
    }

    indexes_.clear(); // all indexes instances destroyed
}

void TocDBWriter::flushDataHandles() {

    for (HandleStore::iterator j = handles_.begin(); j != handles_.end(); ++j) {
        eckit::DataHandle *dh = j->second;
        dh->flush();
    }
}


void TocDBWriter::print(std::ostream &out) const {
    out << "TocDBWriter(" << directory() << ")";
}

static DBBuilder<TocDBWriter> builder("toc.writer", false, true);

//----------------------------------------------------------------------------------------------------------------------

} // namespace fdb5
