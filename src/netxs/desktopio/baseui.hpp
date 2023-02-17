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
    using sysmouse = directvt::binary::sysmouse_t;
    using syskeybd = directvt::binary::syskeybd_t;
    using sysfocus = directvt::binary::sysfocus_t;
}
namespace netxs::ui
{
    class face;
    class base;

    using namespace netxs::input;
    using focus_test_t = std::pair<id_t, si32>;
    using gear_id_list_t = std::list<id_t>;
    using functor = std::function<void(sptr<base>)>;
    using proc = std::function<void(hids&)>;
    using s11n = directvt::binary::s11n;
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
            EVENT_XS( shutdown  , const view     ), // general: Server shutdown.
            GROUP_XS( extra     , si32           ), // Event extension slot.
            GROUP_XS( timer     , time           ), // timer tick, arg: current moment (now).
            GROUP_XS( render    , ui::face       ), // release: UI-tree rendering.
            GROUP_XS( conio     , si32           ),
            GROUP_XS( size      , twod           ), // release: Object size.
            GROUP_XS( coor      , twod           ), // release: Object coor.
            GROUP_XS( form      , bool           ),
            GROUP_XS( data      , si32           ),
            GROUP_XS( config    , si32           ), // set/notify/get/global_set configuration data.
            GROUP_XS( command   , si32           ), // exec UI command.

