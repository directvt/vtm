// Copyright (c) NetXS Group.
// Licensed under the MIT license.

#ifndef NETXS_CONSOLE_HPP
#define NETXS_CONSOLE_HPP

#include "input.hpp"
#include "../abstract/iterator.hpp"
#include "../os/system.hpp"

#include <iostream>
#include <typeindex>

#define SPD            10   // console: Auto-scroll initial speed component ΔR.
#define PLS            167  // console: Auto-scroll initial speed component ΔT.
#define CCL            120  // console: Auto-scroll duration in ms.
#define SPD_ACCEL      3    // console: Auto-scroll speed accelation.
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
    class face;
    class base;
    class form;
    class link;
    class host;
    class site;

    using namespace netxs::input;
    using drawfx = std::function<bool(face&, page const&)>;
    using registry_t = std::map<id_t, std::list<sptr<base>>>;
}

namespace netxs::events::userland
{
    using namespace netxs::ui::atoms;
    using namespace netxs::datetime;
    using utf::text;
    using utf::view;

    struct e2
    {
        using type = netxs::events::type;
        static constexpr auto dtor = netxs::events::userland::root::dtor;

        EVENTPACK( e2, netxs::events::userland::root::base )
        {
            EVENT_XS( tick      , moment              ), // timer tick, arg: current moment (now).
            EVENT_XS( postrender, console::face       ), // release: UI-tree post-rendering.
            GROUP_XS( render    , console::face       ), // release: UI-tree rendering.
            GROUP_XS( conio     , iota                ),
            GROUP_XS( size      , twod                ), // release: Object size.
            GROUP_XS( coor      , twod                ), // release: Object coor.
            GROUP_XS( form      , bool                ),
            GROUP_XS( data      , iota                ),
            GROUP_XS( debug     , const view          ), // return info struct with telemtry data.
            GROUP_XS( config    , iota                ), // set/notify/get/global_set configuration data.
            GROUP_XS( command   , iota                ), // exec UI command.
            GROUP_XS( bindings  , sptr<console::base> ), // Dynamic Data Bindings.

            SUBSET_XS( render ) // release any: UI-tree default rendering submission.
            {
                EVENT_XS( prerender, console::face ), // release: UI-tree pre-rendering, used by pro::cache (can interrupt SIGNAL) and any kind of highlighters.
            };
            SUBSET_XS( size ) // preview: checking by pro::limit.
            {
                EVENT_XS( set, twod ), // preview: checking by object; release: apply to object.
            };
            SUBSET_XS( coor ) // preview any: checking by pro::limit.
            {
                EVENT_XS( set, twod ), // preview: checking by object; release: apply to object.
            };
            SUBSET_XS( bindings )
            {
                GROUP_XS( list, iota ), // UI-tree pre-rendering, used by pro::cache (can interrupt SIGNAL) and any kind of highlighters, release only.

                SUBSET_XS( list )
                {
                    EVENT_XS( users, sptr<std::list<sptr<console::base>>> ), // list of connected users.
                    EVENT_XS( apps , sptr<console::registry_t>            ), // list of running apps.
                };
            };
            SUBSET_XS( debug )
            {
                EVENT_XS( logs  , const view          ), // logs output.
                EVENT_XS( output, const view          ), // logs has to be parsed.
                EVENT_XS( parsed, const console::page ), // output parced logs.
            };
            SUBSET_XS( config )
            {
                EVENT_XS( broadcast, sptr<bell> ), // release: broadcast source changed.
                EVENT_XS( fps      , iota       ), // request to set new fps, arg: new fps (iota); the value == -1 is used to request current fps.
                GROUP_XS( caret    , period     ), // any kind of intervals property.

                SUBSET_XS( caret )
                {
                    EVENT_XS( blink, period ), // caret blinking interval.
                    EVENT_XS( style, iota   ), // caret style: 0 - underline, 1 - box.
                };
            };
            SUBSET_XS( conio )
            {
                EVENT_XS( unknown , iota              ), // return platform unknown event code.
                EVENT_XS( error   , iota              ), // return error code.
                EVENT_XS( focus   , bool              ), // order to change focus.
                EVENT_XS( mouse   , console::sysmouse ), // mouse activity.
                EVENT_XS( key     , console::syskeybd ), // keybd activity.
                EVENT_XS( size    , twod              ), // order to update terminal primary overlay.
                EVENT_XS( native  , bool              ), // extended functionality.
                EVENT_XS( layout  , const twod        ),
                EVENT_XS( preclose, const bool        ), // signal to quit after idle timeout, arg: bool - ready to shutdown.
                EVENT_XS( quit    , const view        ), // quit, arg: text - bye msg.
                EVENT_XS( pointer , const bool        ), // mouse pointer visibility.
                //EVENT_XS( menu  , iota ), 
            };
            SUBSET_XS( data )
            {
                EVENT_XS( changed, iota            ), // return digest.
                EVENT_XS( request, iota            ),
                EVENT_XS( disable, iota            ),
                EVENT_XS( flush  , iota            ),
                EVENT_XS( text   , const utf::text ), // signaling with a text string, release only.
            };
            SUBSET_XS( command )
            {
                EVENT_XS( quit  , const view ), // return bye msg, arg: errcode.
                EVENT_XS( cout  , const text ), // Append extra data to output.
                EVENT_XS( custom, iota       ), // Custom command, arg: cmd_id.
            };
            SUBSET_XS( form )
            {
                EVENT_XS( canvas   , sptr<console::core> ), // request global canvas.
                GROUP_XS( layout   , const twod          ),
                GROUP_XS( draggable, bool                ), // signal to the form to enable draggablity for specified mouse button.
                GROUP_XS( highlight, bool                ),
                GROUP_XS( upon     , bool                ),
                GROUP_XS( proceed  , bool                ),
                GROUP_XS( cursor   , bool                ),
                GROUP_XS( drag     , console::hids       ),
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
                    EVENT_XS( shift , const twod    ), // request a global shifting  with delta.
                    EVENT_XS( convey, cube          ), // request a global conveying with delta (Inform all children to be conveyed).
                    EVENT_XS( bubble, console::base ), // order to popup the requested item through the visual tree.
                    EVENT_XS( expose, console::base ), // order to bring the requested item on top of the visual tree (release: ask parent to expose specified child; preview: ask child to expose itself).
                    EVENT_XS( appear, twod          ), // fly to the specified coords.
                    //EVENT_XS( order     , iota       ), // return
                    //EVENT_XS( strike    , rect       ), // inform about the child canvas has changed, only preview.
                    //EVENT_XS( coor      , twod       ), // return client rect coor, only preview.
                    //EVENT_XS( size      , twod       ), // return client rect size, only preview.
                    //EVENT_XS( rect      , rect       ), // return client rect.
                    //EVENT_XS( show      , bool       ), // order to make it visible.
                    //EVENT_XS( hide      , bool       ), // order to make it hidden.
                    //EVENT_XS( next      , sptr<base> ), // request client for next child object, only request.
                    //EVENT_XS( prev      , sptr<base> ), // request client for prev child object, only request.
                    //EVENT_XS( footer    , const rich ), // notify the client has changed footer, only release.
                    //EVENT_XS( clientrect, rect       ), // notify the client area has changed, only release.
                };
                SUBSET_XS( highlight )
                {
                    EVENT_XS( on , bool ),
                    EVENT_XS( off, bool ),
                };
                SUBSET_XS( upon )
                {
                    EVENT_XS( redrawn, console::face       ), // inform about camvas is completely redrawn.
                    EVENT_XS( cached , console::face       ), // inform about camvas is cached.
                    EVENT_XS( wiped  , console::face       ), // event after wipe the canvas.
                    EVENT_XS( changed, twod                ), // event after resize, arg: diff bw old and new size.
                    EVENT_XS( dragged, console::hids       ), // event after drag.
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
                        EVENT_XS( x     , rack ), // event after scroll along X.
                        EVENT_XS( y     , rack ), // event after scroll along Y.
                        EVENT_XS( resetx, rack ), // event reset scroll along X.
                        EVENT_XS( resety, rack ), // event reset scroll along Y.

                        INDEX_XS( x, y, resetx, resety ),
                    };
                };
                SUBSET_XS( proceed )
                {
                    EVENT_XS( create  , rect                ), // return coordinates of the new object placeholder.
                    EVENT_XS( createby, console::hids       ), // return gear with coordinates of the new object placeholder gear::slot.
                    EVENT_XS( destroy , console::base       ), // ??? bool return reference to the parent.
                    EVENT_XS( render  , console::drawfx     ), // ask children to render itself to the parent canvas, arg is a function drawfx to perform drawing.
                    EVENT_XS( attach  , sptr<console::base> ), // order to attach a child, arg is a parent base_sptr.
                    EVENT_XS( detach  , sptr<console::base> ), // order to detach a child, tier::release - kill itself, tier::preview - detach the child specified in args, arg is a child sptr.
                    //EVENT_XS( commit     , iota                     ), // order to output the targets, arg is a frame number.
                    //EVENT_XS( multirender, vector<shared_ptr<face>> ), // ask children to render itself to the set of canvases, arg is an array of the face sptrs.
                    //EVENT_XS( draw       , face                     ), // ????  order to render itself to the canvas.
                    //EVENT_XS( checkin    , face_sptr                ), // order to register an output client canvas.
                };
                SUBSET_XS( cursor )
                {
                    EVENT_XS(blink, bool),
                };
                SUBSET_XS( animate )
                {
                    EVENT_XS( start, id_t ),
                    EVENT_XS( stop , id_t ),
                };
                SUBSET_XS( drag )
                {
                    GROUP_XS( start , console::hids ), // notify about mouse drag start by pro::mouse.
                    GROUP_XS( pull  , console::hids ), // notify about mouse drag pull by pro::mouse.
                    GROUP_XS( cancel, console::hids ), // notify about mouse drag cancel by pro::mouse.
                    GROUP_XS( stop  , console::hids ), // notify about mouse drag stop by pro::mouse.

                    SUBSET_XS( start )
                    {
                        EVENT_XS( left     , console::hids ),
                        EVENT_XS( right    , console::hids ),
                        EVENT_XS( leftright, console::hids ),
                        EVENT_XS( middle   , console::hids ),
                        EVENT_XS( wheel    , console::hids ),
                        EVENT_XS( win      , console::hids ),

                        INDEX_XS( left, right, leftright, middle, wheel, win ),
                    };
                    SUBSET_XS( pull )
                    {
                        EVENT_XS( left     , console::hids ),
                        EVENT_XS( right    , console::hids ),
                        EVENT_XS( leftright, console::hids ),
                        EVENT_XS( middle   , console::hids ),
                        EVENT_XS( wheel    , console::hids ),
                        EVENT_XS( win      , console::hids ),

                        INDEX_XS( left, right, leftright, middle, wheel, win ),
                    };
                    SUBSET_XS( cancel )
                    {
                        EVENT_XS( left     , console::hids ),
                        EVENT_XS( right    , console::hids ),
                        EVENT_XS( leftright, console::hids ),
                        EVENT_XS( middle   , console::hids ),
                        EVENT_XS( wheel    , console::hids ),
                        EVENT_XS( win      , console::hids ),

                        INDEX_XS( left, right, leftright, middle, wheel, win ),
                    };
                    SUBSET_XS( stop )
                    {
                        EVENT_XS( left     , console::hids ),
                        EVENT_XS( right    , console::hids ),
                        EVENT_XS( leftright, console::hids ),
                        EVENT_XS( middle   , console::hids ),
                        EVENT_XS( wheel    , console::hids ),
                        EVENT_XS( win      , console::hids ),

                        INDEX_XS( left, right, leftright, middle, wheel, win ),
                    };
                };
                SUBSET_XS( prop )
                {
                    EVENT_XS( header    , text       ), // set form caption header.
                    EVENT_XS( footer    , text       ), // set form caption footer.
                    EVENT_XS( name      , text       ), // user name.
                    EVENT_XS( zorder    , iota       ), // set form z-order, iota: -1 backmost, 0 plain, 1 topmost.
                    EVENT_XS( brush     , const cell ), // set form brush/color.
                    EVENT_XS( fullscreen, bool       ), // set fullscreen flag.
                    EVENT_XS( viewport  , rect       ), // request: return form actual viewport.
                };
                SUBSET_XS( global )
                {
                    EVENT_XS( ctxmenu , twod ), // request context menu at specified coords.
                    EVENT_XS( lucidity, iota ), // set or request global window transparency, iota: 0-255, -1 to request.
                    //EVENT_XS( prev  , twod ), // request the prev scene window.
                    //EVENT_XS( next  , twod ), // request the next scene window.
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
                    EVENT_XS( mouse , iota          ), // notify the client is mouse active or not. The form is active when the number of client (form::eventa::mouse::enter - mouse::leave) is not zero, only release, iota - number of clients.
                    EVENT_XS( keybd , bool          ), // notify the client is keybd active or not. The form is active when the number of client (form::eventa::keybd::got - keybd::lost) is not zero, only release.
                    EVENT_XS( header, console::para ), // notify the client has changed title.
                    EVENT_XS( footer, console::para ), // notify the client has changed footer.
                    EVENT_XS( params, console::para ), // notify the client has changed title params.
                    EVENT_XS( color , console::tone ), // notify the client has changed tone, preview to set.
                };
            };
        };
    };
}

