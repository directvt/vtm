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
    class link;

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
            EVENT_XS( extra     , si32           ), // Event extension slot.
            GROUP_XS( timer     , time           ), // timer tick, arg: current moment (now).
            GROUP_XS( render    , ui::face       ), // release: UI-tree rendering.
            GROUP_XS( conio     , si32           ),
            GROUP_XS( size      , twod           ), // release: Object size.
            GROUP_XS( coor      , twod           ), // release: Object coor.
            GROUP_XS( form      , bool           ),
            GROUP_XS( data      , si32           ),
            GROUP_XS( config    , si32           ), // set/notify/get/global_set configuration data.
            GROUP_XS( command   , si32           ), // exec UI command.

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
                EVENT_XS( unknown , const si32      ), // release: return platform unknown event code.
                EVENT_XS( error   , const si32      ), // release: return error code.
                EVENT_XS( focus   , input::sysfocus ), // release: focus activity.
                EVENT_XS( mouse   , input::sysmouse ), // release: mouse activity.
                EVENT_XS( keybd   , input::syskeybd ), // release: keybd activity.
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
                EVENT_XS( changed, netxs::text       ), // release/preview/request: current menu item id(text).
                EVENT_XS( request, si32              ),
                EVENT_XS( disable, si32              ),
                EVENT_XS( flush  , si32              ),
                EVENT_XS( text   , const netxs::text ), // signaling with a text string, release only.
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
}

#include "system.hpp"

namespace netxs::ui
{
    using os::tty::xipc;

    // console: Client properties.
    struct conf
    {
        text ip;
        text port;
        text fullname;
        text region;
        text name;
        text os_user_id;
        text title;
        text selected;
        twod coor;
        span clip_preview_time;
        cell clip_preview_clrs;
        byte clip_preview_alfa;
        bool clip_preview_show;
        twod clip_preview_size;
        si32 clip_preview_glow;
        cell background_color;
        face background_image;
        si32 legacy_mode;
        si32 session_id;
        span dblclick_timeout; // conf: Double click timeout.
        span tooltip_timeout; // conf: Timeout for tooltip.
        cell tooltip_colors; // conf: Tooltip rendering colors.
        bool tooltip_enabled; // conf: Enable tooltips.
        bool glow_fx; // conf: Enable glow effect in main menu.
        bool debug_overlay; // conf: Enable to show debug overlay.
        text debug_toggle; // conf: Debug toggle shortcut.
        bool show_regions; // conf: Highlight region ownership.
        bool simple; // conf: Isn't it a directvt app.
        bool is_standalone_app; // conf: .

        void read(xmls& config)
        {
            config.cd("/config/client/");
            clip_preview_clrs = config.take("clipboard/preview", cell{}.bgc(bluedk).fgc(whitelt));
            clip_preview_time = config.take("clipboard/preview/timeout", span{ 3s });
            clip_preview_alfa = config.take("clipboard/preview/alpha", 0xFF);
            clip_preview_glow = config.take("clipboard/preview/shadow", 7);
            clip_preview_show = config.take("clipboard/preview/enabled", true);
            clip_preview_size = config.take("clipboard/preview/size", twod{ 80,25 });
            coor              = config.take("viewport/coor", dot_00); //todo Move user's viewport to the last saved position
            dblclick_timeout  = config.take("mouse/dblclick",  span{ 500ms });
            tooltip_colors    = config.take("tooltip", cell{}.bgc(0xFFffffff).fgc(0xFF000000));
            tooltip_timeout   = config.take("tooltip/timeout", span{ 500ms });
            tooltip_enabled   = config.take("tooltip/enabled", true);
            debug_overlay     = config.take("debug/overlay", faux);
            debug_toggle      = config.take("debug/toggle", "🐞"s);
            show_regions      = config.take("regions/enabled", faux);
            clip_preview_glow = std::clamp(clip_preview_glow, 0, 10);
        }

        conf() = default;
        conf(si32 mode, xmls& config)
            : session_id{ 0 },
              legacy_mode{ mode }
        {
            read(config);
            simple            = !(legacy_mode & os::vt::direct);
            glow_fx           = faux;
            is_standalone_app = true;
            title             = "";
        }
        conf(xipc peer, si32 session_id, xmls& config)
            : session_id{ session_id }
        {
            auto init = directvt::binary::startdata_t{};
            if (!init.load([&](auto... args){ return peer->recv(args...); }))
            {
                log("conf: init data corrupted");
            }
            config.fuse(init.conf);
            init.user = "[" + init.user + ":" + std::to_string(session_id) + "]";
            auto c_info = utf::divide(init.ip, " ");
            ip                = c_info.size() > 0 ? c_info[0] : text{};
            port              = c_info.size() > 1 ? c_info[1] : text{};
            legacy_mode       = init.mode;
            os_user_id        = init.user;
            fullname          = init.name;
            name              = init.user;
            title             = init.user;
            selected          = config.take("/config/menu/selected", ""s);
            read(config);
            background_color  = cell{}.fgc(config.take("background/fgc", rgba{ whitedk }))
                                      .bgc(config.take("background/bgc", rgba{ 0xFF000000 }));
            auto utf8_tile = config.take("background/tile", ""s);
            if (utf8_tile.size())
            {
                auto block = page{ utf8_tile };
                background_image.size(block.limits());
                background_image.output(block);
            }
            glow_fx           = config.take("glowfx", true);
            simple            = faux;
            is_standalone_app = faux;
        }

