// Copyright (c) NetXS Group.
// Licensed under the MIT license.

#ifndef NETXS_CONSOLE_HPP
#define NETXS_CONSOLE_HPP

#include "../abstract/iterator.hpp"
#include "../os/system.hpp"

#include <iostream>
#include <typeindex>

#define SPD            10   // console: Auto-scroll initial speed component ΔR.
#define PLS            167  // console: Auto-scroll initial speed component ΔT.
#define CCL            120  // console: Auto-scroll duration in ms.
#define SPD_ACCEL      1    // console: Auto-scroll speed accelation.
#define CCL_ACCEL      30   // console: Auto-scroll additional duration in ms.
#define SPD_MAX        100  // console: Auto-scroll max speed.
#define CCL_MAX        1000 // console: Auto-scroll max duration in ms.

#define STOPPING_TIME  2s    // console: Object state stopping duration in s.
#define SWITCHING_TIME 200   // console: Object state switching duration in ms.
#define BLINK_PERIOD   400ms // console: Period in ms between the blink states of the cursor.

#define MENU_TIMEOUT   250ms // console: Taskbar collaplse timeout.

#define ACTIVE_TIMEOUT 1s    // console: Timeout off the active object.
#define REPEAT_DELAY   500ms // console: Repeat delay.
#define REPEAT_RATE    30ms  // console: Repeat rate.

namespace netxs::console
{
    namespace fs = std::filesystem;

    class face;
    class base;
    class form;
    class link;
    class site;

    struct create_t
    {
        using sptr = netxs::sptr<base>;
        text menuid{};
        text header{};
        text footer{};
        rect square{};
        bool forced{};
        sptr object{};
    };
    struct menuitem_t
    {
        text       id{};
        text    alias{};
        bool   hidden{};
        text    label{};
        text    notes{};
        text    title{};
        text   footer{};
        rgba  bgcolor{};
        rgba  fgcolor{};
        twod  winsize{};
        twod  wincoor{};
        bool slimmenu{};
        bool splitter{};
        text   hotkey{};
        text      cwd{};
        text     type{};
        text    param{};
    };

    using namespace netxs::input;
    using registry_t = netxs::imap<text, std::pair<bool, std::list<sptr<base>>>>;
    using links_t = std::unordered_map<text, menuitem_t>;
    using focus_test_t = std::pair<id_t, si32>;
    using gear_id_list_t = std::list<id_t>;
    using functor = std::function<void(sptr<base>)>;
    using proc = std::function<void(hids&)>;
    using s11n = netxs::ansi::dtvt::binary::s11n;
    using os::xipc;
}

namespace netxs::events::userland
{
    using namespace netxs::ui::atoms;
    using namespace netxs::datetime;

    struct e2
    {
        using type = netxs::events::type;
        static constexpr auto dtor = netxs::events::userland::root::dtor;
        static constexpr auto cascade = netxs::events::userland::root::cascade;
        static constexpr auto cleanup = netxs::events::userland::root::cleanup;

        EVENTPACK( e2, netxs::events::userland::root::base )
        {
            EVENT_XS( postrender, console::face       ), // release: UI-tree post-rendering. Draw debug overlay, maker, titles, etc.
            EVENT_XS( nextframe , bool                ), // general: Signal for rendering the world, the parameter indicates whether the world has been modified since the last rendering.
            EVENT_XS( depth     , si32                ), // request: Determine the depth of the hierarchy.
            EVENT_XS( shutdown  , const view          ), // general: Server shutdown.
            GROUP_XS( timer     , moment              ), // timer tick, arg: current moment (now).
            GROUP_XS( render    , console::face       ), // release: UI-tree rendering.
            GROUP_XS( conio     , si32                ),
            GROUP_XS( size      , twod                ), // release: Object size.
            GROUP_XS( coor      , twod                ), // release: Object coor.
            GROUP_XS( form      , bool                ),
            GROUP_XS( data      , si32                ),
            GROUP_XS( debug     , const view          ), // return info struct with telemtry data.
            GROUP_XS( config    , si32                ), // set/notify/get/global_set configuration data.
            GROUP_XS( command   , si32                ), // exec UI command.
            GROUP_XS( bindings  , sptr<console::base> ), // Dynamic Data Bindings.

            SUBSET_XS( timer )
            {
                EVENT_XS( tick, moment ), // relaese: execute before e2::timer::any (rendering)
            };
            SUBSET_XS( render ) // release any: UI-tree default rendering submission.
            {
                EVENT_XS( prerender, console::face ), // release: UI-tree pre-rendering, used by pro::cache (can interrupt SIGNAL) and any kind of highlighters.
            };
            SUBSET_XS( size ) // preview: checking by pro::limit.
            {
                EVENT_XS( set, twod ), // preview: checking by object; release: apply to object; request: request object size.
            };
            SUBSET_XS( coor ) // preview any: checking by pro::limit.
            {
                EVENT_XS( set, twod ), // preview: checking by object; release: apply to object; request: request object coor.
            };
            SUBSET_XS( bindings )
            {
                GROUP_XS( list, si32 ), // UI-tree pre-rendering, used by pro::cache (can interrupt SIGNAL) and any kind of highlighters, release only.

                SUBSET_XS( list )
                {
                    EVENT_XS( users, sptr<std::list<sptr<console::base>>> ), // list of connected users.
                    EVENT_XS( apps , sptr<console::registry_t>            ), // list of running apps.
                    EVENT_XS( links, sptr<console::links_t>               ), // list of registered apps.
                };
            };
            SUBSET_XS( debug )
            {
                EVENT_XS( logs  , const view          ), // logs output.
                EVENT_XS( output, const view          ), // logs has to be parsed.
                EVENT_XS( parsed, const console::page ), // output parced logs.
                EVENT_XS( request, si32               ), // request debug data.
                GROUP_XS( count , si32                ), // global: log listeners.

                SUBSET_XS( count )
                {
                    EVENT_XS( set, si32 ), // global: log listeners.
                };
            };
            SUBSET_XS( config )
            {
                EVENT_XS( whereami , sptr<console::base> ), // request: pointer to world object.
                EVENT_XS( fps      , si32                ), // request to set new fps, arg: new fps (si32); the value == -1 is used to request current fps.
                GROUP_XS( caret    , period              ), // any kind of intervals property.
                GROUP_XS( plugins  , si32                ),

                SUBSET_XS( caret )
                {
                    EVENT_XS( blink, period ), // caret blinking interval.
                    EVENT_XS( style, si32   ), // caret style: 0 - underline, 1 - box.
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
                EVENT_XS( unknown , const si32              ), // release: return platform unknown event code.
                EVENT_XS( error   , const si32              ), // release: return error code.
                EVENT_XS( focus   , const console::sysfocus ), // release: focus activity.
                EVENT_XS( mouse   , const console::sysmouse ), // release: mouse activity.
                EVENT_XS( keybd   , const console::syskeybd ), // release: keybd activity.
                EVENT_XS( winsz   , const twod              ), // release: order to update terminal primary overlay.
                EVENT_XS( native  , const bool              ), // release: extended functionality.
                EVENT_XS( preclose, const bool              ), // release: signal to quit after idle timeout, arg: bool - ready to shutdown.
                EVENT_XS( quit    , const text              ), // release: quit, arg: text - bye msg.
                EVENT_XS( pointer , const bool              ), // release: mouse pointer visibility.
                //EVENT_XS( menu  , si32 ),
            };
            SUBSET_XS( data )
            {
                //todo revise (see app::desk)
                EVENT_XS( changed, utf::text       ), // release/preview/request: current menu item id(text).
                EVENT_XS( request, si32            ),
                EVENT_XS( disable, si32            ),
                EVENT_XS( flush  , si32            ),
                EVENT_XS( text   , const utf::text ), // signaling with a text string, release only.
            };
            SUBSET_XS( command )
            {
                EVENT_XS( quit       , const view  ), // return bye msg, arg: errcode.
                EVENT_XS( cout       , const text  ), // Append extra data to output.
                EVENT_XS( custom     , si32        ), // Custom command, arg: cmd_id.
                EVENT_XS( printscreen, input::hids ), // Copy screen area to clipboard.
            };
            SUBSET_XS( form )
            {
                EVENT_XS( canvas   , sptr<console::core> ), // request global canvas.
                EVENT_XS( maximize , input::hids         ), // request to toggle maximize/restore.
                EVENT_XS( restore  , sptr<console::base> ), // request to toggle restore.
                EVENT_XS( quit     , sptr<console::base> ), // request parent for destroy.
                GROUP_XS( layout   , const twod          ),
                GROUP_XS( draggable, bool                ), // signal to the form to enable draggablity for specified mouse button.
                GROUP_XS( highlight, bool                ),
                GROUP_XS( upon     , bool                ),
                GROUP_XS( proceed  , bool                ),
                GROUP_XS( cursor   , bool                ),
                GROUP_XS( drag     , input::hids         ),
                GROUP_XS( prop     , text                ),
                GROUP_XS( global   , twod                ),
                GROUP_XS( state    , const twod          ),
                GROUP_XS( animate  , id_t                ),

                SUBSET_XS( draggable )
                {
                    EVENT_XS( left     , bool ),
                    EVENT_XS( right    , bool ),
                    EVENT_XS( leftright, bool ),
                    EVENT_XS( middle   , bool ),
                    EVENT_XS( wheel    , bool ),
                    EVENT_XS( win      , bool ),

                    INDEX_XS( left, right, leftright, middle, wheel, win ),
                };
                SUBSET_XS( layout )
                {
                    EVENT_XS( shift , const twod         ), // request a global shifting  with delta.
                    EVENT_XS( convey, cube               ), // request a global conveying with delta (Inform all children to be conveyed).
                    EVENT_XS( bubble, console::base      ), // order to popup the requested item through the visual tree.
                    EVENT_XS( expose, console::base      ), // order to bring the requested item on top of the visual tree (release: ask parent to expose specified child; preview: ask child to expose itself).
                    EVENT_XS( appear, twod               ), // fly to the specified coords.
                    EVENT_XS( gonext, sptr<console::base>), // request: proceed request for available objects (next)
                    EVENT_XS( goprev, sptr<console::base>), // request: proceed request for available objects (prev)
                    EVENT_XS( swarp , const dent         ), // preview: form swarping
                    //EVENT_XS( order     , si32       ), // return
                    //EVENT_XS( strike    , rect       ), // inform about the child canvas has changed, only preview.
                    //EVENT_XS( coor      , twod       ), // return client rect coor, only preview.
                    //EVENT_XS( size      , twod       ), // return client rect size, only preview.
                    //EVENT_XS( rect      , rect       ), // return client rect.
                    //EVENT_XS( show      , bool       ), // order to make it visible.
                    //EVENT_XS( hide      , bool       ), // order to make it hidden.
                };
                SUBSET_XS( highlight )
                {
                    EVENT_XS( on , bool ),
                    EVENT_XS( off, bool ),
                    EVENT_XS( set, bool ),
                };
                SUBSET_XS( upon )
                {
                    EVENT_XS( redrawn, console::face       ), // inform about camvas is completely redrawn.
                    EVENT_XS( cached , console::face       ), // inform about camvas is cached.
                    EVENT_XS( wiped  , console::face       ), // event after wipe the canvas.
                    EVENT_XS( changed, twod                ), // event after resize, arg: diff bw old and new size.
                    EVENT_XS( dragged, input::hids         ), // event after drag.
                    EVENT_XS( created, input::hids         ), // release: notify the instance of who created it.
                    EVENT_XS( started, sptr<console::base> ), // release: notify the instance is commissioned. arg: visual root.
                    GROUP_XS( vtree  , sptr<console::base> ), // visual tree events, arg: parent base_sptr.
                    GROUP_XS( scroll , rack                ), // event after scroll.
                    //EVENT_XS( created    , sptr<console::base> ), // event after itself creation, arg: itself bell_sptr.
                    //EVENT_XS( detached   , bell_sptr           ), // inform that subject is detached, arg: parent bell_sptr.
                    //EVENT_XS( invalidated, bool                ),
                    //EVENT_XS( moved      , twod                ), // release: event after moveto, arg: diff bw old and new coor twod. preview: event after moved by somebody.

                    SUBSET_XS( vtree )
                    {
                        EVENT_XS( attached, sptr<console::base> ), // Child has been attached, arg: parent sptr<base>.
                        EVENT_XS( detached, sptr<console::base> ), // Child has been detached, arg: parent sptr<base>.
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

                            INDEX_XS( x, y ),
                        };
                        SUBSET_XS( to_end )
                        {
                            EVENT_XS( x, rack ), // scroll to_end along X.
                            EVENT_XS( y, rack ), // scroll to_end along Y.

                            INDEX_XS( x, y ),
                        };
                        SUBSET_XS( bycoor )
                        {
                            EVENT_XS( x, rack ), // scroll absolute along X.
                            EVENT_XS( y, rack ), // scroll absolute along Y.

                            INDEX_XS( x, y ),
                        };
                        SUBSET_XS( bystep )
                        {
                            EVENT_XS( x, rack ), // scroll by delta along X.
                            EVENT_XS( y, rack ), // scroll by delta along Y.

                            INDEX_XS( x, y ),
                        };
                        SUBSET_XS( bypage )
                        {
                            EVENT_XS( x, rack ), // scroll by page along X.
                            EVENT_XS( y, rack ), // scroll by page along Y.

                            INDEX_XS( x, y ),
                        };
                        SUBSET_XS( cancel )
                        {
                            EVENT_XS( x, rack ), // cancel scrolling along X.
                            EVENT_XS( y, rack ), // cancel scrolling along Y.

                            INDEX_XS( x, y ),
                        };
                    };
                };
                SUBSET_XS( proceed )
                {
                    EVENT_XS( create  , rect                ), // return coordinates of the new object placeholder.
                    EVENT_XS( createat, console::create_t   ), // general: create an intance at the specified location and return sptr<base>.
                    EVENT_XS( createfrom, console::create_t ), // general: attach spcified intance and return sptr<base>.
                    EVENT_XS( createby, input::hids         ), // return gear with coordinates of the new object placeholder gear::slot.
                    EVENT_XS( destroy , console::base       ), // ??? bool return reference to the parent.
                    EVENT_XS( render  , bool                ), // ask children to render itself to the parent canvas, arg is the world is damaged or not.
                    EVENT_XS( attach  , sptr<console::base> ), // order to attach a child, arg is a parent base_sptr.
                    EVENT_XS( detach  , sptr<console::base> ), // order to detach a child, tier::release - kill itself, tier::preview - detach the child specified in args, arg is a child sptr.
                    EVENT_XS( unfocus , sptr<console::base> ), // order to unset focus on the specified object, arg is a object sptr.
                    EVENT_XS( swap    , sptr<console::base> ), // order to replace existing client. See tiling manager empty slot.
                    EVENT_XS( functor , console::functor    ), // exec functor (see pro::focus).
                    EVENT_XS( onbehalf, console::proc       ), // exec functor on behalf (see gate).
                    GROUP_XS( d_n_d   , sptr<console::base> ), // drag&drop functionality. See tiling manager empty slot and pro::d_n_d.
                    //EVENT_XS( focus      , sptr<console::base>      ), // order to set focus to the specified object, arg is a object sptr.
                    //EVENT_XS( commit     , si32                     ), // order to output the targets, arg is a frame number.
                    //EVENT_XS( multirender, vector<shared_ptr<face>> ), // ask children to render itself to the set of canvases, arg is an array of the face sptrs.
                    //EVENT_XS( draw       , face                     ), // ????  order to render itself to the canvas.
                    //EVENT_XS( checkin    , face_sptr                ), // order to register an output client canvas.

                    SUBSET_XS(d_n_d)
                    {
                        EVENT_XS(ask  , sptr<console::base>),
                        EVENT_XS(drop , console::create_t  ),
                        EVENT_XS(abort, sptr<console::base>),
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
                        EVENT_XS( leftright, input::hids ),
                        EVENT_XS( middle   , input::hids ),
                        EVENT_XS( wheel    , input::hids ),
                        EVENT_XS( win      , input::hids ),

                        INDEX_XS( left, right, leftright, middle, wheel, win ),
                    };
                    SUBSET_XS( pull )
                    {
                        EVENT_XS( left     , input::hids ),
                        EVENT_XS( right    , input::hids ),
                        EVENT_XS( leftright, input::hids ),
                        EVENT_XS( middle   , input::hids ),
                        EVENT_XS( wheel    , input::hids ),
                        EVENT_XS( win      , input::hids ),

                        INDEX_XS( left, right, leftright, middle, wheel, win ),
                    };
                    SUBSET_XS( cancel )
                    {
                        EVENT_XS( left     , input::hids ),
                        EVENT_XS( right    , input::hids ),
                        EVENT_XS( leftright, input::hids ),
                        EVENT_XS( middle   , input::hids ),
                        EVENT_XS( wheel    , input::hids ),
                        EVENT_XS( win      , input::hids ),

                        INDEX_XS( left, right, leftright, middle, wheel, win ),
                    };
                    SUBSET_XS( stop )
                    {
                        EVENT_XS( left     , input::hids ),
                        EVENT_XS( right    , input::hids ),
                        EVENT_XS( leftright, input::hids ),
                        EVENT_XS( middle   , input::hids ),
                        EVENT_XS( wheel    , input::hids ),
                        EVENT_XS( win      , input::hids ),

                        INDEX_XS( left, right, leftright, middle, wheel, win ),
                    };
                };
                SUBSET_XS( prop )
                {
                    EVENT_XS( name      , text        ), // user name.
                    EVENT_XS( zorder    , si32        ), // set form z-order, si32: -1 backmost, 0 plain, 1 topmost.
                    EVENT_XS( brush     , const cell  ), // set form brush/color.
                    EVENT_XS( fullscreen, bool        ), // set fullscreen flag.
                    EVENT_XS( viewport  , rect        ), // request: return form actual viewport.
                    EVENT_XS( lucidity  , si32        ), // set or request window transparency, si32: 0-255, -1 to request.
                    EVENT_XS( fixedsize , bool        ), // set ui::fork ratio.
                    GROUP_XS( window    , twod        ), // set or request window properties.
                    GROUP_XS( ui        , text        ), // set or request textual properties.
                    GROUP_XS( colors    , rgba        ), // set or request bg/fg colors.

                    SUBSET_XS( window )
                    {
                        EVENT_XS( size  , twod ), // set window size.
                    };
                    SUBSET_XS( ui )
                    {
                        EVENT_XS( header  , text ), // set/get form caption header.
                        EVENT_XS( footer  , text ), // set/get form caption footer.
                        EVENT_XS( tooltip , text ), // set/get tooltip text.
                        EVENT_XS( slimmenu, bool ), // set/get window menu size.
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
                    //    EVENT_XS( attached, sptr<base> ), // global: object attached to the world.
                    //    EVENT_XS( detached, sptr<base> ), // global: object detached from the world.
                    //};
                    //SUBSET_XS( user )
                    //{
                    //    EVENT_XS( attached, sptr<base> ), // global: user attached to the world.
                    //    EVENT_XS( detached, sptr<base> ), // global: user detached from the world.
                    //};
                };
                SUBSET_XS( state )
                {
                    EVENT_XS( mouse , si32          ), // notify the client if mouse is active or not. The form is active when the number of clients (form::eventa::mouse::enter - mouse::leave) is not zero, only release, si32 - number of clients.
                    EVENT_XS( header, console::para ), // notify the client has changed title.
                    EVENT_XS( footer, console::para ), // notify the client has changed footer.
                    EVENT_XS( params, console::para ), // notify the client has changed title params.
                    EVENT_XS( color , console::tone ), // notify the client has changed tone, preview to set.
                    GROUP_XS( keybd , bool          ), // notify the client if keybd is active or not. The form is active when the number of clients (form::eventa::keybd::got - keybd::lost) is not zero, only release.

                    SUBSET_XS( keybd )
                    {
                        EVENT_XS( got     , input::hids             ), // release: got  keyboard focus.
                        EVENT_XS( lost    , input::hids             ), // release: lost keyboard focus.
                        EVENT_XS( handover, console::gear_id_list_t ), // request: Handover all available foci.
                        EVENT_XS( enlist  , console::gear_id_list_t ), // anycast: Enumerate all available foci.
                        EVENT_XS( find    , console::focus_test_t   ), // request: Check the focus.
                        EVENT_XS( check   , bool                    ), // anycast: Check any focus.
                    };
                };
            };
        };
    };
}

namespace netxs::console
{
    using e2 = netxs::events::userland::e2;

    static constexpr auto max_value = twod{ 2000, 1000 }; //todo unify

    //todo OMG!, make it in another way.
    class skin
    {
        //todo revise
    public:
        //#define PROP_LIST                       \
        //X(brighter , "Highlighter modificator") \
        //X(shadower , "Darklighter modificator") \
        //X(shadow   , "Light Darklighter modificator") \
        //X(lucidity , "Global transparency")     \
        //X(selector , "Selection overlay")       \
        //X(bordersz , "Border size")
        //
        //#define X(a, b) a,
        //enum prop { PROP_LIST count };
        //#undef X
        //
        //#define X(a, b) b,
        //text description[prop::count] = { PROP_LIST };
        //#undef X
        //#undef PROP_LIST

        cell kb_colors;
        poly kb_grades;

        cell hi_colors;
        poly hi_grades;

        cell lo_colors;
        poly lo_grades;

        cell sh_colors;
        poly sh_grades;

        cell sl_colors;
        poly sl_grades;

        twod border = dot_11;
        si32 opaque = 0xFF;

        template<class V>
        struct _globals
        {
            static skin global; //extn link: skin: shared gradients.
        };

        static void setup(tone::prop parameter, uint8_t value)
        {
            auto& global = _globals<void>::global;
            switch (parameter)
            {
                case tone::prop::kb_focus:
                    global.kb_colors.bgc(tint::bluelt).bga(value)
                                    .fgc(tint::bluelt).fga(value);
                    global.kb_grades.recalc(global.kb_colors);
                    break;
                case tone::prop::brighter:
                    global.hi_colors.bgc(rgba(0xFF, 0xFF, 0xFF, value))
                                    .fgc(rgba(0xFF, 0xFF, 0xFF, value)).alpha(value);
                    global.hi_grades.recalc(global.hi_colors);
                    break;
                case tone::prop::shadower:
                    global.lo_colors.bgc(rgba(0x20, 0x20, 0x20, value)).bga(value);
                    global.lo_grades.recalc(global.lo_colors);
                    break;
                case tone::prop::shadow:
                    global.sh_colors.bgc(rgba(0x20, 0x20, 0x20, value)).bga(value);
                    global.sh_grades.recalc(global.sh_colors);
                    break;
                case tone::prop::selector:
                    global.sl_colors.txt(whitespace)
                        .bgc(rgba(0xFF, 0xFF, 0xFF, value)).bga(value);
                    global.sl_grades.recalc(global.sl_colors);
                    break;
                case tone::prop::lucidity:
                    global.opaque = value;
                    break;
                default:
                    break;
            }
        }
        static void setup(tone::prop parameter, twod const& size)
        {
            auto& global = _globals<void>::global;
            switch (parameter)
            {
                case tone::prop::bordersz:
                    global.border = size;
                    break;
                default:
                    break;
            }
        }

