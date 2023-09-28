// Copyright (c) NetXS Group.
// Licensed under the MIT license.

#pragma once

#include "richtext.hpp"
#include "events.hpp"
#include "xml.hpp"

#include <typeindex>

namespace netxs::input
{
    struct hids;
    struct foci;
    using sysmouse = directvt::binary::sysmouse_t;
    using syskeybd = directvt::binary::syskeybd_t;
    using sysfocus = directvt::binary::sysfocus_t;
    using syswinsz = directvt::binary::syswinsz_t;
    using sysclose = directvt::binary::sysclose_t;
    using syspaste = directvt::binary::syspaste_t;
    using sysboard = directvt::binary::sysboard_t;
    using clipdata = directvt::binary::clipdata_t;
}
namespace netxs::ui
{
    enum axis { X, Y };

    enum class sort
    {
        forward,
        reverse,
    };

    enum class snap
    {
        none,
        head,
        tail,
        both,
        center,
    };

    enum class slot { _1, _2, _I };

    enum class axes
    {
        none   = 0,
        X_only = 1 << 0,
        Y_only = 1 << 1,
        all    = X_only | Y_only,
    };
    constexpr auto operator & (axes l, axes r) { return static_cast<si32>(l) & static_cast<si32>(r); }

    struct bind
    {
        snap x = snap::none;
        snap y = snap::none;
    };

    struct base;

    using namespace netxs::input;
    using sptr = netxs::sptr<base>;
    using wptr = netxs::wptr<base>;
    using focus_test_t = std::pair<id_t, si32>;
    using gear_id_list_t = std::list<id_t>;
    using functor = std::function<void(sptr)>;
    using proc = std::function<void(hids&)>;
    using s11n = directvt::binary::s11n;
    using escx = ansi::escx;
    using book = std::list<sptr>;
}

namespace netxs::events::userland
{
    struct e2
    {
        static constexpr auto dtor = netxs::events::userland::root::dtor;
        static constexpr auto cascade = netxs::events::userland::root::cascade;
        static constexpr auto cleanup = netxs::events::userland::root::cleanup;

