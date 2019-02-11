#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include <plog/Log.h>
#include <plog/Appenders/ColorConsoleAppender.h>

using namespace Catch;

struct MyListener : Catch::TestEventListenerBase {

    using TestEventListenerBase::TestEventListenerBase; // inherit constructor

    virtual void testRunStarting( TestRunInfo const& testRunInfo ) override
    {
        static plog::ColorConsoleAppender<plog::TxtFormatter> consoleAppender;
        plog::init(plog::verbose, &consoleAppender);
    }
};
CATCH_REGISTER_LISTENER( MyListener );
