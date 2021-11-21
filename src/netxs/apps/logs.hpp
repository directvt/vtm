// Copyright (c) NetXS Group.
// Licensed under the MIT license.

#ifndef NETXS_APP_LOGS_HPP
#define NETXS_APP_LOGS_HPP

namespace netxs::events::userland
{
    struct logs
    {
        EVENTPACK( logs, netxs::events::userland::root::custom )
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

    struct logs
        : public ui::form<logs>
    {
        // logs: .
        using events = netxs::events::userland::logs;
        // ...
    };
}

#endif // NETXS_APP_LOGS_HPP