        EVENTPACK( e2, netxs::events::userland::root::base )
        {
            EVENT_XS( postrender, ui::face       ), // release: UI-tree post-rendering. Draw debug overlay, maker, titles, etc.
            EVENT_XS( nextframe , bool           ), // general: Signal for rendering the world, the parameter indicates whether the world has been modified since the last rendering.
            EVENT_XS( depth     , si32           ), // request: Determine the depth of the hierarchy.
            EVENT_XS( shutdown  , const text     ), // general: Server shutdown.
            GROUP_XS( extra     , si32           ), // Event extension slot.
            GROUP_XS( timer     , time           ), // timer tick, arg: current moment (now).
            GROUP_XS( render    , ui::face       ), // release: UI-tree rendering.
            GROUP_XS( conio     , si32           ),
            GROUP_XS( area      , rect           ), // release: Object rectangle.
            GROUP_XS( form      , bool           ),
            GROUP_XS( data      , si32           ),
            GROUP_XS( config    , si32           ), // set/notify/get/global_set configuration data.
            GROUP_XS( command   , si32           ), // exec UI command.

            SUBSET_XS( extra )
            {
                EVENT_XS( slot1, si32 ),
                EVENT_XS( slot2, si32 ),
                EVENT_XS( slot3, si32 ),
                EVENT_XS( slot4, si32 ),
                EVENT_XS( slot5, si32 ),
                EVENT_XS( slot6, si32 ),
                EVENT_XS( slot7, si32 ),
                EVENT_XS( slot8, si32 ),
                EVENT_XS( slot9, si32 ),
                EVENT_XS( slotA, si32 ),
                EVENT_XS( slotB, si32 ),
                EVENT_XS( slotC, si32 ),
                EVENT_XS( slotD, si32 ),
                EVENT_XS( slotE, si32 ),
                EVENT_XS( slotF, si32 ),
            };
            SUBSET_XS( timer )
            {
                EVENT_XS( tick, time ), // relaese: execute before e2::timer::any (rendering)
            };
            SUBSET_XS( render ) // release any: UI-tree default rendering submission.
            {
                EVENT_XS( prerender, ui::face ), // release: UI-tree pre-rendering, used by pro::cache (can interrupt SIGNAL) and any kind of highlighters.
            };
            SUBSET_XS( area )
            {
                EVENT_XS( set, rect ), // preview: request new area for object. release: apply new area to object.
            };
            SUBSET_XS( config )
            {
                EVENT_XS( creator, ui::sptr ), // request: pointer to world object.
                EVENT_XS( fps    , si32     ), // request to set new fps, arg: new fps (si32); the value == -1 is used to request current fps.
                GROUP_XS( caret  , span     ), // any kind of intervals property.
                GROUP_XS( plugins, si32     ),

                SUBSET_XS( caret )
                {
                    EVENT_XS( blink, span ), // caret blinking interval.
                    EVENT_XS( style, si32 ), // caret style: 0 - underline, 1 - box.
                };
                SUBSET_XS( plugins )
                {
                    EVENT_XS( align, bool ), // release: enable/disable align plugin.
                    GROUP_XS( sizer, dent ), // configure sizer.

                    SUBSET_XS( sizer )
                    {
                        EVENT_XS( inner, dent ), // release: set inner size; request: request unner size.
                        EVENT_XS( outer, dent ), // release: set outer size; request: request outer size.
                        EVENT_XS( inert, bool ), // release: set read only mode (no active actions, follow only).
                        EVENT_XS( alive, bool ), // release: shutdown the sizer.
                    };
                };
            };
            SUBSET_XS( conio )
            {
                EVENT_XS( mouse   , input::sysmouse ), // release: mouse activity.
                EVENT_XS( keybd   , input::syskeybd ), // release: keybd activity.
                EVENT_XS( focus   , input::sysfocus ), // release: focus activity.
                EVENT_XS( board   , input::sysboard ), // release: Clipboard preview.
                //EVENT_XS( paste   , input::syspaste ), // release: clipboard activity.
                EVENT_XS( error   , const si32      ), // release: return error code.
                EVENT_XS( winsz   , const twod      ), // release: order to update terminal primary overlay.
                EVENT_XS( preclose, const bool      ), // release: signal to quit after idle timeout, arg: bool - ready to shutdown.
                EVENT_XS( quit    , const si32      ), // release: quit, arg: si32 - quit reason.
                EVENT_XS( pointer , const bool      ), // release: mouse pointer visibility.
                EVENT_XS( logs    , const text      ), // logs output.
                EVENT_XS( readline, text            ), // Standard input (scripting).
                //EVENT_XS( menu  , si32 ),
            };
            SUBSET_XS( data )
            {
                //todo revise (see app::desk)
                EVENT_XS( changed, text       ), // release/preview/request: current menu item id(text).
                EVENT_XS( request, si32       ),
                EVENT_XS( disable, si32       ),
                EVENT_XS( flush  , si32       ),
                EVENT_XS( utf8   , const text ), // signaling with a text string, release only.
            };
            SUBSET_XS( command )
            {
                EVENT_XS( cout       , const text  ), // Append extra data to output.
                EVENT_XS( custom     , si32        ), // Custom command, arg: cmd_id.
                EVENT_XS( printscreen, input::hids ), // Copy screen area to clipboard.
            };
            SUBSET_XS( form )
            {
                EVENT_XS( canvas   , sptr<core> ), // request global canvas.
                GROUP_XS( layout   , const twod ),
                GROUP_XS( draggable, bool       ), // signal to the form to enable draggablity for specified mouse button.
                GROUP_XS( upon     , bool       ),
                GROUP_XS( proceed  , bool       ),
                GROUP_XS( cursor   , bool       ),
                GROUP_XS( drag     , input::hids),
                GROUP_XS( prop     , text       ),
                GROUP_XS( global   , twod       ),
                GROUP_XS( state    , const twod ),
                GROUP_XS( animate  , id_t       ),

                SUBSET_XS( draggable )
                {
                    EVENT_XS( left     , bool ),
                    EVENT_XS( right    , bool ),
                    EVENT_XS( middle   , bool ),
                    EVENT_XS( wheel    , bool ),
                    EVENT_XS( win      , bool ),
                    EVENT_XS( leftright, bool ),

                    INDEX_XS( left, right, middle, wheel, win, leftright ),
                };
                SUBSET_XS( layout )
                {
                    EVENT_XS( fullscreen, input::hids ), // request to fullscreen.
                    EVENT_XS( minimize  , input::hids ), // request to minimize.
                    EVENT_XS( unselect  , input::hids ), // inform if unselected.
                    EVENT_XS( selected  , input::hids ), // inform if selected.
                    EVENT_XS( restore   , ui::sptr    ), // request to restore.
                    EVENT_XS( shift     , const twod  ), // request a global shifting with delta.
                    EVENT_XS( jumpto    , ui::base    ), // fly to the specified object.
                    EVENT_XS( convey    , cube        ), // request a global conveying with delta (Inform all children to be conveyed).
                    EVENT_XS( bubble    , rect        ), // order to popup the requested item through the visual tree.
                    EVENT_XS( expose    , rect        ), // order to bring the requested item on top of the visual tree.
                    EVENT_XS( appear    , twod        ), // fly to the specified coords.
                    EVENT_XS( swarp     , const dent  ), // preview: form swarping
                    GROUP_XS( go        , ui::sptr    ), // preview: form swarping
                    //EVENT_XS( order     , si32       ), // return
                    //EVENT_XS( strike    , rect       ), // inform about the child canvas has changed, only preview.
                    //EVENT_XS( coor      , twod       ), // return object rect coor, only preview.
                    //EVENT_XS( size      , twod       ), // return object rect size, only preview.
                    //EVENT_XS( rect      , rect       ), // return object rect.
                    //EVENT_XS( show      , bool       ), // order to make it visible.
                    //EVENT_XS( hide      , bool       ), // order to make it hidden.
                    
                    SUBSET_XS( go )
                    {
                        EVENT_XS( next , ui::sptr ), // request: proceed request for available objects (next)
                        EVENT_XS( prev , ui::sptr ), // request: proceed request for available objects (prev)
                        EVENT_XS( item , ui::sptr ), // request: proceed request for available objects (current)
                    };
                };
                SUBSET_XS( upon )
                {
                    EVENT_XS( created, input::hids ), // release: notify the instance of who created it.
                    EVENT_XS( started, ui::sptr    ), // release: notify the instance is commissioned. arg: visual root.
                    EVENT_XS( resized, const rect  ), // anycast: notify about the actual window area.
                    EVENT_XS( changed, twod        ), // event after resize, arg: diff bw old and new size.
                    EVENT_XS( dragged, input::hids ), // event after drag.
                    EVENT_XS( stopped, bool        ), // release: notify that the main reading loop has exited. arg bool: fast or not.
                    GROUP_XS( vtree  , ui::sptr    ), // visual tree events, arg: parent base_sptr.
                    GROUP_XS( scroll , rack        ), // event after scroll.
                    //EVENT_XS( redrawn, ui::face       ), // inform about camvas is completely redrawn.
                    //EVENT_XS( cached , ui::face       ), // inform about camvas is cached.
                    //EVENT_XS( wiped  , ui::face       ), // event after wipe the canvas.
                    //EVENT_XS( created    , ui::sptr   ), // event after itself creation, arg: itself bell_sptr.
                    //EVENT_XS( detached   , bell_sptr      ), // inform that subject is detached, arg: parent bell_sptr.
                    //EVENT_XS( invalidated, bool           ),
                    //EVENT_XS( moved      , twod           ), // release: event after moveto, arg: diff bw old and new coor twod. preview: event after moved by somebody.

                    SUBSET_XS( vtree )
                    {
                        EVENT_XS( attached, ui::sptr ), // Child has been attached, arg: parent ui::sptr.
                        EVENT_XS( detached, ui::sptr ), // Child has been detached, arg: parent ui::sptr.
                    };
                    SUBSET_XS( scroll )
                    {
                        GROUP_XS( to_top, rack ), // scroll to top.
                        GROUP_XS( to_end, rack ), // scroll to end.
                        GROUP_XS( bycoor, rack ), // scroll absolute.
                        GROUP_XS( bystep, rack ), // scroll by delta.
                        GROUP_XS( bypage, rack ), // scroll by page.
                        GROUP_XS( cancel, rack ), // reset scrolling.

                        SUBSET_XS( to_top )
                        {
                            EVENT_XS( x, rack ), // scroll to_top along X.
                            EVENT_XS( y, rack ), // scroll to_top along Y.
                            EVENT_XS( v, rack ), // scroll to_top along XY.

                            INDEX_XS( x, y, v ),
                        };
                        SUBSET_XS( to_end )
                        {
                            EVENT_XS( x, rack ), // scroll to_end along X.
                            EVENT_XS( y, rack ), // scroll to_end along Y.
                            EVENT_XS( v, rack ), // scroll to_end along XY.

                            INDEX_XS( x, y, v ),
                        };
                        SUBSET_XS( bycoor )
                        {
                            EVENT_XS( x, rack ), // scroll absolute along X.
                            EVENT_XS( y, rack ), // scroll absolute along Y.
                            EVENT_XS( v, rack ), // scroll absolute along XY.

                            INDEX_XS( x, y, v ),
                        };
                        SUBSET_XS( bystep )
                        {
                            EVENT_XS( x, rack ), // scroll by delta along X.
                            EVENT_XS( y, rack ), // scroll by delta along Y.
                            EVENT_XS( v, rack ), // scroll by delta along XY.

                            INDEX_XS( x, y, v ),
                        };
                        SUBSET_XS( bypage )
                        {
                            EVENT_XS( x, rack ), // scroll by page along X.
                            EVENT_XS( y, rack ), // scroll by page along Y.
                            EVENT_XS( v, rack ), // scroll by page along XY.

                            INDEX_XS( x, y, v ),
                        };
                        SUBSET_XS( cancel )
                        {
                            EVENT_XS( x, rack ), // cancel scrolling along X.
                            EVENT_XS( y, rack ), // cancel scrolling along Y.
                            EVENT_XS( v, rack ), // cancel scrolling along XY.

                            INDEX_XS( x, y, v ),
                        };
                    };
                };
                SUBSET_XS( proceed )
                {
                    EVENT_XS( create    , rect        ), // return coordinates of the new object placeholder.
                    EVENT_XS( createby  , input::hids ), // return gear with coordinates of the new object placeholder gear::slot.
                    EVENT_XS( render    , bool        ), // ask children to render itself to the parent canvas, arg is the world is damaged or not.
                    EVENT_XS( attach    , ui::sptr    ), // order to attach a child, arg is a parent base_sptr.
                    EVENT_XS( swap      , ui::sptr    ), // order to replace existing object. See tiling manager empty slot.
                    EVENT_XS( functor   , ui::functor ), // exec functor (see pro::focus).
                    EVENT_XS( onbehalf  , ui::proc    ), // exec functor on behalf (see gate).
                    GROUP_XS( quit      , bool        ), // request to quit/detach (arg: fast or not).
                    //EVENT_XS( focus      , ui::sptr           ), // order to set focus to the specified object, arg is a object sptr.
                    //EVENT_XS( commit     , si32               ), // order to output the targets, arg is a frame number.
                    //EVENT_XS( multirender, vector<sptr<face>> ), // ask children to render itself to the set of canvases, arg is an array of the face sptrs.
                    //EVENT_XS( draw       , face               ), // ????  order to render itself to the canvas.
                    //EVENT_XS( checkin    , face_sptr          ), // order to register an output object canvas.

                    SUBSET_XS( quit )
                    {
                        EVENT_XS( one, bool ), // Signal to close (fast or not).
                    };
                };
                SUBSET_XS( cursor )
                {
                    EVENT_XS(blink, bool),
                };
                SUBSET_XS( animate )
                {
                    EVENT_XS( start, id_t ),
                    EVENT_XS( stop , id_t ),
                    EVENT_XS( reset, id_t ),
                };
                SUBSET_XS( drag )
                {
                    GROUP_XS( start , input::hids ), // notify about mouse drag start by pro::mouse.
                    GROUP_XS( pull  , input::hids ), // notify about mouse drag pull by pro::mouse.
                    GROUP_XS( cancel, input::hids ), // notify about mouse drag cancel by pro::mouse.
                    GROUP_XS( stop  , input::hids ), // notify about mouse drag stop by pro::mouse.

                    SUBSET_XS( start )
                    {
                        EVENT_XS( left     , input::hids ),
                        EVENT_XS( right    , input::hids ),
                        EVENT_XS( middle   , input::hids ),
                        EVENT_XS( wheel    , input::hids ),
                        EVENT_XS( win      , input::hids ),
                        EVENT_XS( leftright, input::hids ),

                        INDEX_XS( left, right, middle, wheel, win, leftright ),
                    };
                    SUBSET_XS( pull )
                    {
                        EVENT_XS( left     , input::hids ),
                        EVENT_XS( right    , input::hids ),
                        EVENT_XS( middle   , input::hids ),
                        EVENT_XS( wheel    , input::hids ),
                        EVENT_XS( win      , input::hids ),
                        EVENT_XS( leftright, input::hids ),

                        INDEX_XS( left, right, middle, wheel, win, leftright ),
                    };
                    SUBSET_XS( cancel )
                    {
                        EVENT_XS( left     , input::hids ),
                        EVENT_XS( right    , input::hids ),
                        EVENT_XS( middle   , input::hids ),
                        EVENT_XS( wheel    , input::hids ),
                        EVENT_XS( win      , input::hids ),
                        EVENT_XS( leftright, input::hids ),

                        INDEX_XS( left, right, middle, wheel, win, leftright ),
                    };
                    SUBSET_XS( stop )
                    {
                        EVENT_XS( left     , input::hids ),
                        EVENT_XS( right    , input::hids ),
                        EVENT_XS( middle   , input::hids ),
                        EVENT_XS( wheel    , input::hids ),
                        EVENT_XS( win      , input::hids ),
                        EVENT_XS( leftright, input::hids ),

                        INDEX_XS( left, right, middle, wheel, win, leftright ),
                    };
                };
                SUBSET_XS( prop )
                {
                    EVENT_XS( name      , text       ), // user name.
                    EVENT_XS( zorder    , zpos       ), // set form z-order, si32: -1 backmost, 0 plain, 1 topmost.
                    EVENT_XS( filler    , const cell ), // set form brush/color.
                    EVENT_XS( fullscreen, ui::sptr   ), // set fullscreen app.
                    EVENT_XS( viewport  , rect       ), // request: return form actual viewport.
                    EVENT_XS( lucidity  , si32       ), // set or request window transparency, si32: 0-255, -1 to request.
                    EVENT_XS( fixedsize , bool       ), // set ui::fork ratio.
                    GROUP_XS( window    , twod       ), // set or request window properties.
                    GROUP_XS( ui        , text       ), // set or request textual properties.
                    GROUP_XS( colors    , rgba       ), // set or request bg/fg colors.

                    SUBSET_XS( window )
                    {
                        EVENT_XS( size  , twod ), // set window size.
                    };
                    SUBSET_XS( ui )
                    {
                        EVENT_XS( title   , text ), // form title + foci status.
                        EVENT_XS( header  , text ), // set/get form caption header.
                        EVENT_XS( footer  , text ), // set/get form caption footer.
                        EVENT_XS( tooltip , text ), // set/get tooltip text.
                        EVENT_XS( slimmenu, bool ), // set/get window menu size.
                        EVENT_XS( acryl   , bool ), // set/get window acrylic effect.
                        EVENT_XS( cache   , bool ), // set/get render cache usage.
                    };
                    SUBSET_XS( colors )
                    {
                        EVENT_XS( bg, rgba ), // set/get rgba color.
                        EVENT_XS( fg, rgba ), // set/get rgba color.
                    };
                };
                SUBSET_XS( global )
                {
                    //EVENT_XS( ctxmenu , twod ), // request context menu at specified coords.
                    //deprecated - use tier::anycast
                    EVENT_XS( lucidity, si32 ), // set or request global window transparency, si32: 0-255, -1 to request.
                    //GROUP_XS( object,      ), // global scene objects events
                    //GROUP_XS( user  ,      ), // global scene users events

                    //SUBSET_XS( object )
                    //{
                    //    EVENT_XS( attached, sptr ), // global: object attached to the world.
                    //    EVENT_XS( detached, sptr ), // global: object detached from the world.
                    //};
                    //SUBSET_XS( user )
                    //{
                    //    EVENT_XS( attached, sptr ), // global: user attached to the world.
                    //    EVENT_XS( detached, sptr ), // global: user detached from the world.
                    //};
                };
                SUBSET_XS( state )
                {
                    EVENT_XS( mouse    , si32     ), // notify the object if mouse is active or not. The form is active when the number of clients (form::eventa::mouse::enter - mouse::leave) is not zero, only release, si32 - number of clients.
                    //EVENT_XS( params   , ui::para ), // notify the object has changed title params.
                    EVENT_XS( color    , ui::tone ), // notify the object has changed tone, preview to set.
                    EVENT_XS( highlight, bool     ),
                    EVENT_XS( visible  , bool     ),
                    GROUP_XS( keybd    , bool     ),

                    SUBSET_XS( keybd )
                    {
                        EVENT_XS( enlist  , ui::gear_id_list_t ), // anycast: Enumerate all available foci.
                        EVENT_XS( find    , ui::focus_test_t   ), // request: Check the focus.
                        EVENT_XS( next    , ui::focus_test_t   ), // request: Next hop count.
                        EVENT_XS( check   , bool               ), // anycast: Check any focus.
                        GROUP_XS( focus   , const id_t         ), // release: Has any keybd focus.

                        SUBSET_XS( focus )
                        {
                            EVENT_XS( on    , const id_t ),
                            EVENT_XS( off   , const id_t ),
                            EVENT_XS( state , bool       ),
                        };
                    };
                };
            };
        };
    };
}

