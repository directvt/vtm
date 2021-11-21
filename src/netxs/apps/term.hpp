// Copyright (c) NetXS Group.
// Licensed under the MIT license.

#ifndef NETXS_APP_TERM_HPP
#define NETXS_APP_TERM_HPP

namespace netxs::events::userland
{
    struct term_
    {
        EVENTPACK( term_, netxs::events::userland::root::custom )
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

    struct term_
        : public ui::form<term_>
    {
        // term_: Built-in terminal app.
        using events = netxs::events::userland::term_;
        // ...
    };
}

#endif // NETXS_APP_TERM_HPP