            SUBSET_XS( extra )
            {
                EVENT_XS( slot0, si32 ),
                EVENT_XS( slot1, si32 ),
                EVENT_XS( slot2, si32 ),
                EVENT_XS( slot3, si32 ),
            };
            SUBSET_XS( timer )
            {
                EVENT_XS( tick, time ), // relaese: execute before e2::timer::any (rendering)
            };
            SUBSET_XS( render ) // release any: UI-tree default rendering submission.
            {
                EVENT_XS( prerender, ui::face ), // release: UI-tree pre-rendering, used by pro::cache (can interrupt SIGNAL) and any kind of highlighters.
            };
            SUBSET_XS( size ) // preview: checking by pro::limit.
            {
                EVENT_XS( set, twod ), // preview: checking by object; release: apply to object; request: request object size.
            };
            SUBSET_XS( coor ) // preview any: checking by pro::limit.
            {
                EVENT_XS( set, twod ), // preview: checking by object; release: apply to object; request: request object coor.
            };
            SUBSET_XS( config )
            {
                EVENT_XS( creator, sptr<ui::base> ), // request: pointer to world object.
                EVENT_XS( fps    , si32           ), // request to set new fps, arg: new fps (si32); the value == -1 is used to request current fps.
                GROUP_XS( caret  , span           ), // any kind of intervals property.
                GROUP_XS( plugins, si32           ),

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
                EVENT_XS( error   , const si32      ), // release: return error code.
                EVENT_XS( winsz   , const twod      ), // release: order to update terminal primary overlay.
                EVENT_XS( preclose, const bool      ), // release: signal to quit after idle timeout, arg: bool - ready to shutdown.
                EVENT_XS( quit    , const text      ), // release: quit, arg: text - bye msg.
                EVENT_XS( pointer , const bool      ), // release: mouse pointer visibility.
                EVENT_XS( clipdata, ansi::clip      ), // release: OS clipboard update.
                EVENT_XS( logs    , const view      ), // logs output.
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
                EVENT_XS( quit       , const view  ), // return bye msg, arg: errcode.
                EVENT_XS( cout       , const text  ), // Append extra data to output.
                EVENT_XS( custom     , si32        ), // Custom command, arg: cmd_id.
                EVENT_XS( printscreen, input::hids ), // Copy screen area to clipboard.
            };
            SUBSET_XS( form )
            {
                EVENT_XS( canvas   , sptr<core>     ), // request global canvas.
                EVENT_XS( maximize , input::hids    ), // request to toggle maximize/restore.
                EVENT_XS( restore  , sptr<ui::base> ), // request to toggle restore.
                EVENT_XS( quit     , sptr<ui::base> ), // request parent for destroy.
                GROUP_XS( layout   , const twod     ),
                GROUP_XS( draggable, bool           ), // signal to the form to enable draggablity for specified mouse button.
                GROUP_XS( highlight, bool           ),
                GROUP_XS( upon     , bool           ),
                GROUP_XS( proceed  , bool           ),
                GROUP_XS( cursor   , bool           ),
                GROUP_XS( drag     , input::hids    ),
                GROUP_XS( prop     , text           ),
                GROUP_XS( global   , twod           ),
                GROUP_XS( state    , const twod     ),
                GROUP_XS( animate  , id_t           ),

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
                    EVENT_XS( shift , const twod    ), // request a global shifting  with delta.
                    EVENT_XS( convey, cube          ), // request a global conveying with delta (Inform all children to be conveyed).
                    EVENT_XS( bubble, ui::base      ), // order to popup the requested item through the visual tree.
                    EVENT_XS( expose, ui::base      ), // order to bring the requested item on top of the visual tree (release: ask parent to expose specified child; preview: ask child to expose itself).
                    EVENT_XS( appear, twod          ), // fly to the specified coords.
                    EVENT_XS( gonext, sptr<ui::base>), // request: proceed request for available objects (next)
                    EVENT_XS( goprev, sptr<ui::base>), // request: proceed request for available objects (prev)
                    EVENT_XS( swarp , const dent    ), // preview: form swarping
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
                    EVENT_XS( redrawn, ui::face       ), // inform about camvas is completely redrawn.
                    EVENT_XS( cached , ui::face       ), // inform about camvas is cached.
                    EVENT_XS( wiped  , ui::face       ), // event after wipe the canvas.
                    EVENT_XS( changed, twod           ), // event after resize, arg: diff bw old and new size.
                    EVENT_XS( dragged, input::hids    ), // event after drag.
                    EVENT_XS( created, input::hids    ), // release: notify the instance of who created it.
                    EVENT_XS( started, sptr<ui::base> ), // release: notify the instance is commissioned. arg: visual root.
                    GROUP_XS( vtree  , sptr<ui::base> ), // visual tree events, arg: parent base_sptr.
                    GROUP_XS( scroll , rack           ), // event after scroll.
                    //EVENT_XS( created    , sptr<ui::base> ), // event after itself creation, arg: itself bell_sptr.
                    //EVENT_XS( detached   , bell_sptr      ), // inform that subject is detached, arg: parent bell_sptr.
                    //EVENT_XS( invalidated, bool           ),
                    //EVENT_XS( moved      , twod           ), // release: event after moveto, arg: diff bw old and new coor twod. preview: event after moved by somebody.

                    SUBSET_XS( vtree )
                    {
                        EVENT_XS( attached, sptr<ui::base> ), // Child has been attached, arg: parent sptr<base>.
                        EVENT_XS( detached, sptr<ui::base> ), // Child has been detached, arg: parent sptr<base>.
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
                    EVENT_XS( create    , rect           ), // return coordinates of the new object placeholder.
                    EVENT_XS( createby  , input::hids    ), // return gear with coordinates of the new object placeholder gear::slot.
                    EVENT_XS( destroy   , ui::base       ), // ??? bool return reference to the parent.
                    EVENT_XS( render    , bool           ), // ask children to render itself to the parent canvas, arg is the world is damaged or not.
                    EVENT_XS( attach    , sptr<ui::base> ), // order to attach a child, arg is a parent base_sptr.
                    EVENT_XS( detach    , sptr<ui::base> ), // order to detach a child, tier::release - kill itself, tier::preview - detach the child specified in args, arg is a child sptr.
                    EVENT_XS( swap      , sptr<ui::base> ), // order to replace existing client. See tiling manager empty slot.
                    EVENT_XS( functor   , ui::functor    ), // exec functor (see pro::focus).
                    EVENT_XS( onbehalf  , ui::proc       ), // exec functor on behalf (see gate).
                    GROUP_XS( autofocus , input::hids    ), // release: restore the last foci state.
                    //EVENT_XS( focus      , sptr<ui::base>     ), // order to set focus to the specified object, arg is a object sptr.
                    //EVENT_XS( commit     , si32               ), // order to output the targets, arg is a frame number.
                    //EVENT_XS( multirender, vector<sptr<face>> ), // ask children to render itself to the set of canvases, arg is an array of the face sptrs.
                    //EVENT_XS( draw       , face               ), // ????  order to render itself to the canvas.
                    //EVENT_XS( checkin    , face_sptr          ), // order to register an output client canvas.

                    SUBSET_XS(autofocus)
                    {
                        EVENT_XS(take, input::hids),
                        EVENT_XS(lost, input::hids),
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
                    EVENT_XS( mouse , si32     ), // notify the client if mouse is active or not. The form is active when the number of clients (form::eventa::mouse::enter - mouse::leave) is not zero, only release, si32 - number of clients.
                    EVENT_XS( header, ui::para ), // notify the client has changed title.
                    EVENT_XS( footer, ui::para ), // notify the client has changed footer.
                    EVENT_XS( params, ui::para ), // notify the client has changed title params.
                    EVENT_XS( color , ui::tone ), // notify the client has changed tone, preview to set.
                    GROUP_XS( keybd , bool     ), // notify the client if keybd is active or not. The form is active when the number of clients (form::eventa::keybd::got - keybd::lost) is not zero, only release.

                    SUBSET_XS( keybd )
                    {
                        EVENT_XS( enlist  , ui::gear_id_list_t ), // anycast: Enumerate all available foci.
                        EVENT_XS( find    , ui::focus_test_t   ), // request: Check the focus.
                        EVENT_XS( check   , bool               ), // anycast: Check any focus.
                    };
                };
            };
        };
    };
}

