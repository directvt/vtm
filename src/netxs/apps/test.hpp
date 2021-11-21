// Copyright (c) NetXS Group.
// Licensed under the MIT license.

#ifndef NETXS_APP_TEST_HPP
#define NETXS_APP_TEST_HPP

namespace netxs::events::userland
{
    struct test
    {
        EVENTPACK( test, netxs::events::userland::root::custom )
        {
            GROUP_XS( ui, input::hids ),

            SUBSET_XS( ui )
            {
                EVENT_XS( create  , input::hids ),
                GROUP_XS( split   , input::hids ),

                SUBSET_XS( split )
                {
                    EVENT_XS( hz, input::hids ),
                };
            };
        };
    };
}

namespace netxs::app
{
    using namespace netxs::console;

    struct test
        : public ui::form<test>
    {
        // test: .
        using events = netxs::events::userland::test;
        // ...
    };
}

#endif // NETXS_APP_TEST_HPP