namespace netxs::ui
{
    using e2 = netxs::events::userland::e2;

    //todo reimplement
    struct skin
    {
        poly kb_focus;
        poly brighter;
        poly shadower;
        poly shadow;
        poly selector;

        cell highlight;
        cell warning;
        cell danger;
        cell action;
        cell label;
        cell inactive;
        cell selected;
        cell menu_white;
        cell menu_black;

        twod bordersz = dot_11;
        si32 lucidity = 0xFF;
        bool tracking = faux;
        bool menuwide = faux;
        bool macstyle = faux;

        si32 spd;
        si32 pls;
        si32 ccl;
        si32 spd_accel;
        si32 ccl_accel;
        si32 spd_max;
        si32 ccl_max;
        si32 switching;
        span deceleration;
        span blink_period;
        span menu_timeout;
        span active_timeout;
        span repeat_delay;
        span repeat_rate;
        span fader_time;
        span fader_fast;

        twod min_value = dot_00;
        twod max_value = twod{ 2000, 1000 }; //todo unify

        static auto& globals()
        {
            static skin _globals;
            return _globals;
        }
        // skin:: Return global brighter/shadower color (cell).
        static cell const& color(si32 property)
        {
            auto& g = globals();
            switch (property)
            {
                case tone::prop::kb_focus:   return g.kb_focus;
                case tone::prop::brighter:   return g.brighter;
                case tone::prop::shadower:   return g.shadower;
                case tone::prop::shadow:     return g.shadow;
                case tone::prop::selector:   return g.selector;
                case tone::prop::highlight:  return g.highlight;
                case tone::prop::selected:   return g.selected;
                case tone::prop::warning:    return g.warning;
                case tone::prop::danger:     return g.danger;
                case tone::prop::action:     return g.action;
                case tone::prop::label:      return g.label;
                case tone::prop::inactive:   return g.inactive;
                case tone::prop::menu_white: return g.menu_white;
                case tone::prop::menu_black: return g.menu_black;
                default:                     return g.brighter;
            }
        }
        // skin:: Return global gradient for brighter/shadower.
        static poly const& grade(si32 property)
        {
            auto& g = globals();
            switch (property)
            {
                case tone::prop::kb_focus: return g.kb_focus;
                case tone::prop::brighter: return g.brighter;
                case tone::prop::shadower: return g.shadower;
                case tone::prop::shadow:   return g.shadow;
                case tone::prop::selector: return g.selector;
                default:                   return g.brighter;
            }
        }
    };