        // skin:: Return global brighter/shadower color (cell).
        static cell const& color(si32 property)
        {
            auto& global = _globals<void>::global;
            switch (property)
            {
                case tone::prop::kb_focus:
                    return global.kb_colors;
                    break;
                case tone::prop::brighter:
                    return global.hi_colors;
                    break;
                case tone::prop::shadower:
                    return global.lo_colors;
                    break;
                case tone::prop::shadow:
                    return global.sh_colors;
                    break;
                case tone::prop::selector:
                    return global.sl_colors;
                    break;
                default:
                    return global.hi_colors;
            }
        }
        // skin:: Return global gradient for brighter/shadower.
        static poly const& grade(si32 property)
        {
            auto& global = _globals<void>::global;
            switch (property)
            {
                case tone::prop::kb_focus:
                    return global.kb_grades;
                    break;
                case tone::prop::brighter:
                    return global.hi_grades;
                    break;
                case tone::prop::shadower:
                    return global.lo_grades;
                    break;
                case tone::prop::shadow:
                    return global.sh_grades;
                    break;
                case tone::prop::selector:
                    return global.sl_grades;
                    break;
                default:
                    return global.hi_grades;
            }
        }
        // skin:: Return global border size.
        static twod const& border_size()
        {
            auto& global = _globals<void>::global;
            return global.border;
        }
        // skin:: Return global transparency.
        static si32 const& shady()
        {
            auto& global = _globals<void>::global;
            return global.opaque;
        }
    };

    template<class V>
    skin skin::_globals<V>::global;

    // console: Textographical canvas.
    class face
        : public rich, public flow, public std::enable_shared_from_this<face>
    {
        twod anker;     // face: The position of the nearest visible paragraph.
        id_t piece = 1; // face: The nearest to top paragraph.
        vrgb cache;     // face: BlurFX temp buffer.

        // face: Is the c inside the viewport?
        bool inside(twod const& c)
        {
            return c.y >= 0 && c.y < region.size.y;
            //todo X-axis
        }

    public:
        //todo revise
        bool caret = faux; // face: Cursor visibility.
        bool moved = faux; // face: Is reflow required.
        bool decoy = true; // face: Is the cursor inside the viewport.

        // face: Print proxy something else at the specified coor.
        template<class T, class P>
        void output_proxy(T const& block, twod const& coord, P proxy)
        {
            flow::ac(coord);
            flow::compose<true>(block, proxy);
        }
        // face: Print something else at the specified coor.
        template<class T, class P = noop>
        void output(T const& block, twod const& coord, P printfx = P())
        {
            flow::ac(coord);
            flow::go(block, *this, printfx);
        }
        // face: Print something else.
        template<bool USE_FWD = faux, class T, class P = noop>
        void output(T const& block, P printfx = P())
        {
            //todo unify
            flow::print<USE_FWD>(block, *this, printfx);
        }
        // face: Print paragraph.
        void output(para const& block)
        {
            flow::print(block, *this);
        }
        // face: Print page.
        template<class P = noop>
        void output(page const& textpage, P printfx = P())
        {
            auto publish = [&](auto const& combo)
            {
                flow::print(combo, *this, printfx);
            };
            textpage.stream(publish);
        }
        // face: Print page with holding top visible paragraph on its own place.
        void output(page const& textpage, bool reset)
        {
            //todo if cursor is visible when tie to the cursor position
            //     else tie to the first visible text line.

            auto done = faux;
            // Get vertical position of the nearest paragraph to the top.
            auto gain = [&](auto const& combo)
            {
                auto pred = flow::print(combo, *this);

                auto post = flow::cp();
                if (!done)
                {
                    if (pred.y <= 0 && post.y >= 0)
                    {
                        anker.y = pred.y;
                        piece = combo.id();
                        done = true;
                    }
                    else
                    {
                        if (std::abs(anker.y) > std::abs(pred.y))
                        {
                            anker.y = pred.y;
                            piece = combo.id();
                        }
                    }
                }
            };

            // Get vertical position of the specified paragraph.
            auto find = [&](auto const& combo)
            {
                auto cp = flow::print(combo);
                if (combo.id() == piece) anker = cp;
            };

            if (reset)
            {
                anker.y = std::numeric_limits<si32>::max();
                textpage.stream(gain);

                decoy = caret && inside(flow::cp());
            }
            else
            {
                textpage.stream(find);
            }
        }
        // face: Reflow text page on the canvas and hold position
        //       of the top visible paragraph while resizing.
        void reflow(page& textpage)
        {
            if (moved)
            {
                flow::zz(); //flow::sc();

                auto delta = anker;
                output(textpage, faux);
                std::swap(delta, anker);

                auto  cover = flow::minmax();
                //auto& basis = flow::origin;
                auto basis = dot_00;// flow::origin;
                basis.y += anker.y - delta.y;

                if (decoy)
                {
                    // Don't tie the first line if it's the only one. Make one step forward.
                    if (anker.y == 0
                     && anker.y == flow::cp().y
                     && cover.height() > 1)
                    {
                        // the increment is removed bcos it shifts mc one row down on Ctrl+O and back
                        //basis.y++;
                    }

                    auto newcp = flow::cp();
                    if (!inside(newcp))
                    {
                        if (newcp.y < 0) basis.y -= newcp.y;
                        else             basis.y -= newcp.y - region.size.y + 1;
                    }
                }
                else
                {
                    basis.y = std::clamp(basis.y, -cover.b, region.size.y - cover.t - 1);
                }

                moved = faux;
            }

            wipe();
        }

        // face: Forward call to the core and reset cursor.
        template<class ...Args>
        void wipe(Args&&... args) // Optional args.
        {
            core::wipe(args...);
            flow::reset();
        }
        // face: Change current context. Return old context.
        auto bump(dent const& delta)
        {
            auto old_full = face::full();
            auto old_view = core::view();
            auto new_view = core::area().clip<true>(old_view + delta);
            auto new_full = old_full + delta;
            face::full(new_full);
            core::view(new_view);
            return std::pair{ old_full, old_view };
        }
        // face: Restore previously saved context.
        void bump(std::pair<rect, rect> ctx)
        {
            face::full(ctx.first);
            core::view(ctx.second);
        }

        // Use a two letter function if we don't need to return *this
        face& cup (twod const& p) { flow::ac( p); return *this; } // face: Cursor 0-based absolute position.
        face& chx (si32 x)        { flow::ax( x); return *this; } // face: Cursor 0-based horizontal absolute.
        face& chy (si32 y)        { flow::ay( y); return *this; } // face: Cursor 0-based vertical absolute.
        face& cpp (twod const& p) { flow::pc( p); return *this; } // face: Cursor percent position.
        face& cpx (si32 x)        { flow::px( x); return *this; } // face: Cursor H percent position.
        face& cpy (si32 y)        { flow::py( y); return *this; } // face: Cursor V percent position.
        face& cuu (si32 n = 1)    { flow::dy(-n); return *this; } // face: cursor up.
        face& cud (si32 n = 1)    { flow::dy( n); return *this; } // face: Cursor down.
        face& cuf (si32 n = 1)    { flow::dx( n); return *this; } // face: Cursor forward.
        face& cub (si32 n = 1)    { flow::dx(-n); return *this; } // face: Cursor backward.
        face& cnl (si32 n = 1)    { flow::dy( n); return *this; } // face: Cursor next line.
        face& cpl (si32 n = 1)    { flow::dy(-n); return *this; } // face: Cursor previous line.

        face& ocp (twod const& p) { flow::oc( p); return *this; } // face: Cursor 1-based absolute position.
        face& ocx (si32 x)        { flow::ox( x); return *this; } // face: Cursor 1-based horizontal absolute.
        face& ocy (si32 y)        { flow::oy( y); return *this; } // face: Cursor 1-based vertical absolute.

        face& scp ()              { flow::sc(  ); return *this; } // face: Save cursor position.
        face& rcp ()              { flow::rc(  ); return *this; } // face: Restore cursor position.
        face& rst ()  { flow::zz(); flow::sc(  ); return *this; } // face: Reset to zero all cursor params.

        face& tab (si32 n = 1)    { flow::tb( n); return *this; } // face: Proceed the \t .
        face& eol (si32 n = 1)    { flow::nl( n); return *this; } // face: Proceed the \r || \n || \r\n .

        void size (twod const& newsize) // face: Change the size of the face/core.
        {
            core::size(newsize);
            flow::size(newsize);
        }
        auto size () // face: Return size of the face/core.
        {
            return core::size();
        }
        template<bool BOTTOM_ANCHORED = faux>
        void crop(twod const& newsize, cell const& c) // face: Resize while saving the bitmap.
        {
            core::crop<BOTTOM_ANCHORED>(newsize, c);
            flow::size(newsize);
        }
        template<bool BOTTOM_ANCHORED = faux>
        void crop(twod const& newsize) // face: Resize while saving the bitmap.
        {
            core::crop<BOTTOM_ANCHORED>(newsize, core::mark());
            flow::size(newsize);
        }
        template<class P = noop>
        void blur(si32 r, P shade = P())
        {
            using irgb = vrgb::value_type;

            auto view = core::view();
            auto size = core::size();

            auto w = view.size.x;
            auto h = view.size.y;

            if (auto size = w * h; cache.size() < size) cache.resize(size);

            auto s_ptr = core::data(view.coor);
            auto d_ptr = cache.data();

            auto s_width = size.x;
            auto d_width = view.size.x;

            auto s_point = [](cell* c)->auto& { return c->bgc(); };
            auto d_point = [](irgb* c)->auto& { return *c;       };

            netxs::bokefy<irgb>(s_ptr,
                                d_ptr, w,
                                       h, r, s_width,
                                             d_width, s_point,
                                                      d_point, shade);
        }
        // face: Render nested object to the canvas using renderproc. TRIM = trim viewport to the client area.
        template<bool TRIM = true, class T>
        void render(sptr<T> nested_ptr, twod const& basis = {})
        {
            if (nested_ptr)
            {
                auto& nested = *nested_ptr;
                face::render<TRIM>(nested, basis);
            }
        }
        // face: Render nested object to the canvas using renderproc. TRIM = trim viewport to the client area.
        template<bool TRIM = true, class T>
        void render(T& nested, twod const& offset_coor)
        {
            auto canvas_view = core::view();
            auto parent_area = flow::full();

            auto object_area = nested.area();
            object_area.coor+= parent_area.coor;

            auto nested_view = canvas_view.clip(object_area);
            //todo revise: why whole canvas is not used
            if (TRIM ? nested_view : canvas_view)
            {
                auto canvas_coor = core::coor();
                if constexpr (TRIM) core::view(nested_view);
                core::back(offset_coor);
                flow::full(object_area);

                nested.SIGNAL(tier::release, e2::render::prerender, *this);
                nested.SIGNAL(tier::release, e2::postrender, *this);

                if constexpr (TRIM) core::view(canvas_view);
                core::move(canvas_coor);
                flow::full(parent_area);
            }
        }
        // face: Render itself to the canvas using renderproc.
        template<bool POST = true, class T>
        void render(T& object)
        {
            auto canvas_view = core::view();
            auto parent_area = flow::full();

            auto object_area = object.area();
            object_area.coor-= core::coor();

            if (auto nested_view = canvas_view.clip(object_area))
            {
                core::view(nested_view);
                flow::full(object_area);

                                    object.SIGNAL(tier::release, e2::render::prerender, *this);
                if constexpr (POST) object.SIGNAL(tier::release, e2::postrender,        *this);

                core::view(canvas_view);
                flow::full(parent_area);
            }
        }
    };