namespace netxs::console
{
    using e2 = netxs::events::userland::e2;

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
        iota opaque = 0xFF;

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
        static cell const& color(iota property)
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
        static poly const& grade(iota property)
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
        static iota const& shady()
        {
            auto& global = _globals<void>::global;
            return global.opaque;
        }
    };

    template<class V>
    skin skin::_globals<V>::global;

    // console: Textographical canvas.
    class face
        : public core, public flow, public std::enable_shared_from_this<face>
    {
        using vrgb = std::vector<irgb>;

        cell brush;
        twod anker;     // face: The position of the nearest visible paragraph.
        id_t piece = 1; // face: The nearest to top paragraph.

        vrgb cache; // face: BlurFX temp buffer.

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
                brush = combo.mark(); // Current mark of the last printed fragment.
            };
            textpage.stream(publish);
        }
        // face: Print page with holding top visible paragraph on its own place.
        void output(page const& textpage, bool reset)
        {
            //todo if cursor is visible when tie to the cursor position
            //     else tie to the first visible text line.

            bool done = faux;
            // Get vertical position of the nearest paragraph to the top.
            auto gain = [&](auto const& combo)
            {
                auto pred = flow::print(combo, *this);
                brush = combo.mark(); // Current mark of the last printed fragment.

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
                anker.y = std::numeric_limits<iota>::max();
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
                    if (anker.y == 0 &&
                        anker.y == flow::cp().y &&
                        cover.height() > 1)
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
        auto  bump(dent const& delta)
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
        void  bump(std::pair<rect, rect> ctx)
        {
            face::full(ctx.first);
            core::view(ctx.second);
        }

        // Use a two letter function if we don't need to return *this
        face& cup (twod const& p) { flow::ac( p); return *this; } // face: Cursor 0-based absolute position.
        face& chx (iota x)        { flow::ax( x); return *this; } // face: Cursor 0-based horizontal absolute.
        face& chy (iota y)        { flow::ay( y); return *this; } // face: Cursor 0-based vertical absolute.
        face& cpp (twod const& p) { flow::pc( p); return *this; } // face: Cursor percent position.
        face& cpx (iota x)        { flow::px( x); return *this; } // face: Cursor H percent position.
        face& cpy (iota y)        { flow::py( y); return *this; } // face: Cursor V percent position.
        face& cuu (iota n = 1)    { flow::dy(-n); return *this; } // face: cursor up.
        face& cud (iota n = 1)    { flow::dy( n); return *this; } // face: Cursor down.
        face& cuf (iota n = 1)    { flow::dx( n); return *this; } // face: Cursor forward.
        face& cub (iota n = 1)    { flow::dx(-n); return *this; } // face: Cursor backward.
        face& cnl (iota n = 1)    { flow::dy( n); return *this; } // face: Cursor next line.
        face& cpl (iota n = 1)    { flow::dy(-n); return *this; } // face: Cursor previous line.

        face& ocp (twod const& p) { flow::oc( p); return *this; } // face: Cursor 1-based absolute position.
        face& ocx (iota x)        { flow::ox( x); return *this; } // face: Cursor 1-based horizontal absolute.
        face& ocy (iota y)        { flow::oy( y); return *this; } // face: Cursor 1-based vertical absolute.

        face& scp ()              { flow::sc(  ); return *this; } // face: Save cursor position.
        face& rcp ()              { flow::rc(  ); return *this; } // face: Restore cursor position.
        face& rst ()  { flow::zz(); flow::sc(  ); return *this; } // face: Reset to zero all cursor params.

        face& tab (iota n = 1)    { flow::tb( n); return *this; } // face: Proceed the \t .
        face& eol (iota n = 1)    { flow::nl( n); return *this; } // face: Proceed the \r || \n || \r\n .

        void size (twod const& newsize) // face: Change the size of the face/core.
        {
            core::size(newsize);
            flow::size(newsize);
        }

        template<class P = noop>
        void blur(iota r, P shade = P())
        {
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
        void render(sptr<T> nested_ptr, twod const& basis)
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
        template<class T>
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
                object.SIGNAL(tier::release, e2::postrender, *this);

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
        hook kb_offer_token;
        hook broadcast_update_token;

    public:
        sptr<bell> broadcast = std::make_shared<bell>(); // base: Broadcast bus.
                                                         //        On attach the broadcast is merged with parent (bell::merge).
                                                         //        On detach the broadcast is duplicated from parent (bell::reset).
        side oversz; // base: Oversize, margin.
        twod anchor; // base: Object balance point. Center point for any transform (on preview).

    protected:
        bool is_attached() const { return kb_offer_token.operator bool(); }
        void switch_to_bus(sptr<bell> parent_bus)
        {
            parent_bus->merge(broadcast);
            broadcast->SIGNAL(tier::release, e2::config::broadcast, parent_bus);
        }

        virtual ~base() = default;
        base()
        {
            SUBMIT(tier::release, e2::coor::set, new_coor) { square.coor = new_coor; };
            SUBMIT(tier::request, e2::coor::set, coor_var) { coor_var = square.coor; };
            SUBMIT(tier::release, e2::size::set, new_size) { square.size = new_size; };
            SUBMIT(tier::request, e2::size::set, size_var) { size_var = square.size; };

            broadcast->SUBMIT_T(tier::release, e2::config::broadcast, bell::tracker, new_broadcast)
            {
                broadcast = new_broadcast;
            };

            SUBMIT(tier::release, e2::form::upon::vtree::attached, parent_ptr)
            {
                if (!visual_root)
                {
                    auto bcast_backup = broadcast;
                    base::switch_to_bus(parent_ptr->base::broadcast);
                    parent_ptr->SUBMIT_T(tier::release, e2::config::broadcast, broadcast_update_token, new_broadcast)
                    {
                        broadcast = new_broadcast;
                        this->SIGNAL(tier::release, e2::config::broadcast, new_broadcast);
                    };
                }
                parent_shadow = parent_ptr;
                // Propagate form events up to the visual branch.
                // Exec after all subscriptions.
                parent_ptr->SUBMIT_T(tier::release, hids::events::upevent::any, kb_offer_token, gear)
                {
                    if (auto parent_ptr = parent_shadow.lock())
                    {
                        if (gear.focus_taken())
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
                    kb_offer_token.reset();
                    if (!visual_root)
                    {
                        broadcast_update_token.reset();
                    }
                }
                parent_ptr->base::reflow();
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
                    parent_canvas.fill([&](cell& c) { c.fusefull(base::brush); });
            };
        }

    public:
        template<class T = base>
        auto  This()       { return std::static_pointer_cast<typename std::remove_reference<T>::type>(shared_from_this()); }
        auto& coor() const { return square.coor; }
        auto& size() const { return square.size; }
        auto& area() const { return square; }
        auto parent()      { return parent_shadow.lock(); }
        void isroot(bool state) { visual_root = state; }
        void ruined(bool state) { invalid = state; }
        auto ruined() const { return invalid; }
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
            SIGNAL(tier::release, e2::coor::set, new_coor);
            auto delta = square.coor - old_coor;
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
        // base: Resize the form relative the center point.
        //       Return center point offset.
        //       The object is responsible for correcting
        //       the center point during resizing.
        auto resize(twod newsize, twod point)
        {
            point -= square.coor;
            anchor = point;
            resize(newsize);
            return point - anchor;
        }
        // base: Dry run (preview then release) current value.
        auto resize()
        {
            auto new_value = square.size;
            return resize(new_value);
        }
        // base: Move the form by the specified step and return the coor delta.
        auto moveby(twod const& step)
        {
            auto delta = moveto(square.coor + step);
            return delta;
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
        void reflow()
        {
            auto parent_ptr = parent();
            if (parent_ptr && !visual_root)
            {
                parent_ptr->reflow();
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
            events::sync lock;
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
        //          base::riseup<tier::preview, e2::form::prop::header>(txt);
        template<tier TIER, class EVENT, class T>
        void riseup(EVENT, T&& data)
        {
            if (!SIGNAL(TIER, EVENT{}, data))
            {
                base::toboss([&](auto& boss)
                {
                    boss.base::template riseup<TIER>(EVENT{}, std::forward<T>(data));
                });
            }
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
                    iota count; // sock: Clients count.
                    sock(id_t ctrl)
                        :    id{ ctrl },
                          count{ 0    }
                    { }
                    operator bool(){ return T::operator bool(); }
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
                        if (item.id == gear.id)
                            return item;

                    if constexpr (CONST_WARN)
                        log("sock: error: access to unregistered input device, id:", gear.id);
                    return items.emplace_back(gear.id);
                }
                template<class P>
                void foreach(P proc)
                {
                    for (auto& item : items)
                        if (item) proc(item);
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
                        if (item.id == gear.id)
                        {
                            if (items.size() > 1) item = items.back(); // Remove an item without allocations.
                            items.pop_back();
                            return;
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
                twod origin; // sock: Grab's initial coord info.
                twod dtcoor; // sock: The form coor parameter change factor while resizing.
                twod dtsize; // sock: The form size parameter change factor while resizing.
                twod sector; // sock: Active quadrant, x,y = {-1|+1}. Border widths.
                rect hzgrip; // sock: Horizontal grip.
                rect vtgrip; // sock: Vertical grip.
                twod widths; // sock: Grip's widths.
                bool inside; // sock: Is active.
                bool seized; // sock: Is seized.

                sock()
                    : inside{ faux },
                      seized{ faux }
                { }
                operator bool(){ return inside || seized; }
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
                        dtsize = dtcoor.less(dot_11, dot_11,-dot_11);
                        sector = dtcoor.less(dot_11,-dot_11, dot_11);
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
                }
                auto drag(base& master, twod const& curpos, dent const& outer)
                {
                    if (seized)
                    {
                        auto width = master.base::size() + outer;
                        auto delta = curpos - corner(width) - origin;
                        if (auto dxdy = master.base::sizeby(delta * dtsize))
                        {
                            master.base::moveby(-dxdy * dtcoor);
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
                  width{ outer - inner }
            {
                boss.SUBMIT_T(tier::release, e2::postrender, memo, canvas)
                {
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
            // pro::sizer: Configuring the mouse button to operate.
            template<sysmouse::bttns BUTTON>
            void engage()
            {
                boss.SIGNAL(tier::release, e2::form::draggable::_<BUTTON>, true);
                boss.SUBMIT_T(tier::release, hids::events::mouse::move, memo, gear)
                {
                    items.take(gear).calc(boss, gear.coord, outer, inner, width);
                    boss.base::deface();
                };
                boss.SUBMIT_T(tier::release, e2::form::drag::start::_<BUTTON>, memo, gear)
                {
                    if (items.take(gear).grab(boss, gear.coord, outer))
                        gear.dismiss();
                };
                boss.SUBMIT_T(tier::release, e2::form::drag::pull::_<BUTTON>, memo, gear)
                {
                    if (items.take(gear).drag(boss, gear.coord, outer))
                        gear.dismiss();
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
                twod  origin; // sock: Grab's initial coord info.
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
                twod cursor;        // sock: Coordinates of the active cursor.
                bool inside = faux; // sock: Is active.
                operator bool(){ return inside; }
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
            list items;

        public:
            track(base&&) = delete;
            track(base& boss)
                : skill{ boss }
            {
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
            using skill::boss,
                  skill::memo;
            using gptr = wptr<bell>;
            rect last; // pro::align: Window size before the fullscreen has applied.
            text head; // pro::align: Main window title the fullscreen has applied.
            id_t weak; // pro::align: Master id.
            rect body; // pro::align: For current coor/size tracking.
            twod pads; // pro::align: Owner's borders.
            hook maxs; // pro::align: Maximize on dblclick token.

            auto seized(id_t master)
            {
                return weak == master;
            }

        public:
            align(base&&) = delete;
            align(base& boss, bool maximize = true) : skill{ boss },
                weak{}
            {
                if (maximize)
                {
                    boss.SUBMIT_T(tier::release, hids::events::mouse::button::dblclick::left, maxs, gear)
                    {
                        auto size = boss.base::size();
                        if (size.inside(gear.coord))
                        {
                            if (seized(gear.id)) unbind();
                            else                 follow(gear.id, dot_00);
                            gear.dismiss();
                        }
                    };
                }
            }
            ~align() { unbind(faux); }

            void follow(id_t master, twod const& borders)
            {
                pads = borders;
                if (auto gate_ptr = bell::getref(master))
                {
                    auto& gate = *gate_ptr;

                    rect area;
                    gate.SIGNAL(tier::request, e2::size::set, area.size);
                    gate.SIGNAL(tier::request, e2::coor::set, area.coor);
                    last = boss.base::area();
                    area.coor -= pads;
                    area.size += pads * 2;
                    body = {}; // In oder to unbind previous subscription if it is.
                    boss.base::extend(area);
                    body = area;

                    text newhead;
                    gate.SIGNAL(tier::request, e2::form::prop::header, head);
                    boss.SIGNAL(tier::request, e2::form::prop::header, newhead);
                    gate.SIGNAL(tier::preview, e2::form::prop::header, newhead);
                    gate.SIGNAL(tier::release, e2::form::prop::fullscreen, true);

                    gate.SUBMIT_T(tier::release, e2::size::set, memo, size)
                    {
                        body.size = size + pads * 2;
                        boss.base::resize(body.size);
                    };
                    gate.SUBMIT_T(tier::release, e2::coor::set, memo, coor)
                    {
                        unbind();
                    };
                    gate.SUBMIT_T(tier::release, e2::dtor, memo, master_id)
                    {
                        unbind();
                    };

                    boss.SUBMIT_T(tier::release, e2::size::set, memo, size)
                    {
                        if (weak && body.size != size) unbind(faux);
                    };
                    boss.SUBMIT_T(tier::release, e2::coor::set, memo, coor)
                    {
                        if (weak && body.coor != coor) unbind();
                    };

                    weak = master;
                    boss.SUBMIT_T(tier::release, e2::form::prop::header, memo, newhead)
                    {
                        if (auto gate_ptr = bell::getref(weak))
                        {
                            gate_ptr->SIGNAL(tier::preview, e2::form::prop::header, newhead);
                        }
                        else unbind();
                    };
                }
            }
            void unbind(bool restor_size = true)
            {
                if (memo.count())
                {
                    memo.clear();
                    if (auto gate_ptr = bell::getref(weak))
                    {
                        gate_ptr->SIGNAL(tier::preview, e2::form::prop::header, head);
                        gate_ptr->SIGNAL(tier::release, e2::form::prop::fullscreen, faux);
                    }
                }
                weak = {};
                if (restor_size) boss.base::extend(last); // Restore previous position
            }
        };

        // pro: Provides functionality for runtime animation (time-based).
        class robot
            : public skill
        {
            using skill::boss;
            using subs = std::map<id_t, hook>;
            subs memo;

        public:
            using skill::skill; // Inherits ctors.

            // pro::robot: Every timer tick, yield the
            //             delta from the flow and, if delta,
            //             Call the proc (millisecond precision).
            template<class P, class S>
            void actify(id_t ID, S flow, P proc)
            {
                auto handler = [&, ID, proc, flow](auto p)
                {
                    auto now = datetime::round<iota>(p);
                    if (auto data = flow(now))
                    {
                        proc(data.value());
                    }
                    else
                    {
                        pacify(ID);
                    }
                };
                //boss.SUBMIT_TV(tier::general, e2::timer::any, memo[ID], handler);
                boss.SUBMIT_TV(tier::general, e2::tick, memo[ID], handler);
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
            using skill::boss;
            using subs = std::map<id_t, hook>;
            subs memo;

        public:
            using skill::skill; // Inherits ctors.

            // pro::timer: Start countdown for specified ID.
            template<class P>
            void actify(id_t ID, period timeout, P lambda)
            {
                auto alarm = tempus::now() + timeout;
                auto handler = [&, ID, timeout, lambda, alarm](auto now)
                {
                    if (now > alarm)
                    {
                        if (!lambda(ID)) pacify(ID);
                    }
                };
                //boss.SUBMIT_TV(tier::general, e2::timer::any, memo[ID], handler);
                boss.SUBMIT_TV(tier::general, e2::tick, memo[ID], handler);
                //boss.SIGNAL(tier::release, e2::form::animate::start, ID);
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
            iota  seat;

        public:
            frame(base&&) = delete;
            frame(base& boss, iota z_order = Z_order::plain) : skill{ boss },
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
                boss.SUBMIT_T(tier::release, e2::form::drag::pull::left, memo, gear)
                {
                    if (gear)
                    {
                        auto delta = gear.delta.get();
                        boss.base::moveby(delta);
                        boss.SIGNAL(tier::preview, e2::form::upon::changed, delta);
                        gear.dismiss();
                    }
                };
                boss.SUBMIT_T(tier::release, e2::form::upon::dragged, memo, gear)
                {
                    if (gear.meta(hids::ANYCTRL))
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
                iota time = SWITCHING_TIME;
                auto func = constlinearAtoB<twod>(path, time, now<iota>());

                robo.pacify();
                robo.actify(func, [&](twod& x) { boss.base::moveby(x); boss.strike(); });
            }
            /*
            // pro::frame: Search for a non-overlapping form position in
            //             the visual tree along a specified direction.
            rect bounce (rect const& block, twod const& dir)
            {
                rect result = block.rotate(dir);
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
            void convey (twod const& delta, rect const& boundary)//, bool notify = true)
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
            void expose (bool subsequent = faux)
            {
                if (auto parent_ptr = boss.parent())
                {
                    parent_ptr->SIGNAL(tier::release, e2::form::layout::expose, boss);
                }
                //return boss.status.exposed;
            }
            // pro::frame: Place the form in front of the visual tree among neighbors.
            void bubble ()
            {
                if (auto parent_ptr = boss.parent())
                {
                    parent_ptr->SIGNAL(tier::release, e2::form::layout::bubble, boss);
                }
            }
        };

        // pro: Form generator functionality.
        class maker
            : public skill
        {
            using skill::boss,
                  skill::memo;
            cell mark;

            struct slot_t
            {
                rect slot;
                twod step;
                twod init;
                bool ctrl = faux;
            };
            std::map<id_t, slot_t> slots;
            ansi::esc coder;

            void check_modifiers(hids& gear)
            {
                auto& data = slots[gear.id];
                auto state = !!gear.meta(hids::ANYCTRL);
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

                    data.ctrl = gear.meta(hids::ANYCTRL);
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
                    gear.release();
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
                        boss.SIGNAL(tier::preview, e2::form::proceed::createby, gear);
                    }
                    slots.erase(gear.id);
                    gear.dismiss();
                    gear.release();
                }
            }

        public:
            maker(base&&) = delete;
            maker(base& boss) : skill{ boss },
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

                boss.SUBMIT_T(tier::general, hids::events::die, memo, gear)
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

                                // Calc average bg brightness.
                                auto count = 0;
                                auto light = 0;
                                auto sumfx = [&](cell& c)
                                {
                                    count++;
                                    auto& clr = c.bgc();
                                    light += clr.chan.r + clr.chan.g + clr.chan.b;
                                };
                                auto head = area;
                                head.size.y = 1;
                                canvas.each(head, sumfx);
                                auto b = count ? light / (count * 3) : 0;

                                // Draw the frame.
                                auto mark = skin::color(tone::kb_focus);
                                auto fill = [&](cell& c) { c.fuse(mark); };
                                canvas.cage(area, dot_11, fill);
                                coder.wrp(wrap::off).fgc(b > 130 ? 0xFF000000
                                                                 : 0xFFFFFFFF).add("capture area: ", slot);
                                auto size = para(coder);
                                coder.clear();
                                //canvas.cup(area.coor);
                                //canvas.output(size);

                                auto header = *size.lyric;
                                header.move(area.coor + canvas.coor());
                                canvas.fill(header);
                            }
                            else
                            {
                                auto temp = canvas.view();
                                canvas.view(area);
                                canvas.fill(area, [&](cell& c) { c.fuse(mark); c.und(faux); });
                                canvas.blur(10);
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
            rect   body; // caret: Caret position.
            period step; // caret: Blink interval. period::zero() if steady.
            moment next; // caret: Time of next blinking.
            bool   form; // caret: Caret style: true - box; faux - underline.

        public:
            caret(base&&) = delete;
            caret(base& boss, bool visible = faux, twod position = dot_00, bool abox = faux) : skill{ boss },
                live{ faux },
                done{ faux },
                form{ abox },
                body{ position, dot_11 }, // Caret is always one cell size (see the term::scrollback definition).
                step{ BLINK_PERIOD }
            {
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

            operator bool() const { return memo.count(); }

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
                        boss.SUBMIT_T(tier::general, e2::tick, memo, timestamp)
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
                        if (live)
                        {
                            auto field = canvas.core::view();
                            auto point = body;
                            point.coor += field.coor + boss.base::coor();
                            if (auto area = field.clip(point))
                            {
                                if (form) canvas.fill(area, [](cell& c) { c.inv(!c.inv()); });
                                else      canvas.fill(area, [](cell& c) { c.und(!c.und()); });
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
                period    render = period::zero();
                period    output = period::zero();
                iota      frsize = 0;
                long long totals = 0;
                //bool      onhold = faux; // info: Indicator that the current frame has been successfully STDOUT
                iota      number = 0;    // info: Current frame number
            }
            track; // debug: Textify the telemetry data for debugging purpose.

            void shadow()
            {
                for (int i = 0; i < prop::count; i++)
                {
                    status[i].ease();
                }
            }

        public:
            bool bypass = faux;
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
            void update(period const& watch, iota delta)
            {
                track.output = watch;
                track.frsize = delta;
                track.totals+= delta;
            }
            void update(moment const& timestamp)
            {
                track.render = tempus::now() - timestamp;
            }

            debug(base&&) = delete;
            debug(base& boss) : skill{ boss }
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
                iota attr = 0;
                for (auto& desc : description)
                {
                    status += coder.add(" ", utf::adjust(desc, maxlen, " ", true), " ").idx(attr++).nop().nil().eol();
                    coder.clear();
                }

                boss.SUBMIT_T(tier::release, e2::postrender, memo, canvas)
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
                };

                //boss.SUBMIT_T(tier::release, e2::debug, owner::memo, track)
                //{
                //	status[prop::render_ns].set(track.output > 12ms ? alerts : stress) =
                //		utf::adjust(utf::format(track.output.count()), 11, " ", true) + "ns";
                //
                //	status[prop::proceed_ns].set(track.render > 12ms ? alerts : stress) =
                //		utf::adjust(utf::format (track.render.count()), 11, " ", true) + "ns";
                //
                //	status[prop::frame_size].set(stress) =
                //		utf::adjust(utf::format(track.frsize), 7, " ", true) + " bytes";
                //});

                //boss.SUBMIT_T(tier::release, e2::conio::size, owner::memo, newsize)
                //{
                //	shadow();
                //	status[prop::last_event].set(stress) = "size";
                //
                //	status[prop::win_size].set(stress) =
                //		std::to_string(newsize.x) + " x " +
                //		std::to_string(newsize.y);
                //});

                boss.SUBMIT_T(tier::release, e2::conio::focus, memo, focusstate)
                {
                    update(focusstate);
                    boss.base::strike(); // to update debug info
                };
                boss.SUBMIT_T(tier::release, e2::size::set, memo, newsize)
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

                    for (int btn = 0; btn < sysmouse::numofbutton; btn++)
                    {
                        auto& state = status[prop::mouse_btn_1 + btn].set(stress);

                        state = m.button[btn].pressed ? "pressed" : "";
                        if (m.button[btn].flipped)
                            state += state.length() ? " | flipped" : "flipped";

                        if (m.button[btn].dragged)
                            state += state.length() ? " | dragged" : "dragged";

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

                boss.SUBMIT_T(tier::release, e2::conio::key, memo, gear)
                {
                    shadow();
                    auto& k = gear;
                    #ifdef KEYLOG
                    log("debug fired ", k.character);
                    #endif
                    status[prop::last_event   ].set(stress) = "key";
                    status[prop::key_pressed  ].set(stress) = k.down ? "pressed" : "idle";
                    status[prop::ctrl_state   ].set(stress) = "0x" + utf::to_hex(k.ctlstate );
                    status[prop::key_code     ].set(stress) = "0x" + utf::to_hex(k.virtcode );
                    status[prop::key_scancode ].set(stress) = "0x" + utf::to_hex(k.scancode );
                    status[prop::key_character].set(stress) = "0x" + utf::to_hex(k.character);
                    //status[prop::key_repeat   ].set(stress) = std::to_string(k.repeatcount);

                    if (!k.character && k.textline.length())
                    {
                        auto t = k.textline;
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
            text head_name; // title: Preserve original header.
            text foot_name; // title: Preserve original footer.
            twod head_size; // title: Header page size.
            twod foot_size; // title: Footer page size
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
                recalc(head_page, head_size);
                recalc(foot_page, foot_size);
            }
            void header(view newtext)
            {
                head_page = newtext;
                head_name = newtext;
                recalc(head_page, head_size);
                boss.SIGNAL(tier::release, e2::form::prop::header, head_name);
                /*
                textline.link(boss.id);
                boss.SIGNAL(tier::release, e2::form::prop::header, head_name);
                boss.SIGNAL(tier::release, e2::form::state::header, textline);
                */
            }
            void footer(view newtext)
            {
                foot_page = newtext;
                foot_name = newtext;
                recalc(foot_page, foot_size);
                boss.SIGNAL(tier::release, e2::form::prop::footer, foot_name);
                /*
                textline.link(boss.id);
                boss.SIGNAL(tier::release, e2::form::prop::footer, foot_name);
                boss.SIGNAL(tier::release, e2::form::state::footer, textline);
                */
            }
            void init()
            {
                boss.SUBMIT_T(tier::release, e2::size::set, memo, new_size)
                {
                    recalc(new_size);
                };
                boss.SUBMIT_T(tier::release, e2::postrender, memo, canvas)
                {
                    if (live)
                    {
                        auto contrast_fx = [](auto& dst, auto& src)
                        {
                            auto& fgc = src.fgc();
                            if (fgc.chan.a == 0x00)
                            {
                                auto constexpr threshold = rgba{ tint::whitedk }.luma();
                                if (dst.bgc().luma() >= threshold) dst.fgc(0xFF000000).fusefull(src);
                                else                               dst.fgc(0xFFffffff).fusefull(src);
                            }
                            else dst.fusefull(src);
                        };
                        auto saved_context = canvas.bump(dent{ 0,0,head_size.y,foot_size.y });
                        canvas.cup(dot_00);
                        canvas.output(head_page, contrast_fx);
                        canvas.cup({ 0, head_size.y + boss.size().y });
                        canvas.output(foot_page, contrast_fx);
                        canvas.bump(saved_context);
                    }
                };
                boss.SUBMIT_T(tier::preview, e2::form::prop::header, memo, newtext)
                {
                    header(newtext);
                };
                boss.SUBMIT_T(tier::preview, e2::form::prop::footer, memo, newtext)
                {
                    footer(newtext);
                };
                boss.SUBMIT_T(tier::request, e2::form::prop::header, memo, curtext)
                {
                    curtext = head_name;
                };
                boss.SUBMIT_T(tier::request, e2::form::prop::footer, memo, curtext)
                {
                    curtext = foot_name;
                };
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
                : skill{ boss }
            {
                init();
            }
            title(base& boss, view title, bool visible = true)
                : skill{ boss }
            {
                init();
                header(title);
                live = visible;
                #ifdef DEMO
                footer(ansi::jet(bias::right) + "test\nmultiline\nfooter");
                #endif
            }
        };

        // pro: Provides functionality for the scene objects manipulations.
        class scene
            : public skill
        {
            class node // pro::scene: Helper-class for the pro::scene. Adapter for the object that going to be attached to the scene.
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
                    iota active = 0;
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

                        int i = 0;
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
                iota z_order = Z_order::plain;

                node(sptr item)
                    : object{ item }
                {
                    auto& inst = *item;
                    obj_id = inst.bell::id;

                    inst.SUBMIT(tier::release, e2::form::prop::zorder, order)
                    {
                        z_order = order;
                    };
                    inst.SUBMIT(tier::release, e2::size::set, size)
                    {
                        region.size = size;
                    };
                    inst.SUBMIT(tier::release, e2::coor::set, coor)
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
                // node: Check equality.
                bool equals(id_t id)
                {
                    return obj_id == id;
                }
                // node: Draw the anchor line func and return true
                //       if the mold is outside the canvas area.
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
                // node: Output the title to the canvas.
                //void enlist(face& canvas)
                //{
                //    if (header)
                //    {
                //        auto& title = header.get();
                //        canvas.output(title);
                //        canvas.eol();
                //    }
                //}
                // node: Visualize the underlying object.
                void render(face& canvas)
                {
                    canvas.render(*object);
                }

                void postrender(face& canvas)
                {
                    object->SIGNAL(tier::release, e2::postrender, canvas);
                }
            };

            class list // pro::scene: Helper-class for the pro::scene. List of objects that can be reordered, etc.
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
                // Draw backpane for spectators.
                void prerender(face& canvas)
                {
                    for (auto& item : items) item->fasten(canvas); // Draw strings.
                    for (auto& item : items) item->render(canvas); // Draw shadows.
                }
                // Draw windows.
                void render(face& canvas)
                {
                    for (auto& item : items) item->fasten(canvas);
                    //todo optimize
                    for (auto& item : items) if (item->z_order == Z_order::backmost) item->render(canvas);
                    for (auto& item : items) if (item->z_order == Z_order::plain   ) item->render(canvas);
                    for (auto& item : items) if (item->z_order == Z_order::topmost ) item->render(canvas);
                }
                // Draw spectator's mouse pointers.
                void postrender(face& canvas)
                {
                    for (auto& item : items) item->postrender(canvas);
                }

                rect remove(id_t id)
                {
                    rect area;
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
                            {
                            }

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

            using skill::boss,
                  skill::memo;
            using proc = drawfx;
            using time = moment;
            using area = std::vector<rect>;

            area edges; // scene: wrecked regions history
            proc paint; // scene: Render all child items to the specified canvas
            list items; // scene: Child visual tree
            list users; // scene: Scene spectators

            sptr<registry_t> app_registry;
            sptr<std::list<sptr<base>>> usr_registry;

        public:
            scene(base&&) = delete;
            scene(base& boss) : skill{ boss },
                app_registry{ std::make_shared<registry_t>() },
                usr_registry{ std::make_shared<std::list<sptr<base>>>() }
            {
                paint = [&](face& canvas, page const& titles) -> bool
                {
                    if (edges.size())
                    {
                        canvas.wipe(boss.id);
                        canvas.output(titles);
                        //todo revise
                        if (users.size() > 1) users.prerender(canvas); // Draw backpane for spectators
                        items.render    (canvas); // Draw objects of the world
                        users.postrender(canvas); // Draw spectator's mouse pointers
                        return true;
                    }
                    else return faux;
                };

                boss.SUBMIT_T(tier::preview, e2::form::proceed::detach, memo, item_ptr)
                {
                    auto& inst = *item_ptr;
                    denote(items.remove(inst.id));
                    denote(users.remove(inst.id));

                    //todo unify
                    bool found = faux;
                    // Remove from active app registry.
                    for (auto& [class_id, app_list] : *app_registry)
                    {
                        auto head = app_list.begin();
                        auto tail = app_list.end();
                        auto iter = std::find_if(head, tail, [&](auto& c) { return c == item_ptr; });
                        if (iter != tail)
                        {
                            app_list.erase(iter);
                            found = true;
                            break;
                        }
                    }
                    { // Remove user.
                        auto& subset = *usr_registry;
                        auto head = subset.begin();
                        auto tail = subset.end();
                        auto item = std::find_if(head, tail, [&](auto& c){ return c == item_ptr; });
                        if (item != tail)
                        {
                            subset.erase(item);
                            found = true;
                        }
                    }
                    if (found)
                        inst.SIGNAL(tier::release, e2::form::upon::vtree::detached, boss.This());
                };
                boss.SUBMIT_T(tier::release, e2::form::layout::bubble, memo, inst)
                {
                    auto region = items.bubble(inst.bell::id);
                    denote(region);
                };
                boss.SUBMIT_T(tier::release, e2::form::layout::expose, memo, inst)
                {
                    auto region = items.expose(inst.bell::id);
                    denote(region);
                };
                boss.SUBMIT_T(tier::request, e2::bindings::list::users, memo, usr_list)
                {
                    usr_list = usr_registry;
                };
                boss.SUBMIT_T(tier::request, e2::bindings::list::apps, memo, app_list)
                {
                    app_list = app_registry;
                };


                // pro::scene: Proceed request for available objects (next)
                boss.SUBMIT_T(tier::request, e2::form::proceed::attach, memo, next)
                {
                    if (items)
                        if (auto next_ptr = items.rotate_next())
                            next = next_ptr->object;
                };
                // pro::scene: Proceed request for available objects (prev)
                boss.SUBMIT_T(tier::request, e2::form::proceed::detach, memo, prev)
                {
                    if (items)
                        if (auto prev_ptr = items.rotate_prev())
                            prev = prev_ptr->object;
                };
            }

            // pro::scene: .
            void redraw()
            {
                boss.SIGNAL(tier::release, e2::form::proceed::render, paint);
                edges.clear();
            }
            // pro::scene: Mark dirty region.
            void denote(rect const& updateregion)
            {
                if (updateregion)
                {
                    edges.push_back(updateregion);
                }
            }

            // pro::scene: Attach a new item to the scene.
            template<class S>
            auto branch(id_t class_id, sptr<S> item)
            {
                items.append(item);
                item->base::isroot(true);
                (*app_registry)[class_id].push_back(item);
                item->SIGNAL(tier::release, e2::form::upon::vtree::attached, boss.base::This());

                boss.SIGNAL(tier::release, e2::bindings::list::apps, app_registry);
                return item;
            }
            // pro::scene: Create a new item of the specified subtype
            //             and attach it to the scene.
            template<class S, class ...Args>
            auto attach(id_t class_id, Args&&... args)
            {
                auto item = boss.indexer<bell>::create<S>(std::forward<Args>(args)...);
                branch(class_id, item);
                return item;
            }
            // pro::scene: Create a new user of the specified subtype
            //             and invite him to the scene.
            template<class S, class ...Args>
            auto invite(Args&&... args)
            {
                auto user = boss.indexer<bell>::create<S>(std::forward<Args>(args)...);
                users.append(user);
                usr_registry->push_back(user);
                user->base::isroot(true);
                user->SIGNAL(tier::release, e2::form::upon::vtree::attached, boss.base::This());

                //todo unify
                tone color{ tone::brighter, tone::shadow};
                user->SIGNAL(tier::preview, e2::form::state::color, color);

                boss.SIGNAL(tier::release, e2::bindings::list::users, usr_registry);
                return user;
            }
        };

        // pro: Perform graceful shutdown functionality. LIMIT in seconds, ESC_THRESHOLD in milliseconds.
        class guard
            : public skill
        {
            using skill::boss,
                  skill::memo;
            constexpr static auto QUIT_MSG = e2::conio::quit;
            constexpr static iota ESC_THRESHOLD = 500; // guard: Double escape threshold in ms.

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
                boss.SUBMIT_T(tier::general, e2::tick, memo, timestamp)
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
            constexpr static auto QUIT_MSG   = e2::command::quit;
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

                    //doubt.reset();
                    //alibi.reset();
                };

                boss.SUBMIT_T(tier::general, e2::tick, ping, something)
                {
                    if (tempus::now() > stop)
                    {
                        auto shadow = boss.This();
                        boss.SIGNAL(tier::release, QUIT_MSG, desc);
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
            hook accept_kbd;
            iota clients = 0;

        public:
            bool focusable = true;

            keybd(base&&) = delete;
            keybd(base& boss) : skill{ boss }
            {
                using bttn = hids::events::mouse::button;

                boss.SUBMIT_T(tier::release, bttn::click::left, memo, gear)
                {
                    // Propagate throughout nested objects by base::
                    gear.kb_focus_taken = faux;
                    boss.SIGNAL(tier::release, hids::events::upevent::kboffer, gear);

                    //gear.set_kb_focus(boss.This());
                    if (gear.focus_taken()) gear.dismiss();

                    //if (!square.size.inside(gear.coord))
                    //{
                    //	auto center = square.coor + (square.size / 2);
                    //	bell::getref(gear.id)->
                    //		SIGNAL(tier::release, e2::form::layout::shift, center);
                    //}
                };

                // pro::keybd: Notify form::state::kbfocus when the number of clients is positive.
                boss.SUBMIT_T(tier::release, hids::events::notify::keybd::got, memo, gear)
                {
                    //if (!highlightable || gear.begin_inform(boss.bell::id))
                    {
                        if (!clients++)
                        {
                            boss.SIGNAL(tier::release, e2::form::state::keybd, true);
                        }
                    }
                };
                // pro::keybd: Notify form::state::active_kbd when the number of clients is zero.
                boss.SUBMIT_T(tier::release, hids::events::notify::keybd::lost, memo, gear)
                {
                    //if (!highlightable || gear.end_inform(boss.bell::id))
                    {
                        if (!--clients)
                        {
                            boss.SIGNAL(tier::release, e2::form::state::keybd, faux);
                        }
                    }
                };
                boss.SUBMIT_T(tier::request, e2::form::state::keybd, memo, state)
                {
                    state = !!clients;
                };
                boss.SUBMIT_T(tier::preview, hids::events::keybd::any, memo, gear)
                {
                    #ifdef KEYLOG
                    log("keybd fired virtcode: ", gear.virtcode,
                                      " chars: ", utf::debase(gear.keystrokes),
                                       " meta: ", gear.meta());
                    #endif

                    boss.SIGNAL(tier::release, hids::events::keybd::any, gear);
                };
            };

            // pro::keybd: Subscribe on keybd offers.
            void accept(bool value)
            {
                if (value)
                {
                    boss.SUBMIT_T(tier::release, hids::events::upevent::kboffer, accept_kbd, gear)
                    {
                        if (!gear.focus_taken())
                        {
                            gear.set_kb_focus(boss.This());
                            boss.bell::expire<tier::release>();
                        }
                    };
                }
                else
                {
                    accept_kbd.reset();
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
            iota       rent; // mouse: Active gears count.
            iota       full; // mouse: All gears count. Counting to keep the entire chain of links in the visual tree.
            bool       omni; // mouse: Ability to accept all hover events (true) or only directly over the object (faux).
            iota       drag; // mouse: Bitfield of buttons subscribed to mouse drag.
        public:
            mouse(base&&) = delete;
            mouse(base& boss, bool take_all_events = true) : skill{ boss },
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
                    if (!full++) soul = boss.This();
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
                    if (!--full) { soul->base::strike(); soul.reset(); }
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
                    switch(auto deed = boss.bell::protos<tier::release>())
                    {
                        default:
                        case e2::form::draggable::left     .id: draggable<sysmouse::left     >(); break;
                        case e2::form::draggable::right    .id: draggable<sysmouse::right    >(); break;
                        case e2::form::draggable::leftright.id: draggable<sysmouse::leftright>(); break;
                        case e2::form::draggable::middle   .id: draggable<sysmouse::middle   >(); break;
                        case e2::form::draggable::wheel    .id: draggable<sysmouse::wheel    >(); break;
                        case e2::form::draggable::win      .id: draggable<sysmouse::win      >(); break;
                    }
                };
            }
            void reset()
            {
                events::sync lock;
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
            void draggable()
            {
                if (!(drag & 1 << BUTTON))
                {
                    drag |= 1 << BUTTON;
                    //using bttn = hids::events::mouse::button; //MSVC 16.9.4 don't get it
                    boss.SUBMIT(tier::release, hids::events::mouse::button::drag::start::_<BUTTON>, gear)
                    {
                        if (gear.capture(boss.bell::id))
                        {
                            boss.SIGNAL(tier::release, e2::form::drag::start::_<BUTTON>, gear);
                            gear.dismiss();
                        }
                    };
                    boss.SUBMIT(tier::release, hids::events::mouse::button::drag::pull::_<BUTTON>, gear)
                    {
                        if (gear.captured(boss.bell::id))
                        {
                            boss.SIGNAL(tier::release, e2::form::drag::pull::_<BUTTON>, gear);
                            gear.dismiss();
                        }
                    };
                    boss.SUBMIT(tier::release, hids::events::mouse::button::drag::cancel::_<BUTTON>, gear)
                    {
                        if (gear.captured(boss.bell::id))
                        {
                            boss.SIGNAL(tier::release, e2::form::drag::cancel::_<BUTTON>, gear);
                            gear.release();
                            gear.dismiss();
                        }
                    };
                    boss.SUBMIT(tier::general, hids::events::die, gear)
                    {
                        if (gear.captured(boss.bell::id))
                        {
                            boss.SIGNAL(tier::release, e2::form::drag::cancel::_<BUTTON>, gear);
                            gear.release();
                            gear.dismiss();
                        }
                    };
                    boss.SUBMIT(tier::release, hids::events::mouse::button::drag::stop::_<BUTTON>, gear)
                    {
                        if (gear.captured(boss.bell::id))
                        {
                            boss.SIGNAL(tier::release, e2::form::drag::stop::_<BUTTON>, gear);
                            gear.release();
                            gear.dismiss();
                        }
                    };
                }
            }
        };

        // pro: Provides functionality related to keyboard interaction.
        class input
            : public skill, public hids
        {
            using lock = std::recursive_mutex;
            using skill::boss,
                  skill::memo;
        public:
            core xmap;
            lock sync;
            iota push = 0; // input: Mouse pressed buttons bits (Used only for foreign mouse pointer in the gate).

            input(base&&) = delete;
            input(base& boss)
                : skill{ boss }, hids{ boss, xmap }
            {
                xmap.move(boss.base::coor());
                xmap.size(boss.base::size());
                boss.SUBMIT_T(tier::release, e2::size::set, memo, newsize)
                {
                    std::unique_lock guard(sync); // Syncing with diff::render thread.
                    xmap.size(newsize);
                };
                boss.SUBMIT_T(tier::release, e2::coor::set, memo, newcoor)
                {
                    xmap.move(newcoor);
                };
                boss.SUBMIT_T(tier::release, e2::conio::mouse, memo, mousestate)
                {
                    push = hids::take(mousestate);
                    boss.strike();
                };
                boss.SUBMIT_T(tier::release, e2::conio::key, memo, keybdstate)
                {
                    hids::take(keybdstate);
                    boss.strike();
                };
            };
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
                    iota size = 5; // grade: Vertical gradient size.
                    iota step = 2; // grade: Vertical gradient step.
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

                    for (int i = 1; i < n; i++)
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
            iota transit;
            cell c1;
            cell c2;
            bool fake = faux;

            //todo use lambda
            void work(iota transit)
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
                transit{ 0 }
            {
                boss.base::color(c1.fgc(), c1.bgc());
                boss.SUBMIT(tier::release, e2::form::prop::brush, brush)
                {
                    if (!fake)
                    {
                        c1.fgc(brush.fgc());
                        c1.bgc(brush.bgc());
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
                            auto limit = datetime::round<iota>(fade);
                            auto start = datetime::now<iota>();
                            robo.actify(constlinearAtoB<iota>(range, limit, start), [&](auto step)
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
            static constexpr auto max_value = twod{ 2000, 1000 }; //todo unify
            using skill::boss,
                  skill::memo;
            struct lims_t
            {
                twod min = min_value;
                twod max = max_value;
            }
            lims;
            bool sure; // limit: Reepeat size checking afetr all.

        public:
            limit(base&&) = delete;
            limit(base& boss, twod const& min_size = -dot_11, twod const& max_size = -dot_11, bool forced = faux)
                : skill{ boss }
            {
                set(min_size, max_size, forced);
                // Clamping before all.
                boss.SUBMIT_T(tier::preview, e2::size::any, memo, new_size)
                {
                    new_size = std::clamp(new_size, lims.min, lims.max);
                };
                // Clamping after all.
                boss.SUBMIT_T(tier::preview, e2::size::set, memo, new_size)
                {
                    if (sure)
                        new_size = std::clamp(new_size, lims.min, lims.max);
                };
            }
            // pro::limit: Set size limits (min, max). Preserve current value if specified arg less than 0.
            void set(twod const& min_size, twod const& max_size = -dot_11, bool forced = faux)
            {
                sure = forced;
                lims.min = min_size.less(dot_00, min_value, min_size);
                lims.max = max_size.less(dot_00, max_value, max_size);
            }
            // pro::limit: Set resize limits (min, max). Preserve current value if specified arg less than 0.
            void set(lims_t const& new_limits, bool forced = faux)
            {
                set(new_limits.min, new_limits.max, forced);
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

        public:
            face& canvas; // cache: Bitmap cache.

            cache(base&&) = delete;
            cache(base& boss, bool rendered = true)
                : skill{ boss },
                  canvas{*(coreface = std::make_shared<face>())}
            {
                canvas.link(boss.bell::id);
                canvas.move(boss.base::coor());
                canvas.size(boss.base::size());
                boss.SUBMIT_T(tier::release, e2::form::upon::vtree::attached, memo, parent_ptr)
                {
                    boss.SIGNAL(tier::general, e2::form::canvas, canvas.shared_from_this());
                };
                boss.SUBMIT_T(tier::release, e2::coor::set, memo, new_xy) { canvas.move(new_xy); };
                boss.SUBMIT_T(tier::release, e2::size::set, memo, new_sz) { canvas.size(new_sz); };
                boss.SUBMIT_T(tier::request, e2::form::canvas, memo, canvas_ptr) { canvas_ptr = coreface; };
                if (rendered)
                {
                    boss.SUBMIT_T(tier::release, e2::render::prerender, memo, parent_canvas)
                    {
                        if (boss.base::ruined())
                        {
                            canvas.wipe();
                            boss.SIGNAL(tier::release, e2::render::any, canvas);
                            boss.base::ruined(faux);
                        }
                        parent_canvas.plot(canvas);
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

            iota width; // acryl: Blur radius.

        public:
            acryl(base&&) = delete;
            acryl(base& boss, iota size = 5)
                : skill{ boss },
                  width{ size }
            {
                boss.SUBMIT_T(tier::release, e2::render::prerender, memo, parent_canvas)
                {
                    auto brush = boss.base::color();
                    if (brush.wdt()) parent_canvas.blur(width, [&](cell& c) { c.fuse(brush); });
                    else             parent_canvas.blur(width);
                };
            }
        };

        // pro: Highlighter.
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

        //todo PoC, unify, too hacky
        // pro: Cell Highlighter.
        class cell_highlight
            : public skill
        {
            struct sock
            {
                twod curpos; // sock: Current coor.
                bool inside; // sock: Is active.
                bool seized; // sock: Is seized.
                rect region; // sock: Selected region.

                sock()
                    : inside{ faux },
                      seized{ faux }
                { }
                operator bool(){ return inside || seized || region.size; }
                auto grab(twod const& coord, bool resume)
                {
                    if (inside)
                    {
                        if (!(region.size && resume))
                        {
                            region.coor = coord;
                            region.size = dot_00;
                        }
                        seized = true;
                    }
                    return seized;
                }
                auto calc(base const& boss, twod const& coord)
                {
                    curpos = coord;
                    auto area = boss.size();
                    area.x += boss.oversz.r;
                    inside = area.inside(curpos);
                }
                auto drag(twod const& coord)
                {
                    if (seized)
                    {
                        region.size = coord - region.coor;
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

        public:
            cell_highlight(base&&) = delete;
            cell_highlight(base& boss)
                : skill{ boss }
            {
                boss.SUBMIT_T(tier::release, e2::postrender, memo, parent_canvas)
                {
                    auto full = parent_canvas.full();
                    auto view = parent_canvas.view();
                    auto mark = cell{}.bgc(bluelt).bga(0x40);
                    auto fill = [&](cell& c) { c.fuse(mark); };
                    auto step = twod{ 5, 1 };
                    auto area = full;
                    area.size.x += boss.oversz.r;
                    items.foreach([&](sock& item)
                    {
                        if (item.region.size)
                        {
                            auto region = item.region.normalize();
                            auto pos1 = region.coor / step * step;
                            auto pos2 = (region.coor + region.size + step) / step * step;
                            auto pick = rect{ full.coor + pos1, pos2 - pos1 }.clip(area).clip(view);
                            parent_canvas.fill(pick, fill);
                        }
                        if (item.inside)
                        {
                            auto pos1 = item.curpos / step * step;
                            auto pick = rect{ full.coor + pos1, step }.clip(view);
                            parent_canvas.fill(pick, fill);
                        }
                    });
                };
                boss.SUBMIT_T(tier::release, hids::events::mouse::button::click::any, memo, gear)
                {
                    auto& item = items.take(gear);
                    if (item.region.size)
                    {
                        if (gear.meta(hids::ANYCTRL)) item.region.size = gear.coord - item.region.coor;
                        else                          item.region.size = dot_00;
                    }
                    recalc();
                };
                boss.SUBMIT_T(tier::release, hids::events::mouse::button::dblclick::any, memo, gear)
                {
                    auto& item = items.take(gear);
                    auto area = boss.size();
                    area.x += boss.oversz.r;
                    item.region.coor = dot_00;
                    item.region.size = area;
                    recalc();
                    gear.dismiss();
                };
                boss.SUBMIT_T(tier::general, hids::events::die, memo, gear)
                {
                    recalc();
                    boss.deface();
                };
                boss.SUBMIT_T(tier::release, hids::events::notify::mouse::enter, memo, gear)
                {
                    items.add(gear);
                };
                boss.SUBMIT_T(tier::release, hids::events::notify::mouse::leave, memo, gear)
                {
                    auto& item = items.take(gear);
                    if (item.region.size)
                    {
                        item.inside = faux;
                    }
                    else items.del(gear);
                    recalc();
                };
                engage<sysmouse::left>();
            }
            void recalc()
            {
                text data;
                auto step = twod{ 5, 1 };
                auto size = boss.size();
                size.x += boss.oversz.r;
                items.foreach([&](sock& item)
                {
                    if (item.region.size)
                    {
                        auto region = item.region.normalize();
                        auto pos1 = region.coor / step;
                        auto pos2 = (region.coor + region.size) / step;
                        pos1 = std::clamp(pos1, dot_00, twod{ 25, 98 } );
                        pos2 = std::clamp(pos2, dot_00, twod{ 25, 98 } );
                        data += 'A'+ (char)pos1.x;
                        data += std::to_string(pos1.y + 1);
                        data += ':';
                        data += 'A' + (char)pos2.x;
                        data += std::to_string(pos2.y + 1);
                        data += ", ";
                    }
                });
                if (data.size())
                {
                    data.pop_back(); // pop", "
                    data.pop_back(); // pop", "
                    data = " =SUM(" + ansi::fgc(bluedk).add(data).fgc(blacklt).add(")");
                }
                else data = " =SUM(" + ansi::itc(true).fgc(reddk).add("select cells by dragging").itc(faux).fgc(blacklt).add(")");
                log("calc: DATA ", data);                        
                boss.SIGNAL(tier::release, e2::data::text, data);
            }
            // pro::cell_highlight: Configuring the mouse button to operate.
            template<sysmouse::bttns BUTTON>
            void engage()
            {
                boss.SIGNAL(tier::release, e2::form::draggable::_<BUTTON>, true);
                boss.SUBMIT_T(tier::release, hids::events::mouse::move, memo, gear)
                {
                    items.take(gear).calc(boss, gear.coord);
                    boss.base::deface();
                };
                boss.SUBMIT_T(tier::release, e2::form::drag::start::_<BUTTON>, memo, gear)
                {
                    if (items.take(gear).grab(gear.coord, gear.meta(hids::ANYCTRL)))
                        gear.dismiss();
                };
                boss.SUBMIT_T(tier::release, e2::form::drag::pull::_<BUTTON>, memo, gear)
                {
                    if (items.take(gear).drag(gear.coord))
                    {
                        recalc();
                        gear.dismiss();
                    }
                };
                boss.SUBMIT_T(tier::release, e2::form::drag::cancel::_<BUTTON>, memo, gear)
                {
                    items.take(gear).drop();
                    recalc();
                };
                boss.SUBMIT_T(tier::release, e2::form::drag::stop::_<BUTTON>, memo, gear)
                {
                    items.take(gear).drop();
                    recalc();
                };
            }
        };
        
        // pro: Keyboard focus highlighter.
        class focus
            : public skill
        {
            using skill::boss,
                  skill::memo;

            bool active = faux; // mold: Keyboard focus.
            rgba title_fg_color = 0xFFffffff;

        public:
            focus(base&&) = delete;
            focus(base& boss)
                : skill{ boss }
            {
                boss.SUBMIT_T(tier::release, e2::form::state::keybd, memo, status)
                {
                    active = status;
                    boss.base::deface();
                };
                boss.SUBMIT_T(tier::release, e2::render::prerender, memo, parent_canvas)
                {
                    //todo revise, too many fillings (mold's artifacts)
                    auto normal = boss.base::color();
                    if (active)
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
                        parent_canvas.cage(area, dot_11, fill);
                    }
                };
            }
        };
    }

    // console: World internals.
    class host
        : public base
    {
        #ifndef PROD
        pro::watch zombi{*this }; // host: Zombie protection.
        #endif
        pro::robot robot{*this }; // host: Amination controller.
        pro::keybd keybd{*this }; // host: Keyboard controller.
        pro::mouse mouse{*this }; // host: Mouse controller.
        pro::scene scene{*this }; // host: Scene controller.

        using tick = quartz<events::reactor<>, e2::type>;
        using hndl = std::function<void(view)>;

        tick synch; // host: Frame rate synchronizator.
        iota frate; // host: Frame rate value.
        hndl close; // host: Quit procedure.

        void deface(rect const& region) override
        {
            base::deface(region);
            scene.denote(region);
        }

    public:
        // host: Create a new item of the specified subtype and attach it.
        template<class T>
        auto branch(id_t class_id, sptr<T> item_ptr)
        {
            return scene.branch(class_id, item_ptr);
        }
        // host: Create a new item of the specified subtype and attach it.
        template<class T, class ...Args>
        auto attach(id_t class_id, Args&&... args)
        {
            return scene.attach<T>(class_id, std::forward<Args>(args)...);
        }
        //todo unify
        // host: .
        template<class T, class ...Args>
        auto invite(Args&&... args)
        {
            return scene.invite<T>(std::forward<Args>(args)...);
        }

    protected:
        host(hndl exit_proc)
            : synch(router<tier::general>(), e2::tick.id),
              frate{ 0 },
              close{ exit_proc }
        {
            using bttn = hids::events::mouse::button;

            keybd.accept(true); // Subscribe on keybd offers.

            SUBMIT(tier::general, e2::tick, timestamp)
            {
                scene.redraw();
            };

            //test
            //SUBMIT(tier::preview, bttn::click::left, gear)
            //{
            //	static iota i = 0;
            //	text data = "click " + std::to_string(i++) + "\n";
            //	SIGNAL_GLOBAL(e2::debug, data);
            //};

            SUBMIT(tier::release, bttn::click::right, gear)
            {
                //auto newpos = gear.mouse.coord + gear.xview.coor;
                this->SIGNAL(tier::general, e2::form::global::ctxmenu, gear.coord);
            };

            //SUBMIT(tier::release, bttn::drag::start::left, gear)
            //{
            //    if (gear.capture(bell::id))
            //    {
            //        robot.pacify();
            //        gear.dismiss();
            //    }
            //};
            //SUBMIT(tier::release, bttn::drag::pull::left, gear)
            //{
            //    if (gear.captured(bell::id))
            //    {
            //        auto data = cube{ gear.mouse::delta.get(), gear.area() };
            //        this->SIGNAL(tier::preview, e2::form::layout::convey, data);
            //        deface(rect{ dot_00, dot_11 }); //todo unify, deface all world
            //        gear.dismiss();
            //    }
            //};
            //SUBMIT(tier::release, bttn::drag::cancel::left, gear)
            //{
            //    if (gear.captured(bell::id))
            //    {
            //        gear.release();
            //        gear.dismiss();
            //    }
            //};
            //SUBMIT(tier::release, bttn::drag::stop::left, gear)
            //{
            //    if (gear.captured(bell::id))
            //    {
            //        auto boundary = gear.area();
            //        gear.release();
            //        robot.actify(gear.mouse::fader<quadratic<twod>>(2s), [&, boundary](auto& x)
            //                     {
            //                         auto data = cube{ x, boundary };
            //                         this->SIGNAL(tier::preview, e2::form::layout::convey, data);
            //                         deface(rect{ dot_00, dot_11 }); //todo unify, deface all world
            //                     });
            //        gear.dismiss();
            //    }
            //};
            SUBMIT(tier::general, hids::events::die, gear)
            {
                if (gear.captured(bell::id))
                {
                    gear.release();
                    gear.dismiss();
                }
            };
            SUBMIT(tier::general, e2::config::fps, fps)
            {
                if (fps > 0)
                {
                    frate = fps;
                    synch.ignite(frate);
                }
                else if (fps == -1)
                {
                    fps = frate;
                }
                else
                {
                    synch.cancel();
                }
            };
            SUBMIT(tier::release, e2::command::quit, reason)
            {
                if (close)
                {
                    close(reason);
                }
            };
        }
        ~host()
        {
            synch.cancel();

            //todo why it is never called?
            //reject();
        }
    };

    // console: TTY session class.
    class link
    {
        using work = std::thread;
        using lock = std::recursive_mutex;
        using cond = std::condition_variable_any;

        bell&     owner; // link: Boss.
        os::xipc  canal; // link: Data highway.
        work      input; // link: Reading thread.
        cond      synch; // link: Thread sync cond variable.
        lock      mutex; // link: Thread sync mutex.
        bool      alive; // link: Working loop state.
        bool      ready; // link: To avoid spuritous wakeup (cv).
        bool      focus; // link: Terminal window focus state.
        iota      iface; // link: Platform specific UI code.
        sysmouse  mouse; // link: Mouse state.
        syskeybd  keybd; // link: Keyboard state.
        bool      close; // link: Pre closing condition.
        text      chunk; // link: The next received chunk of data input.
        text      total; // link: Accumulated unparsed input.

        void reader()
        {
            log("link: std_input thread started");
            while (auto yield = canal->recv())
            {
                std::lock_guard guard{ mutex };

                chunk.resize(yield.length());
                std::copy(yield.begin(), yield.end(), chunk.data());

                ready = true;
                synch.notify_one();
            }

            if (alive)
            {
                log("link: signaling to close read channel ", canal);
                owner.SIGNAL(tier::release, e2::conio::quit, "link: read channel is closed");
                log("link: sig to close read channel complete", canal);
            }
            log("link: std_input thread is going to close");
        }

    public:
        link(bell& boss, os::xipc sock)
            : owner { boss },
              canal { sock },
              alive { true },
              ready { faux },
              focus { faux },
              close { faux },
              iface { 0    }
        { }
        ~link()
        {
            canal->shut(); // Terminate all blocking calls.
            if (input.joinable())
            {
                input.join();
            }
            log("link: std_input thread joined");
        }

        void output (view buffer)
        {
            canal->send(buffer);
        }
        void session(text title)
        {
            auto is_digit = [](auto c) { return c >= '0' && c <= '9'; };
            std::unique_lock guard{ mutex };

            input = std::thread([&] { reader(); });

            output(ansi::ext(true));
            if (title.size()) output(ansi::tag(title));

            while ((void)synch.wait(guard, [&] { return ready; }), alive)
            {
                ready = faux;

                total += chunk;
                //todo why?
                //todo separate commands and keypress
                //
                // commands are:
                // - esc
                // - ctl keys and Fns
                // - mouse tracking - '\e[< Ctrl; Px; Py M' '\e[< Ctrl; Px; Py m'    M - pressed, m - released
                //	Ctrl:	7654 3210
                //	              ||└---
                //                |└----
                //	              └----- Ctrl
                // - cursor

                view strv = total;

                #ifdef KEYLOG
                log("link: input data (", chunk.size(), " bytes):\n", utf::debase(chunk));
                #endif

                #ifndef PROD
                if (close)
                {
                    close = faux;
                    owner.SIGNAL(tier::release, e2::conio::preclose, close);
                    if (chunk.front() == '\x1b') // two consecutive escapes
                    {
                        log("\t - two consecutive escapes: \n\tstrv:        ", strv);
                        owner.SIGNAL(tier::release, e2::conio::quit, "pipe two consecutive escapes");
                        return;
                    }
                }
                #endif

                //todo unify (it is just a proof of concept)
                while (auto len = strv.size())
                {
                    auto pos = 0_sz;
                    bool unk = faux;

                    if (strv.at(0) == '\x1b')
                    {
                        ++pos;

                        #ifndef PROD
                        if (pos == len) // the only one esc
                        {
                            close = true;
                            total = strv;
                            log("\t - preclose: ", canal);
                            owner.SIGNAL(tier::release, e2::conio::preclose, close);
                            break;
                        }
                        else if (strv.at(pos) == '\x1b') // two consecutive escapes
                        {
                            total.clear();
                            log("\t - two consecutive escapes: ", canal);
                            owner.SIGNAL(tier::release, e2::conio::quit, "pipe2: two consecutive escapes");
                            break;
                        }
                        #else
                        if (pos == len) // the only one esc
                        {
                            // Pass Esc.
                            keybd.textline = strv.substr(0, 1);
                            owner.SIGNAL(tier::release, e2::conio::key, keybd);
                            total.clear();
                            //strv = total;
                            break;
                        }
                        else if (strv.at(pos) == '\x1b') // two consecutive escapes
                        {
                            //  Pass Esc.
                            keybd.textline = strv.substr(0, 1);
                            owner.SIGNAL(tier::release, e2::conio::key, keybd);
                            total = strv.substr(1);
                            //strv = total;
                            break;
                        }
                        #endif
                        else if (strv.at(pos) == '[')
                        {
                            if (++pos == len) { total = strv; break; }//incomlpete
                            if (strv.at(pos) == 'I')
                            {
                                focus = true;
                                owner.SIGNAL(tier::release, e2::conio::focus, focus);
                                log("\t - focus on ", canal);
                                ++pos;
                            }
                            else if (strv.at(pos) == 'O')
                            {
                                focus = faux;
                                owner.SIGNAL(tier::release, e2::conio::focus, focus);
                                log("\t - focus off: ", canal);
                                ++pos;
                            }
                            else if (strv.at(pos) == '<') // "\033[<0;x;yM/m"
                            {
                                if (++pos == len) { total = strv; break; }// incomlpete sequence

                                auto tmp = strv.substr(pos);
                                auto l = tmp.size();
                                if (auto ctrl = utf::to_int(tmp))
                                {
                                    pos += l - tmp.size();
                                    if (pos == len) { total = strv; break; }// incomlpete sequence
                                    {
                                        if (++pos == len) { total = strv; break; }// incomlpete sequence

                                        view tmp = strv.substr(pos);
                                        auto l = tmp.size();
                                        if (auto pos_x = utf::to_int(tmp))
                                        {
                                            pos += l - tmp.size();
                                            if (pos == len) { total = strv; break; }// incomlpete sequence
                                            {
                                                if (++pos == len) { total = strv; break; }// incomlpete sequence

                                                view tmp = strv.substr(pos);
                                                auto l = tmp.size();
                                                if (auto pos_y = utf::to_int(tmp))
                                                {
                                                    pos += l - tmp.size();
                                                    if (pos == len) { total = strv; break; }// incomlpete sequence
                                                    {
                                                        auto ispressed = (strv.at(pos) == 'M');
                                                        ++pos;

                                                        auto clamp = [](auto a) { return std::clamp(a,
                                                            std::numeric_limits<iota>::min() / 2,
                                                            std::numeric_limits<iota>::max() / 2); };

                                                        auto x = clamp(pos_x.value() - 1);
                                                        auto y = clamp(pos_y.value() - 1);
                                                        auto ctl = ctrl.value();

                                                        // ks & 0x10 ? f + ";2" // shift
                                                        // ks & 0x02 || ks & 0x01 ? f + ";3" // alt
                                                        // ks & 0x04 || ks & 0x08 ? f + ";5" // ctrl
                                                        // 00000 000
                                                        //   ||| |||
                                                        //   ||| |------ btn state
                                                        //   |---------- ctl state
                                                        bool k_shift = ctl & 0x4;
                                                        bool k_alt   = ctl & 0x8;
                                                        bool k_ctrl  = ctl & 0x10;
                                                        mouse.ctlstate = (k_shift ? hids::SHIFT : 0)
                                                                       + (k_alt   ? hids::ALT   : 0)
                                                                       + (k_ctrl  ? hids::CTRL  : 0);
                                                        //if ( mouse.ctlstate ) log(" mouse.ctlstate =",  mouse.ctlstate );
                                                        ctl = ctl & ~0b00011100;

                                                        mouse.wheeled = faux;
                                                        mouse.wheeldt = 0;
                                                        mouse.shuffle = faux;

                                                        bool fire = true;

                                                        constexpr static int total = sysmouse::numofbutton;
                                                        constexpr static int first = sysmouse::left;
                                                        constexpr static int midst = sysmouse::middle;
                                                        constexpr static int other = sysmouse::right;
                                                        constexpr static int winbt = sysmouse::win;
                                                        constexpr static int wheel = sysmouse::wheel;
                                                        constexpr static int joint = sysmouse::leftright;

                                                        if (ctl == 35 &&(mouse.button[first]
                                                                      || mouse.button[midst]
                                                                      || mouse.button[other]
                                                                      || mouse.button[winbt]))
                                                        {
                                                            // Moving without buttons (case when second release not fired: wezterm, terminal.app)
                                                            mouse.button[first] = faux;
                                                            mouse.button[midst] = faux;
                                                            mouse.button[other] = faux;
                                                            mouse.button[winbt] = faux;
                                                            owner.SIGNAL(tier::release, e2::conio::mouse, mouse);
                                                        }
                                                        // Moving should be fired first
                                                        if ((mouse.ismoved = mouse.coor({ x, y })))
                                                        {
                                                            owner.SIGNAL(tier::release, e2::conio::mouse, mouse);
                                                            mouse.ismoved = faux;
                                                        }

                                                        switch (ctl)
                                                        {
                                                            case 0:
                                                                mouse.button[first] = ispressed;
                                                                break;
                                                            case 1:
                                                                mouse.button[midst] = ispressed;
                                                                break;
                                                            case 2:
                                                                mouse.button[other] = ispressed;
                                                                break;
                                                            case 3:
                                                                mouse.button[winbt] = ispressed;
                                                                //if (!ispressed) // WinSrv2019 vtmouse bug workaround
                                                                //{               //  - release any button always fires winbt release
                                                                //	mouse.button[first] = ispressed;
                                                                //	mouse.button[midst] = ispressed;
                                                                //	mouse.button[other] = ispressed;
                                                                //}
                                                                break;
                                                            case 64:
                                                                mouse.wheeled = true;
                                                                mouse.wheeldt = 1;
                                                                break;
                                                            case 65:
                                                                mouse.wheeled = true;
                                                                mouse.wheeldt = -1;
                                                                break;
                                                            default:
                                                                fire = faux;
                                                                mouse.shuffle = !mouse.ismoved;
                                                                break;
                                                        }

                                                        if (fire)
                                                        {
                                                            owner.SIGNAL(tier::release, e2::conio::mouse, mouse);
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                            else if (is_digit(strv.at(pos)))
                            {
again:
                                view tmp = strv.substr(pos);
                                auto l = tmp.size();
                                auto event_id = utf::to_int(tmp).value();
                                if (event_id > ansi::W32_START_EVENT
                                 && event_id < ansi::W32_FINAL_EVENT)
                                {
                                    //log("win32input: ", strv);
                                    pos += l - tmp.size();
                                    if (pos == len) { total = strv; break; }// incomlpete sequence
                                    {
                                        auto take = [&]() {
                                            view tmp = strv.substr(pos);
                                            if (auto l = tmp.size())
                                            {
                                                tmp.remove_prefix(1); // pop ':'
                                                if (tmp.size())
                                                {
                                                    if (tmp.at(0) == ':')
                                                    {
                                                        pos++;
                                                        return 0;
                                                    }
                                                    auto p = utf::to_int(tmp).value();
                                                    pos += l - tmp.size();
                                                    return p;
                                                }
                                            }
                                            return 0;
                                        };
                                        switch (event_id)
                                        {
                                            case ansi::W32_MOUSE_EVENT:
                                            {
                                                iota id    = take();
                                                iota bttns = take();
                                                iota ctrls = take();
                                                iota flags = take();
                                                iota wheel = take();
                                                iota xcoor = take();
                                                iota ycoor = take();

                                                auto coord = twod{ xcoor, ycoor };

                                                mouse.button[0] = bttns & (1 << 0); // FROM_LEFT_1ST_BUTTON_PRESSED
                                                mouse.button[1] = bttns & (1 << 1); // RIGHTMOST_BUTTON_PRESSED;
                                                mouse.button[3] = bttns & (1 << 2); // FROM_LEFT_2ND_BUTTON_PRESSED;
                                                mouse.button[2] = bttns & (1 << 3); // FROM_LEFT_3RD_BUTTON_PRESSED;
                                                mouse.button[4] = bttns & (1 << 4); // FROM_LEFT_4TH_BUTTON_PRESSED;

                                                mouse.ismoved = mouse.coor(coord);
                                                mouse.shuffle = !mouse.ismoved && (flags & (1 << 0)); // MOUSE_MOVED
                                                // Makes no sense (ignored)
                                                mouse.doubled = flags & (1 << 1); // DOUBLE_CLICK;
                                                mouse.wheeled = flags & (1 << 2); // MOUSE_WHEELED;
                                                mouse.hzwheel = flags & (1 << 3); // MOUSE_HWHEELED;
                                                mouse.wheeldt = wheel;

                                                bool k_ralt  = ctrls & 0x1;
                                                bool k_alt   = ctrls & 0x2;
                                                bool k_rctrl = ctrls & 0x4;
                                                bool k_ctrl  = ctrls & 0x8;
                                                bool k_shift = ctrls & 0x10;
                                                mouse.ctlstate = (k_shift ? hids::SHIFT : 0)
                                                               + (k_alt   ? hids::ALT   : 0)
                                                               + (k_ralt  ? hids::ALT   : 0)
                                                               + (k_rctrl ? hids::RCTRL : 0)
                                                               + (k_ctrl  ? hids::CTRL  : 0);

                                                if (!mouse.shuffle)
                                                    owner.SIGNAL(tier::release, e2::conio::mouse, mouse);
                                                break;
                                            }
                                            case ansi::W32_KEYBD_EVENT:
                                            {
                                                iota id = take();
                                                iota kc = take();
                                                iota sc = take();
                                                iota kd = take();
                                                iota ks = take();
                                                iota rc = take();
                                                iota uc = take();
                                                keybd.virtcode    = kc;
                                                keybd.ctlstate    = ks & 0x1f; // only modifiers
                                                keybd.down        = kd;
                                                keybd.repeatcount = rc;
                                                keybd.scancode    = sc;
                                                keybd.character   = uc;
                                                auto ctrl = [ks](text f, auto e)
                                                {
                                                    auto b = ks & 0x10 ? f + ";2" // shift
                                                           : ks & 0x02 || ks & 0x01 ? f + ";3" // alt
                                                           : ks & 0x04 || ks & 0x08 ? f + ";5" // ctrl
                                                           : f;
                                                    return "\033[" + b + e;
                                                };
                                                using key = syskeybd;
                                                if (keybd.down)
                                                {
                                                    switch (kc)
                                                    {
                                                        //todo Ctrl+Space
                                                        //     Ctrl+Backspace
                                                        //     Alt+0..9
                                                        //     Ctrl/Shift+Enter
                                                        case key::Backspace: keybd.textline = "\177"; break;
                                                        case key::Tab:       keybd.textline = ks & 0x10 ? "\033[Z" : "\t"; break;
                                                        case key::PageUp:    keybd.textline = ctrl("5",  "~"); break;
                                                        case key::PageDown:  keybd.textline = ctrl("6",  "~"); break;
                                                        case key::End:       keybd.textline = ctrl("1",  "F"); break;
                                                        case key::Home:      keybd.textline = ctrl("1",  "H"); break;
                                                        case key::Insert:    keybd.textline = ctrl("2",  "~"); break;
                                                        case key::Delete:    keybd.textline = ctrl("3",  "~"); break;
                                                        case key::Up:        keybd.textline = ctrl("1",  "A"); break;
                                                        case key::Down:      keybd.textline = ctrl("1",  "B"); break;
                                                        case key::Right:     keybd.textline = ctrl("1",  "C"); break;
                                                        case key::Left:      keybd.textline = ctrl("1",  "D"); break;
                                                        case key::F1:        keybd.textline = ctrl("1",  "P"); break;
                                                        case key::F2:        keybd.textline = ctrl("1",  "Q"); break;
                                                        case key::F3:        keybd.textline = ctrl("1",  "R"); break;
                                                        case key::F4:        keybd.textline = ctrl("1",  "S"); break;
                                                        case key::F5:        keybd.textline = ctrl("15", "~"); break;
                                                        case key::F6:        keybd.textline = ctrl("17", "~"); break;
                                                        case key::F7:        keybd.textline = ctrl("18", "~"); break;
                                                        case key::F8:        keybd.textline = ctrl("19", "~"); break;
                                                        case key::F9:        keybd.textline = ctrl("20", "~"); break;
                                                        case key::F10:       keybd.textline = ctrl("21", "~"); break;
                                                        case key::F11:       keybd.textline = ctrl("23", "~"); break;
                                                        case key::F12:       keybd.textline = ctrl("24", "~"); break;
                                                        default:
                                                            //log("uc = ", uc);
                                                            if (uc)
                                                                keybd.textline = utf::to_utf_from_code(uc);
                                                            break;
                                                    }
                                                }
                                                else
                                                {
                                                    keybd.textline.clear();
                                                }
                                                owner.SIGNAL(tier::release, e2::conio::key, keybd);
                                                break;
                                            }
                                            case ansi::W32_WINSZ_EVENT:
                                            {
                                                iota xsize = take();
                                                iota ysize = take();
                                                twod winsz{ xsize,ysize };
                                                owner.SIGNAL(tier::release, e2::conio::size, winsz);
                                                break;
                                            }
                                            case ansi::W32_FOCUS_EVENT:
                                            {
                                                //todo clear pressed keys on lost focus
                                                iota id    = take();
                                                bool focus = take();
                                                owner.SIGNAL(tier::release, e2::conio::focus, focus);
                                                break;
                                            }
                                            default:
                                                break;
                                        }
                                        // pop '_' or ';'
                                        if (strv.at(pos) == ';')
                                        {
                                            pos++;
                                            goto again;
                                        }
                                        else pos++; // pop '_'
                                    }
                                }
                                else if (event_id == ansi::CCC_EXT && l > 2
                                    && tmp.at(0) == ':' && tmp.at(2) == 'p')
                                {
                                    pos += 5 /* 25:1p */;
                                    owner.SIGNAL(tier::release, e2::conio::native, tmp.at(1) == '1');
                                }
                                else if (event_id == ansi::CCC_SMS && l > 2
                                    && tmp.at(0) == ':' && tmp.at(2) == 'p')
                                {
                                    pos += 5 /* 26:1p */;
                                    owner.SIGNAL(tier::release, e2::conio::pointer, tmp.at(1) == '1');
                                }
                                else if (event_id == ansi::CCC_KBD && l > 2
                                    && tmp.at(0) == ':')
                                {
                                    tmp.remove_prefix(1); // pop ':'
                                    if(auto v = utf::to_int(tmp))
                                    {
                                        if (tmp.size() && tmp.at(0) == 'p')
                                        {
                                            tmp.remove_prefix(1); // pop 'p'
                                            pos += l - tmp.size();
                                            auto ctrls = v.value();
                                                bool k_ralt  = ctrls & 0x1;
                                                bool k_alt   = ctrls & 0x2;
                                                bool k_rctrl = ctrls & 0x4;
                                                bool k_ctrl  = ctrls & 0x8;
                                                bool k_shift = ctrls & 0x10;
                                                keybd.ctlstate = (k_shift ? hids::SHIFT : 0)
                                                               + (k_alt   ? hids::ALT   : 0)
                                                               + (k_ralt  ? hids::ALT   : 0)
                                                               + (k_rctrl ? hids::RCTRL : 0)
                                                               + (k_ctrl  ? hids::CTRL  : 0);
                                        }
                                    }
                                }
                                else
                                {
                                    unk = true;
                                    pos = 0_sz;
                                }
                            }
                            else
                            {
                                unk = true;
                                pos = 0_sz;
                            }
                        }
                        else if (strv.at(pos) == ']')
                        {
                            if (++pos == len) { total = strv; break; }//incomlpete

                            auto tmp = strv.substr(pos);
                            auto l = tmp.size();
                            if (auto pos_x = utf::to_int(tmp))
                            {
                                pos += l - tmp.size();
                                if (pos == len) { total = strv; break; }//incomlpete
                                {
                                    if (++pos == len) { total = strv; break; }//incomlpete

                                    auto tmp = strv.substr(pos);
                                    auto l = tmp.size();
                                    if (auto pos_y = utf::to_int(tmp))
                                    {
                                        pos += l - tmp.size();
                                        if (pos == len) { total = strv; break; }//incomlpete
                                        {
                                            ++pos;

                                            auto x = pos_x.value();
                                            auto y = pos_y.value();

                                            twod winsz{ x,y };
                                            owner.SIGNAL(tier::release, e2::conio::size, winsz);
                                        }
                                    }
                                }
                            }
                        }
                        else
                        {
                            unk = true;
                            pos = 0_sz;
                        }
                    }

                    if (!unk)
                    {
                        total = strv.substr(pos);
                        strv = total;
                    }

                    if (auto size = strv.size())
                    {
                        auto i = unk ? 1_sz : 0_sz;
                        while (i != size && (strv.at(i) != '\x1b'))
                        {
                            // Pass SIGINT inside the desktop
                            //if (strv.at(i) == 3 /*3 - SIGINT*/)
                            //{
                            //	log(" - SIGINT in stdin");
                            //	owner.SIGNAL(tier::release, e2::conio::quit, "pipe: SIGINT");
                            //	return;
                            //}
                            i++;
                        }

                        if (i)
                        {
                            keybd.textline = strv.substr(0, i);
                            owner.SIGNAL(tier::release, e2::conio::key, keybd);
                            total = strv.substr(i);
                            strv = total;
                        }
                    }
                }
            }

            log("link: std_input thread ended");
        }
        // link: Interrupt the run only.
        void shutdown ()
        {
            mutex.lock();
            canal->shut(); // Terminate all blocking calls.

            alive = faux;
            ready = true;
            synch.notify_one(); // Interrupt reading session.
            mutex.unlock();
        }
    };

    // console: Bitmap changes analyzer.
    class diff
    {
        using work = std::thread;
        using lock = std::recursive_mutex;
        using cond = std::condition_variable_any;
        using ansi = ansi::esc;
        using span = period;
        using pair = std::optional<std::pair<span, iota>>;

        link& conio;
        lock& mutex; // diff: Mutex between renderer and committer threads.
        cond  synch; // diff: Synchronization between renderer and committer.

        grid& cache; // diff: The current content buffer which going to be checked and processed.
        grid  front; // diff: The Shadow copy of the terminal/screen.

        iota  rhash; // diff: Rendered buffer genus. The genus changes when the size of the buffer changes.
        iota  dhash; // diff: Unchecked buffer genus. The genus changes when the size of the buffer changes.
        twod  field; // diff: Current terminal/screen window size.
        span  watch; // diff: Duration of the STDOUT rendering.
        iota  delta; // diff: Last ansi-rendered frame size.
        ansi  frame; // diff: Text screen representation.
        bool  alive; // diff: Working loop state.
        bool  ready; // diff: Conditional variable to avoid spurious wakeup.
        svga  video; // diff: VGA 16/256-color compatibility mode.
        work  paint; // diff: Rendering thread.
        pair  debug; // diff: Debug info.

        text  extra; // diff: Extra data to cout.
        text  extra_cached; // diff: Cached extra data to cout.

        // diff: Render current buffer to the screen.
        template<svga VGAMODE = svga::truecolor>
        void render()
        {
            using time = moment;

            log("rend: thread started");

            auto fallback = [&](auto& c, auto& state, auto& frame)
            {
                auto dumb = c;
                dumb.txt(utf::REPLACEMENT_CHARACTER_UTF8_VIEW);
                dumb.template scan<VGAMODE>(state, frame);
            };

            std::unique_lock guard{ mutex };

            cell state;
            time start;

            //todo unify (it is just a proof of concept)
            //todo switch VGAMODE on fly
            while ((void)synch.wait(guard, [&]{ return ready; }), alive)
            {
                ready = faux;
                start = tempus::now();

                if (extra_cached.length())
                {
                    frame.add(extra_cached);
                    extra_cached.clear();
                }

                if (rhash != dhash)
                {
                    rhash = dhash;
                    auto src = front.data();
                    auto end = src + front.size();
                    auto row = 0;
                    frame.scroll_wipe();
                    while (row++ < field.y)
                    {
                        frame.locate(1, row);
                        auto end_line = src + field.x;
                        while (src != end_line)
                        {
                            auto& c = *(src++);
                            if (c.wdt() < 2)
                            {
                                c.scan<VGAMODE>(state, frame);
                            }
                            else
                            {
                                if (c.wdt() == 2)
                                {
                                    if (src != end_line)
                                    {
                                        auto& d = *(src++);
                                        if (d.wdt() < 3)
                                        {
                                            fallback(c, state, frame); // Left part alone.
                                            src--; // Repeat all for d again.
                                        }
                                        else
                                        {
                                            if (!c.scan<VGAMODE>(d, state, frame))
                                            {
                                                fallback(c, state, frame); // Left part alone.
                                                src--; // Repeat all for d again.
                                            }
                                        }
                                    }
                                    else
                                    {
                                        fallback(c, state, frame); // Left part alone.
                                    }
                                }
                                else
                                {
                                    fallback(c, state, frame); // Right part alone.
                                }
                            }
                        }
                    }
                }
                else
                {
                    auto src = cache.data();
                    auto dst = front.data();
                    auto beg = src;
                    auto end = src;
                    iota row = 0;

                    while (row++ < field.y)
                    {
                        end += field.x;

                        while (src != end)
                        {
                            auto& fore = *src++;
                            auto& back = *dst++;

                            auto w = fore.wdt();
                            if (w < 2)
                            {
                                if (back != fore)
                                {
                                    auto col = static_cast<iota>(src - beg);
                                    frame.locate(col, row);

                                    back = fore;
                                    fore.scan<VGAMODE>(state, frame);

                                    /* optimizations */
                                    while (src != end)
                                    {
                                        auto& fore = *src++;
                                        auto& back = *dst++;

                                        auto w = fore.wdt();
                                        if (w < 2)
                                        {
                                            if (back == fore) break;
                                            else
                                            {
                                                back = fore;
                                                fore.scan<VGAMODE>(state, frame);
                                            }
                                        }
                                        else if (w == 2) // Check left part.
                                        {
                                            if (src != end)
                                            {
                                                if (back == fore)
                                                {
                                                    auto& d = *(src++);
                                                    auto& g = *(dst++);
                                                    if (g == d) break;
                                                    else
                                                    {
                                                        if (d.wdt() < 3)
                                                        {
                                                            fallback(fore, state, frame); // Left part alone.
                                                            src--; // Repeat all for d again.
                                                            dst--; // Repeat all for g again.
                                                        }
                                                        else // d.wdt() == 3
                                                        {
                                                            if (!fore.scan<VGAMODE>(d, state, frame))
                                                            {
                                                                fallback(fore, state, frame); // Left part alone.
                                                                fallback(d,    state, frame); // Right part alone.
                                                            }
                                                            g = d;
                                                        }
                                                    }
                                                }
                                                else
                                                {
                                                    back = fore;

                                                    auto& d = *(src++);
                                                    auto& g = *(dst++);
                                                    if (d.wdt() < 3)
                                                    {
                                                        fallback(fore, state, frame); // Left part alone.
                                                        src--; // Repeat all for d again.
                                                        dst--; // Repeat all for g again.
                                                    }
                                                    else // d.wdt() == 3
                                                    {
                                                        if (!fore.scan<VGAMODE>(d, state, frame))
                                                        {
                                                            fallback(fore, state, frame); // Left part alone.
                                                            fallback(d, state, frame); // Right part alone.
                                                        }
                                                        g = d;
                                                    }
                                                }
                                            }
                                            else
                                            {
                                                if (back != fore) back = fore;
                                                fallback(fore, state, frame); // Left part alone.
                                            }
                                        }
                                        else // w == 3
                                        {
                                            if (back != fore) back = fore;
                                            fallback(fore, state, frame); // Right part alone.
                                        }
                                    }
                                    /* optimizations */
                                }
                            }
                            else
                            {
                                if (w == 2) // Left part has changed.
                                {
                                    if (back != fore)
                                    {
                                        back = fore;

                                        auto col = static_cast<iota>(src - beg);
                                        frame.locate(col, row);

                                        if (src != end)
                                        {
                                            auto& d = *(src++);
                                            auto& g = *(dst++);
                                            if (d.wdt() < 3)
                                            {
                                                fallback(fore, state, frame); // Left part alone.
                                                src--; // Repeat all for d again.
                                                dst--; // Repeat all for g again.
                                            }
                                            else // d.wdt() == 3
                                            {
                                                if (!fore.scan<VGAMODE>(d, state, frame))
                                                {
                                                    
                                                    fallback(fore, state, frame); // Left part alone.
                                                    fallback(d, state, frame); // Right part alone.
                                                }
                                                g = d;
                                            }
                                        }
                                        else
                                        {
                                            fallback(fore, state, frame); // Left part alone.
                                        }
                                    }
                                    else // Check right part.
                                    {
                                        if (src != end)
                                        {
                                            auto& d = *(src++);
                                            auto& g = *(dst++);
                                            if (d.wdt() < 3)
                                            {
                                                auto col = static_cast<iota>(src - beg - 1);
                                                frame.locate(col, row);
                                                fallback(fore, state, frame); // Left part alone.
                                                src--; // Repeat all for d again.
                                                dst--; // Repeat all for g again.
                                            }
                                            else /// d.wdt() == 3
                                            {
                                                if (g != d)
                                                {
                                                    g = d;
                                                    auto col = static_cast<iota>(src - beg - 1);
                                                    frame.locate(col, row);

                                                    if (!fore.scan<VGAMODE>(d, state, frame))
                                                    {
                                                        fallback(fore, state, frame); // Left part alone.
                                                        fallback(d, state, frame); // Right part alone.
                                                    }
                                                }
                                            }
                                        }
                                        else
                                        {
                                            auto col = static_cast<iota>(src - beg);
                                            frame.locate(col, row);
                                            fallback(fore, state, frame); // Left part alone.
                                        }
                                    }
                                }
                                else // w == 3 // Right part has changed.
                                {
                                    auto col = static_cast<iota>(src - beg);
                                    frame.locate(col, row);
                                    back = fore;
                                    fallback(fore, state, frame); // Right part alone.
                                }
                            }
                        }
                        beg += field.x;
                    }
                }

                auto size = static_cast<iota>(frame.size());
                if (size)
                {
                    guard.unlock();
                    conio.output(frame);
                    frame.clear();
                    guard.lock();
                }
                delta = size;
                watch = tempus::now() - start;
            }
        }

    public:
        // diff: Obtain new content to render.
        pair commit(core& canvas) // Run inside the e2::sync.
        {
            std::unique_lock guard(mutex, std::try_to_lock);
            if (guard.owns_lock())
            {
                dhash = canvas.hash();
                field = canvas.swap(cache); // Use one surface for reading (cache), one for writing (canvas).
                //field = canvas.copy(cache);
                if (rhash != dhash) front = cache; // Cache may be further resized before it rendered.
                debug = { watch, delta };

                if (extra.length())
                {
                    extra_cached += extra;
                    extra.clear();
                }

                ready = true;
                synch.notify_one();
                return debug;
            }
            return std::nullopt;
        }

        diff(link& conio, pro::input& input, svga vga_mode)
            : rhash{ 0 },
              dhash{ 0 },
              delta{ 0 },
              watch{ 0 },
              alive{ true },
              ready{ faux },
              conio{ conio },
              video{ vga_mode },
              mutex{ input.sync },
              cache{ input.xmap.pick() }
        {
            log("diff: ctor start");
            paint = work([&]
                { 
                    switch(video)
                    {
                        case svga::truecolor: render<svga::truecolor>(); break;
                        case svga::vga16:     render<svga::vga16    >(); break;
                        case svga::vga256:    render<svga::vga256   >(); break;
                        default: break;
                    }
                });
            log("diff: ctor complete");
        }
        ~diff()
        {
            log("diff: dtor");
            if (paint.joinable())
            {
                mutex.lock();
                alive = faux;
                ready = true;
                synch.notify_all();
                mutex.unlock();
                paint.join();
                log("diff: render thread joined");
            }
        }

        void append(view utf8)
        {
            extra = utf8;
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
        pro::cache cache{*this, faux }; // gate: Object map.
        #ifdef DEBUG_OVERLAY
        pro::debug debug{*this }; // gate: Debug telemetry controller.
        #endif

        using pair = std::optional<std::pair<period, iota>>;
        pair yield; // gate: Indicator that the current frame has been successfully STDOUT'd.
        para uname; // gate: Client name.
        text uname_txt; // gate: Client name (original).
        bool native = faux; //gate: Extended functionality support.
        bool fullscreen = faux; //gate: Fullscreen mode.
        iota legacy = os::legacy::clean;

    public:
        // todo unify
        page watermark;
        sptr<base> uibar; // gate: Local UI overlay, UI bar/taskbar/sidebar.

        // Main loop.
        void proceed(os::xipc media /*session socket*/, text title)
        {
            if (auto world = base::parent())
            {
                auto vga_mode = legacy & os::legacy::vga16  ? svga::vga16
                              : legacy & os::legacy::vga256 ? svga::vga256
                                                            : svga::truecolor;
                link conio{ *this, media }; // gate: Terminal IO.
                diff paint{ conio, input, vga_mode }; // gate: Rendering loop.
                subs token;                 // gate: Subscription tokens array.

                // conio events.
                SUBMIT_T(tier::release, e2::conio::size, token, newsize)
                {
                    base::resize(newsize);
                };
                SUBMIT_T(tier::release, e2::conio::unknown, token, unkstate)
                {
                };
                SUBMIT_T(tier::release, e2::conio::focus, token, unkstate)
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
                    text msg = "\n\rgate: Term error: " + std::to_string(errcode) + "\r\n";
                    log("gate: stop byemsg: ", msg);
                    conio.shutdown();
                };
                SUBMIT_T(tier::release, e2::conio::quit, token, msg)
                {
                    log("gate: stop byemsg: ", msg);
                    conio.shutdown();
                };
                //SUBMIT_T(tier::release, e2::form::state::header, token, newheader)
                //{
                //    text title;
                //    newheader.lyric->each([&](auto c) { title += c.txt(); });
                //    conio.output(ansi::tag(title));
                //};
                SUBMIT_T(tier::release, e2::form::prop::header, token, newheader)
                {
                    text title;
                    title.reserve(newheader.length());
                    if (native)
                    {
                        title = newheader;
                    }
                    else
                    {
                        para{ newheader }.lyric->each([&](auto c) { title += c.txt(); });
                    }
                    log("gate: title changed to '", title, ansi::nil().add("'"));
                    conio.output(ansi::tag(title));
                };
                SUBMIT_T(tier::release, e2::command::cout, token, extra_data)
                {
                    paint.append(extra_data);
                };

                world->SUBMIT_T(tier::release, e2::form::proceed::render, token, render_scene)
                {
                    auto stamp = tempus::now();
                    if (render_scene(cache.canvas, watermark) || !yield) // Put the world to the my canvas.
                    {
                        // Update objects under mouse cursor.
                        //input.fire(hids::events::mouse::hover);
                        #ifdef DEBUG_OVERLAY
                            debug.bypass = true;
                            //input.fire(hids::events::mouse::hover);
                            input.fire(hids::events::mouse::move.id);
                            debug.bypass = faux;
                        #else
                            input.fire(hids::events::mouse::move.id);
                        #endif

                        // Draw debug overlay, maker, titles, etc.
                        this->SIGNAL(tier::release, e2::postrender, cache.canvas);
                        #ifdef DEBUG_OVERLAY
                            if ((yield = paint.commit(cache.canvas)))
                            {
                                auto& watch = yield.value().first;
                                auto& delta = yield.value().second;
                                debug.update(watch, delta);
                            }
                            debug.update(stamp);
                        #else
                            yield = paint.commit(cache.canvas); // Try output my canvas to the my console.
                        #endif
                    }
                };

                conio.session(title);
                mouse.reset(); // Reset active mouse clients to avoid hanging pointers.
            }
        }

    protected:
        gate(view user_name, iota legacy_mode)
        {
            //todo unify
            uname = uname_txt = user_name;
            title.live = faux;
            legacy = legacy_mode;
            mouse.draggable<sysmouse::leftright>();
            mouse.draggable<sysmouse::left>();
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
                robot.actify(gear.mouse::fader<quadratic<twod>>(2s), [&](auto& x)
                             {
                                base::moveby(-x);
                                base::deface();
                             });
            };

            //todo unify (use uibar)
            SUBMIT(tier::preview, e2::form::prop::footer, newfooter)
            {
                watermark = ansi::cup(dot_00).rlf(feed::rev).jet(bias::right).add(newfooter);
            };
            SUBMIT(tier::release, e2::form::prop::fullscreen, state)
            {
                fullscreen = state;
            };
            SUBMIT(tier::request, e2::form::prop::name, user_name)
            {
                user_name = uname_txt;
            };
            SUBMIT(tier::request, e2::form::prop::viewport, viewport)
            {
                broadcast->SIGNAL(tier::request, e2::form::prop::viewport, viewport);
                viewport.coor += base::coor();
            };
            //todo unify creation (delete simple create wo gear)
            SUBMIT(tier::preview, e2::form::proceed::create, region)
            {
                if (auto world = base::parent())
                {
                    region.coor += base::coor();
                    world->SIGNAL(tier::release, e2::form::proceed::create, region);
                }
            };
            SUBMIT(tier::preview, e2::form::proceed::createby, gear)
            {
                if (auto world = base::parent())
                {
                    gear.slot.coor += base::coor();
                    world->SIGNAL(tier::release, e2::form::proceed::createby, gear);
                }
            };
            SUBMIT(tier::preview, hids::events::keybd::any, gear)
            {
                //todo unify
                //if (gear.meta(hids::CTRL | hids::RCTRL))
                {
                    //todo unify
                    auto pgup = gear.keystrokes == "\033[5;5~"s
                            || (gear.keystrokes == "\033[5~"s && gear.meta(hids::CTRL | hids::RCTRL));
                    auto pgdn = gear.keystrokes == "\033[6;5~"s
                            || (gear.keystrokes == "\033[6~"s && gear.meta(hids::CTRL | hids::RCTRL));
                    if (pgup || pgdn)
                    {
                        if (auto world = base::parent())
                        {
                            sptr<base> item_ptr;
                            if (pgdn) world->SIGNAL(tier::request, e2::form::proceed::detach, item_ptr); // Take prev item
                            else      world->SIGNAL(tier::request, e2::form::proceed::attach, item_ptr); // Take next item

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
                }
            };
            SUBMIT(tier::release, e2::form::layout::shift, newpos)
            {
                rect viewport;
                this->SIGNAL(tier::request, e2::form::prop::viewport, viewport);
                auto oldpos = viewport.coor + (viewport.size / 2);

                auto path = oldpos - newpos;
                iota time = SWITCHING_TIME;
                auto func = constlinearAtoB<twod>(path, time, now<iota>());

                robot.pacify();
                robot.actify(func, [&](auto& x) {
                                     base::moveby(-x);
                                     base::strike();
                                 });
            };
            SUBMIT(tier::release, e2::form::prop::brush, brush)
            {
                cache.canvas.mark(brush);
            };
            SUBMIT(tier::release, e2::size::set, newsz)
            {
                if (uibar) uibar->base::resize(newsz);
            };
            SUBMIT(tier::release, e2::render::prerender, parent_canvas)
            {
                // Draw a shadow of user's terminal window for other users (spectators).
                // see pro::scene.
                if (&parent_canvas != &cache.canvas)
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
                if (&parent_canvas != &cache.canvas)
                {
                    //if (parent.test(area.coor))
                    //{
                    //	auto hover_id = parent[area.coor].link();
                    //	log ("---- hover id ", hover_id);
                    //}
                    //auto& header = *title.header().lyric;
                    if (uname.lyric)
                    {
                        auto& header = *uname.lyric;
                        auto area = base::area();
                        area.coor += input.coord;
                        area.size = dot_11;
                        area.coor.y--;
                        area.coor.x -= (iota)header.size().x / 2;
                        //todo unify header coords
                        header.move(area.coor);
                        parent_canvas.fill(header);
                    }
                }
                else
                {
                    if (uibar && !fullscreen) parent_canvas.render(uibar, base::coor());
                }
                bool show_mouse = legacy & os::legacy::mouse;
                if (&parent_canvas != &cache.canvas || show_mouse)
                {
                    auto area = base::area();
                    area.coor += input.coord;
                    area.coor -= parent_canvas.area().coor;
                    area.size = dot_11;
                    cell brush;
                    if (input.push)
                    {
                        brush.txt(64 + input.push).bgc(reddk).fgc(0xFFffffff);
                    }
                    else
                    {
                        if (show_mouse) brush.txt("\u2588"/* █ */).fgc(0xFF00ff00);
                        else            brush.txt(whitespace).bgc(greenlt);
                    }
                    parent_canvas.fill(area, brush);
                }
                #ifdef REGIONS
                parent_canvas.each([](cell& c){
                    auto mark = rgba{ rgba::color256[c.link() % 256] };
                    auto bgc = c.bgc();
                    mark.alpha(64);
                    bgc.mix(mark);
                    c.bgc(bgc);
                });
                #endif
            };
        }

    public:
        // gate: Attach a new item.
        template<class T>
        auto attach(sptr<T> item)
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
    };
}

#endif // NETXS_CONSOLE_HPP