    // console: Base visual.
    struct base
        : public bell, public std::enable_shared_from_this<base>
    {
        enum type
        {
            reflow_root = -1,
            client = 0,
            node = 1,
            placeholder = 2,
        };

        book subset; // base: List of nested objects.
        wptr father; // base: Reference to parent.
        subs relyon; // base: Subscription on parent events.
        rect region; // base: The region occupied by the object.
        rect socket; // base: The region provided for the object.

        //todo deprecated
        cell filler; // base: Object color.

        //todo ?bound rect? ?limits
        twod minlim; // base: Minimal size.
        twod maxlim; // base: Maximal size.
        twod minpos; // base: Minimal coor.
        twod maxpos; // base: Maximal coor.

        dent extpad; // base: Pads around object.
        dent intpad; // base: Pads inside object.
        bind atgrow; // base: Bindings on enlarging.
        bind atcrop; // base: Bindings on shrinking.
        bool wasted; // base: Should the object be redrawn.
        bool hidden; // base: Ignore rendering and resizing.
        bool master; // base: Anycast root.
        si32 family; // base: Object type.
        //todo it is a dent. not a side
        side oversz; // base: Oversize, for scrolling.
        twod anchor; // base: Object balance point. Center point for any transform (on preview).

        template<class T = base>
        auto   This()       { return std::static_pointer_cast<std::remove_reference_t<T>>(shared_from_this()); }
        auto&  coor() const { return region.coor;          }
        auto&  size() const { return region.size;          }
        auto&  area() const { return region;               }
        void   root(bool b) { master = b;                  }
        bool   root()       { return master;               }
        si32   kind()       { return family;               }
        void   kind(si32 k) { family = k;                  }
        auto center() const { return region.center();      }
        auto parent()       { return father.lock();        }
        void ruined(bool s) { wasted = s;                  }
        auto ruined() const { return wasted;               }
        template<bool Absolute = true>
        auto actual_area() const
        {
            auto area = rect{ -oversz.topleft(), region.size + oversz.summ() };
            if constexpr (Absolute) area.coor += region.coor;
            return area;
        }
        auto color() const { return filler; }
        void color(rgba const& fg_color, rgba const& bg_color)
        {
            // To make an object transparent to mouse events,
            // no id (cell::id = 0) is used by default in the filler.
            // The bell::id is configurable only with pro::mouse.
            base::filler.bgc(bg_color)
                        .fgc(fg_color)
                        .txt(whitespace);
            SIGNAL(tier::release, e2::form::prop::filler, filler);
        }
        void color(cell const& new_filler)
        {
            base::filler = new_filler;
            SIGNAL(tier::release, e2::form::prop::filler, filler);
        }

        // base: Align object.
        void xform(snap align, si32& coor, si32& size, si32& width)
        {
            switch (align)
            {
                case snap::head:
                    coor = 0;
                    break;
                case snap::tail:
                    coor = width - size;
                    break;
                case snap::both:
                    coor = (width - size) / 2;
                //    coor = 0;
                //    size = width;
                    break;
                case snap::center:
                    coor = (width - size) / 2;
                    break;
                default:
                    break;
            }
        }
        // base: Recalc actual region for the object.
        void recalc(rect& new_area)
        {
            if (base::hidden) return;
            auto required = new_area;
            new_area -= base::extpad;
            new_area.size = std::clamp(new_area.size, base::minlim, base::maxlim);
            auto c = new_area.coor;
            new_area.coor = dot_00;
            new_area -= base::intpad;
            deform(new_area);
            new_area.size += base::intpad;
            new_area.coor = c;
            new_area += base::extpad;
            if (required.size.x < new_area.size.x && base::atcrop.x == snap::both
             || required.size.x > new_area.size.x && base::atgrow.x == snap::both)
            {
                required.size.x = new_area.size.x;
            }
            if (required.size.y < new_area.size.y && base::atcrop.y == snap::both
             || required.size.y > new_area.size.y && base::atgrow.y == snap::both)
            {
                required.size.y = new_area.size.y;
            }
            base::socket = new_area;
        }
        // base: Notify about actual region for the object.
        void notify(rect new_area)
        {
            if (base::hidden) return;
            xform(socket.size.x > new_area.size.x ? atcrop.x : atgrow.x, socket.coor.x, socket.size.x, new_area.size.x);
            xform(socket.size.y > new_area.size.y ? atcrop.y : atgrow.y, socket.coor.y, socket.size.y, new_area.size.y);
            std::swap(new_area, base::socket);
            new_area -= base::extpad;
            auto c = new_area.coor;
            new_area.coor = dot_00;
            new_area -= base::intpad;
            inform(new_area);
            new_area.size += base::intpad;
            new_area.coor = c;
            SIGNAL(tier::release, e2::area::set, new_area);
            base::region = new_area;
        }
        // base: Change object area, and return delta.
        void change(rect new_area)
        {
            recalc(new_area);
            notify(new_area);
        }
        // base: Set new size, and return delta.
        auto resize(twod new_size)
        {
            auto new_area = base::region;
            auto old_size = base::region.size;
            new_area.size = new_size;
            new_area += base::extpad;
            change(new_area);
            return base::region.size - old_size;
        }
        // base: Move and return delta.
        auto moveto(twod new_coor)
        {
            auto new_area = base::region;
            auto old_coor = base::region.coor;
            new_area.coor = new_coor;
            new_area += base::extpad;
            notify(new_area);
            return base::region.coor - old_coor;
        }
        // base: Dry run. Recheck current position.
        auto moveto()
        {
            return moveto(base::region.coor);
        }
        // base: Move by the specified step and return the coor delta.
        auto moveby(twod step)
        {
            return moveto(base::region.coor + step);
        }
        // base: Resize relative the center point.
        //       Return center point offset.
        //       The object is responsible for correcting
        //       the center point during resizing.
        auto resize(twod new_size, twod point)
        {
            point -= base::region.coor;
            point += base::intpad.corner();
            base::anchor = point; //todo use dot_00 instead of point
            auto old_coor = base::region.coor;
            auto new_area = base::region;
            new_area.size = new_size;
            new_area += base::extpad;
            recalc(new_area);
            new_area.coor += point - base::anchor;
            notify(new_area);
            return base::region.coor - old_coor;
        }
        // base: Dry run (preview then release) current value.
        auto resize()
        {
            return resize(base::region.size);
        }
        // base: Resize by step, and return delta.
        auto sizeby(twod const& step)
        {
            return resize(base::region.size + step);
        }
        // base: Resize and move, and return delta.
        auto extend(rect new_area)
        {
            new_area += base::extpad;
            auto old_area = base::region;
            change(new_area);
            auto delta = base::region;
            delta -= old_area;
            return delta;
        }
        // base: Mark the visual subtree as requiring redrawing.
        void strike(rect area)
        {
            area.coor += base::region.coor;
            if (auto parent_ptr = parent())
            {
                parent_ptr->deface(area);
            }
        }
        // base: Mark the visual subtree as requiring redrawing.
        void strike()
        {
            strike(base::region);
        }
        // base: Mark the form and its subtree as requiring redrawing.
        virtual void deface(rect const& area)
        {
            base::wasted = true;
            strike(area);
        }
        // base: Mark the form and its subtree as requiring redrawing.
        void deface()
        {
            deface(base::region);
        }
        // base: Going to rebuild visual tree. Retest current size, ask parent if it is linked.
        template<bool Forced = faux>
        void reflow()
        {
            auto parent_ptr = parent();
            if (parent_ptr && (!base::master || (Forced && (base::family != base::reflow_root)))) //todo unify -- See basewindow in vtm.cpp
            {
                parent_ptr->reflow<Forced>();
            }
            else change(base::region + base::extpad);
        }
        // base: Remove the form from the visual tree.
        void detach()
        {
            if (auto parent_ptr = parent())
            {
                strike();
                parent_ptr->remove(This());
            }
        }
        // base: Remove visual tree branch.
        void destroy()
        {
            auto lock = events::sync{};
            auto shadow = This();
            if (auto parent_ptr = parent())
            {
                parent_ptr->destroy();
            }
            detach();
        }
        // base: Recursively calculate global coordinate.
        void global(twod& coor)
        {
            coor -= base::region.coor;
            if (auto parent_ptr = parent())
            {
                parent_ptr->global(coor);
            }
        }
        // base: Recursively find the root of the visual tree.
        netxs::sptr<bell> gettop() override
        {
            auto parent_ptr = parent();
            if (!base::master && parent_ptr) return parent_ptr->gettop();
            else                             return This();
        }
        // base: Invoke a lambda with parent as a parameter.
        // Usage example:
        //     toboss([&](auto& parent_ptr) { c.fuse(parent.filler); });
        template<class T>
        void toboss(T proc)
        {
            if (auto parent_ptr = parent())
            {
                proc(*parent_ptr);
            }
        }
        // base: Fire an event on yourself and pass it parent if not handled.
        // Usage example:
        //          base::riseup<tier::preview, e2::form::prop::ui::header>(txt);
        template<tier Tier, class Event, class T>
        void riseup(Event, T&& data, bool forced = faux)
        {
            if (forced)
            {
                SIGNAL(Tier, Event{}, data);
                base::toboss([&](auto& boss)
                {
                    boss.base::template riseup<Tier>(Event{}, std::forward<T>(data), forced);
                });
            }
            else
            {
                if (!SIGNAL(Tier, Event{}, data))
                {
                    base::toboss([&](auto& boss)
                    {
                        boss.base::template riseup<Tier>(Event{}, std::forward<T>(data), forced);
                    });
                }
            }
        }
        // base: Fire an event on yourself and pass it parent if not handled.
        // Warning: The parameter type is not checked/casted.
        // Usage example:
        //          base::raw_riseup<tier::preview, e2::form::prop::ui::header>(txt);
        template<tier Tier, class T>
        void raw_riseup(hint event_id, T&& param, bool forced = faux)
        {
            if (forced)
            {
                bell::template signal<Tier>(event_id, param);
                base::toboss([&](auto& boss)
                {
                    boss.base::template raw_riseup<Tier>(event_id, std::forward<T>(param), forced);
                });
            }
            else
            {
                if (!bell::template signal<Tier>(event_id, param))
                {
                    base::toboss([&](auto& boss)
                    {
                        boss.base::template raw_riseup<Tier>(event_id, std::forward<T>(param), forced);
                    });
                }
            }
        }
        // base: Syntax sugar helper.
        void _saveme()
        {
            bell::_saveme();
        }

        void limits(twod minlim = -dot_11, twod maxlim = -dot_11)
        {
            base::minlim = minlim.less(dot_00, skin::globals().min_value, minlim);
            base::maxlim = maxlim.less(dot_00, skin::globals().max_value, maxlim);
        }
        void alignment(bind atgrow, bind atcrop = {})
        {
            base::atgrow = atgrow;
            base::atcrop.x = atcrop.x == snap::none ? atgrow.x : atcrop.x;
            base::atcrop.y = atcrop.y == snap::none ? atgrow.y : atcrop.y;
        }
        void setpad(dent intpad, dent extpad = {})
        {
            base::intpad = intpad;
            base::extpad = extpad;
        }
        // base.: Render to the canvas. Trim = trim viewport to the nested object region.
        void render(face& canvas, twod const& offset_coor, bool trim = true)
        {
            if (hidden) return;
            auto canvas_view = canvas.core::view();
            auto parent_area = canvas.flow::full();

            auto object_area = base::region;
            object_area.coor+= parent_area.coor;

            auto nested_view = canvas_view.clip(object_area);
            //todo revise: why whole canvas is not used
            if (trim ? nested_view : canvas_view)
            {
                auto canvas_coor = canvas.core::coor();
                if (trim) canvas.core::view(nested_view);
                canvas.core::back(offset_coor);
                canvas.flow::full(object_area);

                SIGNAL(tier::release, e2::render::prerender, canvas);
                SIGNAL(tier::release, e2::postrender, canvas);

                if (trim) canvas.core::view(canvas_view);
                canvas.core::move(canvas_coor);
                canvas.flow::full(parent_area);
            }
        }
        // base: Render itself to the canvas.
        void render(face& canvas, bool pred = true, bool post = true)
        {
            if (hidden) return;
            auto canvas_view = canvas.core::view();
            auto parent_area = canvas.flow::full();

            auto object_area = base::region;
            object_area.coor-= canvas.core::coor();

            if (auto nested_view = canvas_view.clip(object_area))
            {
                canvas.core::view(nested_view);
                canvas.flow::full(object_area);

                if (pred) SIGNAL(tier::release, e2::render::prerender, canvas);
                if (post) SIGNAL(tier::release, e2::postrender,        canvas);

                canvas.core::view(canvas_view);
                canvas.flow::full(parent_area);
            }
        }

    protected:
        virtual void deform(rect& new_area) {}
        virtual void inform(rect new_area) {}
        // base: Remove nested object.
        virtual void remove(sptr item_ptr)
        {
            auto head = subset.begin();
            auto tail = subset.end();
            auto iter = std::find_if(head, tail, [&](auto& c){ return c == item_ptr; });
            if (iter != tail)
            {
                auto backup = This();
                subset.erase(iter);
                item_ptr->SIGNAL(tier::release, e2::form::upon::vtree::detached, backup);
            }
        }
        // base: Update nested object.
        virtual void update(sptr old_item_ptr, sptr new_item_ptr)
        {
            auto head = subset.begin();
            auto tail = subset.end();
            auto iter = std::find_if(head, tail, [&](auto& c){ return c == old_item_ptr; });
            if (iter != tail)
            {
                auto backup = This();
                auto pos = subset.erase(iter);
                old_item_ptr->SIGNAL(tier::release, e2::form::upon::vtree::detached, backup);
                subset.insert(pos, new_item_ptr);
                new_item_ptr->SIGNAL(tier::release, e2::form::upon::vtree::attached, backup);
            }
        }
        virtual ~base() = default;
        base(size_t nested_count = 0)
            : subset{ nested_count },
              minlim{ skin::globals().min_value },
              maxlim{ skin::globals().max_value },
              wasted{ true },
              hidden{ faux },
              master{ faux },
              family{ type::client }
        {
            LISTEN(tier::request, e2::depth, depth)
            {
                depth++;
            };
            LISTEN(tier::release, e2::cascade, proc)
            {
                auto backup = This();
                auto keepon = proc(backup);
                if (!keepon) this->bell::expire<tier::release>();
            };
            LISTEN(tier::release, e2::form::upon::vtree::attached, parent_ptr)
            {
                if (!master)
                {
                    parent_ptr->LISTEN(tier::release, e2::cascade, proc, relyon)
                    {
                        auto backup = This(); // Object can be deleted inside proc.
                        backup->SIGNAL(tier::release, e2::cascade, proc);
                    };
                }
                father = parent_ptr;
                // Propagate form events up to the visual branch ends (children).
                // Exec after all subscriptions.
                //todo implement via e2::cascade
            };
            LISTEN(tier::release, e2::form::upon::vtree::any, parent_ptr) // any: Run after all.
            {
                if (this->bell::protos<tier::release>(e2::form::upon::vtree::detached))
                {
                    relyon.reset();
                }
                if (parent_ptr) parent_ptr->base::reflow(); //todo too expensive. ? accumulate deferred reflow? or make it when stated?
            };
            LISTEN(tier::release, e2::render::any, parent_canvas)
            {
                if (base::filler.wdt() && !base::hidden)
                {
                    parent_canvas.fill([&](cell& c) { c.fusefull(base::filler); });
                }
            };
        }
    };
}