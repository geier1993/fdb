/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Simon Smart
/// @author Tiago Quintino
/// @date   Apr 2018

#include <unistd.h>
#include <stdlib.h>

#include "eckit/net/Port.h"
#include "eckit/net/TCPServer.h"
#include "eckit/net/TCPSocket.h"
#include "eckit/runtime/Application.h"
#include "eckit/runtime/Monitorable.h"
#include "eckit/runtime/Monitor.h"
#include "eckit/runtime/ProcessControler.h"
#include "eckit/thread/ThreadControler.h"

#include "fdb5/remote/Handler.h"
#include "fdb5/config/Config.h"

using namespace eckit;

namespace fdb5 {
namespace remote {

//----------------------------------------------------------------------------------------------------------------------

class FDBForker : public eckit::ProcessControler {

public: // methods

    FDBForker(eckit::TCPSocket& socket, const Config& config = fdb5::Config()) :
        ProcessControler(true),
        socket_(socket),
        config_(config) {}

private: // methods

    virtual void run() {

        Monitor::instance().reset(); // needed to the monitor to work on forked (but not execed process)

        // Ensure random state is reset after fork
        ::srand(::getpid() + ::time(0));
        ::srandom(::getpid() + ::time(0));

        Log::info() << "FDB forked pid " << ::getpid() << std::endl;

        RemoteHandler handler(socket_, config_);
        handler.handle();
    }

    eckit::TCPSocket socket_;
    eckit::LocalConfiguration config_;
};

//--------------------------------------------------------------------------------------------------

class FDBSvrApp : public eckit::Application {
public:
    FDBSvrApp(int argc, char** argv) :
        eckit::Application(argc, argv, "FDB_HOME") {
    }

    ~FDBSvrApp() {}

private:

    int port_;

    FDBSvrApp(const FDBSvrApp&) = delete;
    FDBSvrApp& operator=(const FDBSvrApp&) = delete;

    virtual void run() {
//        unique();

        LocalConfiguration config;
        config.set("statistics", true);

        TCPServer server(Port("fdb", 7654), "", true);
        server.closeExec(false);

        port_ = server.localPort();

        while (true) {
            try {
                FDBForker f(server.accept(), config);
                f.start();
            }
            catch (std::exception& e) {
                Log::error() << "** " << e.what() << " Caught in " << Here() << std::endl;
                Log::error() << "** Exception is ignored" << std::endl;
            }
        }
    }
};

//----------------------------------------------------------------------------------------------------------------------

} // namespace remote
} // namespace fdb5


int main(int argc, char** argv) {

#if 0
    fdb5::remote::FdbServer app(argc, argv);
#else
    fdb5::remote::FDBSvrApp app(argc, argv);
#endif
    app.start();
    return 0;
}