    // console: Base visual.
    class base
        : public bell, public std::enable_shared_from_this<base>
    {
        wptr<base> parent_shadow; // base: Parental visual tree weak-pointer.
        cell brush;
        rect square;
        bool invalid = true; // base: Should the object be redrawn.
        bool visual_root = faux; // Whether the size is tied to the size of the clients.
        hook kb_token;
        hook cascade_token;
        si32 object_kind = {};

    public:
        static constexpr si32 reflow_root = -1; //todo unify

        //todo replace "side" with "dent<si32>"
        side oversz; // base: Oversize, margin.
        twod anchor; // base: Object balance point. Center point for any transform (on preview).

        template<class T = base>
        auto  This()       { return std::static_pointer_cast<std::remove_reference_t<T>>(shared_from_this()); }
        auto& coor() const { return square.coor; }
        auto& size() const { return square.size; }
        auto& area() const { return square; }
        void  root(bool b) { assert(!kb_token); visual_root = b; }
        bool  root()       { return visual_root; }
        si32  kind()       { return object_kind; }
        void  kind(si32 k) { object_kind = k; }
        auto parent()      { return parent_shadow.lock(); }
        void ruined(bool state) { invalid = state; }
        auto ruined() const { return invalid; }
        template<bool ABSOLUTE_POS = true>
        auto actual_area() const
        {
            auto area = rect{ -oversz.topleft(), square.size + oversz.summ() };
            if constexpr (ABSOLUTE_POS) area.coor += square.coor;
            return area;
        }
        auto color() const { return brush; }
        void color(rgba const& fg_color, rgba const& bg_color)
        {
            // To make an object transparent to mouse events,
            // no id (cell::id = 0) is used by default in the brush.
            // The bell::id is configurable only with pro::mouse.
            base::brush.bgc(bg_color)
                       .fgc(fg_color)
                       .txt(whitespace);
            SIGNAL(tier::release, e2::form::prop::brush, brush);
        }
        void color(cell const& new_brush)
        {
            base::brush = new_brush;
            SIGNAL(tier::release, e2::form::prop::brush, brush);
        }
        // base: Move the form to a new place, and return the delta.
        auto moveto(twod new_coor)
        {
            auto old_coor = square.coor;
            SIGNAL(tier::preview, e2::coor::set, new_coor);
            SIGNAL(tier::release, e2::coor::set, new_coor);
            auto delta = square.coor - old_coor;
            return delta;
        }
        // base: Dry run. Check current position.
        auto moveto()
        {
            auto new_value = square.coor;
            return moveto(new_value);
        }
        // base: Move the form by the specified step and return the coor delta.
        auto moveby(twod const& step)
        {
            auto delta = moveto(square.coor + step);
            return delta;
        }
        // base: Resize the form, and return the size delta.
        auto resize(twod new_size)
        {
            auto old_size = square.size;
            SIGNAL(tier::preview, e2::size::set, new_size);
            SIGNAL(tier::release, e2::size::set, new_size);
            return square.size - old_size;
        }
        // base: Resize the form, and return the new size.
        auto& resize(si32 x, si32 y)
        {
            auto new_size = twod{ x,y };
            SIGNAL(tier::preview, e2::size::set, new_size);
            SIGNAL(tier::release, e2::size::set, new_size);
            return size();
        }
        // base: Resize the form relative the center point.
        //       Return center point offset.
        //       The object is responsible for correcting
        //       the center point during resizing.
        auto resize(twod newsize, twod point)
        {
            point -= square.coor;
            anchor = point; //todo use dot_00 instead of point
            resize(newsize);
            auto delta = moveby(point - anchor);
            return delta;
        }
        // base: Dry run (preview then release) current value.
        auto resize()
        {
            auto new_value = square.size;
            return resize(new_value);
        }
        // base: Resize the form by step, and return delta.
        auto sizeby(twod const& step)
        {
            auto delta = resize(square.size + step);
            return delta;
        }
        // base: Resize and move the form, and return delta.
        auto extend(rect newloc)
        {
            return rect{ moveto(newloc.coor), resize(newloc.size) };
        }
        // base: Mark the visual subtree as requiring redrawing.
        void strike(rect region)
        {
            region.coor += square.coor;
            if (auto parent_ptr = parent())
            {
                parent_ptr->deface(region);
            }
        }
        // base: Mark the visual subtree as requiring redrawing.
        void strike()
        {
            strike(square);
        }
        // base: Mark the form and its subtree as requiring redrawing.
        virtual void deface(rect const& region)
        {
            invalid = true;
            strike(region);
        }
        // base: Mark the form and its subtree as requiring redrawing.
        void deface()
        {
            deface(square);
        }
        // base: Going to rebuild visual tree. Retest current size, ask parent if it is linked.
        template<bool FORCED = faux>
        void reflow()
        {
            auto parent_ptr = parent();
            if (parent_ptr && (!visual_root || (FORCED && (kind() != base::reflow_root)))) //todo unify -- See basewindow in vtm.cpp
            {
                parent_ptr->reflow<FORCED>();
            }
            else
            {
                if (auto delta = resize())
                {
                    //SIGNAL(tier::release, e2::form::upon::resized, delta);
                }
            }
        }
        // base: Remove the form from the visual tree.
        void detach()
        {
            if (auto parent_ptr = parent())
            {
                strike();
                parent_ptr->SIGNAL(tier::preview, e2::form::proceed::detach, This());
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
        void global(twod& coor) override
        {
            coor -= square.coor;
            if (auto parent_ptr = parent())
            {
                parent_ptr->global(coor);
            }
        }
        // base: Recursively find the root of the visual tree.
        sptr<bell> gettop() override
        {
            auto parent_ptr = parent();
            if (!visual_root && parent_ptr) return parent_ptr->gettop();
            else                            return This();
        }
        // base: Invoke a lambda with parent as a parameter.
        // Usage example:
        //     toboss([&](auto& parent_ptr) { c.fuse(parent.brush); });
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
        template<tier TIER, class EVENT, class T>
        void riseup(EVENT, T&& data, bool forced = faux)
        {
            if (forced)
            {
                SIGNAL(TIER, EVENT{}, data);
                base::toboss([&](auto& boss)
                {
                    boss.base::template riseup<TIER>(EVENT{}, std::forward<T>(data), forced);
                });
            }
            else
            {
                if (!SIGNAL(TIER, EVENT{}, data))
                {
                    base::toboss([&](auto& boss)
                    {
                        boss.base::template riseup<TIER>(EVENT{}, std::forward<T>(data), forced);
                    });
                }
            }
        }
        // base: Fire an event on yourself and pass it parent if not handled.
        // Warning: The parameter type is not checked/casted.
        // Usage example:
        //          base::raw_riseup<tier::preview, e2::form::prop::ui::header>(txt);
        template<tier TIER, class T>
        void raw_riseup(hint event_id, T&& param, bool forced = faux)
        {
            if (forced)
            {
                bell::template signal<TIER>(event_id, param);
                base::toboss([&](auto& boss)
                {
                    boss.base::template raw_riseup<TIER>(event_id, std::forward<T>(param), forced);
                });
            }
            else
            {
                if (!bell::template signal<TIER>(event_id, param))
                {
                    base::toboss([&](auto& boss)
                    {
                        boss.base::template raw_riseup<TIER>(event_id, std::forward<T>(param), forced);
                    });
                }
            }
        }

    protected:
        virtual ~base() = default;
        base()
        {
            SUBMIT(tier::request, e2::depth, depth) { depth++; };

            SUBMIT(tier::release, e2::coor::any, new_coor) { square.coor = new_coor; };
            SUBMIT(tier::request, e2::coor::set, coor_var) { coor_var = square.coor; };
            SUBMIT(tier::release, e2::size::any, new_size) { square.size = new_size; };
            SUBMIT(tier::request, e2::size::set, size_var) { size_var = square.size; };

            SUBMIT(tier::release, e2::cascade, proc)
            {
                auto backup = This();
                auto keepon = proc(backup);
                if (!keepon) this->bell::expire<tier::release>();
            };
            SUBMIT(tier::release, e2::form::upon::vtree::attached, parent_ptr)
            {
                if (!visual_root)
                {
                    parent_ptr->SUBMIT_T(tier::release, e2::cascade, cascade_token, proc)
                    {
                        auto backup = This();
                        backup->SIGNAL(tier::release, e2::cascade, proc);
                    };
                }
                parent_shadow = parent_ptr;
                // Propagate form events up to the visual branch.
                // Exec after all subscriptions.
                //todo implement via e2::cascade
                parent_ptr->SUBMIT_T(tier::release, hids::events::upevent::any, kb_token, gear)
                {
                    if (auto parent_ptr = parent_shadow.lock())
                    {
                        if (gear.focus_changed()) //todo unify, upevent::kbannul using it
                        {
                            parent_ptr->bell::expire<tier::release>();
                        }
                        else
                        {
                            if (auto deed = parent_ptr->bell::protos<tier::release>())
                            {
                                this->bell::signal<tier::release>(deed, gear);
                            }
                        }
                    }
                };
            };
            SUBMIT(tier::release, e2::form::upon::vtree::any, parent_ptr)
            {
                if (this->bell::protos<tier::release>(e2::form::upon::vtree::detached))
                {
                    kb_token.reset();
                    cascade_token.reset();
                }
                if (parent_ptr) parent_ptr->base::reflow(); //todo too expensive
            };

            // Propagate form events down to the visual branch.
            // Exec after all subscriptions.
            SUBMIT(tier::release, hids::events::notify::any, gear)
            {
                if (auto parent_ptr = parent_shadow.lock())
                {
                    if (auto deed = this->bell::protos<tier::release>())
                    {
                        parent_ptr->bell::signal<tier::release>(deed, gear);
                    }
                }
                //strike();
            };
            SUBMIT(tier::release, e2::render::any, parent_canvas)
            {
                if (base::brush.wdt())
                {
                    parent_canvas.fill([&](cell& c) { c.fusefull(base::brush); });
                }
            };
        }
    };

    // console: Template modules for the base class behavior extension.
    namespace pro
    {
        // pro: Base class for plugins.
        struct skill
        {
            base& boss;
            subs  memo;

            skill(base&&) = delete;
            skill(base& boss) : boss{ boss } { }
            virtual ~skill() = default; // In order to allow man derived class via base ptr.

            template<class T>
            struct socks
            {
                struct sock : public T
                {
                    id_t    id; // sock: Hids ID.
                    si32 count; // sock: Clients count.

                    sock(id_t ctrl)
                        :    id{ ctrl },
                          count{ 0    }
                    { }

                    operator bool () { return T::operator bool(); }
                };

                std::vector<sock> items; // sock: Registered hids.
                hook              token; // sock: Hids dtor submission.

                socks()
                {
                    SUBMIT_GLOBAL(hids::events::die, token, gear)
                    {
                        del(gear);
                    };
                }
                template<bool CONST_WARN = true>
                auto& take(hids& gear)
                {
                    for (auto& item : items) // Linear search, because a few items.
                    {
                        if (item.id == gear.id) return item;
                    }

                    if constexpr (CONST_WARN)
                    {
                        log("sock: error: access to unregistered input device, ", gear.id);
                    }

                    return items.emplace_back(gear.id);
                }
                template<class P>
                void foreach(P proc)
                {
                    for (auto& item : items)
                    {
                        if (item) proc(item);
                    }
                }
                void add(hids& gear)
                {
                    auto& item = take<faux>(gear);
                    ++item.count;
                }
                void dec(hids& gear)
                {
                    auto& item = take(gear);
                    if (--item.count < 1) // item.count could but equal to 0 due to unregistered access.
                    {
                        if (items.size() > 1) item = items.back(); // Remove an item without allocations.
                        items.pop_back();
                    }
                }
                void del(hids& gear)
                {
                    for (auto& item : items) // Linear search, because a few items.
                    {
                        if (item.id == gear.id)
                        {
                            if (items.size() > 1) item = items.back(); // Remove an item without allocations.
                            items.pop_back();
                            return;
                        }
                    }
                }
            };
        };

        // pro: Provides resizing by dragging.
        class sizer
            : public skill
        {
            struct sock
            {
                using test = testy<twod>;

                twod origin; // sock: Grab's initial coord info.
                twod dtcoor; // sock: The form coor parameter change factor while resizing.
                twod sector; // sock: Active quadrant, x,y = {-1|+1}. Border widths.
                rect hzgrip; // sock: Horizontal grip.
                rect vtgrip; // sock: Vertical grip.
                twod widths; // sock: Grip's widths.
                bool inside; // sock: Is active.
                bool seized; // sock: Is seized.
                test lastxy; // sock: Change tracker.

                sock()
                    : inside{ faux },
                      seized{ faux }
                { }

                operator bool () { return inside || seized; }
                auto corner(twod const& length)
                {
                    return dtcoor.less(dot_11, length, dot_00);
                }
                auto grab(base const& master, twod curpos, dent const& outer)
                {
                    if (inside)
                    {
                        origin = curpos - corner(master.base::size() + outer);
                        seized = true;
                    }
                    return seized;
                }
                auto calc(base const& master, twod curpos, dent const& outer, dent const& inner, dent const& border)
                {
                    auto area = rect{ dot_00, master.base::size() };
                    auto inner_rect = area + inner;
                    auto outer_rect = area + outer;
                    inside = !inner_rect.hittest(curpos)
                           && outer_rect.hittest(curpos);
                    auto& length = outer_rect.size;
                    curpos += outer.corner();
                    auto center = std::max(length / 2, dot_11);
                    if (!seized)
                    {
                        dtcoor = curpos.less(center + (length & 1), dot_11, dot_00);
                        sector = dtcoor.less(dot_11, -dot_11, dot_11);
                        widths = sector.less(dot_00, twod{-border.east.step,-border.foot.step },
                                                     twod{ border.west.step, border.head.step });
                    }
                    auto l = sector * (curpos - corner(length));
                    auto a = center * l / center;
                    auto b = center *~l /~center;
                    auto s = sector * std::max(a - b + center, dot_00);

                    hzgrip.coor.x = widths.x;
                    hzgrip.coor.y = 0;
                    hzgrip.size.y = widths.y;
                    hzgrip.size.x = s.x;

                    vtgrip.coor = dot_00;
                    vtgrip.size = widths;
                    vtgrip.size.y += s.y;
                    return lastxy(curpos);
                }
                auto drag(base& master, twod const& curpos, dent const& outer)
                {
                    if (seized)
                    {
                        auto width = master.base::size() + outer;
                        auto delta = corner(width) + origin - curpos;
                        if (auto dxdy = master.base::sizeby(delta * sector))
                        {
                            auto step = -dxdy * dtcoor;
                            master.base::moveby(step);
                            master.SIGNAL(tier::preview, e2::form::upon::changed, dxdy);
                        }
                    }
                    return seized;
                }
                void drop()
                {
                    seized = faux;
                }
            };

            using list = socks<sock>;
            using skill::boss,
                  skill::memo;

            list items;
            dent outer;
            dent inner;
            dent width;
            bool alive; // pro::sizer: The sizer state.

        public:
            void props(dent const& outer_rect = {2,2,1,1}, dent const& inner_rect = {})
            {
                outer = outer_rect;
                inner = inner_rect;
                width = outer - inner;
            }
            auto get_props()
            {
                return std::pair{ outer, inner };
            }

            sizer(base&&) = delete;
            sizer(base& boss, dent const& outer_rect = {2,2,1,1}, dent const& inner_rect = {})
                : skill{ boss          },
                  outer{ outer_rect    },
                  inner{ inner_rect    },
                  width{ outer - inner },
                  alive{ true          }
            {
                boss.SUBMIT_T(tier::release, e2::config::plugins::sizer::alive, memo, state)
                {
                    alive = state;
                };
                boss.SUBMIT_T(tier::release, e2::postrender, memo, canvas)
                {
                    if (!alive) return;
                    auto area = canvas.full() + outer;
                    auto fuse = [&](cell& c){ c.xlight(); };
                    canvas.cage(area, width, [&](cell& c){ c.link(boss.id); });
                    items.foreach([&](sock& item)
                    {
                        auto corner = item.corner(area.size);
                        auto side_x = item.hzgrip.shift(corner).normalize_itself()
                                                 .shift_itself(area.coor).clip(area);
                        auto side_y = item.vtgrip.shift(corner).normalize_itself()
                                                 .shift_itself(area.coor).clip(area);
                        canvas.fill(side_x, fuse);
                        canvas.fill(side_y, fuse);
                    });
                };
                boss.SUBMIT_T(tier::release, e2::form::layout::swarp, memo, warp)
                {
                    auto area = boss.base::area();
                    auto next = area + warp;
                    auto step = boss.extend(next);
                };
                boss.SUBMIT_T(tier::release, hids::events::notify::mouse::enter, memo, gear)
                {
                    items.add(gear);
                };
                boss.SUBMIT_T(tier::release, hids::events::notify::mouse::leave, memo, gear)
                {
                    items.dec(gear);
                };
                boss.SUBMIT_T(tier::release, e2::config::plugins::sizer::outer, memo, outer_rect)
                {
                    outer = outer_rect;
                    width = outer - inner;
                };
                boss.SUBMIT_T(tier::release, e2::config::plugins::sizer::inner, memo, inner_rect)
                {
                    inner = inner_rect;
                    width = outer - inner;
                };
                boss.SUBMIT_T(tier::request, e2::config::plugins::sizer::inner, memo, inner_rect)
                {
                    inner_rect = inner;
                };
                boss.SUBMIT_T(tier::request, e2::config::plugins::sizer::outer, memo, outer_rect)
                {
                    outer_rect = outer;
                };

                engage<sysmouse::left>();
                engage<sysmouse::leftright>();
            }
            // pro::sizer: Configuring the mouse button to operate.
            template<sysmouse::bttns BUTTON>
            void engage()
            {
                boss.SIGNAL(tier::release, e2::form::draggable::_<BUTTON>, true);
                boss.SUBMIT_T(tier::release, hids::events::mouse::move, memo, gear)
                {
                    if (items.take(gear).calc(boss, gear.coord, outer, inner, width))
                    {
                        boss.base::deface(); // Deface only if mouse moved.
                    }
                };
                boss.SUBMIT_T(tier::release, e2::form::drag::start::_<BUTTON>, memo, gear)
                {
                    if (items.take(gear).grab(boss, gear.coord, outer))
                    {
                        gear.dismiss();
                    }
                };
                boss.SUBMIT_T(tier::release, e2::form::drag::pull::_<BUTTON>, memo, gear)
                {
                    if (items.take(gear).drag(boss, gear.coord, outer))
                    {
                        gear.dismiss();
                    }
                };
                boss.SUBMIT_T(tier::release, e2::form::drag::cancel::_<BUTTON>, memo, gear)
                {
                    items.take(gear).drop();
                };
                boss.SUBMIT_T(tier::release, e2::form::drag::stop::_<BUTTON>, memo, gear)
                {
                    items.take(gear).drop();
                    boss.SIGNAL(tier::release, e2::form::upon::dragged, gear);
                };
            }
        };

        // pro: Provides moving by dragging.
        class mover
            : public skill
        {
            struct sock
            {
                twod origin; // sock: Grab's initial coord info.
                void grab(base const& master, twod const& curpos)
                {
                    auto center = master.base::size() / 2;
                    origin = curpos - center;
                }
                void drag(base& master, twod const& coord)
                {
                    auto delta = coord - origin;
                    auto center = master.base::size() / 2;
                    delta -= center;
                    master.base::moveby(delta);
                }
            };

            using list = socks<sock>;
            using skill::boss,
                  skill::memo;

            list       items;
            wptr<base> dest_shadow;
            sptr<base> dest_object;

        public:
            mover(base&&) = delete;
            mover(base& boss, sptr<base> subject)
                : skill{ boss },
                  dest_shadow{ subject }
            {
                boss.SUBMIT_T(tier::release, hids::events::notify::mouse::enter, memo, gear)
                {
                    items.add(gear);
                };
                boss.SUBMIT_T(tier::release, hids::events::notify::mouse::leave, memo, gear)
                {
                    items.dec(gear);
                };
                engage<sysmouse::left>();
            }
            mover(base& boss)
                : mover{ boss, boss.This() }
            { }
            // pro::mover: Configuring the mouse button to operate.
            template<sysmouse::bttns BUTTON>
            void engage()
            {
                boss.SIGNAL(tier::release, e2::form::draggable::_<BUTTON>, true);
                boss.SUBMIT_T(tier::release, e2::form::drag::start::_<BUTTON>, memo, gear)
                {
                    if ((dest_object = dest_shadow.lock()))
                    {
                        items.take(gear).grab(*dest_object, gear.coord);
                        gear.dismiss();
                    }
                };
                boss.SUBMIT_T(tier::release, e2::form::drag::pull::_<BUTTON>, memo, gear)
                {
                    if (dest_object)
                    {
                        items.take(gear).drag(*dest_object, gear.coord);
                        auto delta = gear.delta.get();
                        dest_object->SIGNAL(tier::preview, e2::form::upon::changed, delta);
                        gear.dismiss();
                    }
                };
                boss.SUBMIT_T(tier::release, e2::form::drag::cancel::_<BUTTON>, memo, gear)
                {
                    if (dest_object)
                    {
                        dest_object.reset();
                        gear.dismiss();
                    }
                };
                boss.SUBMIT_T(tier::release, e2::form::drag::stop::_<BUTTON>, memo, gear)
                {
                    if (dest_object)
                    {
                        dest_object->SIGNAL(tier::release, e2::form::upon::dragged, gear);
                        dest_object.reset();
                        gear.dismiss();
                    }
                };
            }
        };

        // pro: Mouse cursor highlighter.
        class track
            : public skill
        {
            struct sock
            {
                twod cursor{}; // sock: Coordinates of the active cursor.
                bool inside{}; // sock: Is active.

                operator bool () { return inside; }
                auto calc(base const& master, twod curpos)
                {
                    auto area = rect{ dot_00, master.base::size() };
                    cursor = curpos;
                    inside = area.hittest(curpos);
                }
            };

            using list = socks<sock>;
            using skill::boss,
                  skill::memo;

            list items; // track: .
            bool alive; // track: Is active.

        public:
            track(base&&) = delete;
            track(base& boss)
                : skill{ boss },
                  alive{ true }
            {
                boss.SUBMIT_T(tier::anycast, e2::form::prop::lucidity, memo, lucidity)
                {
                    if (lucidity != -1) alive = lucidity == 0xFF;
                };
                boss.SUBMIT_T(tier::release, hids::events::mouse::move, memo, gear)
                {
                    items.take(gear).calc(boss, gear.coord);
                };
                boss.SUBMIT_T(tier::release, hids::events::notify::mouse::enter, memo, gear)
                {
                    items.add(gear);
                };
                boss.SUBMIT_T(tier::release, hids::events::notify::mouse::leave, memo, gear)
                {
                    items.dec(gear);
                };
                boss.SUBMIT_T(tier::release, e2::render::prerender, memo, parent_canvas)
                {
                    if (!alive) return;
                    auto full = parent_canvas.full();
                    auto view = parent_canvas.view();
                    auto mark = cell{}.bgc(0xFFffffff);
                    auto fill = [&](cell& c) { c.fuse(mark); };
                    items.foreach([&](sock& item)
                    {
                        auto area = rect{ item.cursor,dot_00 } + dent{ 6,6,3,3 };
                        area.coor += full.coor;
                        parent_canvas.fill(area.clip(full), fill);
                    });
                };
            }
        };

        // pro: Provides size-binding functionality.
        class align
            : public skill
        {
            using gptr = wptr<bell>;
            using skill::boss,
                  skill::memo;

            rect last{}; // pro::align: Window size before the fullscreen has applied.
            text head{}; // pro::align: Main window title the fullscreen has applied.
            id_t weak{}; // pro::align: Master id.
            rect body{}; // pro::align: For current coor/size tracking.
            twod pads{}; // pro::align: Owner's borders.
            hook maxs{}; // pro::align: Maximize on dblclick token.

            auto seized(id_t master)
            {
                return weak == master;
            }

        public:
            align(base&&) = delete;
            align(base& boss, bool maximize = true)
                : skill{ boss }
            {
                boss.SUBMIT_T(tier::release, e2::config::plugins::align, memo, set)
                {
                    if (set)
                    {
                        boss.SUBMIT_T(tier::release, e2::form::maximize, maxs, gear)
                        {
                            auto area = boss.base::area();
                            auto home = rect{ -dot_21, area.size + dot_21 * 2};
                            if (home.hittest(gear.coord)) // Including resizer grips.
                            {
                                if (seized(gear.owner.id)) unbind();
                                else                       follow(gear.owner.id, dot_00);
                            }
                        };
                    }
                    else maxs.reset();
                };

                boss.SIGNAL(tier::release, e2::config::plugins::align, maximize);
            }
           ~align() { unbind(faux); }

            void follow(id_t master, twod const& borders)
            {
                pads = borders;
                if (auto gate_ptr = bell::getref(master))
                {
                    auto& gate = *gate_ptr;

                    auto area = rect{};
                    gate.SIGNAL(tier::request, e2::size::set, area.size);
                    gate.SIGNAL(tier::request, e2::coor::set, area.coor);
                    last = boss.base::area();
                    area.coor -= pads;
                    area.size += pads * 2;
                    body = {}; // In oder to unbind previous subscription if it is.
                    boss.base::extend(area);
                    body = area;

                    auto newhead = text{};
                    gate.SIGNAL(tier::request, e2::form::prop::ui::header, head);
                    boss.SIGNAL(tier::request, e2::form::prop::ui::header, newhead);
                    gate.SIGNAL(tier::preview, e2::form::prop::ui::header, newhead);
                    gate.SIGNAL(tier::release, e2::form::prop::fullscreen, true);

                    gate.SUBMIT_T(tier::release, e2::size::any, memo, size)
                    {
                        body.size = size + pads * 2;
                        boss.base::resize(body.size);
                    };
                    gate.SUBMIT_T(tier::release, e2::coor::any, memo, coor)
                    {
                        unbind();
                    };
                    gate.SUBMIT_T(tier::release, e2::dtor, memo, master_id)
                    {
                        unbind();
                    };

                    boss.SUBMIT_T(tier::release, e2::size::any, memo, size)
                    {
                        if (weak && body.size != size) unbind(faux);
                    };
                    boss.SUBMIT_T(tier::release, e2::coor::any, memo, coor)
                    {
                        if (weak && body.coor != coor) unbind(true, faux);
                    };

                    weak = master;
                    boss.SUBMIT_T(tier::release, e2::form::prop::ui::header, memo, newhead)
                    {
                        if (auto gate_ptr = bell::getref(weak))
                        {
                            gate_ptr->SIGNAL(tier::preview, e2::form::prop::ui::header, newhead);
                        }
                        else unbind();
                    };
                }
            }
            void unbind(bool restor_size = true, bool restor_coor = true)
            {
                if (memo.count())
                {
                    memo.clear();
                    if (auto gate_ptr = bell::getref(weak))
                    {
                        gate_ptr->SIGNAL(tier::preview, e2::form::prop::ui::header, head);
                        gate_ptr->SIGNAL(tier::release, e2::form::prop::fullscreen, faux);
                    }
                }
                weak = {};
                if (restor_size && restor_coor) boss.base::extend(last); // Restore previous position
                else
                {
                    if (restor_size)
                    {
                        boss.base::resize(last.size);
                        boss.base::moveby(boss.base::anchor - last.size / twod{ 2,4 }); // Centrify on mouse. See pro::frame pull.
                    }
                    else if (restor_coor) boss.base::moveto(last.coor);
                }
            }
        };

        // pro: Provides functionality for runtime animation (time-based).
        class robot
            : public skill
        {
            using subs = std::map<id_t, hook>;
            using skill::boss;

            subs memo;

        public:
            using skill::skill; // Inherits ctors.

            // pro::robot: Every timer tick, yield the
            //             delta from the flow and, if delta,
            //             Call the proc (millisecond precision).
            template<class P, class S>
            void actify(id_t ID, S flow, P proc)
            {
                auto init = tempus::now();
                auto handler = [&, ID, proc, flow, init](auto p)
                {
                    auto now = datetime::round<si32>(p - init);
                    if (auto data = flow(now))
                    {
                        proc(data.value());
                    }
                    else
                    {
                        pacify(ID);
                    }
                };
                boss.SUBMIT_TV(tier::general, e2::timer::any, memo[ID], handler);
                boss.SIGNAL(tier::release, e2::form::animate::start, ID);
            }
            // pro::robot: Optional proceed every timer tick,
            //             yield the delta from the flow and,
            //             if delta, Call the proc (millisecond precision).
            template<class P, class S>
            void actify(id_t ID, std::optional<S> flow, P proc)
            {
                if (flow)
                {
                    actify(ID, flow.value(), proc);
                }
            }
            template<class P, class S>
            void actify(S flow, P proc)
            {
                actify(bell::noid, flow, proc);
            }
            template<class P, class S>
            void actify(std::optional<S> flow, P proc)
            {
                if (flow)
                {
                    actify(bell::noid, flow.value(), proc);
                }
            }
            // pro::robot: Cancel tick activity.
            void pacify(id_t id = bell::noid)
            {
                if (id == bell::noid) memo.clear(); // Stop all animations.
                else                  memo.erase(id);
                boss.SIGNAL(tier::release, e2::form::animate::stop, id);
            }
            // pro::robot: Check activity by id.
            bool active(id_t id)
            {
                return memo.contains(id);
            }
            // pro::robot: Check any activity.
            operator bool ()
            {
                return !memo.empty();
            }
        };

        // pro: Invokes specified proc after timeout.
        class timer
            : public skill
        {
            using subs = std::map<id_t, hook>;
            using skill::boss;

            subs memo;

        public:
            using skill::skill; // Inherits ctors.

            // pro::timer: Start countdown for specified ID.
            template<class P>
            void actify(id_t ID, period timeout, P lambda)
            {
                auto alarm = tempus::now() + timeout;
                auto handler = [&, ID, timeout, lambda, alarm](auto now) mutable
                {
                    if (now > alarm)
                    {
                        alarm = now + timeout;
                        if (!lambda(ID)) pacify(ID);
                    }
                };
                boss.SUBMIT_TV(tier::general, e2::timer::any, memo[ID], handler);
            }
            // pro::timer: Start countdown.
            template<class P>
            void actify(period timeout, P lambda)
            {
                actify(bell::noid, timeout, lambda);
            }
            // pro::timer: Cancel timer ('id=noid' for all).
            void pacify(id_t id = bell::noid)
            {
                if (id == bell::noid) memo.clear(); // Stop all timers.
                else                  memo.erase(id);
                //boss.SIGNAL(tier::release, e2::form::animate::stop, id);
            }
            // pro::timer: Check activity by id.
            bool active(id_t id)
            {
                return memo.contains(id);
            }
            // pro::timer: Check any activity.
            operator bool ()
            {
                return !memo.empty();
            }
        };

        // pro: Provides functionality for manipulating objects with a frame structure.
        class frame
            : public skill
        {
            using skill::boss,
                  skill::memo;

            subs  link;
            robot robo;
            si32  seat;

        public:
            frame(base&&) = delete;
            frame(base& boss, si32 z_order = Z_order::plain) : skill{ boss },
                robo{ boss    },
                seat{ z_order }
            {
                boss.SUBMIT_T(tier::release, e2::form::upon::vtree::attached, memo, parent)
                {
                    parent->SUBMIT_T(tier::preview, e2::form::global::lucidity, link, alpha)
                    {
                        boss.SIGNAL(tier::preview, e2::form::global::lucidity, alpha);
                    };
                    parent->SUBMIT_T(tier::preview, e2::form::layout::convey, link, convey_data)
                    {
                        convey(convey_data.delta, convey_data.stuff);
                    };
                    parent->SUBMIT_T(tier::preview, e2::form::layout::shift, link, delta)
                    {
                        //boss.base::coor += delta;
                        boss.moveby(delta);
                    };
                    parent->SUBMIT_T(tier::preview, e2::form::upon::vtree::detached, link, p)
                    {
                        frame::link.clear();
                    };
                    boss.SIGNAL(tier::release, e2::form::prop::zorder, seat);
                };
                boss.SUBMIT(tier::preview, e2::form::prop::zorder, order)
                {
                    seat = order;
                    boss.SIGNAL(tier::release, e2::form::prop::zorder, seat);
                };
                boss.SUBMIT_T(tier::preview, e2::form::layout::expose, memo, boss)
                {
                    expose();
                };
                boss.SUBMIT_T(tier::preview, hids::events::mouse::button::click::left, memo, gear)
                {
                    expose();
                };
                boss.SUBMIT_T(tier::preview, hids::events::mouse::button::click::right, memo, gear)
                {
                    expose();
                };
                boss.SUBMIT_T(tier::preview, e2::form::layout::appear, memo, newpos)
                {
                    appear(newpos);
                };
                //boss.SUBMIT_T(tier::preview, e2::form::upon::moved, memo, delta)
                //{
                //    bubble();
                //};
                boss.SUBMIT_T(tier::preview, e2::form::upon::changed, memo, delta)
                {
                    bubble();
                };
                boss.SUBMIT_T(tier::preview, hids::events::mouse::button::down::any, memo, gear)
                {
                    robo.pacify();
                };
                boss.SUBMIT_T(tier::release, e2::form::drag::pull::any, memo, gear)
                {
                    if (gear)
                    {
                        auto deed = boss.bell::template protos<tier::release>();
                        switch (deed)
                        {
                            case e2::form::drag::pull::left.id:
                            case e2::form::drag::pull::leftright.id:
                            {
                                auto delta = gear.delta.get();
                                boss.base::anchor = gear.coord; // See pro::align unbind.
                                boss.base::moveby(delta);
                                boss.SIGNAL(tier::preview, e2::form::upon::changed, delta);
                                gear.dismiss();
                                break;
                            }
                            default: break;
                        }
                    }
                };
                boss.SUBMIT_T(tier::release, e2::form::upon::dragged, memo, gear)
                {
                    if (gear.meta(hids::anyCtrl))
                    {
                        robo.actify(gear.fader<quadratic<twod>>(2s), [&](auto x)
                        {
                            boss.base::moveby(x);
                            boss.strike();
                        });
                    }
                    else
                    {
                        auto boundary = gear.area();
                        robo.actify(gear.fader<quadratic<twod>>(2s), [&, boundary](auto x)
                        {
                            convey(x, boundary);
                            boss.strike();
                        });
                    }
                };
                boss.SUBMIT_T(tier::release, hids::events::mouse::button::click::right, memo, gear)
                {
                    auto& area = boss.base::area();
                    auto coord = gear.coord + area.coor;
                    if (!area.hittest(coord))
                    {
                        appear(coord);
                    }
                    gear.dismiss();
                };
            };

            // pro::frame: Fly to the specified position.
            void appear(twod const& target)
            {
                auto& screen = boss.base::area();
                auto  oldpos = screen.coor;
                auto  newpos = target - screen.size / 2;;

                auto path = newpos - oldpos;
                auto time = SWITCHING_TIME;
                auto init = 0;
                auto func = constlinearAtoB<twod>(path, time, init);

                robo.pacify();
                robo.actify(func, [&](twod& x) { boss.base::moveby(x); boss.strike(); });
            }
            /*
            // pro::frame: Search for a non-overlapping form position in
            //             the visual tree along a specified direction.
            rect bounce(rect const& block, twod const& dir)
            {
                auto result = block.rotate(dir);
                auto parity = std::abs(dir.x) > std::abs(dir.y);

                for (auto xy : { parity, !parity })
                {
                    auto ray = result;
                    ray.coor[xy] += ray.size[xy];
                    ray.size[xy] = dir[xy] > 0 ? std::numeric_limits<int>::max()
                                               : std::numeric_limits<int>::min();

                    if (auto shadow = ray.trunc(boss.base::size))
                    {
                        auto direct = shadow.rotate(dir);
                        auto nearby = direct.coor[xy] + direct.size[xy];

                        foreach(boss.branch, boss.status.is.visible, [&](auto item)
                                {
                                    if (auto s = shadow.clip(item->square()))
                                    {
                                        auto next = dir[xy] > 0 ? s.coor[xy] : -(s.coor[xy] + s.size[xy]);
                                        if (next < nearby) nearby = next;
                                    }
                                });

                        result.size[xy] = (dir[xy] > 0 ? nearby : -nearby) - result.coor[xy];
                    }
                }

                return result;
            }
            */
            // pro::frame: Move the form no further than the parent canvas.
            void convey(twod const& delta, rect const& boundary)//, bool notify = true)
            {
                auto& r0 = boss.base::area();
                if (delta && r0.clip(boundary))
                {
                    auto r1 = r0;
                    auto r2 = boundary;
                    r1.coor -= r2.coor;

                    auto c = r1.rotate(-delta);
                    auto s = r2.size;
                    auto o = delta.less(dot_00, dot_00, dot_11);
                    if ((s + o).twod::inside(c.coor))
                    {
                        c.coor = std::clamp(c.coor + delta, dot_00, s);
                        auto newcoor = c.normalize().coor + r2.coor;
                        boss.moveto(newcoor);
                    }
                    else if (!r2.clip(r0))
                    {
                        boss.moveby(delta);
                    }
                }
            }
            // pro::frame: Check if it is under the rest, and moves it to the top of the visual tree.
            //             Return "true" if it is NOT under the rest.
            void expose(bool subsequent = faux)
            {
                if (auto parent_ptr = boss.parent())
                {
                    parent_ptr->SIGNAL(tier::release, e2::form::layout::expose, boss);
                }
                //return boss.status.exposed;
            }
            // pro::frame: Place the form in front of the visual tree among neighbors.
            void bubble()
            {
                if (auto parent_ptr = boss.parent())
                {
                    parent_ptr->SIGNAL(tier::release, e2::form::layout::bubble, boss);
                }
            }
        };

        // pro: Form generator.
        class maker
            : public skill
        {
            using skill::boss,
                  skill::memo;

            cell mark;

            struct slot_t
            {
                rect slot{};
                twod step{};
                twod init{};
                bool ctrl{};
            };
            std::unordered_map<id_t, slot_t> slots;
            ansi::esc coder;

            void check_modifiers(hids& gear)
            {
                auto& data = slots[gear.id];
                auto state = !!gear.meta(hids::anyCtrl);
                if (data.ctrl != state)
                {
                    data.ctrl = state;
                    boss.deface(data.slot);
                }
            }
            void handle_init(hids& gear)
            {
                if (gear.capture(boss.bell::id))
                {
                    auto& data = slots[gear.id];
                    auto& slot = data.slot;
                    auto& init = data.init;
                    auto& step = data.step;

                    data.ctrl = gear.meta(hids::anyCtrl);
                    slot.coor = init = step = gear.coord;
                    slot.size = dot_00;
                    boss.deface(slot);
                    gear.dismiss();
                }
            }
            void handle_pull(hids& gear)
            {
                if (gear.captured(boss.bell::id))
                {
                    check_modifiers(gear);
                    auto& data = slots[gear.id];
                    auto& slot = data.slot;
                    auto& init = data.init;
                    auto& step = data.step;

                    step += gear.delta.get();
                    slot.coor = std::min(init, step);
                    slot.size = std::max(std::abs(step - init), dot_00);
                    boss.deface(slot);
                    gear.dismiss();
                }
            }
            void handle_drop(hids& gear)
            {
                if (gear.captured(boss.bell::id))
                {
                    slots.erase(gear.id);
                    gear.dismiss();
                    gear.setfree();
                }
            }
            void handle_stop(hids& gear)
            {
                if (gear.captured(boss.bell::id))
                {
                    check_modifiers(gear);
                    auto& data = slots[gear.id];
                    if (data.slot)
                    {
                        gear.slot = data.slot;
                        gear.slot_forced = true;
                        boss.SIGNAL(tier::preview, e2::form::proceed::createby, gear);
                    }
                    slots.erase(gear.id);
                    gear.dismiss();
                    gear.setfree();
                }
            }

        public:
            maker(base&&) = delete;
            maker(base& boss)
                : skill{ boss },
                   mark{ skin::color(tone::selector) }
            {
                using drag = hids::events::mouse::button::drag;

                boss.SUBMIT_T(tier::preview, hids::events::keybd::any, memo, gear)
                {
                    if (gear.captured(boss.bell::id)) check_modifiers(gear);
                };

                //todo unify - args... + template?
                //middle button
                boss.SUBMIT_T(tier::preview, drag::start::middle, memo, gear)
                {
                    handle_init(gear);
                };
                boss.SUBMIT_T(tier::release, drag::pull::middle, memo, gear)
                {
                    handle_pull(gear);
                };
                boss.SUBMIT_T(tier::release, drag::cancel::middle, memo, gear)
                {
                    handle_drop(gear);
                };
                boss.SUBMIT_T(tier::release, drag::stop::middle, memo, gear)
                {
                    handle_stop(gear);
                };

                //todo unify
                //right button
                boss.SUBMIT_T(tier::release, drag::start::right, memo, gear)
                {
                    handle_init(gear);
                };
                boss.SUBMIT_T(tier::release, drag::pull::right, memo, gear)
                {
                    handle_pull(gear);
                };
                boss.SUBMIT_T(tier::release, drag::cancel::right, memo, gear)
                {
                    handle_drop(gear);
                };
                boss.SUBMIT_T(tier::release, drag::stop::right, memo, gear)
                {
                    handle_stop(gear);
                };

                boss.SUBMIT_T(tier::general, hids::events::halt, memo, gear)
                {
                    handle_drop(gear);
                };

                boss.SUBMIT_T(tier::release, e2::postrender, memo, canvas)
                {
                    //todo Highlighted area drawn twice
                    auto offset = boss.coor() - canvas.coor();
                    for (auto const& [key, data] : slots)
                    {
                        auto slot = data.slot;
                        slot.coor += offset;
                        if (auto area = canvas.area().clip<true>(slot))
                        {
                            if (data.ctrl)
                            {
                                area.coor -= dot_11;
                                area.size += dot_22;
                                auto mark = skin::color(tone::kb_focus);
                                auto fill = [&](cell& c) { c.fuse(mark); };
                                canvas.cage(area, dot_11, fill);
                                coder.wrp(wrap::off).add("capture area: ", slot);
                                //todo optimize para
                                auto caption = para(coder);
                                coder.clear();
                                auto header = *caption.lyric;
                                auto coor = area.coor + canvas.coor();
                                coor.y--;
                                header.move(coor);
                                canvas.fill(header, cell::shaders::contrast);
                            }
                            else
                            {
                                auto temp = canvas.view();
                                canvas.view(area);
                                canvas.fill(area, [&](cell& c) { c.fuse(mark); c.und(faux); });
                                canvas.blur(10);
                                coder.wrp(wrap::off).add(' ').add(slot.size.x).add(" × ").add(slot.size.y).add(' ');
                                //todo optimize para
                                auto caption = para(coder);
                                coder.clear();
                                auto header = *caption.lyric;
                                auto coor = area.coor + area.size + canvas.coor();
                                coor.x -= caption.length() - 1;
                                header.move(coor);
                                canvas.fill(header, cell::shaders::contrast);
                                canvas.view(temp);
                            }
                        }
                    }
                };
            }
        };

        // pro: The text caret controller.
        class caret
            : public skill
        {
            using skill::boss,
                  skill::memo;

            subs   conf; // caret: Configuration subscriptions.
            bool   live; // caret: Should the caret be drawn.
            bool   done; // caret: Is the caret already drawn.
            bool   down; // caret: Is the caret suppressed (lost focus).
            rect   body; // caret: Caret position.
            period step; // caret: Blink interval. period::zero() if steady.
            moment next; // caret: Time of next blinking.
            bool   form; // caret: Caret style: true - box; faux - underline.

        public:
            caret(base&&) = delete;
            caret(base& boss, bool visible = faux, twod position = dot_00, bool abox = faux) : skill{ boss },
                live{ faux },
                done{ faux },
                down{ faux },
                form{ abox },
                body{ position, dot_11 }, // Caret is always one cell size (see the term::scrollback definition).
                step{ BLINK_PERIOD }
            {
                boss.SUBMIT_T(tier::anycast, e2::form::highlight::any, conf, state)
                {
                    down = !state;
                };
                boss.SUBMIT_T(tier::request, e2::config::caret::blink, conf, req_step)
                {
                    req_step = step;
                };
                boss.SUBMIT_T(tier::request, e2::config::caret::style, conf, req_style)
                {
                    req_style = form ? 1 : 0;
                };
                boss.SUBMIT_T(tier::general, e2::config::caret::blink, conf, new_step)
                {
                    blink_period(new_step);
                };
                boss.SUBMIT_T(tier::preview, e2::config::caret::blink, conf, new_step)
                {
                    blink_period(new_step);
                };
                boss.SUBMIT_T(tier::general, e2::config::caret::style, conf, new_style)
                {
                    style(new_style);
                };
                boss.SUBMIT_T(tier::preview, e2::config::caret::style, conf, new_style)
                {
                    style(new_style);
                };
                if (visible) show();
            }

            operator bool () const { return memo.count(); }

            // pro::caret: Set caret style.
            void style(bool new_form)
            {
                if (form != new_form)
                {
                    hide();
                    form = new_form;
                    show();
                }
            }
            // pro::caret: Set blink period.
            void blink_period(period const& new_step = BLINK_PERIOD)
            {
                auto changed = (step == period::zero()) != (new_step == period::zero());
                step = new_step;
                if (changed)
                {
                    hide();
                    show();
                }
            }
            void style(si32 mode)
            {
                switch (mode)
                {
                    case 0: // n = 0  blinking box
                    case 1: // n = 1  blinking box (default)
                        blink_period();
                        style(true);
                        break;
                    case 2: // n = 2  steady box
                        blink_period(period::zero());
                        style(true);
                        break;
                    case 3: // n = 3  blinking underline
                        blink_period();
                        style(faux);
                        break;
                    case 4: // n = 4  steady underline
                        blink_period(period::zero());
                        style(faux);
                        break;
                    case 5: // n = 5  blinking I-bar
                        blink_period();
                        style(true);
                        break;
                    case 6: // n = 6  steady I-bar
                        blink_period(period::zero());
                        style(true);
                        break;
                    default:
                        log("pro::caret: unsupported cursor style requested, ", mode);
                        break;
                }
            }
            void toggle()
            {
                style(!form);
                reset();
            }
            // pro::caret: Set caret position.
            void coor(twod const& coor)
            {
                if (body.coor != coor)
                {
                    reset();
                    body.coor = coor;
                }
            }
            // pro::caret: Get caret position.
            auto& coor() const
            {
                return body.coor;
            }
            // pro::caret: Get caret style.
            auto style() const
            {
                return std::pair{ form, !!(*this) };
            }
            // pro::caret: Force to redraw caret.
            void reset()
            {
                if (step != period::zero())
                {
                    live = faux;
                    next = {};
                }
            }
            // pro::caret: Enable caret.
            void show()
            {
                if (!*this)
                {
                    done = faux;
                    live = step == period::zero();
                    if (!live)
                    {
                        boss.SUBMIT_T(tier::general, e2::timer::tick, memo, timestamp)
                        {
                            if (timestamp > next)
                            {
                                next = timestamp + step;
                                live = !live;
                                boss.deface(body);
                            }
                        };
                    }
                    boss.SUBMIT_T(tier::release, e2::postrender, memo, canvas)
                    {
                        done = live;
                        auto state = down ? (step == period::zero() ? faux : true)
                                          : live;
                        if (state)
                        {
                            auto field = canvas.core::view();
                            auto point = body;
                            point.coor += field.coor + boss.base::coor();
                            if (auto area = field.clip(point))
                            {
                                auto& test = canvas.peek(point.coor);
                                if (test.wdt() == 2) // Extend cursor to adjacent halves.
                                {
                                    if (field.hittest(point.coor + dot_10))
                                    {
                                        auto& next = canvas.peek(point.coor + dot_10);
                                        if (next.wdt() == 3 && test.same_txt(next))
                                        {
                                            area.size.x++;
                                        }
                                    }
                                }
                                else if (test.wdt() == 3)
                                {
                                    if (field.hittest(point.coor - dot_10))
                                    {
                                        auto& prev = canvas.peek(point.coor - dot_10);
                                        if (prev.wdt() == 2 && test.same_txt(prev))
                                        {
                                            area.size.x++;
                                            area.coor.x--;
                                        }
                                    }
                                }

                                if (form)
                                {
                                    canvas.fill(area, [](cell& c)
                                    {
                                        auto b = c.bgc();
                                        auto f = c.fgc();
                                        if (c.inv()) c.bgc(f).fgc(cell::shaders::contrast.invert(f));
                                        else         c.fgc(b).bgc(cell::shaders::contrast.invert(b));
                                    });
                                }
                                else canvas.fill(area, [](cell& c) { c.und() ? c.und(0) : c.und(1); });
                            }
                            else if (area.size.y)
                            {
                                auto chr = area.coor.x ? '>' : '<';
                                area.coor.x -= area.coor.x ? 1 : 0;
                                area.size.x = 1;
                                canvas.fill(area, [&](auto& c){ c.txt(chr).fgc(cell::shaders::contrast.invert(c.bgc())); });
                            }
                        }
                    };
                }
            }
            // pro::caret: Disable caret.
            void hide()
            {
                if (*this)
                {
                    memo.clear();
                    if (done)
                    {
                        boss.deface(body);
                        done = faux;
                    }
                }
            }
        };

        // pro: Textify the telemetry data for debugging purpose.
        class debug
            : public skill
        {
            using skill::boss,
                  skill::memo;

            #define PROP_LIST                     \
            X(total_size   , "total sent"       ) \
            X(proceed_ns   , "rendering time"   ) \
            X(render_ns    , "stdout time"      ) \
            X(frame_size   , "frame size"       ) \
            X(frame_rate   , "frame rate"       ) \
            X(focused      , "focus"            ) \
            X(win_size     , "win size"         ) \
            X(key_code     , "key virt"         ) \
            X(key_scancode , "key scan"         ) \
            X(key_character, "key char"         ) \
            X(key_pressed  , "key push"         ) \
            X(ctrl_state   , "controls"         ) \
            X(mouse_pos    , "mouse coord"      ) \
            X(mouse_wheeldt, "wheel delta"      ) \
            X(mouse_hzwheel, "H wheel"          ) \
            X(mouse_vtwheel, "V wheel"          ) \
            X(mouse_btn_1  , "left button"      ) \
            X(mouse_btn_2  , "right button"     ) \
            X(mouse_btn_3  , "left+right combo" ) \
            X(mouse_btn_4  , "middle button"    ) \
            X(mouse_btn_5  , "4th button"       ) \
            X(mouse_btn_6  , "5th button"       ) \
            X(last_event   , "event"            )

            //X(key_repeat   , "key copy"         )
            //X(menu_id      , "menu id"          )

            #define X(a, b) a,
            enum prop { PROP_LIST count };
            #undef X

            #define X(a, b) b,
            text description[prop::count] = { PROP_LIST };
            #undef X
            #undef PROP_LIST

            cell alerts;
            cell stress;
            page status;
            ansi::esc coder;

            struct
            {
                period render = period::zero();
                period output = period::zero();
                si32   frsize = 0;
                si64   totals = 0;
                si32   number = 0;    // info: Current frame number
                //bool   onhold = faux; // info: Indicator that the current frame has been successfully STDOUT
            }
            track; // debug: Textify the telemetry data for debugging purpose.

            void shadow()
            {
                for (auto i = 0; i < prop::count; i++)
                {
                    status[i].ease();
                }
            }

        public:
            bool bypass = faux;

            debug(base&&) = delete;
            debug(base& boss) : skill{ boss }
            { }

            operator bool () const { return memo.count(); }

            void update(bool focus_state)
            {
                shadow();
                status[prop::last_event].set(stress) = "focus";
                status[prop::focused].set(stress) = focus_state ? "active" : "lost";
            }
            void update(twod const& new_size)
            {
                shadow();
                status[prop::last_event].set(stress) = "size";

                status[prop::win_size].set(stress) =
                    std::to_string(new_size.x) + " x " +
                    std::to_string(new_size.y);
            }
            void update(period const& watch, si32 delta)
            {
                track.output = watch;
                track.frsize = delta;
                track.totals+= delta;
            }
            void update(moment const& timestamp)
            {
                track.render = tempus::now() - timestamp;
            }
            void output(face& canvas)
            {
                status[prop::render_ns].set(track.output > 12ms ? alerts : stress) =
                    utf::adjust(utf::format(track.output.count()), 11, " ", true) + "ns";

                status[prop::proceed_ns].set(track.render > 12ms ? alerts : stress) =
                    utf::adjust(utf::format (track.render.count()), 11, " ", true) + "ns";

                status[prop::frame_size].set(stress) =
                    utf::adjust(utf::format(track.frsize), 7, " ", true) + " bytes";

                status[prop::total_size].set(stress) =
                    utf::format(track.totals) + " bytes";

                track.number++;
                canvas.output(status);
            }
            void stop()
            {
                track = {};
                memo.clear();
            }
            void start()
            {
                //todo use skin
                stress = cell{}.fgc(whitelt);
                alerts = cell{}.fgc(rgba{ 0xFFd0d0FFu });

                status.style.wrp(wrap::on).jet(bias::left).rlf(feed::rev).mgl(4);
                status.current().locus.cup(dot_00).cnl(2);

                auto maxlen = 0_sz;
                for (auto& desc : description)
                {
                    maxlen = std::max(maxlen, desc.size());
                }
                auto attr = si32{ 0 };
                for (auto& desc : description)
                {
                    status += coder.add(" ", utf::adjust(desc, maxlen, " ", true), " ").idx(attr++).nop().nil().eol();
                    coder.clear();
                }

                boss.SUBMIT_T(tier::general, e2::config::fps, memo, fps)
                {
                    status[prop::frame_rate].set(stress) = std::to_string(fps);
                    boss.base::strike(); // to update debug info
                };
                {
                    auto fps = e2::config::fps.param(-1);
                    boss.SIGNAL(tier::general, e2::config::fps, fps);
                }
                boss.SUBMIT_T(tier::release, e2::conio::focus, memo, focusstate)
                {
                    update(focusstate.enabled);
                    boss.base::strike(); // to update debug info
                };
                boss.SUBMIT_T(tier::release, e2::size::any, memo, newsize)
                {
                    update(newsize);
                };

                boss.SUBMIT_T(tier::preview, hids::events::mouse::any, memo, gear)
                {
                    if (bypass) return;
                    shadow();
                    auto& m = gear;
                    status[prop::last_event].set(stress) = "mouse";
                    status[prop::mouse_pos ].set(stress) =
                        (m.coord.x < 10000 ? std::to_string(m.coord.x) : "-") + " : " +
                        (m.coord.y < 10000 ? std::to_string(m.coord.y) : "-") ;

                    for (auto btn = 0; btn < sysmouse::numofbutton; btn++)
                    {
                        auto& state = status[prop::mouse_btn_1 + btn].set(stress);

                        state = m.buttons[btn].pressed ? "pressed" : "";
                        if (m.buttons[btn].flipped)
                        {
                            state += state.length() ? " | flipped" : "flipped";
                        }

                        if (m.buttons[btn].dragged)
                        {
                            state += state.length() ? " | dragged" : "dragged";
                        }

                        state += state.length() ? "" : "idle";
                    }
                    status[prop::mouse_wheeldt].set(stress) = std::to_string(m.whldt);
                    status[prop::mouse_hzwheel].set(stress) = m.hzwhl ? "active" : "idle";
                    status[prop::mouse_vtwheel].set(stress) = m.scrll ? "active" : "idle";
                    status[prop::ctrl_state   ].set(stress) = "0x" + utf::to_hex(m.meta());
                };

                //boss.SUBMIT_T(tier::release, e2::conio::menu, memo, iface)
                //{
                //	//shadow();
                //	status[prop::last_event].set(stress) = "UI";
                //	status[prop::menu_id].set(stress) = "UI:" + std::to_string(iface);
                //};

                boss.SUBMIT_T(tier::release, e2::conio::keybd, memo, gear)
                {
                    shadow();
                    auto& k = gear;

                    #if defined(KEYLOG)
                        log("debug fired ", utf::debase(k.cluster));
                    #endif

                    status[prop::last_event   ].set(stress) = "keybd";
                    status[prop::key_pressed  ].set(stress) = k.pressed ? "pressed" : "idle";
                    status[prop::ctrl_state   ].set(stress) = "0x" + utf::to_hex(k.ctlstat );
                    status[prop::key_code     ].set(stress) = "0x" + utf::to_hex(k.virtcod );
                    status[prop::key_scancode ].set(stress) = "0x" + utf::to_hex(k.scancod );
                    //status[prop::key_repeat   ].set(stress) = std::to_string(k.imitate);

                    if (k.cluster.length())
                    {
                        auto t = k.cluster;
                        for (auto i = 0; i < 0x20; i++)
                        {
                            utf::change(t, text{ (char)i }, "^" + utf::to_utf_from_code(i + 0x40));
                        }
                        utf::change(t, text{ (char)0x7f }, "\\x7F");
                        utf::change(t, text{ (char)0x20 }, "\\x20");
                        status[prop::key_character].set(stress) = t;
                    }
                };

                //boss.SUBMIT_T(tier::release, e2::conio::focus, owner::memo, f)
                //{
                //	shadow();
                //	status[prop::last_event].set(stress) = "focus";
                //	status[prop::focused].set(stress) = f ? "active" : "lost";
                //});

                boss.SUBMIT_T(tier::release, e2::conio::error, memo, e)
                {
                    shadow();
                    status[prop::last_event].set(stress) = "error";
                    throw;
                };
            }
        };

        // pro: Provides functionality for the title support.
        class title
            : public skill
        {
            using skill::boss,
                  skill::memo;

            page head_page; // title: Owner's caption header.
            page foot_page; // title: Owner's caption footer.
            text head_text; // title: Preserve original header.
            text foot_text; // title: Preserve original footer.
            twod head_size; // title: Header page size.
            twod foot_size; // title: Footer page size.
            bool head_live; // title: Handle header events.
            bool foot_live; // title: Handle footer events.
            flow ooooooooo; // title: .

        public:
            bool live = true; // title: Title visibility.

            auto& titles() const
            {
                return head_page;
            }
            void recalc(page& object, twod& size)
            {
                ooooooooo.flow::reset();
                ooooooooo.flow::size(size);
                auto publish = [&](auto const& combo)
                {
                    auto cp = ooooooooo.flow::print(combo);
                };
                object.stream(publish);
                auto& cover = ooooooooo.flow::minmax();
                size.y = cover.height() + 1;
            }
            void recalc(twod const& new_size)
            {
                head_size = new_size;
                foot_size = new_size;
                if (head_live) recalc(head_page, head_size);
                if (foot_live) recalc(foot_page, foot_size);
            }
            auto& header() { return head_text; }
            auto& footer() { return foot_text; }
            void header(view newtext)
            {
                head_page = newtext;
                head_text = newtext;
                recalc(head_page, head_size);
                boss.SIGNAL(tier::release, e2::form::prop::ui::header, head_text);
                /*
                textline.link(boss.id);
                boss.SIGNAL(tier::release, e2::form::prop::ui::header, head_text);
                boss.SIGNAL(tier::release, e2::form::state::header, textline);
                */
            }
            void footer(view newtext)
            {
                foot_page = newtext;
                foot_text = newtext;
                recalc(foot_page, foot_size);
                boss.SIGNAL(tier::release, e2::form::prop::ui::footer, foot_text);
                /*
                textline.link(boss.id);
                boss.SIGNAL(tier::release, e2::form::prop::ui::footer, foot_text);
                boss.SIGNAL(tier::release, e2::form::state::footer, textline);
                */
            }
            void init()
            {
                boss.SUBMIT_T(tier::release, e2::size::any, memo, new_size)
                {
                    recalc(new_size);
                };
                boss.SUBMIT_T(tier::release, e2::postrender, memo, canvas)
                {
                    if (live)
                    {
                        auto saved_context = canvas.bump(dent{ 0,0,head_size.y,foot_size.y });
                        if (head_live)
                        {
                            canvas.cup(dot_00);
                            canvas.output(head_page, cell::shaders::contrast);
                        }
                        if (foot_live)
                        {
                            canvas.cup({ 0, head_size.y + boss.size().y });
                            canvas.output(foot_page, cell::shaders::contrast);
                        }
                        canvas.bump(saved_context);
                    }
                };
                if (head_live)
                {
                    boss.SUBMIT_T(tier::preview, e2::form::prop::ui::header, memo, newtext)
                    {
                        header(newtext);
                    };
                    boss.SUBMIT_T(tier::request, e2::form::prop::ui::header, memo, curtext)
                    {
                        curtext = head_text;
                    };
                }
                if (foot_live)
                {
                    boss.SUBMIT_T(tier::preview, e2::form::prop::ui::footer, memo, newtext)
                    {
                        footer(newtext);
                    };
                    boss.SUBMIT_T(tier::request, e2::form::prop::ui::footer, memo, curtext)
                    {
                        curtext = foot_text;
                    };
                }
                /*
                boss.SUBMIT_T(tier::request, e2::form::state::header, memo, caption)
                {
                    caption = header();
                };
                boss.SUBMIT_T(tier::request, e2::form::state::footer, memo, caption)
                {
                    caption = footer();
                };
                */
            }

            title(base&&) = delete;
            title(base& boss)
                : skill{ boss },
                  head_live{ true },
                  foot_live{ true }
            {
                init();
            }
            title(base& boss, view title, view foots = {}, bool visible = true,
                                                           bool on_header = true,
                                                           bool on_footer = true)
                : skill{ boss },
                  head_live{ on_header },
                  foot_live{ on_footer }
            {
                init();
                header(title);
                footer(foots);
                live = visible;
                //footer(ansi::jet(bias::right) + "test\nmultiline\nfooter");
            }
        };

        // pro: Perform graceful shutdown functionality. LIMIT in seconds, ESC_THRESHOLD in milliseconds.
        class guard
            : public skill
        {
            using skill::boss,
                  skill::memo;

            constexpr static auto QUIT_MSG = e2::conio::quit;
            constexpr static si32 ESC_THRESHOLD = 500; // guard: Double escape threshold in ms.

            bool   wait; // guard: Ready to close.
            moment stop; // guard: Timeout for single Esc.
            text   desc = "exit after preclose";

        public:
            guard(base&&) = delete;
            guard(base& boss) : skill{ boss },
                wait{ faux }
            {
                // Suspected early completion.
                boss.SUBMIT_T(tier::release, e2::conio::preclose, memo, pre_close)
                {
                    if ((wait = pre_close))
                    {
                        stop = tempus::now() + std::chrono::milliseconds(ESC_THRESHOLD);
                    }
                };

                // Double escape catcher.
                boss.SUBMIT_T(tier::general, e2::timer::any, memo, timestamp)
                {
                    if (wait && (timestamp > stop))
                    {
                        wait = faux;
                        auto shadow = boss.This();
                        boss.SIGNAL(tier::release, QUIT_MSG, desc);
                        memo.clear();
                    }
                };
            }
        };

        // pro: Perform graceful shutdown functionality. LIMIT in seconds, ESC_THRESHOLD in milliseconds.
        class watch
            : public skill
        {
            using skill::boss,
                  skill::memo;

            constexpr static auto EXCUSE_MSG = hids::events::mouse::any;
            constexpr static auto QUIT_MSG   = e2::shutdown;
            //todo unify
            constexpr static int LIMIT = 60 * 10; // watch: Idle timeout in seconds.

            hook   pong; // watch: Alibi subsciption token.
            hook   ping; // watch: Zombie check countdown token.
            moment stop; // watch: Timeout for zombies.
            text   desc = "no mouse clicking events";

        public:
            watch(base&&) = delete;
            watch(base& boss) : skill{ boss }
            {
                stop = tempus::now() + std::chrono::seconds(LIMIT);

                // No mouse events watchdog.
                boss.SUBMIT_T(tier::preview, EXCUSE_MSG, pong, something)
                {
                    stop = tempus::now() + std::chrono::seconds(LIMIT);
                };
                boss.SUBMIT_T(tier::general, e2::timer::any, ping, something)
                {
                    if (tempus::now() > stop)
                    {
                        auto shadow = boss.This();
                        boss.SIGNAL(tier::general, QUIT_MSG, desc);
                        ping.reset();
                        memo.clear();
                    }
                };
            }
        };

        // pro: Provides functionality related to keyboard input.
        class keybd
            : public skill
        {
            using skill::boss,
                  skill::memo;

            subs kb_subs{};

        public:
            keybd(base&&) = delete;
            keybd(base& boss) : skill{ boss }
            {
                // pro::keybd: Notify form::state::kbfocus when the number of clients is positive.
                boss.SUBMIT_T(tier::release, hids::events::notify::keybd::got, memo, gear)
                {
                    boss.SIGNAL(tier::release, e2::form::state::keybd::got, gear);
                };
                // pro::keybd: Notify form::state::active_kbd when the number of clients is zero.
                boss.SUBMIT_T(tier::release, hids::events::notify::keybd::lost, memo, gear)
                {
                    boss.SIGNAL(tier::release, e2::form::state::keybd::lost, gear);
                };
                boss.SUBMIT_T(tier::preview, hids::events::keybd::any, memo, gear)
                {
                    #if defined(KEYLOG)
                        log("keybd fired virtcode: ", gear.virtcod,
                                          " chars: ", utf::debase(gear.cluster),
                                           " meta: ", gear.meta());
                    #endif

                    boss.SIGNAL(tier::release, hids::events::keybd::any, gear);
                };
            };

            // pro::keybd: Keybd offers promoter.
            void active()
            {
                boss.SUBMIT_T(tier::release, hids::events::mouse::button::any, kb_subs, gear)
                {
                    if (!gear) return;
                    auto deed = boss.bell::protos<tier::release>();
                    if (deed == hids::events::mouse::button::click::left.id) //todo make it configurable (left click)
                    {
                        // Propagate throughout nested objects by base::
                        gear.kb_focus_changed = faux;
                        boss.SIGNAL(tier::release, hids::events::upevent::kboffer, gear);
                        gear.dismiss();
                    }
                    else if (deed == hids::events::mouse::button::click::right.id) //todo make it configurable (left click)
                    {
                        // Propagate throughout nested objects by base::
                        auto state = gear.state();
                        gear.kb_focus_changed = faux;
                        gear.force_group_focus = true;
                        gear.combine_focus = faux;
                        boss.SIGNAL(tier::release, hids::events::upevent::kboffer, gear);
                        gear.state(state);
                        gear.dismiss();
                    }
                };
            }
            // pro::keybd: Subscribe on keybd offers.
            void accept(bool value)
            {
                if (value)
                {
                    active();
                    boss.SUBMIT_T(tier::release, hids::events::upevent::kboffer, kb_subs, gear)
                    {
                        if (!gear.focus_changed())
                        {
                            gear.set_kb_focus(boss.This());
                            boss.bell::expire<tier::release>();
                        }
                    };
                    boss.SUBMIT_T(tier::release, hids::events::upevent::kbannul, kb_subs, gear)
                    {
                        gear.remove_from_kb_focus(boss.This());
                    };
                }
                else
                {
                    kb_subs.clear();
                }
            }
        };

        // pro: Provides functionality related to mouse interaction.
        class mouse
            : public skill
        {
            using skill::boss,
                  skill::memo;

            sptr<base> soul; // mouse: Boss cannot be removed while it has active gears.
            si32       rent; // mouse: Active gears count.
            si32       full; // mouse: All gears count. Counting to keep the entire chain of links in the visual tree.
            bool       omni; // mouse: Ability to accept all hover events (true) or only directly over the object (faux).
            si32       drag; // mouse: Bitfield of buttons subscribed to mouse drag.
            std::map<si32, subs> dragmemo; // mouse: Draggable subs.

        public:
            mouse(base&&) = delete;
            mouse(base& boss, bool take_all_events = true)
                : skill{ boss            },
                   omni{ take_all_events },
                   rent{ 0               },
                   full{ 0               },
                   drag{ 0               }
            {
                auto brush = boss.base::color();
                boss.base::color(brush.link(boss.bell::id));
                // pro::mouse: Forward preview to all parents.
                boss.SUBMIT_T(tier::preview, hids::events::mouse::any, memo, gear)
                {
                    auto& offset = boss.base::coor();
                    gear.pass<tier::preview>(boss.parent(), offset);

                    if (gear) gear.okay(boss);
                    else      boss.bell::expire<tier::preview>();
                };
                // pro::mouse: Forward all not expired mouse events to all parents.
                boss.SUBMIT_T(tier::release, hids::events::mouse::any, memo, gear)
                {
                    if (gear && !gear.locks)
                    {
                        auto& offset = boss.base::coor();
                        gear.pass<tier::release>(boss.parent(), offset);
                    }
                };
                // pro::mouse: Notify form::state::active when the number of clients is positive.
                boss.SUBMIT_T(tier::release, hids::events::notify::mouse::enter, memo, gear)
                {
                    if (!full++)
                    {
                        soul = boss.This();
                    }
                    if (gear.direct<true>(boss.bell::id) || omni)
                    {
                        if (!rent++)
                        {
                            boss.SIGNAL(tier::release, e2::form::state::mouse, rent);
                        }
                    }
                };
                // pro::mouse: Notify form::state::active when the number of clients is zero.
                boss.SUBMIT_T(tier::release, hids::events::notify::mouse::leave, memo, gear)
                {
                    if (!--full)
                    {
                        soul->base::strike();
                        soul.reset();
                    }
                    if (gear.direct<faux>(boss.bell::id) || omni)
                    {
                        if (!--rent)
                        {
                            boss.SIGNAL(tier::release, e2::form::state::mouse, rent);
                        }
                    }
                };
                boss.SUBMIT_T(tier::request, e2::form::state::mouse, memo, state)
                {
                    state = rent;
                };
                boss.SUBMIT_T(tier::release, e2::form::draggable::any, memo, enabled)
                {
                    switch (auto deed = boss.bell::protos<tier::release>())
                    {
                        default:
                        case e2::form::draggable::left     .id: draggable<sysmouse::left     >(enabled); break;
                        case e2::form::draggable::right    .id: draggable<sysmouse::right    >(enabled); break;
                        case e2::form::draggable::leftright.id: draggable<sysmouse::leftright>(enabled); break;
                        case e2::form::draggable::middle   .id: draggable<sysmouse::middle   >(enabled); break;
                        case e2::form::draggable::wheel    .id: draggable<sysmouse::wheel    >(enabled); break;
                        case e2::form::draggable::win      .id: draggable<sysmouse::win      >(enabled); break;
                    }
                };
            }
            void reset()
            {
                auto lock = events::sync{};
                if (full)
                {
                    full = 0;
                    soul.reset();
                }
            }
            void take_all_events(bool b)
            {
                omni = b;
            }
            template<sysmouse::bttns BUTTON>
            void draggable(bool enabled)
            {
                if (!enabled)
                {
                    dragmemo[BUTTON].clear();
                    drag &= ~(1 << BUTTON);
                }
                else if (!(drag & 1 << BUTTON))
                {
                    drag |= 1 << BUTTON;
                    //using bttn = hids::events::mouse::button; //MSVC 16.9.4 don't get it
                    boss.SUBMIT_T(tier::release, hids::events::mouse::button::drag::start::_<BUTTON>, dragmemo[BUTTON], gear)
                    {
                        if (gear.capture(boss.bell::id))
                        {
                            boss.SIGNAL(tier::release, e2::form::drag::start::_<BUTTON>, gear);
                            gear.dismiss();
                        }
                    };
                    boss.SUBMIT_T(tier::release, hids::events::mouse::button::drag::pull::_<BUTTON>, dragmemo[BUTTON], gear)
                    {
                        if (gear.captured(boss.bell::id))
                        {
                            boss.SIGNAL(tier::release, e2::form::drag::pull::_<BUTTON>, gear);
                            gear.dismiss();
                        }
                    };
                    boss.SUBMIT_T(tier::release, hids::events::mouse::button::drag::cancel::_<BUTTON>, dragmemo[BUTTON], gear)
                    {
                        if (gear.captured(boss.bell::id))
                        {
                            boss.SIGNAL(tier::release, e2::form::drag::cancel::_<BUTTON>, gear);
                            gear.setfree();
                            gear.dismiss();
                        }
                    };
                    boss.SUBMIT_T(tier::general, hids::events::halt, dragmemo[BUTTON], gear)
                    {
                        if (gear.captured(boss.bell::id))
                        {
                            boss.SIGNAL(tier::release, e2::form::drag::cancel::_<BUTTON>, gear);
                            gear.setfree();
                            gear.dismiss();
                        }
                    };
                    boss.SUBMIT_T(tier::release, hids::events::mouse::button::drag::stop::_<BUTTON>, dragmemo[BUTTON], gear)
                    {
                        if (gear.captured(boss.bell::id))
                        {
                            boss.SIGNAL(tier::release, e2::form::drag::stop::_<BUTTON>, gear);
                            gear.setfree();
                            gear.dismiss();
                        }
                    };
                }
            }
        };

        // pro: Provides functionality related to keyboard interaction.
        class input
            : public skill
        {
            struct topgear
                : public hids
            {
                clip clip_rawdata{}; // topgear: Clipboard data.
                face clip_preview{}; // topgear: Clipboard preview render.
                twod preview_size{}; // topgear: Clipboard preview render size.
                bool not_directvt{}; // topgear: Is it the top level gear (not directvt).

                template<class ...Args>
                topgear(bool not_directvt, Args&&... args)
                    : hids{ std::forward<Args>(args)... },
                      not_directvt{ not_directvt }
                { }

                bool clear_clip_data() override
                {
                    auto not_empty = !!clip_rawdata.utf8.size();
                    preview_size = dot_00;
                    clip_rawdata.clear();
                    owner.SIGNAL(tier::release, hids::events::clipbrd::set, *this);
                    if (not_directvt)
                    {
                        clip_preview.size(preview_size);
                    }
                    return not_empty;
                }
                void set_clip_data(twod const& size, clip const& data) override
                {
                    if (data.utf8.size())
                    {
                        preview_size = size != dot_00 ? size
                                                      : preview_size == dot_00 ? twod{ 80,25 } //todo make it configurable
                                                                               : preview_size;
                    }
                    else preview_size = dot_00;
                    clip_rawdata = data;
                    if (not_directvt)
                    {
                        auto block = page{ data.utf8 };
                        clip_preview.mark(cell{});
                        clip_preview.size(preview_size);
                        clip_preview.wipe();
                        clip_preview.output(block, cell::shaders::xlucent(0x1f)); //todo make transparency configurable
                    }
                    owner.SIGNAL(tier::release, hids::events::clipbrd::set, *this);
                }
                clip get_clip_data() override
                {
                    auto data = clip{};
                    owner.SIGNAL(tier::release, hids::events::clipbrd::get, *this);
                    if (not_directvt) data.utf8 = clip_rawdata.utf8;
                    else              data.utf8 = std::move(clip_rawdata.utf8);
                    data.kind = clip_rawdata.kind;
                    return data;
                }
            };

            using depo = std::unordered_map<id_t, sptr<topgear>>;
            using lock = std::recursive_mutex;
            using skill::boss,
                  skill::memo;

            bool simple_instance;
            bool standalone_instance;

        public:
            face xmap;
            lock sync;
            depo gears;

            input(base&&) = delete;
            input(base& boss)
                : skill{ boss },
                  simple_instance{ faux },
                  standalone_instance{ faux }
            {
                xmap.link(boss.bell::id);
                xmap.move(boss.base::coor());
                xmap.size(boss.base::size());
                boss.SUBMIT_T(tier::release, e2::command::printscreen, memo, gear)
                {
                    auto data = ansi::esc{};
                    data.s11n(xmap, gear.slot);
                    if (data.length())
                    {
                        gear.set_clip_data(gear.slot.size, clip{ data, clip::ansitext });
                    }
                };
                boss.SUBMIT_T(tier::release, e2::form::prop::brush, memo, brush)
                {
                    xmap.mark(brush);
                };
                boss.SUBMIT_T(tier::release, e2::size::any, memo, newsize)
                {
                    auto guard = std::lock_guard{ sync }; // Syncing with diff::render thread.
                    xmap.size(newsize);
                };
                boss.SUBMIT_T(tier::release, e2::coor::any, memo, newcoor)
                {
                    xmap.move(newcoor);
                };
                boss.SUBMIT_T(tier::release, e2::conio::mouse, memo, mousestate)
                {
                    auto id = mousestate.mouseid;
                    auto gear_it = gears.find(id);
                    if (mousestate.control != sysmouse::stat::ok)
                    {
                        if (gear_it != gears.end())
                        {
                            switch (mousestate.control)
                            {
                                case sysmouse::stat::ok:
                                    break;
                                case sysmouse::stat::halt:
                                    gear_it->second->deactivate();
                                    break;
                                case sysmouse::stat::die:
                                    gears.erase(gear_it);
                                    break;
                            }
                        }
                    }
                    else if (gear_it == gears.end())
                    {
                        gear_it = gears.try_emplace(id, bell::create<topgear>(id == 0, boss, xmap)).first;
                        auto& [_id, gear_ptr] = *gear_it;
                        auto& gear = *gear_ptr;
                        gear.set_simple_instance(simple_instance);
                        gear.hids::take(mousestate);
                    }
                    else
                    {
                        auto& [_id, gear_ptr] = *gear_it;
                        auto& gear = *gear_ptr;
                        gear.hids::take(mousestate);
                    }
                    boss.strike();
                };
                boss.SUBMIT_T(tier::release, e2::conio::keybd, memo, keybdstate)
                {
                    auto id = keybdstate.keybdid;
                    auto gear_it = gears.find(id);
                    if (gear_it == gears.end())
                    {
                        gear_it = gears.try_emplace(id, bell::create<topgear>(id == 0, boss, xmap)).first;
                        auto& [_id, gear_ptr] = *gear_it;
                        auto& gear = *gear_ptr;
                        gear.set_simple_instance(simple_instance);
                        gear.hids::take(keybdstate);
                    }
                    else
                    {
                        auto& [_id, gear_ptr] = *gear_it;
                        auto& gear = *gear_ptr;
                        gear.hids::take(keybdstate);
                    }
                    boss.strike();
                };
                boss.SUBMIT_T(tier::release, e2::conio::focus, memo, focusstate)
                {
                    auto id = focusstate.focusid;
                    auto gear_it = gears.find(id);
                    if (gear_it == gears.end())
                    {
                        gear_it = gears.try_emplace(id, bell::create<topgear>(id == 0, boss, xmap)).first;
                        auto& [_id, gear_ptr] = *gear_it;
                        auto& gear = *gear_ptr;
                        gear.set_simple_instance(simple_instance);
                        gear.hids::take(focusstate);
                    }
                    else
                    {
                        auto& [_id, gear_ptr] = *gear_it;
                        auto& gear = *gear_ptr;
                        gear.hids::take(focusstate);
                    }
                    boss.strike();
                };
            }
            void check_focus()
            {
                if (simple_instance)
                {
                    auto focusstate = sysfocus{ .focusid = 0, .enabled = true };
                    boss.SIGNAL(tier::release, e2::conio::focus, focusstate);
                    gears[focusstate.focusid]->set_simple_instance(true);
                }
            }
            void set_instance_type(bool simple, bool standalone)
            {
                simple_instance = simple;
                standalone_instance = standalone;
                for (auto& [id, gear_ptr] : gears)
                {
                    gear_ptr->set_simple_instance(simple);
                }
            }
            auto is_not_standalone_instance()
            {
                return !standalone_instance;
            }
            void fire(events::id_t event_id)
            {
                for (auto& [id, gear_ptr] : gears)
                {
                    gear_ptr->fire(event_id);
                }
            }
            auto get_foreign_gear_id(id_t gear_id)
            {
                for (auto& [foreign_id, gear_ptr] : gears)
                {
                    if (gear_ptr->id == gear_id) return std::pair{ foreign_id, gear_ptr };
                }
                return std::pair{ id_t{}, sptr<topgear>{} };
            }
        };

        // pro: Glow gradient filter.
        class grade
            : public skill
        {
            using skill::boss,
                  skill::memo;

        public:
            grade(base&&) = delete;
            grade(base& boss) : skill{ boss }
            {
                boss.SUBMIT_T(tier::release, e2::postrender, memo, parent_canvas)
                {
                    auto size = si32{ 5 }; // grade: Vertical gradient size.
                    auto step = si32{ 2 }; // grade: Vertical gradient step.
                    //cell shadow{ cell{}.vis(cell::highlighter) };
                    //cell bright{ cell{}.vis(cell::darklighter) };
                    auto shadow = rgba{0xFF000000};
                    auto bright = rgba{0xFFffffff};

                    //todo optimize - don't fill the head and foot twice
                    auto area = parent_canvas.view();
                    auto n = std::clamp(size, 0, area.size.y / 2) + 1;
                    //auto n = std::clamp(size, 0, boss.base::size().y / 2) + 1;

                    auto head = area;
                    head.size.y = 1;
                    auto foot = head;
                    head.coor.y = area.coor.y + n - 2;
                    foot.coor.y = area.coor.y + area.size.y - n + 1;

                    for (auto i = 1; i < n; i++)
                    {
                        bright.alpha(i * step);
                        shadow.alpha(i * step);

                        parent_canvas.core::fill(head, [&](cell& c) { c.bgc().mix(bright); });
                        parent_canvas.core::fill(foot, [&](cell& c) { c.bgc().mix(shadow); });

                        head.coor.y--;
                        foot.coor.y++;
                    }
                };
            }
        };

        // pro: Fader animation.
        class fader
            : public skill
        {
            using skill::boss,
                  skill::memo;

            robot  robo;   // fader: .
            period fade;
            si32 transit;
            cell c1;
            cell c2;
            cell c2_orig;
            bool fake = faux;

            //todo use lambda
            void work(si32 transit)
            {
                auto brush = boss.base::color();
                brush.avg(c1, c2, transit);
                fake = true;
                boss.base::color(brush);
                fake = faux;
                boss.base::deface();
            }

        public:
            fader(base&&) = delete;
            fader(base& boss, cell default_state, cell highlighted_state, period fade_out = 250ms)
                : skill{ boss },
                robo{ boss },
                fade{ fade_out },
                c1 { default_state },
                c2 { highlighted_state },
                c2_orig { highlighted_state },
                transit{ 0 }
            {
                boss.base::color(c1.fgc(), c1.bgc());
                boss.SUBMIT(tier::release, e2::form::prop::brush, brush)
                {
                    if (!fake)
                    {
                        auto& fgc = brush.fgc();
                        auto& bgc = brush.bgc();
                        c1.fgc(fgc);
                        c1.bgc(bgc);
                        if (brush.fga()) c2.fgc(fgc);
                        else             c2.fgc(c2_orig.fgc());
                        if (brush.bga()) c2.bgc(bgc);
                        else             c2.bgc(c2_orig.bgc());
                        work(transit);
                    }
                };
                boss.SUBMIT_T(tier::release, e2::form::state::mouse, memo, active)
                {
                    robo.pacify();
                    if (active)
                    {
                        transit = 256;
                        work(transit);
                    }
                    else
                    {
                        if (fade != fade.zero())
                        {
                            auto range = transit;
                            auto limit = datetime::round<si32>(fade);
                            auto start = 0;
                            robo.actify(constlinearAtoB<si32>(range, limit, start), [&](auto step)
                            {
                                transit -= step;
                                work(transit);
                            });
                        }
                        else work(transit = 0);
                    }
                };
            }
        };

        // pro: Limits manager.
        class limit
            : public skill
        {
            static constexpr auto min_value = dot_00;

            using skill::boss,
                  skill::memo;

            struct lims_t
            {
                twod min = min_value;
                twod max = max_value;
                void fixed_size(twod const& m)
                {
                    min = max = std::clamp(m, min, max);;
                }
            }
            lims;
            bool sure; // limit: Reepeat size checking afetr all.

        public:
            limit(base&&) = delete;
            limit(base& boss, twod const& min_size = -dot_11, twod const& max_size = -dot_11, bool forced_clamp = faux, bool forced_resize = faux)
                : skill{ boss }
            {
                set(min_size, max_size, forced_clamp);
                // Clamping before all.
                boss.SUBMIT_T(tier::preview, e2::size::any, memo, new_size)
                {
                    new_size = std::clamp(new_size, lims.min, lims.max);
                };
                // Clamping after all.
                boss.SUBMIT_T(tier::preview, e2::size::set, memo, new_size)
                {
                    if (sure)
                    {
                        new_size = std::clamp(new_size, lims.min, lims.max);
                    }
                };
                if (forced_resize)
                {
                    boss.SUBMIT_T(tier::release, e2::form::prop::window::size, memo, new_size)
                    {
                        auto reserv = lims;
                        lims.fixed_size(new_size);
                        boss.base::template riseup<tier::release>(e2::form::prop::fixedsize, true, true); //todo unify - Inform ui::fork to adjust ratio.
                        boss.base::template reflow<true>();
                        boss.base::template riseup<tier::release>(e2::form::prop::fixedsize, faux, true);
                        lims = reserv;
                    };
                }
            }
            // pro::limit: Set size limits (min, max). Preserve current value if specified arg less than 0.
            void set(twod const& min_size, twod const& max_size = -dot_11, bool forced_clamp = faux)
            {
                sure = forced_clamp;
                lims.min = min_size.less(dot_00, min_value, min_size);
                lims.max = max_size.less(dot_00, max_value, max_size);
            }
            // pro::limit: Set resize limits (min, max). Preserve current value if specified arg less than 0.
            void set(lims_t const& new_limits, bool forced_clamp = faux)
            {
                set(new_limits.min, new_limits.max, forced_clamp);
            }
            auto& get() const
            {
                return lims;
            }
        };

        // pro: UI-control cache.
        class cache
            : public skill
        {
            using skill::boss,
                  skill::memo;

            sptr<face> coreface;
            byte       lucidity;

        public:
            face& canvas; // cache: Bitmap cache.

            cache(base&&) = delete;
            cache(base& boss, bool rendered = true)
                : skill{ boss },
                  canvas{*(coreface = std::make_shared<face>())},
                  lucidity{ 0xFF }
            {
                canvas.link(boss.bell::id);
                canvas.move(boss.base::coor());
                canvas.size(boss.base::size());
                boss.SUBMIT_T(tier::anycast, e2::form::prop::lucidity, memo, value)
                {
                    if (value == -1)
                    {
                        value = lucidity;
                    }
                    else
                    {
                        lucidity = value;
                        //boss.deface();
                    }
                };
                boss.SUBMIT_T(tier::release, e2::form::upon::vtree::attached, memo, parent_ptr)
                {
                    boss.SIGNAL(tier::general, e2::form::canvas, canvas.shared_from_this());
                };
                boss.SUBMIT_T(tier::release, e2::coor::any, memo, new_xy) { canvas.move(new_xy); };
                boss.SUBMIT_T(tier::release, e2::size::any, memo, new_sz) { canvas.size(new_sz); };
                boss.SUBMIT_T(tier::request, e2::form::canvas, memo, canvas_ptr) { canvas_ptr = coreface; };
                if (rendered)
                {
                    boss.SUBMIT_T(tier::release, e2::render::prerender, memo, parent_canvas)
                    {
                        if (boss.base::ruined())
                        {
                            canvas.wipe();
                            boss.base::ruined(faux);
                            boss.SIGNAL(tier::release, e2::render::any, canvas);
                        }
                        if (lucidity == 0xFF) parent_canvas.fill(canvas, cell::shaders::fusefull);
                        else                  parent_canvas.fill(canvas, cell::shaders::transparent(lucidity));
                        boss.bell::expire<tier::release>();
                    };
                }
            }
        };

        // pro: Acrylic blur.
        class acryl
            : public skill
        {
            using skill::boss,
                  skill::memo;

            si32 width; // acryl: Blur radius.
            bool alive; // acryl: Is active.

        public:
            acryl(base&&) = delete;
            acryl(base& boss, si32 size = 5)
                : skill{ boss },
                  width{ size },
                  alive{ true }
            {
                boss.SUBMIT_T(tier::anycast, e2::form::prop::lucidity, memo, lucidity)
                {
                    if (lucidity != -1) alive = lucidity == 0xFF;
                };
                boss.SUBMIT_T(tier::release, e2::render::prerender, memo, parent_canvas)
                {
                    if (!alive) return;
                    auto brush = boss.base::color();
                    if (brush.wdt()) parent_canvas.blur(width, [&](cell& c) { c.fuse(brush); });
                    else             parent_canvas.blur(width);
                };
            }
        };

        // pro: Background Highlighter.
        class light
            : public skill
        {
            using skill::boss,
                  skill::memo;

            bool highlighted = faux; // light: .
            rgba title_fg_color = 0xFFffffff;

        public:
            light(base&&) = delete;
            light(base& boss, bool track_mouse = faux)
                : skill{ boss }
            {
                boss.SUBMIT_T(tier::release, e2::form::highlight::any, memo, state)
                {
                    highlighted = state;
                    boss.base::deface();
                };
                boss.SUBMIT_T(tier::release, e2::render::prerender, memo, parent_canvas)
                {
                    if (highlighted)
                    {
                        auto area = parent_canvas.full();
                        auto mark = skin::color(tone::brighter);
                             mark.fgc(title_fg_color); //todo unify, make it more contrast
                        auto fill = [&](cell& c) { c.fuse(mark); };
                        parent_canvas.fill(area, fill);
                    }
                };
            }
        };

        // pro: Custom highlighter.
        //template<auto fuse> //todo apple clang doesn't get it
        class shade
            : public skill
        {
            using skill::boss,
                  skill::memo;

            bool highlighted = faux; // light2: .

        public:
            shade(base&&) = delete;
            shade(base& boss)
                : skill{ boss }
            {
                boss.SUBMIT_T(tier::release, e2::form::state::mouse, memo, active)
                {
                    highlighted = active;
                    boss.base::deface();
                };
                boss.SUBMIT_T(tier::release, e2::postrender, memo, parent_canvas)
                {
                    if (highlighted)
                    {
                        auto area = parent_canvas.full();
                        //parent_canvas.fill(area, fuse); //todo apple clang doesn't get it
                        parent_canvas.fill(area, cell::shaders::xlight);
                    }
                };
            }
        };

        // pro: Keyboard focus highlighter.
        class focus
            : public skill
        {
            using list = gear_id_list_t;
            using skill::boss,
                  skill::memo;

            list pool; // focus: List of active input devices.

            template<class T>
            bool find(T test_id)
            {
                for (auto id : pool)
                {
                    if (test_id == id) return true;
                }
                return faux;
            }

        public:
            focus(base&&) = delete;
            focus(base& boss)
                : skill{ boss }
            {
                boss.SUBMIT_T(tier::general, e2::form::proceed::functor, memo, proc)
                {
                    if (pool.size()) proc(boss.This());
                };
                boss.SUBMIT_T(tier::anycast, e2::form::state::keybd::find, memo, gear_test)
                {
                    if (find(gear_test.first)) gear_test.second++;
                };
                boss.SUBMIT_T(tier::anycast, e2::form::state::keybd::enlist, memo, gear_id_list)
                {
                    if (pool.size())
                    {
                        auto tail = gear_id_list.end();
                        gear_id_list.insert(tail, pool.begin(), pool.end());
                    }
                };
                boss.SUBMIT_T(tier::request, e2::form::state::keybd::find, memo, gear_test)
                {
                    if (find(gear_test.first)) gear_test.second++;
                };
                boss.SUBMIT_T(tier::anycast, e2::form::state::keybd::handover, memo, gear_id_list)
                {
                    if (pool.size())
                    {
                        auto This = boss.This();
                        auto head = gear_id_list.end();
                        gear_id_list.insert(head, pool.begin(), pool.end());
                        auto tail = gear_id_list.end();
                        while (head != tail)
                        {
                            auto gear_id = *head++;
                            if (auto gate_ptr = bell::getref(gear_id))
                            {
                                gate_ptr->SIGNAL(tier::preview, e2::form::proceed::unfocus, This);
                            }
                        }
                        boss.base::deface();
                    }
                };
                boss.SUBMIT_T(tier::anycast, e2::form::state::keybd::check, memo, state)
                {
                    state = !pool.empty();
                };
                boss.SUBMIT_T(tier::anycast, e2::form::highlight::set, memo, state)
                {
                    state = !pool.empty();
                    boss.template riseup<tier::preview>(e2::form::highlight::any, state);
                };
                boss.SUBMIT_T(tier::anycast, e2::form::upon::started, memo, root)
                {
                    auto state = !pool.empty();
                    boss.template riseup<tier::preview>(e2::form::highlight::any, state);
                };
                boss.SUBMIT_T(tier::release, e2::form::state::keybd::got, memo, gear)
                {
                    boss.template riseup<tier::preview>(e2::form::highlight::any, true);
                    boss.SIGNAL(tier::anycast, e2::form::highlight::any, true);
                    pool.push_back(gear.id);
                    boss.base::deface();
                };
                boss.SUBMIT_T(tier::release, e2::form::state::keybd::lost, memo, gear)
                {
                    assert(!pool.empty());
                    if (!pool.empty())
                    {
                        auto head = pool.begin();
                        auto tail = pool.end();
                        auto item = std::find_if(head, tail, [&](auto& c) { return c == gear.id; });
                        if (item != tail)
                        {
                            pool.erase(item);
                        }
                        boss.base::deface();
                    }

                    if (pool.empty())
                    {
                        boss.template riseup<tier::preview>(e2::form::highlight::any, faux);
                        boss.SIGNAL(tier::anycast, e2::form::highlight::any, faux);
                    }
                };
                boss.SUBMIT_T(tier::release, e2::render::prerender, memo, parent_canvas)
                {
                    //todo revise, too many fillings (mold's artifacts)
                    auto normal = boss.base::color();
                    auto title_fg_color = rgba{ 0xFFffffff };
                    if (!pool.empty())
                    {
                        auto bright = skin::color(tone::brighter);
                        auto shadow = skin::color(tone::shadower);
                        //todo unify, make it more contrast
                        shadow.alpha(0x80);
                        bright.fgc(title_fg_color);
                        shadow.fgc(title_fg_color);
                        auto fillup = [&](auto bright, auto shadow)
                        {
                            parent_canvas.fill(shadow);
                        };
                        if (normal.bgc().alpha())
                        {
                            auto fuse_bright = [&](cell& c) { c.fuse(normal); c.fuse(bright); };
                            auto fuse_shadow = [&](cell& c) { c.fuse(normal); c.fuse(shadow); };
                            fillup(fuse_shadow, fuse_bright);
                        }
                        else
                        {
                            auto only_bright = [&](cell& c) { c.fuse(bright); };
                            auto only_shadow = [&](cell& c) { c.fuse(shadow); };
                            fillup(only_shadow, only_bright);
                        }
                        // Draw the border around
                        auto area = parent_canvas.full();
                        auto mark = skin::color(tone::kb_focus);
                        mark.fgc(title_fg_color); //todo unify, make it more contrast
                        auto fill = [&](cell& c) { c.fuse(mark); };
                        parent_canvas.cage(area, dot_21, fill);
                    }
                };
            }
        };

        // pro: Drag&drop functionality.
        class d_n_d
            : public skill
        {
            using wptr = netxs::wptr<base>;
            using skill::boss,
                  skill::memo;

            id_t under;
            bool drags;
            twod coord;
            wptr cover;

            void proceed(bool keep)
            {
                drags = faux;
                boss.SIGNAL(tier::anycast, e2::form::prop::lucidity, 0xFF); // Make target opaque.
                if (auto object = cover.lock())
                {
                    if (keep)
                    {
                        auto what = e2::form::proceed::d_n_d::drop.param();
                        what.object = object;
                        boss.SIGNAL(tier::preview, e2::form::proceed::d_n_d::drop, what);
                    }
                    else object->SIGNAL(tier::release, e2::form::proceed::d_n_d::abort, boss.This());
                }
                cover.reset();
                under = {};
            }

        public:
            d_n_d(base&&) = delete;
            d_n_d(base& boss)
                : skill{ boss },
                  drags{ faux },
                  under{      }
            {
                boss.SUBMIT_T(tier::release, hids::events::mouse::button::drag::start::any, memo, gear)
                {
                    if (boss.size().inside(gear.coord)
                    && !gear.kbmod())
                    {
                        drags = true;
                        coord = gear.coord;
                        under = {};
                    }
                };
                boss.SUBMIT_T(tier::release, hids::events::mouse::button::drag::pull::any, memo, gear)
                {
                    if (!drags) return;
                    if (gear.kbmod()) proceed(faux);
                    else              coord = gear.coord - gear.delta.get();
                };
                boss.SUBMIT_T(tier::release, hids::events::mouse::button::drag::stop::any, memo, gear)
                {
                    if (!drags) return;
                    if (gear.kbmod()) proceed(faux);
                    else              proceed(true);
                };
                boss.SUBMIT_T(tier::release, hids::events::mouse::button::drag::cancel::any, memo, gear)
                {
                    if (!drags) return;
                    //todo revise (panoramic scrolling with left + right)
                    //proceed(faux);
                    if (gear.kbmod()) proceed(faux);
                    else              proceed(true);
                };
                boss.SUBMIT_T(tier::release, e2::render::prerender, memo, parent_canvas)
                {
                    if (!drags) return;
                    auto full = parent_canvas.face::full();
                    auto size = parent_canvas.core::size();
                    auto coor = full.coor + coord;
                    if (size.inside(coor))
                    {
                        auto& c = parent_canvas[coor];
                        auto new_under = c.link();
                        if (under != new_under)
                        {
                            auto object = e2::form::proceed::d_n_d::ask.param();
                            if (auto old_object = std::dynamic_pointer_cast<base>(bell::getref(under)))
                            {
                                old_object->riseup<tier::release>(e2::form::proceed::d_n_d::abort, object);
                            }
                            if (auto new_object = std::dynamic_pointer_cast<base>(bell::getref(new_under)))
                            {
                                new_object->riseup<tier::release>(e2::form::proceed::d_n_d::ask, object);
                            }
                            boss.SIGNAL(tier::anycast, e2::form::prop::lucidity, object ? 0x80
                                                                                        : 0xFF); // Make it semi-transparent on success and opaque otherwise.
                            cover = object;
                            under = new_under;
                        }
                    }
                };
            }
        };

        // pro: Drag&roll.
        class glide
            : public skill
        {
            using skill::boss,
                  skill::memo;

        public:
            glide(base&&) = delete;
            glide(base& boss)
                : skill{ boss }
            {

            }
        };

        // pro: Tooltip.
        class notes
            : public skill
        {
            using skill::boss,
                  skill::memo;

            text note;

        public:
            notes(base&&) = delete;
            notes(base& boss, view data)
                : skill{ boss },
                  note { data }
            {
                boss.SUBMIT_T(tier::release, hids::events::notify::mouse::enter, memo, gear)
                {
                    gear.set_tooltip(boss.id, note);
                };
                boss.SUBMIT_T(tier::preview, e2::form::prop::ui::tooltip, memo, new_note)
                {
                    note = new_note;
                };
            }
            void update(view new_note)
            {
                note = new_note;
            }
        };
    }

    // console: World aether.
    class host
        : public base
    {
        using tick = quartz<events::reactor<>, e2::type>;
        using list = std::vector<rect>;

        pro::keybd keybd{*this }; // host: Keyboard controller.
        pro::mouse mouse{*this }; // host: Mouse controller.

        subs token; // host: Subscription tokens.
        tick synch; // host: Frame rate synchronizator.
        si32 hertz; // host: Frame rate value.
        list edges; // host: Wrecked regions list.
        xipc joint;

    public:
        host(xipc server_pipe, si32 fps)
            : synch{ bell::router<tier::general>(), e2::timer::tick.id },
              hertz{ fps },
              joint{ server_pipe }
        {
            keybd.accept(true); // Subscribe on keybd offers.

            SUBMIT_T(tier::general, e2::timer::any, token, timestamp)
            {
                auto damaged = !edges.empty();
                edges.clear();
                SIGNAL_GLOBAL(e2::nextframe, damaged);
            };
            SUBMIT_T(tier::general, e2::config::whereami, token, world_ptr)
            {
                world_ptr = base::This();
            };
            SUBMIT_T(tier::general, e2::config::fps, token, fps)
            {
                if (fps > 0)
                {
                    hertz = fps;
                    synch.ignite(hertz);
                }
                else if (fps == -1)
                {
                    fps = hertz;
                }
                else
                {
                    synch.cancel();
                }
            };
            SUBMIT_T(tier::general, e2::cleanup, token, counter)
            {
                this->template router<tier::general>().cleanup(counter.ref_count, counter.del_count);
            };
            SUBMIT_T(tier::general, hids::events::halt, token, gear)
            {
                if (gear.captured(bell::id))
                {
                    gear.setfree();
                    gear.dismiss();
                }
            };
            SUBMIT_T(tier::general, e2::shutdown, token, msg)
            {
                log("host: shutdown: ", msg);
                joint->shut();
            };
            synch.ignite(hertz);
            log("host: started at ", hertz, "fps");
        }
        // host: Initiate redrawing.
        virtual void redraw(face& canvas)
        {
            SIGNAL_GLOBAL(e2::shutdown, "host: rendering is not provided");
        }
        // host: Mark dirty region.
        void denote(rect const& updateregion)
        {
            if (updateregion)
            {
                edges.push_back(updateregion);
            }
        }
        void deface(rect const& region) override
        {
            base::deface(region);
            denote(region);
        }
        // host: Create a new root of the specified subtype and attach it.
        template<class S, class ...Args>
        auto invite(Args&&... args)
        {
            auto lock = events::sync{};
            auto root = base::create<S>(*this, std::forward<Args>(args)...);
            //stuff = root;
            root->base::root(true);
            root->SIGNAL(tier::release, e2::form::upon::vtree::attached, base::This());

            //todo unify
            auto color = tone{ tone::brighter, tone::shadow};
            root->SIGNAL(tier::preview, e2::form::state::color, color);
            return root;
        }
        // host: Shutdown.
        void shutdown()
        {
            auto lock = events::sync{};
            mouse.reset();
        }
    };

    // console: Desktopio Workspace.
    class hall
        : public host
    {
        class node // hall: Helper-class for the pro::scene. Adapter for the object that going to be attached to the scene.
        {
            struct ward
            {
                enum states
                {
                    unused_hidden, // 00
                    unused_usable, // 01
                    active_hidden, // 10
                    active_usable, // 11
                    count
                };

                para title[states::count];
                cell brush[states::count];
                para basis;
                bool usable = faux;
                bool highlighted = faux;
                si32 active = 0;
                tone color;

                operator bool ()
                {
                    return basis.size() != dot_00;
                }
                void set(para const& caption)
                {
                    basis = caption;
                    basis.decouple();
                    recalc();
                }
                auto& get()
                {
                    return title[(active || highlighted ? 2 : 0) + usable];
                }
                void recalc()
                {
                    brush[active_hidden] = skin::color(color.active);
                    brush[active_usable] = skin::color(color.active);
                    brush[unused_hidden] = skin::color(color.passive);
                    brush[unused_usable] = skin::color(color.passive);

                    brush[unused_usable].bga(brush[unused_usable].bga() << 1);

                    auto i = 0;
                    for (auto& label : title)
                    {
                        auto& c = brush[i++];
                        label = basis;
                        label.decouple();

                        //todo unify clear formatting/aligning in header
                        label.locus.kill();
                        label.style.rst();
                        label.lyric->each([&](auto& a) { a.meta(c); });
                    }
                }
            };

            using sptr = netxs::sptr<base>;

            ward header;

        public:
            rect region;
            sptr object;
            id_t obj_id;
            si32 z_order = Z_order::plain;

            node(sptr item)
                : object{ item }
            {
                auto& inst = *item;
                obj_id = inst.bell::id;

                inst.SUBMIT(tier::release, e2::form::prop::zorder, order)
                {
                    z_order = order;
                };
                inst.SUBMIT(tier::release, e2::size::any, size)
                {
                    region.size = size;
                };
                inst.SUBMIT(tier::release, e2::coor::any, coor)
                {
                    region.coor = coor;
                };
                inst.SUBMIT(tier::release, e2::form::state::mouse, state)
                {
                    header.active = state;
                };
                inst.SUBMIT(tier::release, e2::form::highlight::any, state)
                {
                    header.highlighted = state;
                };
                inst.SUBMIT(tier::release, e2::form::state::header, caption)
                {
                    header.set(caption);
                };
                inst.SUBMIT(tier::release, e2::form::state::color, color)
                {
                    header.color = color;
                };

                inst.SIGNAL(tier::request, e2::size::set,  region.size);
                inst.SIGNAL(tier::request, e2::coor::set,  region.coor);
                inst.SIGNAL(tier::request, e2::form::state::mouse,  header.active);
                inst.SIGNAL(tier::request, e2::form::state::header, header.basis);
                inst.SIGNAL(tier::request, e2::form::state::color,  header.color);

                header.recalc();
            }
            // hall::node: Check equality.
            bool equals(id_t id)
            {
                return obj_id == id;
            }
            // hall::node: Draw the anchor line func and return true
            //             if the mold is outside the canvas area.
            void fasten(face& canvas)
            {
                auto window = canvas.area();
                auto origin = window.size / 2;
                //auto origin = twod{ 6, window.size.y - 3 };
                auto offset = region.coor - window.coor;
                auto center = offset + (region.size / 2);
                header.usable = window.overlap(region);
                auto active = header.active || header.highlighted;
                auto& grade = skin::grade(active ? header.color.active
                                                    : header.color.passive);
                auto pset = [&](twod const& p, uint8_t k)
                {
                    //canvas[p].fuse(grade[k], obj_id, p - offset);
                    //canvas[p].fuse(grade[k], obj_id);
                    canvas[p].link(obj_id).bgc().mix_one(grade[k].bgc());
                };
                window.coor = dot_00;
                netxs::online(window, origin, center, pset);
            }
            // hall::node: Visualize the underlying object.
            template<bool POST = true>
            void render(face& canvas)
            {
                canvas.render<POST>(*object);
            }
            void postrender(face& canvas)
            {
                object->SIGNAL(tier::release, e2::postrender, canvas);
            }
        };

        class list // hall: Helper-class. List of objects that can be reordered, etc.
        {
            std::list<sptr<node>> items;

            template<class D>
            auto search(D head, D tail, id_t id)
            {
                if (items.size())
                {
                    auto test = [id](auto& a) { return a->equals(id); };
                    return std::find_if(head, tail, test);
                }
                return tail;
            }

        public:
            operator bool () { return items.size(); }
            auto size()      { return items.size(); }
            void append(sptr<base> item)
            {
                items.push_back(std::make_shared<node>(item));
            }
            //hall::list: Draw backpane for spectators.
            void prerender(face& canvas)
            {
                for (auto& item : items) item->fasten(canvas); // Draw strings.
                for (auto& item : items) item->render<faux>(canvas); // Draw shadows without postrendering.
            }
            //hall::list: Draw windows.
            void render(face& canvas)
            {
                for (auto& item : items) item->fasten(canvas);
                //todo optimize
                for (auto& item : items) if (item->z_order == Z_order::backmost) item->render(canvas);
                for (auto& item : items) if (item->z_order == Z_order::plain   ) item->render(canvas);
                for (auto& item : items) if (item->z_order == Z_order::topmost ) item->render(canvas);
            }
            //hall::list: Draw spectator's mouse pointers.
            void postrender(face& canvas)
            {
                for (auto& item : items) item->postrender(canvas);
            }
            //hall::list: Delete all items.
            void reset()
            {
                items.clear();
            }
            rect remove(id_t id)
            {
                auto area = rect{};
                auto head = items.begin();
                auto tail = items.end();
                auto item = search(head, tail, id);
                if (item != tail)
                {
                    area = (**item).region;
                    items.erase(item);
                }
                return area;
            }
            rect bubble(id_t id)
            {
                auto head = items.rbegin();
                auto tail = items.rend();
                auto item = search(head, tail, id);

                if (item != head && item != tail)
                {
                    auto& area = (**item).region;
                    if (!area.clip((**std::prev(item)).region))
                    {
                        auto shadow = *item;
                        items.erase(std::next(item).base());

                        while (--item != head
                            && !area.clip((**std::prev(item)).region))
                        { }

                        items.insert(item.base(), shadow);
                        return area;
                    }
                }

                return rect_00;
            }
            rect expose(id_t id)
            {
                auto head = items.rbegin();
                auto tail = items.rend();
                auto item = search(head, tail, id);

                if (item != head && item != tail)
                {
                    auto shadow = *item;
                    items.erase(std::next(item).base());
                    items.push_back(shadow);
                    return shadow->region;
                }

                return rect_00;
            }
            auto rotate_next()
            {
                items.push_back(items.front());
                items.pop_front();
                return items.back();
            }
            auto rotate_prev()
            {
                items.push_front(items.back());
                items.pop_back();
                return items.back();
            }
        };

        class depo // hall: Helper-class. Actors registry.
        {
        public:
            sptr<registry_t>            app_ptr = std::make_shared<registry_t>();
            sptr<std::list<sptr<base>>> usr_ptr = std::make_shared<std::list<sptr<base>>>();
            sptr<links_t>               lnk_ptr = std::make_shared<links_t>();
            registry_t&                 app = *app_ptr;
            std::list<sptr<base>>&      usr = *usr_ptr;
            links_t&                    lnk = *lnk_ptr;

            auto remove(sptr<base> item_ptr)
            {
                auto found = faux;
                // Remove from active app registry.
                for (auto& [class_id, fxd_app_list] : app)
                {
                    auto& [fixed, app_list] = fxd_app_list;
                    auto head = app_list.begin();
                    auto tail = app_list.end();
                    auto iter = std::find_if(head, tail, [&](auto& c) { return c == item_ptr; });
                    if (iter != tail)
                    {
                        app_list.erase(iter);
                        if (app_list.empty() && !fixed)
                        {
                            app.erase(class_id);
                        }
                        found = true;
                        break;
                    }
                }
                { // Remove user.
                    auto head = usr.begin();
                    auto tail = usr.end();
                    auto iter = std::find_if(head, tail, [&](auto& c){ return c == item_ptr; });
                    if (iter != tail)
                    {
                        usr.erase(iter);
                        found = true;
                    }
                }
                return found;
            }
            void reset()
            {
                app_ptr.reset();
            }
        };

        list items; // hall: Child visual tree.
        list users; // hall: Scene spectators.
        depo regis; // hall: Actors registry.

    protected:
        hall(xipc server_pipe, si32 maxfps)
            : host{ server_pipe, maxfps }
        {
            SUBMIT(tier::general, e2::form::global::lucidity, alpha)
            {
                if (alpha == -1)
                {
                    alpha = skin::shady();
                }
                else
                {
                    alpha = std::clamp(alpha, 0, 255);
                    skin::setup(tone::lucidity, alpha);
                    this->SIGNAL(tier::preview, e2::form::global::lucidity, alpha);
                }
            };
            SUBMIT(tier::preview, e2::form::proceed::detach, item_ptr)
            {
                auto& inst = *item_ptr;
                host::denote(items.remove(inst.id));
                host::denote(users.remove(inst.id));
                if (regis.remove(item_ptr))
                {
                    inst.SIGNAL(tier::release, e2::form::upon::vtree::detached, This());
                }
            };
            SUBMIT(tier::release, e2::form::layout::bubble, inst)
            {
                auto region = items.bubble(inst.bell::id);
                host::denote(region);
            };
            SUBMIT(tier::release, e2::form::layout::expose, inst)
            {
                auto region = items.expose(inst.bell::id);
                host::denote(region);
            };
            SUBMIT(tier::request, e2::bindings::list::users, usr_list_ptr)
            {
                usr_list_ptr = regis.usr_ptr;
            };
            SUBMIT(tier::request, e2::bindings::list::apps, app_list_ptr)
            {
                app_list_ptr = regis.app_ptr;
            };
            SUBMIT(tier::request, e2::bindings::list::links, list_ptr)
            {
                list_ptr = regis.lnk_ptr;
            };
            //todo unify
            SUBMIT(tier::request, e2::form::layout::gonext, next)
            {
                if (items)
                if (auto next_ptr = items.rotate_next())
                {
                    next = next_ptr->object;
                }
            };
            SUBMIT(tier::request, e2::form::layout::goprev, prev)
            {
                if (items)
                if (auto prev_ptr = items.rotate_prev())
                {
                    prev = prev_ptr->object;
                }
            };
        }

    public:
       ~hall()
        {
            auto lock = events::sync{};
            regis.reset();
            items.reset();
        }
        void redraw(face& canvas) override
        {
            if (users.size() > 1) users.prerender (canvas); // Draw backpane for spectators.
                                  items.render    (canvas); // Draw objects of the world.
                                  users.postrender(canvas); // Draw spectator's mouse pointers.
        }
        // hall: Attach a new item to the scene.
        template<class S>
        auto branch(text const& class_id, sptr<S> item, bool fixed = true)
        {
            items.append(item);
            item->base::root(true); //todo move it to the window creator (main)
            auto& [stat, list] = regis.app[class_id];
            stat = fixed;
            list.push_back(item);
            item->SIGNAL(tier::release, e2::form::upon::vtree::attached, base::This());
            SIGNAL(tier::release, e2::bindings::list::apps, regis.app_ptr);
        }
        // hall: Create a new user of the specified subtype and invite him to the scene.
        template<class S, class ...Args>
        auto invite(Args&&... args)
        {
            auto lock = events::sync{};
            auto user = host::invite<S>(std::forward<Args>(args)...);
            users.append(user);
            regis.usr.push_back(user);
            SIGNAL(tier::release, e2::bindings::list::users, regis.usr_ptr);
            return user;
        }
    };

    // console: TTY session manager.
    class link
        : public s11n
    {
        using sptr = netxs::sptr<bell>;
        using ipc  = os::ipc::iobase;

    public:
        struct relay_t
        {
            using lock = std::recursive_mutex;
            using cond = std::condition_variable_any;

            struct clip_t
            {
                lock mutex{};
                cond synch{};
                bool ready{};
                twod block{};
                clip chunk{};
            };
            using umap = std::unordered_map<id_t, clip_t>;

            umap depot{};
            lock mutex{};

            void set(id_t id, view utf8, clip::mime kind)
            {
                auto lock = std::lock_guard{ mutex };
                auto iter = depot.find(id);
                if (iter != depot.end())
                {
                    auto& item = iter->second;
                    auto  lock = std::lock_guard{ item.mutex };
                    item.chunk.utf8 = utf8;
                    item.chunk.kind = kind;
                    item.ready = true;
                    item.synch.notify_all();
                }
            }
        };

        struct sysgears
        {
            sysmouse mouse = {};
            syskeybd keybd = {};
            sysfocus focus = {};
        };

        ipc&                               canal; // link: Data highway.
        sptr                               owner; // link: Link owner.
        relay_t                            relay; // link: Clipboard relay.
        std::unordered_map<id_t, sysgears> gears; // link: Input devices state.

    public:
        // link: Send data outside.
        void output(view data)
        {
            canal.output(data);
        }
        // link: .
        auto request_clip_data(id_t ext_gear_id, clip& clip_rawdata)
        {
            relay.mutex.lock();
            auto& selected_depot = relay.depot[ext_gear_id]; // If rehashing occurs due to the insertion, all iterators are invalidated.
            relay.mutex.unlock();
            auto lock = std::unique_lock{ selected_depot.mutex };
            selected_depot.ready = faux;
            request_clipboard.send(canal, ext_gear_id);
            auto maxoff = 100ms; //todo magic numbers
            auto received = std::cv_status::timeout != selected_depot.synch.wait_for(lock, maxoff);
            if (received)
            {
                clip_rawdata = selected_depot.chunk;
            }
            return received;
        }

        link(ipc& canal, sptr owner)
            : s11n{ *this },
             canal{ canal },
             owner{ owner }
        { }

        // link: Send an event message to the link owner.
        template<tier TIER = tier::release, class E, class T>
        void notify(E, T& data)
        {
            netxs::events::enqueue(owner, [d = data](auto& boss) mutable
            {
                boss.SIGNAL(TIER, E{}, d);
            });
        }
        void handle(s11n::xs::focus       lock)
        {
            auto& item = lock.thing;
            auto& f = gears[item.gear_id].focus;
            f.focusid = item.gear_id;
            f.enabled = item.state;
            f.combine_focus = item.combine_focus;
            f.force_group_focus = item.force_group_focus;
            notify(e2::conio::focus, f);
        }
        void handle(s11n::xs::winsz       lock)
        {
            auto& item = lock.thing;
            notify(e2::conio::winsz, item.winsize);
        }
        void handle(s11n::xs::clipdata    lock)
        {
            auto& item = lock.thing;
            relay.set(item.gear_id, item.data, static_cast<clip::mime>(item.mimetype));
        }
        void handle(s11n::xs::keybd       lock)
        {
            auto& item = lock.thing;
            auto& k = gears[item.gear_id].keybd;
            k.keybdid = item.gear_id;
            k.virtcod = item.virtcod;
            k.scancod = item.scancod;
            k.pressed = item.pressed;
            k.ctlstat = item.ctlstat;
            k.winctrl = item.winctrl;
            k.imitate = item.imitate;
            k.cluster = item.cluster;
            k.winchar = item.winchar;
            notify(e2::conio::keybd, k);
        }
        void handle(s11n::xs::plain       lock)
        {
            auto& item = lock.thing;
            auto& k = gears[item.gear_id].keybd;
            k.keybdid = item.gear_id;
            k.cluster = item.utf8txt;
            k.pressed = true;
            notify(e2::conio::keybd, k);
            k.pressed = faux;
            notify(e2::conio::keybd, k);
        }
        void handle(s11n::xs::ctrls       lock)
        {
            auto& item = lock.thing;
            auto& k = gears[item.gear_id].keybd;
            k.keybdid = item.gear_id;
            k.ctlstat = item.ctlstat;
            k.cluster = {};
            k.pressed = faux;
            notify(e2::conio::keybd, k);
        }
        void handle(s11n::xs::mouse       lock)
        {
            auto& item = lock.thing;
            auto gear_id = item.gear_id;
            auto buttons = item.buttons;
            auto ctlstat = item.ctlstat;
            auto msflags = item.msflags;
            auto wheeldt = item.wheeldt;
            auto coordxy = item.coordxy;
            auto& m = gears[gear_id].mouse;
            m.set_buttons(buttons);
            m.mouseid = gear_id;
            m.control = sysmouse::stat::ok;
            m.ismoved = m.coor(coordxy);
            m.shuffle = !m.ismoved && (msflags & (1 << 0)); // MOUSE_MOVED
            m.doubled = msflags & (1 << 1); // DOUBLE_CLICK -- Makes no sense (ignored)
            m.wheeled = msflags & (1 << 2); // MOUSE_WHEELED
            m.hzwheel = msflags & (1 << 3); // MOUSE_HWHEELED
            m.wheeldt = wheeldt;
            m.ctlstat = ctlstat;
            m.winctrl = item.winctrl;
            if (!m.shuffle)
            {
                m.update_buttons();
                notify(e2::conio::mouse, m);
            }
        }
        void handle(s11n::xs::mouse_stop  lock)
        {
            auto& item = lock.thing;
            auto& m = gears[item.gear_id].mouse;
            m.mouseid = item.gear_id;
            m.control = sysmouse::stat::die;
            notify(e2::conio::mouse, m);
        }
        void handle(s11n::xs::mouse_halt  lock)
        {
            auto& item = lock.thing;
            auto& m = gears[item.gear_id].mouse;
            m.mouseid = item.gear_id;
            m.control = sysmouse::stat::halt;
            notify(e2::conio::mouse, m);
        }
        void handle(s11n::xs::mouse_show  lock)
        {
            auto& item = lock.thing;
            notify(e2::conio::pointer, item.mode);
        }
        void handle(s11n::xs::native      lock)
        {
            auto& item = lock.thing;
            notify(e2::conio::native, item.mode);
        }
        void handle(s11n::xs::request_gc  lock)
        {
            auto& items = lock.thing;
            auto list = jgc_list.freeze();
            for (auto& gc : items)
            {
                auto cluster = cell::gc_get_data(gc.token);
                list.thing.push(gc.token, cluster);
            }
            list.thing.sendby(canal);
        }
        void handle(s11n::xs::fps         lock)
        {
            auto& item = lock.thing;
            notify(e2::config::fps, item.frame_rate);
        }
        void handle(s11n::xs::bgcolor     lock)
        {
            auto& item = lock.thing;
            notify<tier::anycast>(e2::form::prop::colors::bg, item.color);
        }
        void handle(s11n::xs::fgcolor     lock)
        {
            auto& item = lock.thing;
            notify<tier::anycast>(e2::form::prop::colors::fg, item.color);
        }
        void handle(s11n::xs::slimmenu    lock)
        {
            auto& item = lock.thing;
            notify<tier::anycast>(e2::form::prop::ui::slimmenu, item.menusize);
        }
        void handle(s11n::xs::debugdata   lock) // For Logs only.
        {
            auto& item = lock.thing;
            notify<tier::anycast>(e2::debug::output, item.data);
        }
        void handle(s11n::xs::debuglogs   lock) // For Logs only.
        {
            auto& item = lock.thing;
            notify<tier::anycast>(e2::debug::logs, item.data);
        }
    };

    // console: Bitmap changes analyzer.
    class diff
    {
        using work = std::thread;
        using lock = std::mutex;
        using cond = std::condition_variable_any;
        using span = period;
        using ipc  = os::ipc::iobase;

        struct stat
        {
            span watch{}; // diff::stat: Duration of the STDOUT rendering.
            sz_t delta{}; // diff::stat: Last ansi-rendered frame size.
        };

        lock mutex; // diff: Mutex between renderer and committer threads.
        cond synch; // diff: Synchronization between renderer and committer.
        core cache; // diff: The current content buffer which going to be checked and processed.
        bool alive; // diff: Working loop state.
        bool ready; // diff: Conditional variable to avoid spurious wakeup.
        bool abort; // diff: Abort building current frame.
        work paint; // diff: Rendering thread.
        stat debug; // diff: Debug info.

        // diff: Render current buffer to the screen.
        template<class Bitmap>
        void render(ipc& canal)
        {
            log("diff: id: ", std::this_thread::get_id(), " rendering thread started");
            auto start = moment{};
            auto image = Bitmap{};
            auto guard = std::unique_lock{ mutex };
            while ((void)synch.wait(guard, [&]{ return ready; }), alive)
            {
                start = tempus::now();
                ready = faux;
                abort = faux;
                auto winid = id_t{ 0xddccbbaa };
                auto coord = dot_00;
                image.set(winid, coord, cache, abort, debug.delta);
                if (debug.delta)
                {
                    image.sendby(canal); // Sending, this is the frame synchronization point.
                }                        // Frames should drop, the rest should wait for the end of sending.
                debug.watch = tempus::now() - start;
            }
            log("diff: id: ", std::this_thread::get_id(), " rendering thread ended");
        }

    public:
        // diff: Get rendering statistics.
        auto status()
        {
            return debug;
        }
        // diff: Discard current frame.
        void cancel()
        {
            abort = true;
        }
        // diff: Obtain new content to render.
        auto commit(core const& canvas)
        {
            if (abort)
            {
                while (alive) // Try to send a new frame as soon as possible (e.g. after resize).
                {
                    auto lock = std::unique_lock{ mutex, std::try_to_lock };
                    if (lock.owns_lock())
                    {
                        cache = canvas;
                        ready = true;
                        synch.notify_one();
                        return true;
                    }
                    else std::this_thread::yield();
                }
            }
            else
            {
                auto lock = std::unique_lock{ mutex, std::try_to_lock };
                if (lock.owns_lock())
                {
                    cache = canvas;
                    ready = true;
                    synch.notify_one();
                    return true;
                }
            }
            return faux;
        }

        diff(ipc& canal, svga vtmode)
            : alive{ true },
              ready{ faux },
              abort{ faux }
        {
            using namespace netxs::ansi::dtvt;
            paint = work([&, vtmode]
            {
                //todo revise (bitmap/bitmap_t)
                     if (vtmode == svga::directvt ) render<binary::bitmap_t>               (canal);
                else if (vtmode == svga::truecolor) render< ascii::bitmap<svga::truecolor>>(canal);
                else if (vtmode == svga::vga16    ) render< ascii::bitmap<svga::vga16    >>(canal);
                else if (vtmode == svga::vga256   ) render< ascii::bitmap<svga::vga256   >>(canal);
            });
        }
       ~diff()
        {
            auto id = paint.get_id();
            mutex.lock();
            alive = faux;
            ready = true;
            synch.notify_all();
            mutex.unlock();
            paint.join();
            log("diff: id: ", id, " rendering thread joined");
        }
    };

    // console: Client properties.
    class conf
    {
    public:
        text ip;
        text port;
        text fullname;
        text region;
        text name;
        text os_user_id;
        text title;
        twod coor;
        twod clip_preview_size;
        cell background_color;
        si32 legacy_mode;
        si32 session_id;
        period tooltip_timeout; // conf: Timeout for tooltip.
        bool tooltip_enabled; // conf: Enable tooltips.
        bool glow_fx; // conf: Enable glow effect in main menu.
        bool debug_overlay; // conf: Enable to show debug overlay.
        text debug_toggle; // conf: Debug toggle shortcut.
        bool show_regions; // conf: Highlight region ownership.
        bool simple; // conf: Isn't it a directvt app.
        bool is_standalone_app; // conf: .

        conf()            = default;
        conf(conf const&) = default;
        conf(conf&&)      = default;
        conf& operator = (conf const&) = default;

        conf(si32 mode)
            : session_id{ 0 },
              legacy_mode{ mode }
        {
            clip_preview_size = twod{ 80,25 };
            coor              = twod{ 0,0 }; //todo Move user's viewport to the last saved position
            tooltip_timeout   = 500ms;
            tooltip_enabled   = true;
            glow_fx           = faux;
            debug_overlay     = faux;
            debug_toggle      = "🐞";
            show_regions      = faux;
            simple            = !(legacy_mode & os::legacy::direct);
            is_standalone_app = true;
        }
        conf(xipc peer, si32 session_id)
            : session_id{ session_id }
        {
            auto _ip     = peer->line(';');
            auto _name   = peer->line(';');
            auto _user   = peer->line(';');
            auto _mode   = peer->line(';');

            _user = "[" + _user + ":" + std::to_string(session_id) + "]";
            auto c_info = utf::divide(_ip, " ");
            ip   = c_info.size() > 0 ? c_info[0] : text{};
            port = c_info.size() > 1 ? c_info[1] : text{};
            legacy_mode       = utf::to_int(_mode, os::legacy::clean);
            os_user_id        = _user;
            clip_preview_size = twod{ 80,25 };
            //background_color  = app::shared::background_color;
            coor              = twod{ 0,0 }; //todo Move user's viewport to the last saved position
            fullname          = _name;
            name              = _user;
            title             = _user;
            tooltip_timeout   = 500ms;
            tooltip_enabled   = true;
            glow_fx           = true;
            debug_overlay     = faux;
            debug_toggle      = "🐞";
            show_regions      = faux;
            simple            = faux;
            is_standalone_app = faux;
        }

        friend auto& operator << (std::ostream& s, conf const& c)
        {
            return s << "\n\t    ip: " <<(c.ip.empty() ? text{} : (c.ip + ":" + c.port))
                     << "\n\tregion: " << c.region
                     << "\n\t  name: " << c.fullname
                     << "\n\t  user: " << c.os_user_id
                     << "\n\t  mode: " << os::legacy::str(c.legacy_mode);
        }
    };

    // console: Client's gate.
    class gate
        : public base
    {
        pro::keybd keybd{*this }; // gate: Keyboard controller.
        pro::mouse mouse{*this }; // gate: Mouse controller.
        pro::robot robot{*this }; // gate: Animation controller.
        pro::maker maker{*this }; // gate: Form generator.
        pro::title title{*this }; // gate: Logo watermark.
        pro::guard guard{*this }; // gate: Watch dog against robots and single Esc detector.
        pro::input input{*this }; // gate: User input event handler.
        pro::debug debug{*this }; // gate: Debug telemetry controller.
        pro::limit limit{*this }; // gate: Limit size to dot_11.

        using sptr = netxs::sptr<base>;

        host& world;
        bool  yield; // gate: Indicator that the current frame has been successfully STDOUT'd.
        para  uname; // gate: Client name.
        text  uname_txt; // gate: Client name (original).
        bool  native = faux; //gate: Extended functionality support.
        bool  fullscreen = faux; //gate: Fullscreen mode.
        si32  legacy = os::legacy::clean;
        conf  props; // gate: Client properties.

        void draw_foreign_names(face& parent_canvas)
        {
            auto& header = *uname.lyric;
            auto  basexy = base::coor();
            auto  half_x = (si32)header.size().x / 2;
            for (auto& [id, gear_ptr] : input.gears)
            {
                auto& gear = *gear_ptr;
                auto coor = basexy;
                coor += gear.coord;
                coor.y--;
                coor.x -= half_x;
                //todo unify header coords
                header.move(coor);
                parent_canvas.fill(header, cell::shaders::fuse);
            }
        }
        void draw_mouse_pointer(face& canvas)
        {
            auto brush = cell{};
            auto coor = base::coor();
            auto area = rect{ coor, dot_11 };
            auto base = canvas.core::coor();
            for (auto& [id, gear_ptr] : input.gears)
            {
                auto& gear = *gear_ptr;
                area.coor = coor + gear.coord;
                area.coor -= base;
                if (gear.push) brush.txt(64 + gear.push).bgc(reddk).fgc(0xFFffffff);
                else           brush.txt("\u2588"/* █ */).bgc(0x00).fgc(0xFF00ff00);
                canvas.fill(area, cell::shaders::fuse(brush));
            }
        }
        void draw_clip_preview(face& canvas)
        {
            for (auto& [id, gear_ptr] : input.gears)
            {
                auto& gear = *gear_ptr;
                auto coor = gear.coord + dot_21 * 2;
                gear.clip_preview.move(coor);
                canvas.plot(gear.clip_preview, cell::shaders::lite);
            }
        }
        void draw_tooltips(face& canvas)
        {
            static constexpr auto def_tooltip = { rgba{ 0xFFffffff }, rgba{ 0xFF000000 } }; //todo unify
            auto full = canvas.full();
            for (auto& [id, gear_ptr] : input.gears)
            {
                auto& gear = *gear_ptr;
                if (gear.tooltip_enabled())
                {
                    auto tooltip_data = gear.get_tooltip();
                    if (tooltip_data)
                    {
                        //todo optimize
                        auto tooltip_page = page{ tooltip_data };
                        auto area = full;
                        area.coor = std::max(dot_00, gear.coord - twod{ 4, tooltip_page.size() + 1 });
                        canvas.full(area);
                        canvas.cup(dot_00);
                        canvas.output(tooltip_page, cell::shaders::selection(def_tooltip));
                        canvas.full(full);
                    }
                }
            }
        }
        void send_tooltips(link& conio)
        {
            auto list = conio.tooltips.freeze();
            for (auto& [gear_id, gear_ptr] : input.gears /* use filter gear.is_tooltip_changed()*/)
            {
                auto& gear = *gear_ptr;
                if (gear.is_tooltip_changed())
                {
                    list.thing.push(gear_id, gear.get_tooltip());
                }
            }
            list.thing.sendby<true>(conio);
        }
        void check_tooltips(moment now)
        {
            auto result = faux;
            for (auto& [gear_id, gear_ptr] : input.gears)
            {
                auto& gear = *gear_ptr;
                result |= gear.tooltip_check(now);
            }
            if (result) base::strike();
        }

    public:
        sptr uibar; // gate: Local UI overlay, UI bar/taskbar/sidebar.
        sptr background; // gate: Local UI background.

        // gate: Attach a new item.
        template<class T>
        auto attach(netxs::sptr<T> item)
        {
            uibar = item;
            item->SIGNAL(tier::release, e2::form::upon::vtree::attached, This());
            return item;
        }
        // gate: Create a new item of the specified subtype and attach it.
        template<class T, class ...Args>
        auto attach(Args&&... args)
        {
            return attach(base::create<T>(std::forward<Args>(args)...));
        }
        // gate: Attach background object.
        template<class T>
        auto ground(netxs::sptr<T> item)
        {
            background = item;
            item->SIGNAL(tier::release, e2::form::upon::vtree::attached, This());
            return item;
        }
        // Main loop.
        void launch(xipc termio, sptr deskmenu, sptr bkground = {})
        {
            auto lock = events::unique_lock();

                legacy |= props.legacy_mode;

                auto vtmode = legacy & os::legacy::vga16  ? svga::vga16
                            : legacy & os::legacy::vga256 ? svga::vga256
                            : legacy & os::legacy::direct ? svga::directvt
                                                          : svga::truecolor;
                auto direct = vtmode == svga::directvt;
                if (props.debug_overlay) debug.start();
                color(props.background_color.fgc(), props.background_color.bgc());
                auto conf_usr_name = props.name;
                SIGNAL(tier::release, e2::form::prop::name, conf_usr_name);
                SIGNAL(tier::preview, e2::form::prop::ui::header, conf_usr_name);
                base::moveby(props.coor);

                //todo hids
                //clip_preview.size(props.clip_preview_size); //todo unify/make it configurable

                auto& canal = *termio;
                link conio{ canal, This() }; // gate: Terminal IO.
                diff paint{ canal, vtmode }; // gate: Rendering loop.
                subs token;                   // gate: Subscription tokens.

                auto rebuild_scene = [&](bool damaged)
                {
                    auto stamp = tempus::now();

                    auto& canvas = input.xmap;
                    if (damaged)
                    {
                        canvas.wipe(world.bell::id);
                        if (input.is_not_standalone_instance())
                        {
                            if (background) // Render active wallpaper.
                            {
                                canvas.render(background);
                            }

                            world.redraw(canvas); // Put the rest of the world on my canvas.
                        }
                        if (uibar && !fullscreen) // Render main menu/application.
                        {
                            //todo too hacky, unify
                            if (props.glow_fx) canvas.render(uibar, base::coor()); // Render the main menu twice to achieve the glow effect.
                                               canvas.render(uibar, base::coor());
                        }
                        if (legacy & os::legacy::mouse) // Render our mouse pointer.
                        {
                            draw_mouse_pointer(canvas);
                        }

                        if (!direct)
                        {
                            draw_clip_preview(canvas);
                        }

                        if (props.tooltip_enabled)
                        {
                            if (direct) send_tooltips(conio);
                            else        draw_tooltips(canvas);
                        }

                        if (debug)
                        {
                            debug.output(canvas);
                        }
                        if (props.show_regions)
                        {
                            canvas.each([](cell& c)
                            {
                                auto mark = rgba{ rgba::color256[c.link() % 256] };
                                auto bgc = c.bgc();
                                mark.alpha(64);
                                bgc.mix(mark);
                                c.bgc(bgc);
                            });
                        }
                    }
                    else if (yield) return;

                    // Note: We have to fire a mouse move event every frame,
                    //       because in the global frame the mouse can stand still,
                    //       but any form can move under the cursor, so for the form itself,
                    //       the mouse cursor moves inside the form.
                    if (debug)
                    {
                        debug.bypass = true;
                        input.fire(hids::events::mouse::move.id);
                        debug.bypass = faux;
                        yield = paint.commit(canvas);
                        if (yield)
                        {
                            auto d = paint.status();
                            debug.update(d.watch, d.delta);
                        }
                        debug.update(stamp);
                    }
                    else
                    {
                        input.fire(hids::events::mouse::move.id);
                        yield = paint.commit(canvas); // Try output my canvas to the my console.
                    }
                };
                // conio events.
                SUBMIT_T(tier::release, e2::conio::winsz, token, newsize)
                {
                    auto delta = base::resize(newsize);
                    if (delta && direct)
                    {
                        paint.cancel();
                        rebuild_scene(true);
                    }
                };
                SUBMIT_T(tier::release, e2::size::any, token, newsz)
                {
                    if (uibar) uibar->base::resize(newsz);
                    if (background) background->base::resize(newsz);
                };
                SUBMIT_T(tier::release, e2::conio::unknown, token, unkstate)
                {
                };
                SUBMIT_T(tier::release, e2::conio::native, token, extended)
                {
                    native = extended;
                };
                SUBMIT_T(tier::release, e2::conio::pointer, token, pointer)
                {
                    legacy |= pointer ? os::legacy::mouse : 0;
                };
                SUBMIT_T(tier::release, e2::conio::error, token, errcode)
                {
                    auto msg = text{ "\n\rgate: Term error: " } + std::to_string(errcode) + "\r\n";
                    log("gate: error byemsg: ", msg);
                    canal.stop();
                };
                SUBMIT_T(tier::release, e2::conio::quit, token, msg)
                {
                    log("gate: quit byemsg: ", msg);
                    canal.stop();
                };
                SUBMIT_T(tier::general, e2::conio::quit, token, msg)
                {
                    log("gate: global shutdown byemsg: ", msg);
                    canal.stop();
                };
                SUBMIT_T(tier::release, e2::form::quit, token, initiator)
                {
                    auto msg = ansi::add("gate: quit message from: ", initiator->id);
                    canal.stop();
                    this->SIGNAL(tier::general, e2::shutdown, msg);
                };
                //SUBMIT_T(tier::release, e2::form::state::header, token, newheader)
                //{
                //    text title;
                //    newheader.lyric->each([&](auto c) { title += c.txt(); });
                //    conio.output(ansi::header(title));
                //};
                SUBMIT_T(tier::release, e2::form::prop::ui::footer, token, newfooter)
                {
                    if (direct)
                    {
                        auto window_id = 0;
                        conio.form_footer.send(canal, window_id, newfooter);
                    }
                };
                SUBMIT_T(tier::release, e2::form::prop::ui::header, token, newheader)
                {
                    if (direct)
                    {
                        auto window_id = 0;
                        conio.form_header.send(canal, window_id, newheader);
                    }
                    else
                    {
                        auto temp = text{};
                        temp.reserve(newheader.length());
                        if (native)
                        {
                            temp = newheader;
                        }
                        else
                        {
                            para{ newheader }.lyric->each([&](auto c) { temp += c.txt(); });
                        }
                        log("gate: title changed to '", temp, ansi::nil().add("'"));
                        conio.output(ansi::header(temp));
                    }
                };
                SUBMIT_T(tier::general, e2::nextframe, token, damaged)
                {
                    rebuild_scene(damaged);
                };

                if (props.tooltip_enabled)
                {
                    SUBMIT_T(tier::general, e2::timer::any, token, now)
                    {
                        check_tooltips(now);
                    };
                }

                // Focus relay.
                SUBMIT_T(tier::release, hids::events::notify::focus::got, token, from_gear)
                {
                    auto myid = from_gear.id;
                    auto [ext_gear_id, gear_ptr] = input.get_foreign_gear_id(myid);
                    auto& gear = *gear_ptr;
                    auto state = gear.state();
                    gear.force_group_focus = true;
                    gear.kb_focus_changed = faux;
                    if (deskmenu) deskmenu->SIGNAL(tier::release, hids::events::upevent::kboffer, gear);
                    if (gear.focus_changed()) gear.dismiss();
                    gear.state(state);
                };
                SUBMIT_T(tier::release, hids::events::notify::focus::lost, token, from_gear)
                {
                    auto myid = from_gear.id;
                    auto [ext_gear_id, gear_ptr] = input.get_foreign_gear_id(myid);
                    auto& gear = *gear_ptr;
                    gear.kb_focus_changed = faux;
                    if (deskmenu) deskmenu->SIGNAL(tier::release, hids::events::upevent::kbannul, gear);
                };

                // Clipboard relay.
                SUBMIT_T(tier::release, hids::events::clipbrd::set, token, from_gear)
                {
                    auto myid = from_gear.id;
                    auto [ext_gear_id, gear_ptr] = input.get_foreign_gear_id(myid);
                    auto& gear =*gear_ptr;
                    auto& data = gear.clip_rawdata;
                    auto& size = gear.preview_size;
                    if (direct) conio.set_clipboard.send(canal, ext_gear_id, size, data.utf8, data.kind);
                    else        conio.output(ansi::clipbuf(data.kind, data.utf8)); // OSC 52
                };
                SUBMIT_T(tier::release, hids::events::clipbrd::get, token, from_gear)
                {
                    if (!direct) return;
                    auto myid = from_gear.id;
                    auto [ext_gear_id, gear_ptr] = input.get_foreign_gear_id(myid);
                    if (!conio.request_clip_data(ext_gear_id, gear_ptr->clip_rawdata))
                    {
                        log("gate: timeout: no clipboard data reply");
                    }
                };

                if (deskmenu)
                {
                    attach(deskmenu); // Our size could be changed here during attaching.
                    deskmenu->SUBMIT_T(tier::preview, hids::events::mouse::button::tplclick::leftright, token, gear)
                    {
                        if (debug)
                        {
                            props.show_regions = true;
                            debug.stop();
                        }
                        else
                        {
                            if (props.show_regions) props.show_regions = faux;
                            else                    debug.start();
                        }
                    };
                }
                if (bkground)
                {
                    ground(bkground);
                }

                auto forward_event = [&](hids& gear)
                {
                    auto deed = bell::protos<tier::release>();
                    auto [ext_gear_id, gear_ptr] = input.get_foreign_gear_id(gear.id);
                    conio.mouse_event.send(canal, ext_gear_id, deed, gear.coord);
                    gear.dismiss();
                };
                if (direct) // Forward unhandled events outside.
                {
                    SUBMIT_T(tier::anycast, e2::debug::request, token, count)
                    {
                        if (count > 0) conio.request_debug.send(conio);
                    };
                    SUBMIT_T(tier::release, e2::config::fps, token, fps)
                    {
                        if (fps > 0)
                        {
                            SIGNAL_GLOBAL(e2::config::fps, fps);
                        }
                    };
                    SUBMIT_T(tier::preview, e2::config::fps, token, fps)
                    {
                        conio.fps.send(conio, fps);
                    };
                    SUBMIT_T(tier::preview, hids::events::mouse::button::click::any, token, gear)
                    {
                        conio.expose.send(conio);
                    };
                    SUBMIT_T(tier::anycast, e2::form::layout::expose, token, item)
                    {
                        conio.expose.send(conio);
                    };
                    SUBMIT_T(tier::preview, e2::form::layout::swarp, token, warp)
                    {
                        conio.warping.send(conio, 0, warp);
                    };
                    SUBMIT_T(tier::release, e2::form::maximize, token, gear)
                    {
                        forward_event(gear);
                    };
                    SUBMIT_T(tier::release, hids::events::notify::keybd::test, token, from_gear)
                    {
                        auto [ext_gear_id, gear_ptr] = input.get_foreign_gear_id(from_gear.id);
                        from_gear.kb_focus_set ? conio.set_focus.send(conio, ext_gear_id, from_gear.combine_focus, from_gear.force_group_focus)
                                               : conio.off_focus.send(conio, ext_gear_id);
                    };
                    SUBMIT_T(tier::release, hids::events::notify::keybd::lost, token, from_gear)
                    {
                        auto [ext_gear_id, gear_ptr] = input.get_foreign_gear_id(from_gear.id);
                        conio.off_focus.send(conio, ext_gear_id);
                    };
                    SUBMIT_T(tier::release, hids::events::mouse::button::tplclick::any, token, gear)
                    {
                        forward_event(gear);
                    };
                    SUBMIT_T(tier::release, hids::events::mouse::button::dblclick::any, token, gear)
                    {
                        forward_event(gear);
                    };
                    SUBMIT_T(tier::release, hids::events::mouse::button::click::any, token, gear)
                    {
                        forward_event(gear);
                    };
                    SUBMIT_T(tier::release, hids::events::mouse::button::drag::start::any, token, gear)
                    {
                        forward_event(gear);
                    };
                    SUBMIT_T(tier::release, hids::events::mouse::button::drag::pull::any, token, gear)
                    {
                        forward_event(gear);
                    };
                }
                else
                {
                    input.check_focus();
                    conio.output(ansi::ext(true));
                    if (props.title.size())
                    {
                        conio.output(ansi::header(props.title));
                    }
                }

                SIGNAL(tier::anycast, e2::form::upon::started, This());

            lock.unlock();

            os::direct::pty::reading_loop(canal, [&](view data){ conio.sync(data); });

            lock.lock();
                log("link: signaling to close the read channel ", canal);
                SIGNAL(tier::release, e2::conio::quit, "link: read channel is closed");
                token.clear();
                mouse.reset(); // Reset active mouse clients to avoid hanging pointers.
                base::detach();
            lock.unlock();
        }

    protected:
        gate(host& world, conf const& client_props)//, bool is_standalone_app = faux)
            : world{ world }
        {
            limit.set(dot_11);
            props = client_props;
            //todo unify
            title.live = faux;
            input.set_instance_type(props.simple, props.is_standalone_app);
            if (!props.is_standalone_app)
            {
                mouse.draggable<sysmouse::leftright>(true);
                mouse.draggable<sysmouse::left>(true);
                SUBMIT(tier::release, e2::form::drag::start::any, gear)
                {
                    robot.pacify();
                };
                SUBMIT(tier::release, e2::form::drag::pull::any, gear)
                {
                    base::moveby(-gear.delta.get());
                    base::deface();
                };
                SUBMIT(tier::release, e2::form::drag::stop::any, gear)
                {
                    robot.pacify();
                    robot.actify(gear.fader<quadratic<twod>>(2s), [&](auto& x)
                                {
                                    base::moveby(-x);
                                    base::deface();
                                });
                };
                SUBMIT(tier::release, e2::form::layout::shift, newpos)
                {
                    auto viewport = e2::form::prop::viewport.param();
                    this->SIGNAL(tier::request, e2::form::prop::viewport, viewport);
                    auto oldpos = viewport.coor + (viewport.size / 2);

                    auto path = oldpos - newpos;
                    auto time = SWITCHING_TIME;
                    auto init = 0;
                    auto func = constlinearAtoB<twod>(path, time, init);

                    robot.pacify();
                    robot.actify(func, [&](auto& x)
                                       {
                                        base::moveby(-x);
                                        base::strike();
                                       });
                };
            }
            SUBMIT(tier::release, e2::form::prop::fullscreen, state)
            {
                fullscreen = state;
            };
            SUBMIT(tier::release, e2::form::prop::name, user_name)
            {
                uname = uname_txt = user_name;
            };
            SUBMIT(tier::request, e2::form::prop::name, user_name)
            {
                user_name = uname_txt;
            };
            SUBMIT(tier::request, e2::form::prop::viewport, viewport)
            {
                this->SIGNAL(tier::anycast, e2::form::prop::viewport, viewport);
                viewport.coor += base::coor();
            };
            //todo unify creation (delete simple create wo gear)
            SUBMIT(tier::preview, e2::form::proceed::create, region)
            {
                region.coor += base::coor();
                world.SIGNAL(tier::release, e2::form::proceed::create, region);
            };
            //todo revise
            SUBMIT(tier::preview, e2::form::proceed::createby, gear)
            {
                gear.slot.coor += base::coor();
                world.SIGNAL(tier::release, e2::form::proceed::createby, gear);
            };
            SUBMIT(tier::release, e2::form::proceed::onbehalf, proc)
            {
                //todo hids
                //proc(input.gear);
            };
            SUBMIT(tier::preview, hids::events::keybd::any, gear)
            {
                //todo unify
                auto keystrokes = gear.interpret();
                if (keystrokes == props.debug_toggle)
                {
                    debug ? debug.stop()
                          : debug.start();
                }
                //todo unify
                //if (gear.meta(hids::CTRL | hids::RCTRL))
                {
                    //todo unify
                    auto pgup = keystrokes == "\033[5;5~"s
                            || (keystrokes == "\033[5~"s && gear.meta(hids::anyCtrl));
                    auto pgdn = keystrokes == "\033[6;5~"s
                            || (keystrokes == "\033[6~"s && gear.meta(hids::anyCtrl));
                    if (pgup || pgdn)
                    {
                        auto item_ptr = e2::form::layout::goprev.param();
                        if (pgdn) world.SIGNAL(tier::request, e2::form::layout::goprev, item_ptr); // Take prev item
                        else      world.SIGNAL(tier::request, e2::form::layout::gonext, item_ptr); // Take next item

                        if (item_ptr)
                        {
                            auto& item = *item_ptr;
                            auto& area = item.area();
                            auto center = area.coor + (area.size / 2);
                            this->SIGNAL(tier::release, e2::form::layout::shift, center);
                            gear.pass_kb_focus(item);
                        }
                        gear.dismiss();
                    }
                }
            };
            SUBMIT(tier::preview, hids::events::mouse::button::click::leftright, gear)
            {
                if (gear.clear_clip_data())
                {
                    bell::template expire<tier::release>();
                    gear.dismiss();
                }
            };

            SUBMIT(tier::release, e2::render::prerender, parent_canvas)
            {
                if (&parent_canvas != &input.xmap) // Draw a shadow of user's terminal window for other users (spectators).
                {
                    auto area = base::area();
                    area.coor-= parent_canvas.area().coor;
                    //todo revise
                    auto mark = skin::color(tone::shadow);
                    mark.bga(mark.bga() / 2);
                    parent_canvas.fill(area, [&](cell& c){ c.fuse(mark); });
                }
                this->bell::expire<tier::release>(); // In order to disable base::render for gate.
            };
            SUBMIT(tier::release, e2::postrender, parent_canvas)
            {
                if (&parent_canvas != &input.xmap)
                {
                    //if (parent.test(area.coor))
                    //{
                    //	auto hover_id = parent[area.coor].link();
                    //	log ("---- hover id ", hover_id);
                    //}
                    //auto& header = *title.header().lyric;
                    if (uname.lyric) // Render foreign user names at their place.
                    {
                        draw_foreign_names(parent_canvas);
                    }
                    draw_mouse_pointer(parent_canvas);
                }
            };
        }
    };
}

#endif // NETXS_CONSOLE_HPP