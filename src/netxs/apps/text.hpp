// Copyright (c) NetXS Group.
// Licensed under the MIT license.

#ifndef NETXS_APP_TEXT_HPP
#define NETXS_APP_TEXT_HPP

namespace netxs::events::userland
{
    struct text_
    {
        EVENTPACK( text_, netxs::events::userland::root::custom )
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

    struct text_
        : public ui::form<text_>
    {
        // text: .
        using events = netxs::events::userland::text_;
        // ...
    };
}

#endif // NETXS_APP_TEXT_HPP