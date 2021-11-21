// Copyright (c) NetXS Group.
// Licensed under the MIT license.

#ifndef NETXS_APP_SHOP_HPP
#define NETXS_APP_SHOP_HPP

namespace netxs::events::userland
{
    struct shop
    {
        EVENTPACK( shop, netxs::events::userland::root::custom )
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

    struct shop
        : public ui::form<shop>
    {
        // shop: .
        using events = netxs::events::userland::shop;
        // ...
    };
}

#endif // NETXS_APP_SHOP_HPP