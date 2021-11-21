// Copyright (c) NetXS Group.
// Licensed under the MIT license.

#ifndef NETXS_APP_CALC_HPP
#define NETXS_APP_CALC_HPP

namespace netxs::events::userland
{
    struct calc
    {
        EVENTPACK( calc, netxs::events::userland::root::custom )
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

    struct calc
        : public ui::form<calc>
    {
        // calc: .
        using events = netxs::events::userland::calc;
        // ...
    };
}

#endif // NETXS_APP_CALC_HPP