        friend auto& operator << (std::ostream& s, conf const& c)
        {
            return s << "\n\t    ip: " <<(c.ip.empty() ? text{} : (c.ip + ":" + c.port))
                     << "\n\tregion: " << c.region
                     << "\n\t  name: " << c.fullname
                     << "\n\t  user: " << c.os_user_id
                     << "\n\t  mode: " << os::vt::str(c.legacy_mode);
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

                socks(base& boss)
                {
                    boss.LISTEN(tier::general, hids::events::die, gear, token)
                    {
                        del(gear);
                    };
                }
                template<bool ConstWarn = true>
                auto& take(hids& gear)
                {
                    for (auto& item : items) // Linear search, because a few items.
                    {
                        if (item.id == gear.id) return item;
                    }

                    if constexpr (ConstWarn)
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
                  items{ boss          },
                  outer{ outer_rect    },
                  inner{ inner_rect    },
                  width{ outer - inner },
                  alive{ true          }
            {
                boss.LISTEN(tier::release, e2::config::plugins::sizer::alive, state, memo)
                {
                    alive = state;
                };
                boss.LISTEN(tier::release, e2::postrender, canvas, memo)
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
                boss.LISTEN(tier::release, e2::form::layout::swarp, warp, memo)
                {
                    auto area = boss.base::area();
                    auto next = area + warp;
                    auto step = boss.extend(next);
                };
                boss.LISTEN(tier::release, hids::events::notify::mouse::enter, gear, memo)
                {
                    items.add(gear);
                };
                boss.LISTEN(tier::release, hids::events::notify::mouse::leave, gear, memo)
                {
                    items.dec(gear);
                };
                boss.LISTEN(tier::release, e2::config::plugins::sizer::outer, outer_rect, memo)
                {
                    outer = outer_rect;
                    width = outer - inner;
                };
                boss.LISTEN(tier::release, e2::config::plugins::sizer::inner, inner_rect, memo)
                {
                    inner = inner_rect;
                    width = outer - inner;
                };
                boss.LISTEN(tier::request, e2::config::plugins::sizer::inner, inner_rect, memo)
                {
                    inner_rect = inner;
                };
                boss.LISTEN(tier::request, e2::config::plugins::sizer::outer, outer_rect, memo)
                {
                    outer_rect = outer;
                };

                engage<hids::buttons::left>();
                engage<hids::buttons::leftright>();
            }
            // pro::sizer: Configuring the mouse button to operate.
            template<hids::buttons Button>
            void engage()
            {
                boss.SIGNAL(tier::release, e2::form::draggable::_<Button>, true);
                boss.LISTEN(tier::release, hids::events::mouse::move, gear, memo)
                {
                    if (items.take(gear).calc(boss, gear.coord, outer, inner, width))
                    {
                        boss.base::deface(); // Deface only if mouse moved.
                    }
                };
                boss.LISTEN(tier::release, e2::form::drag::start::_<Button>, gear, memo)
                {
                    if (items.take(gear).grab(boss, gear.coord, outer))
                    {
                        gear.dismiss();
                    }
                };
                boss.LISTEN(tier::release, e2::form::drag::pull::_<Button>, gear, memo)
                {
                    if (items.take(gear).drag(boss, gear.coord, outer))
                    {
                        gear.dismiss();
                    }
                };
                boss.LISTEN(tier::release, e2::form::drag::cancel::_<Button>, gear, memo)
                {
                    items.take(gear).drop();
                };
                boss.LISTEN(tier::release, e2::form::drag::stop::_<Button>, gear, memo)
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
                  items{ boss },
                  dest_shadow{ subject }
            {
                boss.LISTEN(tier::release, hids::events::notify::mouse::enter, gear, memo)
                {
                    items.add(gear);
                };
                boss.LISTEN(tier::release, hids::events::notify::mouse::leave, gear, memo)
                {
                    items.dec(gear);
                };
                engage<hids::buttons::left>();
            }
            mover(base& boss)
                : mover{ boss, boss.This() }
            { }
            // pro::mover: Configuring the mouse button to operate.
            template<hids::buttons Button>
            void engage()
            {
                boss.SIGNAL(tier::release, e2::form::draggable::_<Button>, true);
                boss.LISTEN(tier::release, e2::form::drag::start::_<Button>, gear, memo)
                {
                    if ((dest_object = dest_shadow.lock()))
                    {
                        items.take(gear).grab(*dest_object, gear.coord);
                        gear.dismiss();
                    }
                };
                boss.LISTEN(tier::release, e2::form::drag::pull::_<Button>, gear, memo)
                {
                    if (dest_object)
                    {
                        items.take(gear).drag(*dest_object, gear.coord);
                        auto delta = gear.delta.get();
                        dest_object->SIGNAL(tier::preview, e2::form::upon::changed, delta);
                        gear.dismiss();
                    }
                };
                boss.LISTEN(tier::release, e2::form::drag::cancel::_<Button>, gear, memo)
                {
                    if (dest_object)
                    {
                        dest_object.reset();
                        gear.dismiss();
                    }
                };
                boss.LISTEN(tier::release, e2::form::drag::stop::_<Button>, gear, memo)
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
                  items{ boss },
                  alive{ true }
            {
                boss.LISTEN(tier::anycast, e2::form::prop::lucidity, lucidity, memo)
                {
                    if (lucidity != -1) alive = lucidity == 0xFF;
                };
                boss.LISTEN(tier::release, hids::events::mouse::move, gear, memo)
                {
                    items.take(gear).calc(boss, gear.coord);
                };
                boss.LISTEN(tier::release, hids::events::notify::mouse::enter, gear, memo)
                {
                    items.add(gear);
                };
                boss.LISTEN(tier::release, hids::events::notify::mouse::leave, gear, memo)
                {
                    items.dec(gear);
                };
                boss.LISTEN(tier::release, e2::render::prerender, parent_canvas, memo)
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
                boss.LISTEN(tier::release, e2::config::plugins::align, set, memo)
                {
                    if (set)
                    {
                        boss.LISTEN(tier::release, e2::form::maximize, gear, maxs)
                        {
                            if (seized(gear.owner.id)) unbind();
                            else                       follow(gear.owner.id, dot_00);
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

                    gate.LISTEN(tier::release, e2::size::any, size, memo)
                    {
                        body.size = size + pads * 2;
                        boss.base::resize(body.size);
                    };
                    gate.LISTEN(tier::release, e2::coor::any, coor, memo)
                    {
                        unbind();
                    };
                    gate.LISTEN(tier::release, e2::dtor, master_id, memo)
                    {
                        unbind();
                    };

                    boss.LISTEN(tier::release, e2::size::any, size, memo)
                    {
                        if (weak && body.size != size) unbind(faux);
                    };
                    boss.LISTEN(tier::release, e2::coor::any, coor, memo)
                    {
                        if (weak && body.coor != coor) unbind(true, faux);
                    };

                    weak = master;
                    boss.LISTEN(tier::release, e2::form::prop::ui::header, newhead, memo)
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
                auto init = datetime::now();
                boss.LISTEN(tier::general, e2::timer::any, p, memo[ID], (ID, proc, flow, init))
                {
                    auto now = datetime::round<si32>(p - init);
                    if (auto data = flow(now))
                    {
                        static constexpr auto zero = std::decay_t<decltype(data.value())>{};
                        auto& v = data.value();
                        if (v != zero) proc(v);
                    }
                    else
                    {
                        pacify(ID);
                    }
                };
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
            void actify(id_t ID, span timeout, P lambda)
            {
                auto alarm = datetime::now() + timeout;
                boss.LISTEN(tier::general, e2::timer::any, now, memo[ID], (ID, timeout, lambda, alarm))
                {
                    if (now > alarm)
                    {
                        alarm = now + timeout;
                        if (!lambda(ID)) pacify(ID);
                    }
                };
            }
            // pro::timer: Start countdown.
            template<class P>
            void actify(span timeout, P lambda)
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
                boss.LISTEN(tier::release, e2::form::upon::vtree::attached, parent, memo)
                {
                    parent->LISTEN(tier::preview, e2::form::global::lucidity, alpha, link)
                    {
                        boss.SIGNAL(tier::preview, e2::form::global::lucidity, alpha);
                    };
                    parent->LISTEN(tier::preview, e2::form::layout::convey, convey_data, link)
                    {
                        convey(convey_data.delta, convey_data.stuff);
                    };
                    parent->LISTEN(tier::preview, e2::form::layout::shift, delta, link)
                    {
                        //boss.base::coor += delta;
                        boss.moveby(delta);
                    };
                    parent->LISTEN(tier::preview, e2::form::upon::vtree::detached, p, link)
                    {
                        frame::link.clear();
                    };
                    boss.SIGNAL(tier::release, e2::form::prop::zorder, seat);
                };
                boss.LISTEN(tier::preview, e2::form::prop::zorder, order)
                {
                    seat = order;
                    boss.SIGNAL(tier::release, e2::form::prop::zorder, seat);
                };
                boss.LISTEN(tier::preview, e2::form::layout::expose, boss, memo)
                {
                    expose();
                };
                boss.LISTEN(tier::preview, hids::events::mouse::button::click::left, gear, memo)
                {
                    expose();
                };
                boss.LISTEN(tier::preview, hids::events::mouse::button::click::right, gear, memo)
                {
                    expose();
                };
                boss.LISTEN(tier::preview, e2::form::layout::appear, newpos, memo)
                {
                    appear(newpos);
                };
                //boss.LISTEN(tier::preview, e2::form::upon::moved, delta, memo)
                //{
                //    bubble();
                //};
                boss.LISTEN(tier::preview, e2::form::upon::changed, delta, memo)
                {
                    bubble();
                };
                boss.LISTEN(tier::preview, hids::events::mouse::button::down::any, gear, memo)
                {
                    robo.pacify();
                };
                boss.LISTEN(tier::release, e2::form::drag::pull::any, gear, memo)
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
                boss.LISTEN(tier::release, e2::form::upon::dragged, gear, memo)
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
                boss.LISTEN(tier::release, hids::events::mouse::button::click::right, gear, memo)
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
                auto time = skin::globals().switching;
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
                        gear.slot.coor += boss.base::coor();
                        gear.slot_forced = true;
                        boss.RISEUP(tier::request, e2::form::proceed::createby, gear);
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

                boss.LISTEN(tier::preview, hids::events::keybd::any, gear, memo)
                {
                    if (gear.captured(boss.bell::id)) check_modifiers(gear);
                };

                //todo unify - args... + template?
                //middle button
                boss.LISTEN(tier::preview, drag::start::middle, gear, memo)
                {
                    handle_init(gear);
                };
                boss.LISTEN(tier::release, drag::pull::middle, gear, memo)
                {
                    handle_pull(gear);
                };
                boss.LISTEN(tier::release, drag::cancel::middle, gear, memo)
                {
                    handle_drop(gear);
                };
                boss.LISTEN(tier::release, drag::stop::middle, gear, memo)
                {
                    handle_stop(gear);
                };

                //todo unify
                //right button
                boss.LISTEN(tier::release, drag::start::right, gear, memo)
                {
                    handle_init(gear);
                };
                boss.LISTEN(tier::release, drag::pull::right, gear, memo)
                {
                    handle_pull(gear);
                };
                boss.LISTEN(tier::release, drag::cancel::right, gear, memo)
                {
                    handle_drop(gear);
                };
                boss.LISTEN(tier::release, drag::stop::right, gear, memo)
                {
                    handle_stop(gear);
                };

                boss.LISTEN(tier::general, hids::events::halt, gear, memo)
                {
                    handle_drop(gear);
                };

                boss.LISTEN(tier::release, e2::postrender, canvas, memo)
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

            subs conf; // caret: Configuration subscriptions.
            bool live; // caret: Should the caret be drawn.
            bool done; // caret: Is the caret already drawn.
            bool down; // caret: Is the caret suppressed (lost focus).
            bool form; // caret: Caret style.
            rect body; // caret: Caret position.
            span step; // caret: Blink interval. span::zero() if steady.
            time next; // caret: Time of next blinking.

        public:
            caret(base&&) = delete;
            caret(base& boss, bool visible = faux, bool abox = faux, twod position = dot_00, span freq = skin::globals().blink_period)
                : skill{ boss },
                   live{ faux },
                   done{ faux },
                   down{ true },
                   form{ abox },
                   body{ position, dot_11 }, // Caret is always one cell size (see the term::scrollback definition).
                   step{ freq }
            {
                boss.LISTEN(tier::anycast, e2::form::highlight::any, state, conf)
                {
                    down = !state;
                };
                boss.LISTEN(tier::request, e2::config::caret::blink, req_step, conf)
                {
                    req_step = step;
                };
                boss.LISTEN(tier::request, e2::config::caret::style, req_style, conf)
                {
                    req_style = form ? 1 : 0;
                };
                boss.LISTEN(tier::general, e2::config::caret::blink, new_step, conf)
                {
                    blink_period(new_step);
                };
                boss.LISTEN(tier::preview, e2::config::caret::blink, new_step, conf)
                {
                    blink_period(new_step);
                };
                boss.LISTEN(tier::general, e2::config::caret::style, new_style, conf)
                {
                    style(new_style);
                };
                boss.LISTEN(tier::preview, e2::config::caret::style, new_style, conf)
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
            void blink_period(span const& new_step = skin::globals().blink_period)
            {
                auto changed = (step == span::zero()) != (new_step == span::zero());
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
                        blink_period(span::zero());
                        style(true);
                        break;
                    case 3: // n = 3  blinking underline
                        blink_period();
                        style(faux);
                        break;
                    case 4: // n = 4  steady underline
                        blink_period(span::zero());
                        style(faux);
                        break;
                    case 5: // n = 5  blinking I-bar
                        blink_period();
                        style(true);
                        break;
                    case 6: // n = 6  steady I-bar
                        blink_period(span::zero());
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
                if (step != span::zero())
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
                    live = step == span::zero();
                    if (!live)
                    {
                        boss.LISTEN(tier::general, e2::timer::tick, timestamp, memo)
                        {
                            if (timestamp > next)
                            {
                                next = timestamp + step;
                                live = !live;
                                boss.deface(body);
                            }
                        };
                    }
                    boss.LISTEN(tier::release, e2::postrender, canvas, memo)
                    {
                        done = live;
                        auto state = down ? (step == span::zero() ? faux : true)
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
            X(mouse_btn_3  , "middle button"    ) \
            X(mouse_btn_4  , "4th button"       ) \
            X(mouse_btn_5  , "5th button"       ) \
            X(mouse_btn_6  , "left+right combo" ) \
            X(last_event   , "event"            )

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
                span render = span::zero();
                span output = span::zero();
                si32 frsize = 0;
                si64 totals = 0;
                si32 number = 0;    // info: Current frame number
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
            void update(span const& watch, si32 delta)
            {
                track.output = watch;
                track.frsize = delta;
                track.totals+= delta;
            }
            void update(time const& timestamp)
            {
                track.render = datetime::now() - timestamp;
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

                boss.LISTEN(tier::general, e2::config::fps, fps, memo)
                {
                    status[prop::frame_rate].set(stress) = std::to_string(fps);
                    boss.base::strike();
                };
                {
                    auto fps = e2::config::fps.param(-1);
                    boss.SIGNAL(tier::general, e2::config::fps, fps);
                }
                boss.LISTEN(tier::release, e2::conio::focus, focusstate, memo)
                {
                    update(focusstate.enabled);
                    boss.base::strike();
                };
                boss.LISTEN(tier::release, e2::size::any, newsize, memo)
                {
                    update(newsize);
                };

                boss.LISTEN(tier::release, e2::conio::mouse, m, memo)
                {
                    if (bypass) return;
                    shadow();
                    status[prop::last_event].set(stress) = "mouse";
                    status[prop::mouse_pos ].set(stress) =
                        (m.coordxy.x < 10000 ? std::to_string(m.coordxy.x) : "-") + " : " +
                        (m.coordxy.y < 10000 ? std::to_string(m.coordxy.y) : "-") ;

                    auto m_buttons = std::bitset<8>(m.buttons);
                    for (auto i = 0; i < hids::numofbuttons; i++)
                    {
                        auto& state = status[prop::mouse_btn_1 + i].set(stress);
                        state = m_buttons[i] ? "pressed" : "idle   ";
                    }

                    status[prop::mouse_wheeldt].set(stress) = m.wheeldt ? std::to_string(m.wheeldt) :  " -- "s;
                    status[prop::mouse_hzwheel].set(stress) = m.hzwheel ? "active" : "idle  ";
                    status[prop::mouse_vtwheel].set(stress) = m.wheeled ? "active" : "idle  ";
                    status[prop::ctrl_state   ].set(stress) = "0x" + utf::to_hex(m.ctlstat);
                };
                boss.LISTEN(tier::release, e2::conio::keybd, k, memo)
                {
                    shadow();
                    status[prop::last_event   ].set(stress) = "keybd";
                    status[prop::key_pressed  ].set(stress) = k.pressed ? "pressed" : "idle";
                    status[prop::ctrl_state   ].set(stress) = "0x" + utf::to_hex(k.ctlstat );
                    status[prop::key_code     ].set(stress) = "0x" + utf::to_hex(k.virtcod );
                    status[prop::key_scancode ].set(stress) = "0x" + utf::to_hex(k.scancod );

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

                boss.LISTEN(tier::release, e2::conio::error, e, memo)
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
                boss.LISTEN(tier::release, e2::size::any, new_size, memo)
                {
                    recalc(new_size);
                };
                boss.LISTEN(tier::release, e2::postrender, canvas, memo)
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
                    boss.LISTEN(tier::preview, e2::form::prop::ui::header, newtext, memo)
                    {
                        header(newtext);
                    };
                    boss.LISTEN(tier::request, e2::form::prop::ui::header, curtext, memo)
                    {
                        curtext = head_text;
                    };
                }
                if (foot_live)
                {
                    boss.LISTEN(tier::preview, e2::form::prop::ui::footer, newtext, memo)
                    {
                        footer(newtext);
                    };
                    boss.LISTEN(tier::request, e2::form::prop::ui::footer, curtext, memo)
                    {
                        curtext = foot_text;
                    };
                }
                /*
                boss.LISTEN(tier::request, e2::form::state::header, caption, memo)
                {
                    caption = header();
                };
                boss.LISTEN(tier::request, e2::form::state::footer, caption, memo)
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

            static constexpr auto QUIT_MSG = e2::conio::quit;
            static constexpr auto ESC_THRESHOLD = si32{ 500 }; // guard: Double escape threshold in ms.

            bool wait; // guard: Ready to close.
            time stop; // guard: Timeout for single Esc.
            text desc = "exit after preclose";

        public:
            guard(base&&) = delete;
            guard(base& boss) : skill{ boss },
                wait{ faux }
            {
                // Suspected early completion.
                boss.LISTEN(tier::release, e2::conio::preclose, pre_close, memo)
                {
                    if ((wait = pre_close))
                    {
                        stop = datetime::now() + std::chrono::milliseconds(ESC_THRESHOLD);
                    }
                };

                // Double escape catcher.
                boss.LISTEN(tier::general, e2::timer::any, timestamp, memo)
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

            static constexpr auto EXCUSE_MSG = hids::events::mouse::any;
            static constexpr auto QUIT_MSG   = e2::shutdown;
            static constexpr auto LIMIT = 60 * 10; //todo unify // watch: Idle timeout in seconds.

            hook pong; // watch: Alibi subsciption token.
            hook ping; // watch: Zombie check countdown token.
            time stop; // watch: Timeout for zombies.
            text desc = "no mouse clicking events";

        public:
            watch(base&&) = delete;
            watch(base& boss) : skill{ boss }
            {
                stop = datetime::now() + std::chrono::seconds(LIMIT);

                // No mouse events watchdog.
                boss.LISTEN(tier::preview, EXCUSE_MSG, something, pong)
                {
                    stop = datetime::now() + std::chrono::seconds(LIMIT);
                };
                boss.LISTEN(tier::general, e2::timer::any, something, ping)
                {
                    if (datetime::now() > stop)
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
            //todo foci
            //std::list<id_t> saved;

        public:
            keybd(base&&) = delete;
            keybd(base& boss) : skill{ boss }
            {
                boss.LISTEN(tier::preview, hids::events::keybd::any, gear, memo)
                {
                    boss.SIGNAL(tier::release, hids::events::keybd::any, gear);
                };
            };

            // pro::keybd: Keybd offers promoter.
            void active()
            {
                boss.LISTEN(tier::release, hids::events::mouse::button::any, gear, kb_subs)
                {
                    if (!gear) return;
                    auto deed = boss.bell::protos<tier::release>();
                    if (deed == hids::events::mouse::button::click::left.id) //todo make it configurable (left click)
                    {
                        if (gear.meta(hids::anyCtrl)) gear.kb_offer_2(boss);
                        else                          gear.kb_offer_10(boss.This());
                        gear.dismiss();
                    }
                    else if (deed == hids::events::mouse::button::click::right.id) //todo make it configurable (left click)
                    {
                        gear.kb_offer_2(boss);
                        gear.dismiss();
                    }
                };
            }
            //todo foci
            // pro::keybd: Set focus root.
            //void master()
            //{
            //    boss.LISTEN(tier::release, hids::events::upevent::kboffer, gear, kb_subs)
            //    {
            //        log("restore");
            //        //if (boss.root()) // Restore focused state.
            //        {
            //            boss.SIGNAL(tier::anycast, hids::events::upevent::kboffer, gear);
            //        }
            //        if (gear.focus_changed())
            //        {
            //            boss.bell::expire<tier::release>();
            //        }
            //    };
            //};
            // pro::keybd: Subscribe on keybd offers.
            void accept(bool value)
            {
                if (value)
                {
                    active();
                    boss.LISTEN(tier::release, hids::events::upevent::kboffer, gear, kb_subs)
                    {
                        if (!gear.focus_changed())
                        {
                            gear.set_kb_focus(boss.This());
                            //todo foci
                            //boss.SIGNAL(tier::anycast, hids::events::upevent::kbannul, gear); // Drop saved foci.
                            boss.bell::expire<tier::release>();
                        }
                    };
                    boss.LISTEN(tier::release, hids::events::upevent::kbannul, gear, kb_subs)
                    {
                        gear.remove_from_kb_focus(boss.This());
                    };

                    ////todo foci
                    //boss.LISTEN(tier::anycast, hids::events::upevent::kboffer, gear, kb_subs) //todo no upevent used
                    //{
                    //    log("restore in place boss-id=", boss.id, " gear_id=", gear.id, " saved_size=", saved.size());
                    //    for (auto gear_id : saved) // Restore saved focus.
                    //    {
                    //        if (gear_id == gear.id)
                    //        {
                    //            log(" good ");
                    //            gear.kb_offer_2(boss);
                    //        }
                    //    }
                    //};
                    //boss.LISTEN(tier::preview, hids::events::notify::keybd::lost, gear, kb_subs) //todo no upevent used
                    //{
                    //    log("save boss.id=", boss.id, " gear_id=", gear.id);
                    //    saved.push_back(gear.id);
                    //};
                    //boss.LISTEN(tier::anycast, hids::events::upevent::kbannul, gear, kb_subs) //todo no upevent used
                    //{
                    //    if (gear.force_group_focus = faux)
                    //    {
                    //        log("wipe ", boss.id);
                    //        saved.remove_if([&](auto&& gear_id) { return gear_id == gear.id; });
                    //    }
                    //};
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
                boss.LISTEN(tier::release, e2::form::upon::vtree::attached, parent_ptr)
                {
                    //todo kb
                    parent_ptr->LISTEN(tier::release, hids::events::upevent::any, gear, boss.kb_token)
                    {
                        if (auto parent_ptr = boss.parent())
                        {
                            if (gear.focus_changed()) //todo unify, upevent::kbannul using it
                            {
                                parent_ptr->bell::expire<tier::release>();
                            }
                            else
                            {
                                if (auto deed = parent_ptr->bell::protos<tier::release>())
                                {
                                    boss.bell::signal<tier::release>(deed, gear);
                                }
                            }
                        }
                    };
                };
                // pro::mouse: Propagate form events down to the visual branch. Executed last.
                boss.LISTEN(tier::release, hids::events::notify::any, gear)
                {
                    if (auto parent_ptr = boss.parent())
                    if (auto deed = boss.bell::protos<tier::release>())
                    {
                        parent_ptr->bell::signal<tier::release>(deed, gear);
                    }
                };
                // pro::mouse: Forward preview to all parents.
                boss.LISTEN(tier::preview, hids::events::mouse::any, gear, memo)
                {
                    auto& offset = boss.base::coor();
                    gear.pass<tier::preview>(boss.parent(), offset);

                    if (gear) gear.okay(boss);
                    else      boss.bell::expire<tier::preview>();
                };
                // pro::mouse: Forward all not expired mouse events to all parents.
                boss.LISTEN(tier::release, hids::events::mouse::any, gear, memo)
                {
                    if (gear && !gear.captured())
                    {
                        auto& offset = boss.base::coor();
                        gear.pass<tier::release>(boss.parent(), offset);
                    }
                };
                // pro::mouse: Notify form::state::active when the number of clients is positive.
                boss.LISTEN(tier::release, hids::events::notify::mouse::enter, gear, memo)
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
                boss.LISTEN(tier::release, hids::events::notify::mouse::leave, gear, memo)
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
                boss.LISTEN(tier::request, e2::form::state::mouse, state, memo)
                {
                    state = rent;
                };
                boss.LISTEN(tier::release, e2::form::draggable::any, enabled, memo)
                {
                    switch (auto deed = boss.bell::protos<tier::release>())
                    {
                        default:
                        case e2::form::draggable::left     .id: draggable<hids::buttons::left     >(enabled); break;
                        case e2::form::draggable::right    .id: draggable<hids::buttons::right    >(enabled); break;
                        case e2::form::draggable::middle   .id: draggable<hids::buttons::middle   >(enabled); break;
                        case e2::form::draggable::wheel    .id: draggable<hids::buttons::wheel    >(enabled); break;
                        case e2::form::draggable::win      .id: draggable<hids::buttons::win      >(enabled); break;
                        case e2::form::draggable::leftright.id: draggable<hids::buttons::leftright>(enabled); break;
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
            template<hids::buttons Button>
            void draggable(bool enabled)
            {
                if (!enabled)
                {
                    dragmemo[Button].clear();
                    drag &= ~(1 << Button);
                }
                else if (!(drag & 1 << Button))
                {
                    drag |= 1 << Button;
                    //using bttn = hids::events::mouse::button; //MSVC 16.9.4 don't get it
                    boss.LISTEN(tier::release, hids::events::mouse::button::drag::start::_<Button>, gear, dragmemo[Button])
                    {
                        if (gear.capture(boss.bell::id))
                        {
                            boss.SIGNAL(tier::release, e2::form::drag::start::_<Button>, gear);
                            gear.dismiss();
                        }
                    };
                    boss.LISTEN(tier::release, hids::events::mouse::button::drag::pull::_<Button>, gear, dragmemo[Button])
                    {
                        if (gear.captured(boss.bell::id))
                        {
                            boss.SIGNAL(tier::release, e2::form::drag::pull::_<Button>, gear);
                            gear.dismiss();
                        }
                    };
                    boss.LISTEN(tier::release, hids::events::mouse::button::drag::cancel::_<Button>, gear, dragmemo[Button])
                    {
                        if (gear.captured(boss.bell::id))
                        {
                            boss.SIGNAL(tier::release, e2::form::drag::cancel::_<Button>, gear);
                            gear.setfree();
                            gear.dismiss();
                        }
                    };
                    boss.LISTEN(tier::general, hids::events::halt, gear, dragmemo[Button])
                    {
                        if (gear.captured(boss.bell::id))
                        {
                            boss.SIGNAL(tier::release, e2::form::drag::cancel::_<Button>, gear);
                            gear.setfree();
                            gear.dismiss();
                        }
                    };
                    boss.LISTEN(tier::release, hids::events::mouse::button::drag::stop::_<Button>, gear, dragmemo[Button])
                    {
                        if (gear.captured(boss.bell::id))
                        {
                            boss.SIGNAL(tier::release, e2::form::drag::stop::_<Button>, gear);
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
                conf& props;
                clip clip_rawdata{}; // topgear: Clipboard data.
                face clip_preview{}; // topgear: Clipboard preview render.
                bool not_directvt{}; // topgear: Is it the top level gear (not directvt).

                template<class ...Args>
                topgear(conf& props, bool not_directvt, Args&&... args)
                    : hids{ std::forward<Args>(args)... },
                      props{ props },
                      not_directvt{ not_directvt }
                { }

                bool clear_clip_data() override
                {
                    auto not_empty = !!clip_rawdata.utf8.size();
                    clip_rawdata.clear();
                    owner.SIGNAL(tier::release, hids::events::clipbrd::set, *this);
                    if (not_directvt)
                    {
                        clip_preview.size(clip_rawdata.size);
                    }
                    return not_empty;
                }
                void set_clip_data(clip const& data, bool forward = true) override
                {
                    clip_rawdata.set(data);
                    if (not_directvt)
                    {
                        auto clip_shadow_size = props.clip_preview_glow;
                        auto draw_shadow = [&](auto& block)
                        {
                            clip_preview.mark(cell{});
                            clip_preview.wipe();
                            clip_preview.size(dot_21 * clip_shadow_size * 2 + clip_rawdata.size);
                            auto full = rect{ dot_21 * clip_shadow_size + dot_21, clip_rawdata.size };
                            while (clip_shadow_size--)
                            {
                                clip_preview.reset();
                                clip_preview.full(full);
                                clip_preview.output(block, cell::shaders::color(cell{}.bgc(0).fgc(0).alpha(0x60)));
                                clip_preview.blur(1, [&](cell& c) { c.fgc(c.bgc()).txt(""); });
                            }
                            full.coor -= dot_21;
                            clip_preview.reset();
                            clip_preview.full(full);
                        };
                        if (clip_rawdata.kind == clip::safetext)
                        {
                            auto blank = ansi::bgc(0x7Fffffff).fgc(0xFF000000).add(" Protected Data "); //todo unify (i18n)
                            auto block = page{ blank };
                            clip_rawdata.size = block.current().size();
                            if (clip_shadow_size) draw_shadow(block);
                            else
                            {
                                clip_preview.size(clip_rawdata.size);
                                clip_preview.wipe();
                            }
                            clip_preview.output(block);
                        }
                        else
                        {
                            auto block = page{ clip_rawdata.utf8 };
                            if (clip_shadow_size) draw_shadow(block);
                            else
                            {
                                clip_preview.size(clip_rawdata.size);
                                clip_preview.wipe();
                            }
                            clip_preview.mark(cell{});
                            if (clip_rawdata.kind == clip::textonly) clip_preview.output(block, cell::shaders::color(  props.clip_preview_clrs));
                            else                                     clip_preview.output(block, cell::shaders::xlucent(props.clip_preview_alfa));
                        }
                    }
                    if (forward) owner.SIGNAL(tier::release, hids::events::clipbrd::set, *this);
                    mouse::delta.set(); // Update time stamp.
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

            conf& props;

            template<class T>
            void forward(T& device)
            {
                auto gear_it = gears.find(device.gear_id);
                if (gear_it == gears.end())
                {
                    gear_it = gears.emplace(device.gear_id, bell::create<topgear>(props, device.gear_id == 0, boss, xmap, props.dblclick_timeout, props.tooltip_timeout, props.simple)).first;
                }
                auto& [_id, gear_ptr] = *gear_it;
                gear_ptr->hids::take(device);
                boss.strike();
            }

        public:
            face xmap;
            lock sync;
            depo gears;

            input(base&&) = delete;
            template<class T>
            input(T& boss)
                : skill{ boss       },
                  props{ boss.props },
                  gears{{ id_t{}, bell::create<topgear>(props, true, boss, xmap, props.dblclick_timeout, props.tooltip_timeout, props.simple) }}
            {
                xmap.link(boss.bell::id);
                xmap.move(boss.base::coor());
                xmap.size(boss.base::size());
                boss.LISTEN(tier::release, e2::command::printscreen, gear, memo)
                {
                    auto data = ansi::esc{};
                    data.s11n(xmap, gear.slot);
                    if (data.length())
                    {
                        gear.set_clip_data(clip{ gear.slot.size, data, clip::ansitext });
                    }
                };
                boss.LISTEN(tier::release, e2::form::prop::brush, brush, memo)
                {
                    auto guard = std::lock_guard{ sync }; // Syncing with diff::render thread.
                    xmap.mark(brush);
                };
                boss.LISTEN(tier::release, e2::size::any, newsize, memo)
                {
                    auto guard = std::lock_guard{ sync }; // Syncing with diff::render thread.
                    xmap.size(newsize);
                };
                boss.LISTEN(tier::release, e2::coor::any, newcoor, memo)
                {
                    auto guard = std::lock_guard{ sync }; // Syncing with diff::render thread.
                    xmap.move(newcoor);
                };
                boss.LISTEN(tier::release, e2::conio::mouse, m, memo)
                {
                    if (m.enabled != hids::stat::ok)
                    {
                        auto gear_it = gears.find(m.gear_id);
                        if (gear_it != gears.end())
                        {
                            switch (m.enabled)
                            {
                                case hids::stat::ok:   break;
                                case hids::stat::halt: gear_it->second->deactivate(); break;
                                case hids::stat::die:  gears.erase(gear_it);          break;
                            }
                        }
                        boss.strike();
                    }
                    else forward(m);
                };
                boss.LISTEN(tier::release, e2::conio::keybd, k, memo)
                {
                    forward(k);
                };
                boss.LISTEN(tier::release, e2::conio::focus, f, memo)
                {
                    forward(f);
                };
            }
            void fire(hint event_id)
            {
                for (auto& [id, gear_ptr] : gears)
                {
                    auto& gear = *gear_ptr;
                    gear.fire_fast();
                    gear.fire(event_id);
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
            auto set_clip_data(clip const& clipdata)
            {
                if (gears.empty())
                {
                    gears.emplace(0, bell::create<topgear>(props, true, boss, xmap, props.dblclick_timeout, props.tooltip_timeout, props.simple));
                }
                for (auto& [id, gear_ptr] : gears)
                {
                    auto& gear = *gear_ptr;
                    gear.set_clip_data(clipdata, faux);
                }
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
                boss.LISTEN(tier::release, e2::postrender, parent_canvas, memo)
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
            span fade;
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
            fader(base& boss, cell default_state, cell highlighted_state, span fade_out = 250ms)
                : skill{ boss },
                robo{ boss },
                fade{ fade_out },
                c1 { default_state },
                c2 { highlighted_state },
                c2_orig { highlighted_state },
                transit{ 0 }
            {
                boss.base::color(c1.fgc(), c1.bgc());
                boss.LISTEN(tier::release, e2::form::prop::brush, brush)
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
                boss.LISTEN(tier::release, e2::form::state::mouse, active, memo)
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
            using skill::boss,
                  skill::memo;

            struct lims_t
            {
                twod min = skin::globals().min_value;
                twod max = skin::globals().max_value;
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
                boss.LISTEN(tier::preview, e2::size::any, new_size, memo)
                {
                    new_size = std::clamp(new_size, lims.min, lims.max);
                };
                // Clamping after all.
                boss.LISTEN(tier::preview, e2::size::set, new_size, memo)
                {
                    if (sure)
                    {
                        new_size = std::clamp(new_size, lims.min, lims.max);
                    }
                };
                if (forced_resize)
                {
                    boss.LISTEN(tier::release, e2::form::prop::window::size, new_size, memo)
                    {
                        auto reserv = lims;
                        lims.fixed_size(new_size);
                        boss.RISEUP(tier::release, e2::form::prop::fixedsize, true, true); //todo unify - Inform ui::fork to adjust ratio.
                        boss.base::template reflow<true>();
                        boss.RISEUP(tier::release, e2::form::prop::fixedsize, faux, true);
                        lims = reserv;
                    };
                }
            }
            // pro::limit: Set size limits (min, max). Preserve current value if specified arg less than 0.
            void set(twod const& min_size, twod const& max_size = -dot_11, bool forced_clamp = faux)
            {
                sure = forced_clamp;
                lims.min = min_size.less(dot_00, skin::globals().min_value, min_size);
                lims.max = max_size.less(dot_00, skin::globals().max_value, max_size);
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
                boss.LISTEN(tier::anycast, e2::form::prop::lucidity, value, memo)
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
                boss.LISTEN(tier::release, e2::form::upon::vtree::attached, parent_ptr, memo)
                {
                    boss.SIGNAL(tier::general, e2::form::canvas, canvas.shared_from_this());
                };
                boss.LISTEN(tier::release, e2::coor::any, new_xy,        memo) { canvas.move(new_xy); };
                boss.LISTEN(tier::release, e2::size::any, new_sz,        memo) { canvas.size(new_sz); };
                boss.LISTEN(tier::request, e2::form::canvas, canvas_ptr, memo) { canvas_ptr = coreface; };
                if (rendered)
                {
                    boss.LISTEN(tier::release, e2::render::prerender, parent_canvas, memo)
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
                boss.LISTEN(tier::anycast, e2::form::prop::lucidity, lucidity, memo)
                {
                    if (lucidity != -1) alive = lucidity == 0xFF;
                };
                boss.LISTEN(tier::release, e2::render::prerender, parent_canvas, memo)
                {
                    if (!alive) return;
                    auto brush = boss.base::color();
                    if (brush.wdt()) parent_canvas.blur(width, [&](cell& c) { c.alpha(0xFF).fuse(brush); });
                    else             parent_canvas.blur(width, [&](cell& c) { c.alpha(0xFF); });
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
                boss.LISTEN(tier::release, e2::form::highlight::any, state, memo)
                {
                    highlighted = state;
                    boss.base::deface();
                };
                boss.LISTEN(tier::release, e2::render::prerender, parent_canvas, memo)
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
                boss.LISTEN(tier::release, e2::form::state::mouse, active, memo)
                {
                    highlighted = active;
                    boss.base::deface();
                };
                boss.LISTEN(tier::release, e2::postrender, parent_canvas, memo)
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
            focus(base& boss, bool visible = true)
                : skill{ boss }
            {
                boss.LISTEN(tier::general, e2::form::proceed::functor, proc, memo)
                {
                    if (pool.size()) proc(boss.This());
                };
                boss.LISTEN(tier::anycast, e2::form::state::keybd::find, gear_test, memo)
                {
                    if (find(gear_test.first)) gear_test.second++;
                };
                boss.LISTEN(tier::anycast, e2::form::state::keybd::enlist, gear_id_list, memo)
                {
                    if (pool.size())
                    {
                        auto tail = gear_id_list.end();
                        gear_id_list.insert(tail, pool.begin(), pool.end());
                    }
                };
                boss.LISTEN(tier::request, e2::form::state::keybd::find, gear_test, memo)
                {
                    if (find(gear_test.first)) gear_test.second++;
                };
                boss.LISTEN(tier::anycast, e2::form::state::keybd::check, state, memo)
                {
                    state = !pool.empty();
                };
                boss.LISTEN(tier::anycast, e2::form::highlight::set, state, memo)
                {
                    state = !pool.empty();
                    boss.RISEUP(tier::preview, e2::form::highlight::any, state);
                };
                boss.LISTEN(tier::anycast, e2::form::upon::started, root, memo)
                {
                    auto state = !pool.empty();
                    boss.RISEUP(tier::preview, e2::form::highlight::any, state);
                };
                boss.LISTEN(tier::release, hids::events::notify::keybd::got, gear, memo)
                {
                    boss.RISEUP(tier::preview, e2::form::highlight::any, true);
                    boss.SIGNAL(tier::anycast, e2::form::highlight::any, true);
                    pool.push_back(gear.id);
                    boss.base::deface();
                };
                boss.LISTEN(tier::release, hids::events::notify::keybd::lost, gear, memo)
                {
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
                        boss.RISEUP(tier::preview, e2::form::highlight::any, faux);
                        boss.SIGNAL(tier::anycast, e2::form::highlight::any, faux);
                    }
                };
                if (visible)
                {
                    boss.LISTEN(tier::release, e2::render::prerender, parent_canvas, memo)
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
                boss.LISTEN(tier::release, hids::events::notify::mouse::enter, gear, memo)
                {
                    gear.set_tooltip(boss.id, note);
                };
                boss.LISTEN(tier::preview, e2::form::prop::ui::tooltip, new_note, memo)
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
    protected:
        using tick = datetime::quartz<events::reactor<>, hint>;
        using list = std::vector<rect>;

        pro::keybd keybd{*this }; // host: Keyboard controller.
        pro::mouse mouse{*this }; // host: Mouse controller.

        subs tokens; // host: Subscription tokens.
        tick quartz; // host: Frame rate synchronizator.
        si32 maxfps; // host: Frame rate.
        list debris; // host: Wrecked regions.
        xipc server; // host: Server pipe end.
        xmls config; // host: Running configuration.

    public:
        host(xipc server, xmls config )
            : quartz{ bell::router<tier::general>(), e2::timer::tick.id },
              server{ server },
              config{ config }
        {
            using namespace std::chrono;
            auto& g = skin::globals();
            g.brighter       = config.take("brighter", cell{});//120);
            g.kb_focus       = config.take("kb_focus", cell{});//60
            g.shadower       = config.take("shadower", cell{});//180);//60);//40);// 20);
            g.shadow         = config.take("shadow"  , cell{});//180);//5);
            g.selector       = config.take("selector", cell{});//48);
            g.highlight      = config.take("highlight"             , cell{});
            g.warning        = config.take("warning"               , cell{});
            g.danger         = config.take("danger"                , cell{});
            g.action         = config.take("action"                , cell{});
            g.label          = config.take("label"                 , cell{});
            g.inactive       = config.take("inactive"              , cell{});
            g.menu_white     = config.take("menu_white"            , cell{});
            g.menu_black     = config.take("menu_black"            , cell{});
            g.lucidity       = config.take("lucidity");
            g.bordersz       = config.take("bordersz"              , dot_11);
            g.spd            = config.take("timings/spd"           , 10  );
            g.pls            = config.take("timings/pls"           , 167 );
            g.spd_accel      = config.take("timings/spd_accel"     , 1   );
            g.spd_max        = config.take("timings/spd_max"       , 100 );
            g.ccl            = config.take("timings/ccl"           , 120 );
            g.ccl_accel      = config.take("timings/ccl_accel"     , 30  );
            g.ccl_max        = config.take("timings/ccl_max"       , 1   );
            g.switching      = config.take("timings/switching"     , 200 );
            g.deceleration   = config.take("timings/deceleration"  , span{ 2s    });
            g.blink_period   = config.take("timings/blink_period"  , span{ 400ms });
            g.menu_timeout   = config.take("timings/menu_timeout"  , span{ 250ms });
            g.active_timeout = config.take("timings/active_timeout", span{ 1s    });
            g.repeat_delay   = config.take("timings/repeat_delay"  , span{ 500ms });
            g.repeat_rate    = config.take("timings/repeat_rate"   , span{ 30ms  });
            g.fader_time     = config.take("timings/fader/duration", span{ 150ms });
            g.fader_fast     = config.take("timings/fader/fast"    , span{ 0ms   });
            g.max_value      = config.take("limits/window/size"    , twod{ 2000, 1000  });

            maxfps = config.take("fps");
            if (maxfps <= 0) maxfps = 60;

            keybd.accept(true); // Subscribe on keybd offers.

            LISTEN(tier::general, e2::timer::any, timestamp, tokens)
            {
                auto damaged = !debris.empty();
                debris.clear();
                this->SIGNAL(tier::general, e2::nextframe, damaged);
            };
            //todo deprecated
            LISTEN(tier::general, e2::config::creator, world_ptr, tokens)
            {
                world_ptr = base::This();
            };
            LISTEN(tier::request, e2::config::creator, world_ptr, tokens)
            {
                world_ptr = base::This();
            };
            LISTEN(tier::general, e2::config::fps, fps, tokens)
            {
                if (fps > 0)
                {
                    maxfps = fps;
                    quartz.ignite(maxfps);
                }
                else if (fps == -1)
                {
                    fps = maxfps;
                }
                else
                {
                    quartz.cancel();
                }
            };
            LISTEN(tier::general, e2::cleanup, counter, tokens)
            {
                this->template router<tier::general>().cleanup(counter.ref_count, counter.del_count);
            };
            LISTEN(tier::general, hids::events::halt, gear, tokens)
            {
                if (gear.captured(bell::id))
                {
                    gear.setfree();
                    gear.dismiss();
                }
            };
            LISTEN(tier::general, e2::shutdown, msg, tokens)
            {
                //todo revise, Deadlock with intensive logging (inside the std::cout.operator<<()).
                log("host: shutdown: ", msg);
                host::server->stop();
            };
            quartz.ignite(maxfps);
            log("host: started at ", maxfps, "fps");
        }
        // host: Initiate redrawing.
        virtual void redraw(face& canvas)
        {
            SIGNAL(tier::general, e2::shutdown, "host: rendering is not provided");
        }
        // host: Mark dirty region.
        void denote(rect const& updateregion)
        {
            if (updateregion)
            {
                debris.push_back(updateregion);
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
            auto root = base::create<S>(*this, std::forward<Args>(args)..., host::config);
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

    // console: TTY session manager.
    class link
        : public s11n
    {
        using sptr = netxs::sptr<bell>;
        using ipc  = os::ipc::stdcon;

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

        ipc&     canal; // link: Data highway.
        sptr     owner; // link: Link owner.
        relay_t  relay; // link: Clipboard relay.

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
        template<tier Tier = tier::release, class E, class T>
        void notify(E, T&& data)
        {
            netxs::events::enqueue(owner, [d = data](auto& boss) mutable
            {
                //boss.SIGNAL(Tier, E{}, d); // VS2022 17.4.1 doesn't get it for some reason (nested lambdas + static_cast + decltype(...)::type).
                boss.bell::template signal<Tier>(E::id, static_cast<typename E::type &&>(d));
            });
        }
        void handle(s11n::xs::sysfocus    lock)
        {
            auto& focus = lock.thing;
            notify(e2::conio::focus, focus);
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
        void handle(s11n::xs::osclipdata  lock)
        {
            auto& item = lock.thing;
            notify(e2::conio::clipdata, clip{ dot_00, item.data, static_cast<clip::mime>(item.mimetype) });
        }
        void handle(s11n::xs::syskeybd    lock)
        {
            auto& keybd = lock.thing;
            notify(e2::conio::keybd, keybd);
        }
        void handle(s11n::xs::plain       lock)
        {
            auto k = s11n::syskeybd.freeze();
            auto& keybd = k.thing;
            auto& item = lock.thing;
            keybd.wipe();
            keybd.gear_id = item.gear_id;
            keybd.cluster = item.utf8txt;
            keybd.pressed = true;
            notify(e2::conio::keybd, keybd);
            keybd.pressed = faux;
            notify(e2::conio::keybd, keybd);
        }
        void handle(s11n::xs::ctrls       lock)
        {
            auto k = s11n::syskeybd.freeze();
            auto& keybd = k.thing;
            auto& item = lock.thing;
            keybd.wipe();
            keybd.gear_id = item.gear_id;
            keybd.ctlstat = item.ctlstat;
            keybd.pressed = faux;
            notify(e2::conio::keybd, keybd);
        }
        void handle(s11n::xs::sysmouse    lock)
        {
            auto& mouse = lock.thing;
            notify(e2::conio::mouse, mouse);
        }
        void handle(s11n::xs::mouse_show  lock)
        {
            auto& item = lock.thing;
            notify(e2::conio::pointer, item.mode);
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
        void handle(s11n::xs::bgc         lock)
        {
            auto& item = lock.thing;
            notify<tier::anycast>(e2::form::prop::colors::bg, item.color);
        }
        void handle(s11n::xs::fgc         lock)
        {
            auto& item = lock.thing;
            notify<tier::anycast>(e2::form::prop::colors::fg, item.color);
        }
        void handle(s11n::xs::slimmenu    lock)
        {
            auto& item = lock.thing;
            notify<tier::anycast>(e2::form::prop::ui::slimmenu, item.menusize);
        }
        void handle(s11n::xs::form_header lock)
        {
            auto& item = lock.thing;
            notify<tier::preview>(e2::form::prop::ui::header, item.new_header); //todo window_id
        }
        void handle(s11n::xs::form_footer lock)
        {
            auto& item = lock.thing;
            notify<tier::preview>(e2::form::prop::ui::footer, item.new_footer); //todo window_id
        }
    };

    // console: Bitmap changes analyzer.
    class diff
    {
        using work = std::thread;
        using lock = std::mutex;
        using cond = std::condition_variable_any;
        using ipc  = os::ipc::stdcon;

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
            auto start = time{};
            auto image = Bitmap{};
            auto guard = std::unique_lock{ mutex };
            while ((void)synch.wait(guard, [&]{ return ready; }), alive)
            {
                start = datetime::now();
                ready = faux;
                abort = faux;
                auto winid = id_t{ 0xddccbbaa };
                auto coord = dot_00;
                image.set(winid, coord, cache, abort, debug.delta);
                if (debug.delta)
                {
                    image.sendby(canal); // Sending, this is the frame synchronization point.
                }                        // Frames should drop, the rest should wait for the end of sending.
                debug.watch = datetime::now() - start;
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
            using namespace netxs::directvt;
            paint = work([&, vtmode]
            {
                //todo revise (bitmap/bitmap_t)
                     if (vtmode == svga::dtvt     ) render<binary::bitmap_t>               (canal);
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

    // console: Client gate.
    class gate
        : public base
    {
    public:
        conf props; // gate: Client properties.

        pro::keybd keybd{*this }; // gate: Keyboard controller.
        pro::mouse mouse{*this }; // gate: Mouse controller.
        pro::robot robot{*this }; // gate: Animation controller.
        pro::maker maker{*this }; // gate: Form generator.
        pro::title title{*this }; // gate: Window title/footer.
        pro::guard guard{*this }; // gate: Watch dog against robots and single Esc detector.
        pro::input input{*this }; // gate: Input event handler.
        pro::debug debug{*this }; // gate: Debug telemetry controller.
        pro::limit limit{*this }; // gate: Limit size to dot_11.

        using sptr = netxs::sptr<base>;

        host& world;
        bool  yield; // gate: Indicator that the current frame has been successfully STDOUT'd.
        para  uname; // gate: Client name.
        text  uname_txt; // gate: Client name (original).
        bool  fullscreen = faux; //gate: Fullscreen mode.
        si32  legacy = os::vt::clean;

        void draw_foreign_names(face& parent_canvas)
        {
            auto& header = *uname.lyric;
            auto  basexy = base::coor();
            auto  half_x = (si32)header.size().x / 2;
            for (auto& [id, gear_ptr] : input.gears)
            {
                auto& gear = *gear_ptr;
                if (gear.disabled) continue;
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
                if (gear.disabled) continue;
                area.coor = coor + gear.coord;
                area.coor -= base;
                if (gear.m.buttons) brush.txt(64 + gear.m.buttons).bgc(reddk).fgc(0xFFffffff);
                else                brush.txt("\u2588"/* █ */).bgc(0x00).fgc(0xFF00ff00);
                canvas.fill(area, cell::shaders::fuse(brush));
            }
        }
        void draw_clip_preview(face& canvas, time const& stamp)
        {
            for (auto& [id, gear_ptr] : input.gears)
            {
                auto& gear = *gear_ptr;
                if (gear.disabled) continue;
                if (props.clip_preview_time == span::zero()
                 || props.clip_preview_time > stamp - gear.delta.stamp())
                {
                    auto coor = gear.coord + dot_21 * 2;
                    auto full = gear.clip_preview.full();
                    gear.clip_preview.move(coor - full.coor);
                    canvas.plot(gear.clip_preview, cell::shaders::mix);
                }
            }
        }
        void draw_tooltips(face& canvas)
        {
            auto full = canvas.full();
            for (auto& [id, gear_ptr] : input.gears)
            {
                auto& gear = *gear_ptr;
                if (gear.disabled) continue;
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
                        canvas.output(tooltip_page, cell::shaders::color(props.tooltip_colors));
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
                if (gear.disabled) continue;
                if (gear.is_tooltip_changed())
                {
                    list.thing.push(gear_id, gear.get_tooltip());
                }
            }
            list.thing.sendby<true>(conio);
        }
        void check_tooltips(time now)
        {
            auto result = faux;
            for (auto& [gear_id, gear_ptr] : input.gears)
            {
                auto& gear = *gear_ptr;
                if (gear.disabled) continue;
                result |= gear.tooltip_check(now);
            }
            if (result) base::strike();
        }

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
        void launch(xipc termio, sptr deskmenu_ptr, sptr bkground = {})
        {
            auto lock = events::unique_lock();

                legacy |= props.legacy_mode;

                auto vtmode = legacy & os::vt::vga16  ? svga::vga16
                            : legacy & os::vt::vga256 ? svga::vga256
                            : legacy & os::vt::direct ? svga::dtvt
                                                      : svga::truecolor;
                input.xmap.cmode = vtmode;
                auto direct = vtmode == svga::dtvt;
                if (props.debug_overlay) debug.start();
                color(props.background_color.fgc(), props.background_color.bgc());
                auto conf_usr_name = props.name;
                SIGNAL(tier::release, e2::form::prop::name, conf_usr_name);
                SIGNAL(tier::preview, e2::form::prop::ui::header, conf_usr_name);
                base::moveby(props.coor);

                auto& canal = *termio;
                auto  conio = link{ canal, This() }; // gate: Terminal IO.
                auto  paint = diff{ canal, vtmode }; // gate: Rendering loop.
                auto  token = subs{};                // gate: Subscription tokens.

                auto rebuild_scene = [&](bool damaged)
                {
                    auto stamp = datetime::now();

                    auto& canvas = input.xmap;
                    if (damaged)
                    {
                        canvas.wipe(world.bell::id);
                        if (!props.is_standalone_app)
                        {
                            if (props.background_image.size())
                            {
                                //todo cache background
                                canvas.tile(props.background_image, cell::shaders::fuse);
                            }
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
                        if (legacy & os::vt::mouse) // Render our mouse pointer.
                        {
                            draw_mouse_pointer(canvas);
                        }

                        if (!direct && props.clip_preview_show)
                        {
                            draw_clip_preview(canvas, stamp);
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
                LISTEN(tier::release, e2::conio::winsz, newsize, token)
                {
                    auto delta = base::resize(newsize);
                    if (delta && direct)
                    {
                        paint.cancel();
                        rebuild_scene(true);
                    }
                };
                LISTEN(tier::release, e2::size::any, newsz, token)
                {
                    if (uibar) uibar->base::resize(newsz);
                    if (background) background->base::resize(newsz);
                };
                LISTEN(tier::release, e2::conio::unknown, unkstate, token)
                {
                };
                LISTEN(tier::release, e2::conio::pointer, pointer, token)
                {
                    legacy |= pointer ? os::vt::mouse : 0;
                };
                LISTEN(tier::release, e2::conio::clipdata, clipdata, token)
                {
                    if (!direct)
                    {
                        clipdata.size = base::size() / 2;
                        input.set_clip_data(clipdata);
                        base::deface();
                    }
                };
                LISTEN(tier::release, e2::conio::error, errcode, token)
                {
                    auto msg = ansi::bgc(reddk).fgc(whitelt).add("\n\rgate: Term error: ", errcode, "\r\n");
                    log("gate: error byemsg: ", msg);
                    canal.shut();
                };
                LISTEN(tier::release, e2::conio::quit, msg, token)
                {
                    log("gate: quit byemsg: ", msg);
                    canal.shut();
                };
                LISTEN(tier::general, e2::conio::quit, msg, token)
                {
                    log("gate: global shutdown byemsg: ", msg);
                    canal.shut();
                };
                LISTEN(tier::release, e2::form::quit, initiator, token)
                {
                    auto msg = ansi::add("gate: quit message from: ", initiator->id);
                    canal.shut();
                    this->SIGNAL(tier::general, e2::shutdown, msg);
                };
                LISTEN(tier::release, e2::form::prop::ui::footer, newfooter, token)
                {
                    if (direct)
                    {
                        auto window_id = 0;
                        conio.form_footer.send(canal, window_id, newfooter);
                    }
                };
                LISTEN(tier::release, e2::form::prop::ui::header, newheader, token)
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
                        para{ newheader }.lyric->utf8(temp);
                        log("gate: title changed to '", temp, ansi::nil().add("'"));
                        conio.output(ansi::header(temp));
                    }
                };
                LISTEN(tier::general, e2::nextframe, damaged, token)
                {
                    rebuild_scene(damaged);
                };

                if (props.tooltip_enabled)
                {
                    LISTEN(tier::general, e2::timer::any, now, token)
                    {
                        check_tooltips(now);
                    };
                }

                if (!props.is_standalone_app)
                {
                    LISTEN(tier::release, hids::events::upevent::kboffer, gear, token)
                    {
                        world.SIGNAL(tier::release, e2::form::proceed::autofocus::take, gear);
                    };
                    LISTEN(tier::release, hids::events::upevent::kbannul, gear, token)
                    {
                        world.SIGNAL(tier::release, e2::form::proceed::autofocus::lost, gear);
                    };
                }
                if (direct)
                {
                    LISTEN(tier::preview, hids::events::notify::focus::any, from_gear, token)
                    {
                        auto [ext_gear_id, gear_ptr] = input.get_foreign_gear_id(from_gear.id);
                        auto deed =this->bell::protos<tier::preview>();
                        switch (deed)
                        {
                            case hids::events::notify::focus::got.id:  conio.set_focus.send(conio, ext_gear_id, from_gear.combine_focus, from_gear.force_group_focus); break;
                            case hids::events::notify::focus::lost.id: conio.off_focus.send(conio, ext_gear_id); break;
                        }
                    };
                }
                // Focus relay.
                if (deskmenu_ptr)
                {
                    LISTEN(tier::release, hids::events::notify::focus::got, from_gear, token)
                    {
                        auto myid = from_gear.id;
                        auto [ext_gear_id, gear_ptr] = input.get_foreign_gear_id(myid);
                        auto& gear = *gear_ptr;
                        gear.kb_offer_4(deskmenu_ptr);
                        if (gear.focus_changed()) gear.dismiss();
                    };
                    LISTEN(tier::release, hids::events::notify::focus::lost, from_gear, token)
                    {
                        auto myid = from_gear.id;
                        auto [ext_gear_id, gear_ptr] = input.get_foreign_gear_id(myid);
                        gear_ptr->kb_offer_10(deskmenu_ptr);
                    };
                }

                // Clipboard relay.
                LISTEN(tier::release, hids::events::clipbrd::set, from_gear, token)
                {
                    auto myid = from_gear.id;
                    auto [ext_gear_id, gear_ptr] = input.get_foreign_gear_id(myid);
                    auto& gear =*gear_ptr;
                    auto& data = gear.clip_rawdata;
                    if (direct) conio.set_clipboard.send(canal, ext_gear_id, data.size, data.utf8, data.kind);
                    else        conio.output(ansi::clipbuf(                  data.size, data.utf8, data.kind));
                };
                LISTEN(tier::release, hids::events::clipbrd::get, from_gear, token)
                {
                    if (!direct) return;
                    auto myid = from_gear.id;
                    auto [ext_gear_id, gear_ptr] = input.get_foreign_gear_id(myid);
                    if (!conio.request_clip_data(ext_gear_id, gear_ptr->clip_rawdata))
                    {
                        log("gate: timeout: no clipboard data reply");
                    }
                };

                if (deskmenu_ptr)
                {
                    attach(deskmenu_ptr); // Our size could be changed here during attaching.
                    deskmenu_ptr->LISTEN(tier::preview, hids::events::mouse::button::tplclick::leftright, gear, token)
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

                if (direct) // Forward unhandled events outside.
                {
                    LISTEN(tier::general, e2::conio::logs, utf8, token)
                    {
                        conio.logs.send(canal, os::process::id.first, os::process::id.second, text{ utf8 });
                    };
                    LISTEN(tier::release, e2::config::fps, fps, token)
                    {
                        if (fps > 0) this->SIGNAL(tier::general, e2::config::fps, fps);
                    };
                    LISTEN(tier::preview, e2::config::fps, fps, token)
                    {
                        conio.fps.send(conio, fps);
                    };
                    LISTEN(tier::preview, hids::events::mouse::button::click::any, gear, token)
                    {
                        conio.expose.send(conio);
                    };
                    LISTEN(tier::anycast, e2::form::layout::expose, item, token)
                    {
                        conio.expose.send(conio);
                    };
                    LISTEN(tier::preview, e2::form::layout::swarp, warp, token)
                    {
                        conio.warping.send(conio, 0, warp);
                    };
                    LISTEN(tier::release, e2::form::maximize, gear, token)
                    {
                        auto [ext_gear_id, gear_ptr] = input.get_foreign_gear_id(gear.id);
                        conio.maximize.send(conio, ext_gear_id);
                    };
                    if (props.is_standalone_app)
                    {
                        LISTEN(tier::release, hids::events::mouse::button::any, gear, token)
                        {
                            using button = hids::events::mouse::button;
                            auto forward = faux;
                            auto cause = gear.mouse::cause;//this->bell::protos<tier::release>();
                            if (events::subevent(cause, button::click     ::any.id)
                             || events::subevent(cause, button::dblclick  ::any.id)
                             || events::subevent(cause, button::tplclick  ::any.id)
                             || events::subevent(cause, button::drag::pull::any.id))
                            {
                                forward = true;
                            }
                            else if (events::subevent(cause, button::drag::start::any.id))
                            {
                                gear.capture(bell::id); // To avoid unhandled mouse pull processing.
                                forward = true;
                            }
                            else if (events::subevent(cause, button::drag::cancel::any.id)
                                  || events::subevent(cause, button::drag::stop  ::any.id))
                            {
                                gear.setfree();
                            }
                            if (forward)
                            {
                                auto [ext_gear_id, gear_ptr] = input.get_foreign_gear_id(gear.id);
                                conio.mouse_event.send(canal, ext_gear_id, cause, gear.coord, gear.delta.get(), gear.take_button_state());
                                gear.dismiss();
                            }
                        };
                    }
                }
                else
                {
                    if (props.title.size())
                    {
                        conio.output(ansi::header(props.title));
                    }
                }

                SIGNAL(tier::anycast, e2::form::upon::started, This());

            lock.unlock();

            directvt::binary::stream::reading_loop(canal, [&](view data){ conio.sync(data); });

            lock.lock();
                log("link: signaling to close read channel ", canal);
                SIGNAL(tier::release, e2::conio::quit, "link: read channel is closed");
                token.clear();
                mouse.reset(); // Reset active mouse clients to avoid hanging pointers.
                base::detach();
            lock.unlock();
        }

    protected:
        template<class ...Args>
        gate(host& world, Args&&... args)
            : props{ std::forward<Args>(args)... },
              world{ world }
        {
            limit.set(dot_11);
            //todo unify
            title.live = faux;
            if (!props.is_standalone_app)
            {
                //todo move it to the desk (dragging)
                mouse.draggable<hids::buttons::leftright>(true);
                mouse.draggable<hids::buttons::left>(true);
                LISTEN(tier::release, e2::form::drag::start::any, gear)
                {
                    robot.pacify();
                };
                LISTEN(tier::release, e2::form::drag::pull::any, gear)
                {
                    base::moveby(-gear.delta.get());
                    base::deface();
                };
                LISTEN(tier::release, e2::form::drag::stop::any, gear)
                {
                    robot.pacify();
                    robot.actify(gear.fader<quadratic<twod>>(2s), [&](auto& x)
                                {
                                    base::moveby(-x);
                                    base::deface();
                                });
                };
                LISTEN(tier::release, e2::form::layout::shift, newpos)
                {
                    auto viewport = e2::form::prop::viewport.param();
                    this->SIGNAL(tier::request, e2::form::prop::viewport, viewport);
                    auto oldpos = viewport.coor + (viewport.size / 2);

                    auto path = oldpos - newpos;
                    auto time = skin::globals().switching;
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
            LISTEN(tier::release, e2::form::prop::fullscreen, state)
            {
                fullscreen = state;
            };
            LISTEN(tier::release, e2::form::prop::name, user_name)
            {
                uname = uname_txt = user_name;
            };
            LISTEN(tier::request, e2::form::prop::name, user_name)
            {
                user_name = uname_txt;
            };
            LISTEN(tier::request, e2::form::prop::viewport, viewport)
            {
                this->SIGNAL(tier::anycast, e2::form::prop::viewport, viewport);
                viewport.coor += base::coor();
            };
            //todo unify creation (delete simple create wo gear)
            LISTEN(tier::preview, e2::form::proceed::create, region)
            {
                region.coor += base::coor();
                world.SIGNAL(tier::release, e2::form::proceed::create, region);
            };
            LISTEN(tier::release, e2::form::proceed::onbehalf, proc)
            {
                //todo hids
                //proc(input.gear);
            };
            LISTEN(tier::preview, hids::events::keybd::any, gear)
            {
                //todo unify
                auto keystrokes = gear.interpret();
                if (keystrokes == props.debug_toggle)
                {
                    debug ? debug.stop()
                          : debug.start();
                }
                //todo unify
                //todo move it to the desk
                //if (gear.meta(hids::CTRL | hids::RCTRL))
                if (!props.is_standalone_app)
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
                            gear.clear_kb_focus();
                            gear.kb_offer_7(item);
                        }
                        gear.dismiss();
                    }
                }
            };
            LISTEN(tier::preview, hids::events::mouse::button::click::leftright, gear)
            {
                if (gear.clear_clip_data())
                {
                    this->bell::template expire<tier::release>();
                    gear.dismiss();
                }
            };

            LISTEN(tier::release, e2::render::prerender, parent_canvas)
            {
                if (parent_canvas.cmode != svga::vga16) // Don't show shadow in poor color environment.
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
            LISTEN(tier::release, e2::postrender, parent_canvas)
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