namespace netxs::ui
{
    using e2 = netxs::events::userland::e2;

    //todo OMG!, make it in another way.
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
        cell menu_white;
        cell menu_black;

        twod bordersz = dot_11;
        si32 lucidity = 0xFF;

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
        svga cmode = svga::truecolor; // face: Color mode.

        // face: Print proxy something else at the specified coor.
        template<class T, class P>
        void output_proxy(T const& block, twod const& coord, P proxy)
        {
            flow::sync(block);
            flow::ac(coord);
            flow::compose<true>(block, proxy);
        }
        // face: Print something else at the specified coor.
        template<class T, class P = noop>
        void output(T const& block, twod const& coord, P printfx = P())
        {
            flow::sync(block);
            flow::ac(coord);
            flow::go(block, *this, printfx);
        }
        // face: Print something else.
        template<bool UseFWD = faux, class T, class P = noop>
        void output(T const& block, P printfx = P())
        {
            //todo unify
            flow::print<UseFWD>(block, *this, printfx);
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
        template<bool BottomAnchored = faux>
        void crop(twod const& newsize, cell const& c) // face: Resize while saving the bitmap.
        {
            core::crop<BottomAnchored>(newsize, c);
            flow::size(newsize);
        }
        template<bool BottomAnchored = faux>
        void crop(twod const& newsize) // face: Resize while saving the bitmap.
        {
            core::crop<BottomAnchored>(newsize, core::mark());
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
        // face: Render nested object to the canvas using renderproc. Trim = trim viewport to the client area.
        template<bool Trim = true, class T>
        void render(sptr<T> nested_ptr, twod const& basis = {})
        {
            if (nested_ptr)
            {
                auto& nested = *nested_ptr;
                face::render<Trim>(nested, basis);
            }
        }
        // face: Render nested object to the canvas using renderproc. Trim = trim viewport to the client area.
        template<bool Trim = true, class T>
        void render(T& nested, twod const& offset_coor)
        {
            auto canvas_view = core::view();
            auto parent_area = flow::full();

            auto object_area = nested.area();
            object_area.coor+= parent_area.coor;

            auto nested_view = canvas_view.clip(object_area);
            //todo revise: why whole canvas is not used
            if (Trim ? nested_view : canvas_view)
            {
                auto canvas_coor = core::coor();
                if constexpr (Trim) core::view(nested_view);
                core::back(offset_coor);
                flow::full(object_area);

                nested.SIGNAL(tier::release, e2::render::prerender, *this);
                nested.SIGNAL(tier::release, e2::postrender, *this);

                if constexpr (Trim) core::view(canvas_view);
                core::move(canvas_coor);
                flow::full(parent_area);
            }
        }
        // face: Render itself to the canvas using renderproc.
        template<bool Post = true, class T>
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
                if constexpr (Post) object.SIGNAL(tier::release, e2::postrender,        *this);

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
        hook cascade_token;
        si32 object_kind = {};

    public:
        hook kb_token;
        static constexpr auto reflow_root = si32{ -1 }; //todo unify

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
        template<bool Absolute = true>
        auto actual_area() const
        {
            auto area = rect{ -oversz.topleft(), square.size + oversz.summ() };
            if constexpr (Absolute) area.coor += square.coor;
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
        template<bool Forced = faux>
        void reflow()
        {
            auto parent_ptr = parent();
            if (parent_ptr && (!visual_root || (Forced && (kind() != base::reflow_root)))) //todo unify -- See basewindow in vtm.cpp
            {
                parent_ptr->reflow<Forced>();
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
        void global(twod& coor)
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
        // base: Initiate redrawing.
        virtual void redraw(face& canvas)
        {
            SIGNAL(tier::general, e2::shutdown, "base: rendering is not provided");
        }

    protected:
        virtual ~base() = default;
        base()
        {
            LISTEN(tier::request, e2::depth, depth) { depth++; };

            LISTEN(tier::release, e2::coor::any, new_coor) { square.coor = new_coor; };
            LISTEN(tier::request, e2::coor::set, coor_var) { coor_var = square.coor; };
            LISTEN(tier::release, e2::size::any, new_size) { square.size = new_size; };
            LISTEN(tier::request, e2::size::set, size_var) { size_var = square.size; };

            LISTEN(tier::release, e2::cascade, proc)
            {
                auto backup = This();
                auto keepon = proc(backup);
                if (!keepon) this->bell::expire<tier::release>();
            };
            LISTEN(tier::release, e2::form::upon::vtree::attached, parent_ptr)
            {
                if (!visual_root)
                {
                    parent_ptr->LISTEN(tier::release, e2::cascade, proc, cascade_token)
                    {
                        auto backup = This();
                        backup->SIGNAL(tier::release, e2::cascade, proc);
                    };
                }
                parent_shadow = parent_ptr;
                // Propagate form events up to the visual branch ends (children).
                // Exec after all subscriptions.
                //todo implement via e2::cascade
            };
            LISTEN(tier::release, e2::form::upon::vtree::any, parent_ptr)
            {
                if (this->bell::protos<tier::release>(e2::form::upon::vtree::detached))
                {
                    kb_token.reset();
                    cascade_token.reset();
                }
                if (parent_ptr) parent_ptr->base::reflow(); //todo too expensive
            };

            LISTEN(tier::release, e2::render::any, parent_canvas)
            {
                if (base::brush.wdt())
                {
                    parent_canvas.fill([&](cell& c) { c.fusefull(base::brush); });
                }
            };
        }
    };

    // console: Fullduplex channel base.
    struct pipe
    {
        using flux = std::ostream;
        using xipc = std::shared_ptr<pipe>;

        bool active; // pipe: Is connected.

        pipe(bool active)
            : active{ active }
        { }
        virtual ~pipe()
        { }
        operator bool () { return active; }

        virtual bool send(view buff) =0;
        virtual qiew recv(char* buff, size_t size) = 0;
        virtual qiew recv() = 0;
        virtual void shut() = 0;
        virtual void stop() = 0;
        virtual flux& show(flux& s) const = 0;
        void output(view data)
        {
            send(data);
        }
        friend auto& operator << (flux& s, pipe const& sock)
        {
            return sock.show(s << "{ xipc: ") << " }";
        }
        friend auto& operator << (flux& s, xipc const& sock)
        {
            return s << *sock;
        }
    };
}